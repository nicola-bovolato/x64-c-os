#ifndef STRING_H
#define STRING_H

void utoa(unsigned int number, char* dest, unsigned int base);
void itoa(int number, char* dest, unsigned int base);
void ultoa(unsigned long number, char* dest, unsigned int base);
void ltoa(long number, char* dest, unsigned int base);

int strlen(const char* str);
int strcmp(const char* str1, const char* str2);

void strcpy(char* dest, const char* source);
void strrev(char* str);

void strcat(char* str, const char* append);

#endif
