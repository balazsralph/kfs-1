#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include "kprintf.h"
#include "vga.h"

static void putchar_internal(char c);
static void putstr_internal(const char *s);
static void putnbr_internal(int n);
static void putunbr_internal(unsigned int n);
static void puthex_internal(unsigned int n);
static void putptr_internal(void *p);

static void putchar_internal(char c)
{
    char str[2];

    if ((unsigned char)c == 127U) {
        vga_delete_forward();
        return;
    }
    if (c == '\b') {
        vga_delete_backward();
        return;
    }

    str[0] = c;
    str[1] = '\0';
    vga_print_string(str);
}

static void putstr_internal(const char *s)
{
    if (s == NULL) {
        vga_print_string("(null)");
        return;
    }
    vga_print_string(s);
}

static void putnbr_internal(int n)
{
    vga_print_int(n);
}

static void putunbr_internal(unsigned int n)
{
    vga_print_uint(n);
}

static void puthex_internal(unsigned int n)
{
    vga_print_hex(n);
}

static void putptr_internal(void *p)
{
    uintptr_t u;

    vga_print_string("0x");
    u = (uintptr_t)p;
    vga_print_hex((unsigned int)u);
}

void kprintf(const char *format, ...)
{
    va_list args;
    size_t i;

    if (format == NULL) {
        return;
    }

    va_start(args, format);

    i = 0U;
    while (format[i] != '\0') {
        if (format[i] != '%') {
            putchar_internal(format[i]);
            i++;
            continue;
        }

        i++;
        if (format[i] == '\0') {
            break;
        }

        if (format[i] == '%') {
            putchar_internal('%');
        } else if (format[i] == 'c') {
            putchar_internal((char)va_arg(args, int));
        } else if (format[i] == 's') {
            putstr_internal(va_arg(args, const char *));
        } else if (format[i] == 'd' || format[i] == 'i') {
            putnbr_internal(va_arg(args, int));
        } else if (format[i] == 'u') {
            putunbr_internal(va_arg(args, unsigned int));
        } else if (format[i] == 'x') {
            puthex_internal(va_arg(args, unsigned int));
        } else if (format[i] == 'p') {
            putptr_internal(va_arg(args, void *));
        } else {
            putchar_internal('%');
            putchar_internal(format[i]);
        }

        i++;
    }

    va_end(args);
}