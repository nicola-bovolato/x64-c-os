#include "debug.h"

#include "drivers/tty.h"

void debug(char *message) {
    set_color(VGA_WHITE, VGA_BLUE);
    print("DEBUG:");
    set_color(VGA_WHITE, VGA_BLACK);
    print_char(' ');
    print_line(message);
}
