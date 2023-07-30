#include "printf.h"
#include "../drivers/tty.h"
#include "string.h"
#include <stdarg.h>
#include <stdint.h>


// size of a pointer + 2 bytes for hex representation + 1 byte for string termination
#define NUMERIC_BUFFER_SIZE sizeof(void*) + 2 + 1


static inline void print_ulong_hex(unsigned long num);
static inline void print_uint_hex(unsigned int num);
static inline void print_uint(unsigned int num);
static inline void print_int(int num);

static char num_buf[NUMERIC_BUFFER_SIZE];


void printf(char* str, ...) {
    va_list args;

    va_start(args, str);

    while (*str != '\0') {

        if (*str == '%') {

            switch (*++str) {
            case 'i':
            case 'd':
                print_int(va_arg(args, int));
                break;

            case 'u':
                print_uint(va_arg(args, unsigned int));
                break;

            case 'x':
                print_uint_hex(va_arg(args, unsigned int));
                break;

            case 'p':
                print_ulong_hex((unsigned long)va_arg(args, void*));
                break;

            case 'c':
                print_char(va_arg(args, int));
                break;

            case 's':
                print(va_arg(args, char*));
                break;

            case '%':
                print_char('%');
                break;
            }
        }
        else switch (*str) {
            case '\n':
                print_line("");
                break;

            case '\t':
                print("    ");
                break;

            default:
                print_char(*str);
            }

        str++;
    }

    va_end(args);
}


static inline void print_int(int num) {
    itoa(num, num_buf, 10);
    print(num_buf);
}

static inline void print_uint(unsigned int num) {
    utoa(num, num_buf, 10);
    print(num_buf);
}

static inline void print_uint_hex(unsigned int num) {
    utoa(num, num_buf, 16);
    print(num_buf);
}

static inline void print_ulong_hex(unsigned long num) {
    num_buf[0] = '0';
    num_buf[1] = 'x';
    ultoa(num, num_buf + 2, 16);
    print(num_buf);
}
