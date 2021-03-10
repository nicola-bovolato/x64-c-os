#include "printf.h"

#include "../drivers/tty.h"
#include "string.h"

#include <stdint.h>
#include <stdarg.h>

//size of a pointer + 2 bytes for hex representation + 1 byte for string termination
#define NUMERIC_BUFFER_SIZE sizeof(void) + 2 + 1

static inline void print_ulong_hex(unsigned long num, char* buf);
static inline void print_uint_hex (unsigned int  num, char* buf);
static inline void print_uint     (unsigned int  num, char* buf);
static inline void print_int      (int num,           char* buf);

void printf(char *str, ...) {

    va_list args;
    char num_buf[NUMERIC_BUFFER_SIZE];

    va_start(args, str);

    while(*str != '\0'){

        if(*str == '%') {

            switch(*++str) {
                case 'i':
                case 'd': {
                    volatile int num = va_arg(args, int);
                    print_int(num, num_buf);
                } break;

                case 'u': {
                    volatile unsigned int num = va_arg(args, unsigned int);
                    print_uint(num, num_buf);
                } break;

                case 'x': {
                    volatile unsigned int num = va_arg(args, unsigned int);
                    print_uint_hex(num, num_buf);
                } break;

                case 'p': {
                    volatile unsigned long num = va_arg(args, unsigned long);
                    print_ulong_hex(num, num_buf);
                } break;

                case 'c': {
                    volatile unsigned char c = va_arg(args, int);
                    print_char(c);
                } break;

                case 's': {
                    char* s = va_arg(args, char*);
                    print(s);
                } break;

                case '%':
                    print_char('%');
                    break;
            }
        }
        else switch(*str) {
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


static inline void print_int (int num, char* buf) {
    itoa(num, buf, 10);
    print(buf);
}

static inline void print_uint (unsigned int num, char* buf) {
    utoa(num, buf, 10);
    print(buf);
}

static inline void print_uint_hex (unsigned int num, char* buf) {
    utoa(num, buf, 16);
    print(buf);
}

static inline void print_ulong_hex (unsigned long num, char* buf) {
    buf[0] = '0';
    buf[1] = 'x';
    ultoa(num, buf + 2, 16);
    print(buf);
}
