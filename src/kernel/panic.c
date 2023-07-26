#include "panic.h"
#include "drivers/tty.h"

void panic(char* message) {
    set_color(VGA_WHITE, VGA_RED);
    print("PANIC:");
    set_color(VGA_WHITE, VGA_BLACK);
    print_char(' ');
    print(message);

    __asm__ volatile("hlt");
}
