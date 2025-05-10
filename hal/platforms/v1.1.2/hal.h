//------------------------------------------------------------------------------

/// @file hal.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef HAL_H_
#define HAL_H_

//------------------------------------------------------------------------------

#include <stdint.h>

//------------------------------------------------------------------------------

void hal_init(void);

void hal_lcd_clear(void);

void hal_lcd_print(const char* str, uint8_t row, uint8_t col);

// TODO: Add comments

// TODO: Add macros

//------------------------------------------------------------------------------

#endif /* HAL_H_ */
