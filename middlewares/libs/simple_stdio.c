//------------------------------------------------------------------------------

/// @file simple_stdio.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "simple_stdio.h"

#include <stdarg.h>

//------------------------------------------------------------------------------

// Helper function to convert integer to string
static void itoa(int value, char* str, int base) 
{
    char* ptr = str, *ptr1 = str, tmp_char;
    int tmp_value;

    // Handle negative integers for decimal base
    if (value < 0 && base == 10) {
        *ptr++ = '-';
        value = -value;
    }

    // Process individual digits
    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "0123456789abcdef"[tmp_value - value * base];
    } while (value);

    // Apply null terminator
    *ptr-- = '\0';

    // Reverse the string
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}

// Helper function to convert unsigned integer to string
static void utoa(unsigned int value, char* str, int base) 
{
    char* ptr = str, *ptr1 = str, tmp_char;
    unsigned int tmp_value;

    // Process individual digits
    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "0123456789abcdef"[tmp_value - value * base];
    } while (value);

    // Apply null terminator
    *ptr-- = '\0';

    // Reverse the string
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}

// Helper function to convert unsigned long to string
static void ultoa(unsigned long value, char* str, int base) 
{
    char* ptr = str, *ptr1 = str, tmp_char;
    unsigned long tmp_value;

    // Process individual digits
    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "0123456789abcdef"[tmp_value - value * base];
    } while (value);

    // Apply null terminator
    *ptr-- = '\0';

    // Reverse the string
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}

//------------------------------------------------------------------------------

void simple_stdio_uint16_to_str(uint16_t value, char *buffer) 
{
    uint8_t i = 0;
    do 
    {
        buffer[i++] = (value % 10) + '0';
        value /= 10;
    } while (value > 0);
    
    buffer[i] = '\0';

    /* Reverse the string */
    for (uint8_t j = 0; j < i / 2; j++) 
    {
        char temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }
}

char *simple_stdio_uint64_to_str(uint64_t n, char dest[static 21]) 
{
    dest += 20;
    *dest-- = 0;
    while (n) {
        *dest-- = (n % 10) + '0';
        n /= 10;
    }
    return dest + 1;
}

int simple_stdio_sprintf(char* buffer, const char* format, ...) 
{
    va_list args;
    va_start(args, format);

    char* buf_ptr = buffer;
    const char* fmt_ptr = format;
    char ch;
    char temp_str[20];

    while ((ch = *fmt_ptr++) != '\0') {
        if (ch == '%') {
            ch = *fmt_ptr++;
            switch (ch) {
                case 'd': {
                    int value = va_arg(args, int);
                    itoa(value, temp_str, 10);
                    for (char* t_ptr = temp_str; *t_ptr != '\0'; t_ptr++) {
                        *buf_ptr++ = *t_ptr;
                    }
                    break;
                }
                case 'u': {
                    unsigned int value = va_arg(args, unsigned int);
                    utoa(value, temp_str, 10);
                    for (char* t_ptr = temp_str; *t_ptr != '\0'; t_ptr++) {
                        *buf_ptr++ = *t_ptr;
                    }
                    break;
                }
                case 'x': {
                    unsigned int value = va_arg(args, unsigned int);
                    utoa(value, temp_str, 16);
                    for (char* t_ptr = temp_str; *t_ptr != '\0'; t_ptr++) {
                        *buf_ptr++ = *t_ptr;
                    }
                    break;
                }
                case 's': {
                    char* str = va_arg(args, char*);
                    while (*str != '\0') {
                        *buf_ptr++ = *str++;
                    }
                    break;
                }
                case 'c': {
                    char value = (char)va_arg(args, int);
                    *buf_ptr++ = value;
                    break;
                }
                case 'l': {
                    ch = *fmt_ptr++;
                    if (ch == 'u') {
                        unsigned long value = va_arg(args, unsigned long);
                        ultoa(value, temp_str, 10);
                        for (char* t_ptr = temp_str; *t_ptr != '\0'; t_ptr++) {
                            *buf_ptr++ = *t_ptr;
                        }
                    }
                    break;
                }
                default: {
                    // Unsupported format specifier, just copy as-is
                    *buf_ptr++ = ch;
                    break;
                }
            }
        } else {
            *buf_ptr++ = ch;
        }
    }

    // Null-terminate the final string
    *buf_ptr = '\0';

    va_end(args);
    
    return (buf_ptr - buffer); // Return the number of characters written
}

//------------------------------------------------------------------------------
