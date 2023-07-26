#ifndef LOG_H
#define LOG_H

#include "drivers/tty.h"
#include "lib/printf.h"

// No, I'm not going to implement vprintf right now
#define __LOG(...) printf(__VA_ARGS__)

#define LOG(...)                             \
    do {                                     \
        set_color(VGA_BLACK, VGA_DARK_GREY); \
        print("LOG:");                       \
        set_color(VGA_WHITE, VGA_BLACK);     \
        print_char(' ');                     \
        __LOG(__VA_ARGS__);                  \
    } while (0);

#define PANIC(...)                       \
    do {                                 \
        set_color(VGA_WHITE, VGA_RED);   \
        print("PANIC:");                 \
        set_color(VGA_WHITE, VGA_BLACK); \
        print_char(' ');                 \
        __LOG(__VA_ARGS__);              \
        __asm__ volatile("hlt");         \
    } while (0);

#ifdef DEBUG
#undef DEBUG
#define DEBUG(...)                       \
    do {                                 \
        set_color(VGA_WHITE, VGA_BLUE);  \
        print("DEBUG:");                 \
        set_color(VGA_WHITE, VGA_BLACK); \
        print_char(' ');                 \
        __LOG(__VA_ARGS__);              \
    } while (0);
#else
#define DEBUG(...) (void)(__VA_ARGS__)
#endif

#endif
