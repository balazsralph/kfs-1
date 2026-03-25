#include <stddef.h>
#include <stdint.h>

#include "vga.h"

#define VGA_DEFAULT_WIDTH   80U
#define VGA_DEFAULT_HEIGHT  25U

#define VGA_DEFAULT_FG      0U
#define VGA_DEFAULT_BG      15U
#define VGA_DEFAULT_BLINK   0U

#define VGA_TAB_WIDTH       4U

static vga_screen_t g_vga;
static vga_color_t  g_color;

static void     color_init(vga_color_t *color, uint8_t foreground, uint8_t background, uint8_t blink);
static uint16_t vga_entry(uint8_t ch, uint8_t attrib);
static void     vga_scroll(void);
static void     vga_newline(void);
static void     vga_putentry_at(char c, size_t row, size_t col, uint8_t attrib);
static void     vga_putchar(char c);

void vga_init(uint32_t video_address)
{
    g_vga.buffer = (volatile uint16_t *)(uintptr_t)video_address;
    g_vga.width = VGA_DEFAULT_WIDTH;
    g_vga.height = VGA_DEFAULT_HEIGHT;
    g_vga.row = 0U;
    g_vga.col = 0U;

    color_init(&g_color, VGA_DEFAULT_FG, VGA_DEFAULT_BG, VGA_DEFAULT_BLINK);
}

static void color_init(vga_color_t *color, uint8_t foreground, uint8_t background, uint8_t blink)
{
    color->foreground = (uint8_t)(foreground & 0x0FU);
    color->background = (uint8_t)(background & 0x0FU);
    color->blink = (uint8_t)(blink ? 1U : 0U);

    color->attrib = (uint8_t)(((color->background & 0x0FU) << 4) | (color->foreground & 0x0FU));
    if (color->blink) {
        color->attrib |= 0x80U;
    }
}

void vga_set_color(uint8_t foreground, uint8_t background, uint8_t blink)
{
    color_init(&g_color, foreground, background, blink);
}

static uint16_t vga_entry(uint8_t ch, uint8_t attrib)
{
    return (uint16_t)ch | ((uint16_t)attrib << 8);
}

static void vga_putentry_at(char c, size_t row, size_t col, uint8_t attrib)
{
    size_t index;

    index = row * (size_t)g_vga.width + col;
    g_vga.buffer[index] = vga_entry((uint8_t)c, attrib);
}

void vga_clear_screen(void)
{
    size_t total;
    size_t i;

    total = (size_t)g_vga.width * (size_t)g_vga.height;
    for (i = 0U; i < total; i++) {
        g_vga.buffer[i] = vga_entry((uint8_t)' ', g_color.attrib);
    }

    g_vga.row = 0U;
    g_vga.col = 0U;
}

static void vga_scroll(void)
{
    size_t row;
    size_t col;

    for (row = 1U; row < (size_t)g_vga.height; row++) {
        for (col = 0U; col < (size_t)g_vga.width; col++) {
            g_vga.buffer[(row - 1U) * (size_t)g_vga.width + col] =
                g_vga.buffer[row * (size_t)g_vga.width + col];
        }
    }

    for (col = 0U; col < (size_t)g_vga.width; col++) {
        vga_putentry_at(' ', (size_t)g_vga.height - 1U, col, g_color.attrib);
    }
}

static void vga_newline(void)
{
    g_vga.col = 0U;
    g_vga.row++;

    if (g_vga.row >= g_vga.height) {
        vga_scroll();
        g_vga.row = g_vga.height - 1U;
    }
}

static void vga_putchar(char c)
{
    if (c == '\n') {
        vga_newline();
        return;
    }

    if (c == '\r') {
        g_vga.col = 0U;
        return;
    }

    if (c == '\t') {
        size_t spaces;
        size_t i;

        spaces = VGA_TAB_WIDTH - ((size_t)g_vga.col % VGA_TAB_WIDTH);
        for (i = 0U; i < spaces; i++) {
            vga_putchar(' ');
        }
        return;
    }

    if (g_vga.row >= g_vga.height) {
        vga_scroll();
        g_vga.row = g_vga.height - 1U;
    }

    vga_putentry_at(c, (size_t)g_vga.row, (size_t)g_vga.col, g_color.attrib);

    g_vga.col++;

    if (g_vga.col >= g_vga.width) {
        vga_newline();
    }
}

void vga_print_string(const char *str)
{
    const unsigned char *p;

    if (str == NULL) {
        return;
    }

    p = (const unsigned char *)str;
    while (*p != '\0') {
        vga_putchar((char)*p);
        p++;
    }
}


void vga_print_uint(unsigned int n)
{
    char buffer[10];
    int i;

    if (n == 0U) {
        vga_print_string("0");
        return;
    }

    i = 0;
    while (n > 0U) {
        buffer[i++] = (char)('0' + (n % 10U));
        n /= 10U;
    }

    while (i > 0) {
        i--;
        char str[2];
        str[0] = buffer[i];
        str[1] = '\0';
        vga_print_string(str);
    }
}

void vga_print_int(int n)
{
    unsigned int value;

    if (n < 0) {
        vga_print_string("-");
        value = (unsigned int)(-n);
    } else {
        value = (unsigned int)n;
    }
    vga_print_uint(value);
}

void vga_print_hex(unsigned int n)
{
    char hex_digits[] = "0123456789abcdef";
    char buffer[8];
    int i;

    if (n == 0U) {
        vga_print_string("0");
        return;
    }

    i = 0;
    while (n > 0U) {
        buffer[i++] = hex_digits[n % 16U];
        n /= 16U;
    }

    while (i > 0) {
        i--;
        char str[2];
        str[0] = buffer[i];
        str[1] = '\0';
        vga_print_string(str);
    }
}

uint32_t vga_get_row(void)
{
    return g_vga.row;
}

uint32_t vga_get_col(void)
{
    return g_vga.col;
}

void vga_screen_copy_out(uint16_t *dst)
{
    size_t i;
    size_t n;

    n = (size_t)g_vga.width * (size_t)g_vga.height;
    for (i = 0U; i < n; i++) {
        dst[i] = g_vga.buffer[i];
    }
}

void vga_screen_copy_in(const uint16_t *src)
{
    size_t i;
    size_t n;

    n = (size_t)g_vga.width * (size_t)g_vga.height;
    for (i = 0U; i < n; i++) {
        g_vga.buffer[i] = src[i];
    }
}

void vga_set_row_col(uint32_t row, uint32_t col)
{
    if (row >= g_vga.height) {
        row = g_vga.height - 1U;
    }
    if (col >= g_vga.width) {
        col = g_vga.width - 1U;
    }
    g_vga.row = row;
    g_vga.col = col;
}
