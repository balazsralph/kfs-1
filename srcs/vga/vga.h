#ifndef VGA_H
#define VGA_H

#include <stdint.h>

typedef struct vga_screen {
    volatile uint16_t *buffer;
    uint32_t width;
    uint32_t height;
    uint32_t row;
    uint32_t col;
}               vga_screen_t;

typedef struct vga_color {
    uint8_t foreground;
    uint8_t background;
    uint8_t blink;
    uint8_t attrib;
}               vga_color_t;

void vga_init(uint32_t video_address);
void vga_set_color(uint8_t foreground, uint8_t background, uint8_t blink);
void vga_clear_screen(void);
void vga_print_string(const char *str);
void vga_print_int(int n);
void vga_print_uint(unsigned int n);
void vga_print_hex(unsigned int n);

uint32_t vga_get_row(void);
uint32_t vga_get_col(void);

void vga_screen_copy_out(uint16_t *dst);
void vga_screen_copy_in(const uint16_t *src);
void vga_set_row_col(uint32_t row, uint32_t col);

void vga_cursor_move_left(void);
void vga_cursor_move_right(void);

void vga_delete_forward(void);
void vga_delete_backward(void);

#endif
