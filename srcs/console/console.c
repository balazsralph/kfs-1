#include <stddef.h>
#include <stdint.h>

#include "console.h"
#include "vga.h"

#define CONSOLE_COUNT 2
#define VGA_CELLS     (80U * 25U)

static uint16_t snapshots[CONSOLE_COUNT][VGA_CELLS];
static uint32_t rows[CONSOLE_COUNT];
static uint32_t cols[CONSOLE_COUNT];
static int      active_console;

void console_switch(int id)
{
    if (id < 0 || id >= CONSOLE_COUNT) {
        return;
    }
    if (id == active_console) {
        return;
    }

    vga_screen_copy_out(snapshots[active_console]);
    rows[active_console] = vga_get_row();
    cols[active_console] = vga_get_col();

    vga_screen_copy_in(snapshots[id]);
    vga_set_row_col(rows[id], cols[id]);
    active_console = id;
}
