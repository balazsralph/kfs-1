#ifndef VGA_CURSOR_H
#define VGA_CURSOR_H

#include <stddef.h>

void vga_update_cursor(size_t row, size_t col);
void vga_enable_cursor(void);
void vga_disable_cursor(void);

#endif