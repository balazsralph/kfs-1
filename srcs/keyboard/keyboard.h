#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

void keyboard_init(void);
char keyboard_getchar(void);

#define KBD_ARROW_LEFT  0x81U
#define KBD_ARROW_RIGHT 0x82U

#endif