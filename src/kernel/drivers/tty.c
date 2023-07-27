#include "tty.h"
#include "../lib/mem.h"
#include <stddef.h>
#include <stdint.h>

static uint16_t* const vga_mem_end = (uint16_t*)(VGA_MEM + (VGA_ROWS * VGA_COLS - 1) * 2);

static uint16_t* cursor    = (uint16_t*)VGA_MEM;
static uint8_t   vga_color = VGA_BLACK << 4 | VGA_WHITE;

static inline void print_char_internal(char to_print);
static inline void scroll(int rows);

static inline void   set_row(size_t row);
static inline void   set_col(size_t col);
static inline size_t get_row();
static inline size_t get_col();

void set_color(vga_colors foreground, vga_colors background) {
    // 4 low bits = foreground, 3 high bits = background
    vga_color = background << 4 | foreground;
}

void clear_screen() {
    // Fills screen with whitespace characters
    for (uint16_t* pointer = (uint16_t*)VGA_MEM; pointer <= vga_mem_end; pointer++) {
        *pointer = (VGA_BLACK << 4 | VGA_WHITE) << 8 | 0x20;
    }

    // Returns cursor pointer to the start of the screen
    cursor = (uint16_t*)VGA_MEM;
}

void print_char(char to_print) { print_char_internal(to_print); }

void print(char* str) {
    char* pointer = str;
    // while the string isn't terminated print a character and advance pointer
    while (*pointer != '\0') print_char_internal(*pointer++);
}

void print_line(char* str) {
    print(str);

    // if the cursor is at the last row scroll the screen, else go to then next row
    if (get_row() == VGA_ROWS - 1) scroll(1);
    else set_row(get_row() + 1);

    set_col(0);
}

static inline void print_char_internal(char to_print) {
    *cursor = vga_color << 8 | to_print;

    // if the end of the vga memory has been reached
    // scroll and set the cursor back to the first column
    if (cursor == vga_mem_end) {
        scroll(1);
        set_col(0);
    }
    else cursor++;
}

static inline void set_row(size_t row) {

    // reset the cursor to the start of vga memory + cols
    cursor = (uint16_t*)(VGA_MEM + get_col() * 2);

    // if the row is valid increase the cursor by 'row' * 80 times
    if (row < VGA_ROWS) cursor += row * VGA_COLS;
}

static inline void set_col(size_t col) {
    // reset the cursor to the start of the current row
    cursor = (uint16_t*)(VGA_MEM + (get_row() * VGA_COLS * 2));

    // if the col is valid increase the cursor by 'col' times
    if (col < VGA_COLS) cursor += col;
}

static inline size_t get_row() {
    // gets the offset and divides it by the number of columns
    // (*2 because each character occupies 2 bytes)
    return ((size_t)cursor - VGA_MEM) / (VGA_COLS * 2);
}

static inline size_t get_col() {
    // gets the offset and subtracts the offset of the current row
    // (/2 because each character occupies 2 bytes)
    return ((size_t)cursor - VGA_MEM - (get_row() * VGA_COLS * 2)) / 2;
}

// Copies memory in the vga buffer, making the screen text scroll
static inline void scroll(int rows) {

    // If the rows to scroll are more than the screen size, then clear the screen
    if (rows > VGA_ROWS - 1) {
        clear_screen();
        return;
    }

    // Calculate how many bits to copy
    size_t to_copy = (VGA_ROWS * VGA_COLS * 2) - (rows * VGA_COLS * 2);

    // Copies the bits from a specified row to the first one
    memcpy((uint16_t*)((size_t)VGA_MEM + (VGA_COLS * rows * 2)), (uint16_t*)VGA_MEM, to_copy);

    // Clears the remaining bottom rows
    for (uint16_t* pointer = (uint16_t*)(VGA_MEM + to_copy); pointer <= vga_mem_end; pointer++) {
        *pointer = (VGA_BLACK << 4 | VGA_WHITE) << 8 | 0x20;
    }
}
