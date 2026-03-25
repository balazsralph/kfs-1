#include <stddef.h>
#include <stdint.h>

#include "console.h"
#include "keyboard.h"
#include "kprintf.h"
#include "vga.h"
#include "vga_cursor.h"

#define VGA_TEXT_BUFFER 0xB8000U

/*
Reference: https://wiki.osdev.org/Printing_To_Screen
*/

static inline void cpu_sleep(void)
{
    __asm__ volatile("pause" ::: "memory");
}


void kernel_main(uint32_t magic, uint32_t info)
{
    (void)magic;
    (void)info;    

    vga_init(VGA_TEXT_BUFFER);
    keyboard_init();

    char *msg = 
                "                                 \n"
                "              :::      ::::::::   \n"
                "            :+:      :+:    :+:   \n"
                "          +:+ +:+         +:+     \n"
                "        +#+  +:+       +#+        \n"
                "      +#+#+#+#+#+   +#+           \n"
                "          #+#    #+#             \n"
                "         ###   ########.fr       \n"
                "                                 \n";
    vga_clear_screen();
    vga_enable_cursor();

    vga_print_string(msg);
    vga_print_string("Hello, World!\n");
    kprintf("Hello, World! %d %x ptr=%p\n", 42, 42, (void *)msg);

    

    while (1) {
        char c;

        c = keyboard_getchar();
        if (c != 0) {
            if (c == '1') {
                console_switch(0);
            } else if (c == '2') {
                console_switch(1);
            } else if ((unsigned char)c == KBD_ARROW_LEFT) {
                vga_cursor_move_left();
            } else if ((unsigned char)c == KBD_ARROW_RIGHT) {
                vga_cursor_move_right();
            } else if ((unsigned char)c == 127U) {
                vga_delete_forward();
            } else {
                kprintf("%c", c);
            }
        }
        cpu_sleep();
    }
}
