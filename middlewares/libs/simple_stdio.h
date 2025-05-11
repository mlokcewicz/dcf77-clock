//------------------------------------------------------------------------------

/// @file simple_stdio.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef SIMPLE_STDIO_H_
#define SIMPLE_STDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>

//------------------------------------------------------------------------------

#define UINT64_TO_STR(n)  simple_stdio_uint64_to_str(n, (char[21]){0})

//------------------------------------------------------------------------------

/// @brief Converts uint16_t number to string
/// @param value number
/// @param buffer destination buffer
void simple_stdio_uint16_to_str(uint16_t value, char* buffer);

/// @brief Converts uint64_t number to string
/// @param n number
/// @param dest destination buffer created inline by macro
/// @return pointer to char buffer
char* simple_stdio_uint64_to_str(uint64_t n, char dest[static 21]);

/// @brief Simple version of sprintf (supports %d, %u, %x, %s, %c, and %l specificatios)
/// @param buffer input string buffer
/// @param format string format 
/// @return number of characters written
int simple_stdio_sprintf(char* buffer, const char* format, ...);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* SIMPLE_STDIO_H_ */

//------------------------------------------------------------------------------
