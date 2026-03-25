#include <stddef.h>
#include <stdint.h>

#include "vga_cursor.h"

#define VGA_INDEX   0x3D4U
#define VGA_DATA    0x3D5U
#define VGA_WIDTH   80U
#define VGA_HEIGHT  25U

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;

    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

void vga_update_cursor(size_t row, size_t col)
{
    uint16_t pos;
    uint16_t max_pos;

    max_pos = (uint16_t)(VGA_WIDTH * VGA_HEIGHT);
    pos = (uint16_t)(row * VGA_WIDTH + col);
    if (pos >= max_pos) {
        pos = (uint16_t)(max_pos - 1U);
    }

    outb(VGA_INDEX, 0x0FU);
    outb(VGA_DATA, (uint8_t)(pos & 0xFFU));
    outb(VGA_INDEX, 0x0EU);
    outb(VGA_DATA, (uint8_t)((pos >> 8) & 0xFFU));
}

void vga_enable_cursor(void)
{
    uint8_t reg;

    outb(VGA_INDEX, 0x0AU);
    reg = inb(VGA_DATA);
    outb(VGA_DATA, (uint8_t)((reg & 0xC0U) | 14U));

    outb(VGA_INDEX, 0x0BU);
    reg = inb(VGA_DATA);
    outb(VGA_DATA, (uint8_t)((reg & 0xE0U) | 15U));
}

void vga_disable_cursor(void)
{
    outb(VGA_INDEX, 0x0AU);
    outb(VGA_DATA, 0x20U);
}
