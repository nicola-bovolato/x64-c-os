#include "printf.h"

#include "tty.h"
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
                case 'd':
                    print_uint(va_arg(args, int), num_buf);
                    break;

                case 'u':
                    print_uint(va_arg(args, unsigned int), num_buf);
                    break;

                case 'x':
                    print_uint_hex(va_arg(args, unsigned int), num_buf);
                    break;

                case 'p':
                    print_ulong_hex(va_arg(args, unsigned long), num_buf);
                    break;

                case 'c':
                    print_char(va_arg(args, int));
                    break;

                case 's':
                    print(va_arg(args, char*));
                    break;

                case '%':
                    print_char('%');
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
    buf[0] = '0';
    buf[1] = 'x';
    utoa(num, buf + 2, 16);
    print(buf);
}

static inline void print_ulong_hex (unsigned long num, char* buf) {
    buf[0] = '0';
    buf[1] = 'x';
    ultoa(num, buf + 2, 16);
    print(buf);
}
