#include <stddef.h>
#include <stdint.h>

#include "keyboard.h"

#define PS2_DATA    0x60U
#define PS2_STATUS  0x64U

#define SC_EXT_DELETE       0x53U
#define SC_EXT_DELETE_SET2  0x71U
#define SC_EXT_LEFT         0x4BU
#define SC_EXT_RIGHT        0x4DU
#define SC_EXT_LEFT_SET2    0x6BU
#define SC_EXT_RIGHT_SET2   0x74U

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;

    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static const char scancode_map[128] = {
    [0x01] = 0,
    [0x02] = '1',
    [0x03] = '2',
    [0x04] = '3',
    [0x05] = '4',
    [0x06] = '5',
    [0x07] = '6',
    [0x08] = '7',
    [0x09] = '8',
    [0x0A] = '9',
    [0x0B] = '0',
    [0x0C] = '-',
    [0x0D] = '=',
    [0x0E] = '\b',
    [0x0F] = '\t',
    [0x10] = 'q',
    [0x11] = 'w',
    [0x12] = 'e',
    [0x13] = 'r',
    [0x14] = 't',
    [0x15] = 'y',
    [0x16] = 'u',
    [0x17] = 'i',
    [0x18] = 'o',
    [0x19] = 'p',
    [0x1A] = '[',
    [0x1B] = ']',
    [0x1C] = '\n',
    [0x1E] = 'a',
    [0x1F] = 's',
    [0x20] = 'd',
    [0x21] = 'f',
    [0x22] = 'g',
    [0x23] = 'h',
    [0x24] = 'j',
    [0x25] = 'k',
    [0x26] = 'l',
    [0x27] = ';',
    [0x28] = '\'',
    [0x29] = '`',
    [0x2B] = '\\',
    [0x2C] = 'z',
    [0x2D] = 'x',
    [0x2E] = 'c',
    [0x2F] = 'v',
    [0x30] = 'b',
    [0x31] = 'n',
    [0x32] = 'm',
    [0x33] = ',',
    [0x34] = '.',
    [0x35] = '/',
    [0x39] = ' ',
};

static const char shift_num[10] = {
    '!', '@', '#', '$', '%', '^', '&', '*', '(', ')'
};

static char apply_shift_caps(uint8_t sc, char c, uint8_t shift, uint8_t caps)
{
    if (c >= 'a' && c <= 'z') {
        if ((caps ^ shift) != 0U) {
            c = (char)(c - 32);
        }
        return c;
    }

    if (shift == 0U) {
        return c;
    }

    if (sc >= 0x02U && sc <= 0x0BU) {
        return shift_num[(size_t)(sc - 0x02U)];
    }

    switch (sc) {
    case 0x0CU:
        return '_';
    case 0x0DU:
        return '+';
    case 0x1AU:
        return '{';
    case 0x1BU:
        return '}';
    case 0x27U:
        return ':';
    case 0x28U:
        return '"';
    case 0x29U:
        return '~';
    case 0x2BU:
        return '|';
    case 0x33U:
        return '<';
    case 0x34U:
        return '>';
    case 0x35U:
        return '?';
    default:
        return c;
    }
}

void keyboard_init(void)
{
    while ((inb(PS2_STATUS) & 1U) != 0U) {
        (void)inb(PS2_DATA);
    }
}

static uint8_t keyboard_read_data(void)
{
    while ((inb(PS2_STATUS) & 1U) == 0U) {
        __asm__ volatile("pause" ::: "memory");
    }
    return inb(PS2_DATA);
}

char keyboard_getchar(void)
{
    static uint8_t shift_l;
    static uint8_t shift_r;
    static uint8_t caps;

    uint8_t sc;
    uint8_t shift;
    char    c;

    if ((inb(PS2_STATUS) & 1U) == 0U) {
        return 0;
    }

    sc = inb(PS2_DATA);

    if (sc == 0xE0U) {
        uint8_t sc2;

        sc2 = keyboard_read_data();
        if ((sc2 & 0x80U) != 0U) {
            return 0;
        }
        if (sc2 == SC_EXT_DELETE || sc2 == SC_EXT_DELETE_SET2) {
            return (char)127;
        }
        if (sc2 == SC_EXT_LEFT || sc2 == SC_EXT_LEFT_SET2) {
            return (char)(unsigned char)KBD_ARROW_LEFT;
        }
        if (sc2 == SC_EXT_RIGHT || sc2 == SC_EXT_RIGHT_SET2) {
            return (char)(unsigned char)KBD_ARROW_RIGHT;
        }
        return 0;
    }

    if (sc == 0xE1U) {
        unsigned int i;

        for (i = 0U; i < 5U; i++) {
            (void)keyboard_read_data();
        }
        return 0;
    }

    if ((sc & 0x80U) != 0U) {
        sc = (uint8_t)(sc & 0x7FU);
        if (sc == 0x2AU) {
            shift_l = 0U;
        } else if (sc == 0x36U) {
            shift_r = 0U;
        }
        return 0;
    }

    if (sc == 0x2AU) {
        shift_l = 1U;
        return 0;
    }
    if (sc == 0x36U) {
        shift_r = 1U;
        return 0;
    }
    if (sc == 0x3AU) {
        caps = (uint8_t)(caps ^ 1U);
        return 0;
    }

    shift = (shift_l | shift_r) ? 1U : 0U;

    if (sc >= 128U) {
        return 0;
    }

    c = scancode_map[sc];
    if (c == 0) {
        return 0;
    }

    return apply_shift_caps(sc, c, shift, caps);
}
