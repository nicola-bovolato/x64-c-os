#ifndef TTY_H
#define TTY_H

#define VGA_MEM  0xb8000
#define VGA_COLS 80
#define VGA_ROWS 25

typedef enum {
    VGA_BLACK         = 0x0,
    VGA_BLUE          = 0x1,
    VGA_GREEN         = 0x2,
    VGA_CYAN          = 0x3,
    VGA_RED           = 0x4,
    VGA_MAGENTA       = 0x5,
    VGA_BROWN         = 0x6,
    VGA_LIGHT_GREY    = 0x7,
    VGA_DARK_GREY     = 0x8,
    VGA_LIGHT_BLUE    = 0x9,
    VGA_LIGHT_GREEN   = 0xa,
    VGA_LIGHT_CYAN    = 0xb,
    VGA_LIGHT_RED     = 0xc,
    VGA_LIGHT_MAGENTA = 0xd,
    VGA_LIGHT_BROWN   = 0xe,
    VGA_WHITE         = 0xf,
} vga_colors;

void set_color(vga_colors foreground, vga_colors background);

void print_char(char to_print);
void print(char* str);
void print_line(char* str);

void clear_screen();

#endif
