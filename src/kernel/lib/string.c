#include "string.h"

#include "mem.h"

#include <stdbool.h>
#include <stdint.h>

// returns string length
int strlen(const char* str) {
    int length = 0;

    while (str[length] != '\0') length++;

    return length;
}

// compares string returns 0 if equal
int strcmp(const char* str_1, const char* str_2) {

    while (*str_1 != '\0' && *str_1 == *str_2) {
        str_1++;
        str_2++;
    }

    return *(const unsigned char*)str_1 - *(const unsigned char*)str_2;
}

// reverses string
void strrev(char* str) {
    int length = strlen(str);

    for (int i = 0, j = length - 1; i < j; i++, j--) {
        char c = str[i];
        str[i] = str[j];
        str[j] = c;
    }
}

// unsigned int to ascii string
void utoa(unsigned int number, char* dest, unsigned int base) {
    ultoa(number, dest, base);
}

void ultoa(unsigned long number, char* dest, unsigned int base) {

    int i = 0;

    if (base < 2 || base > 36) base = 10;

    do {
        long tmp = number % base;

        if (tmp < 0x0A) dest[i] = tmp + '0';
        // Ascii numbers and letter are not contiguous
        else dest[i] = tmp - 0x0A + 'A';

        i++;
    } while ((number /= base) > 0);

    dest[i] = '\0';

    strrev(dest);
}

// int to ascii string
void itoa(int number, char* dest, unsigned int base) {
    ltoa(number, dest, base);
}

void ltoa(long number, char* dest, unsigned int base) {
    int  i   = 0;
    bool neg = number < 0;

    if (neg) number = -number;

    do {
        long tmp = number % base;

        // Ascii numbers and letter are not contiguous
        if (tmp < 0x0A) dest[i] = tmp + '0';
        else dest[i] = tmp - 0x0A + 'A';

        i++;
    } while ((number /= base) > 0);

    if (neg) {
        dest[i] = '-';
        i++;
    }
    dest[i] = '\0';

    strrev(dest);
}

// append string to another string
void strcat(char* str, const char* append) {

    int length = strlen(str);

    int i, j;

    for (i = length, j = 0; append[j] != '\0'; i++, j++) str[i] = append[j];

    str[i] = '\0';
}

// copy string to another string
void strcpy(const char* source, char* dest) {

    memcpy((uint8_t*)source, (uint8_t*)dest, strlen(source) + 1);
}
