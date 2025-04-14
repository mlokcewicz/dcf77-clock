//------------------------------------------------------------------------------

/// @file hd44780.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef HD44780_H_
#define HD44780_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>

//------------------------------------------------------------------------------

typedef void (*hd44780_pin_init_cb)(void);
typedef void (*hd44780_pin_deinit_cb)(void);
typedef void (*hd44780_set_pin_cb)(uint8_t pin, bool state);
typedef void (*hd44780_delay_us_cb)(uint16_t microseconds);

//------------------------------------------------------------------------------

enum hd44780_pin 
{
    LCD_RS = 0,
    LCD_RW,
    LCD_E,
    LCD_D4,
    LCD_D5,
    LCD_D6,
    LCD_D7,
};

//------------------------------------------------------------------------------

struct hd44780_cfg 
{
    hd44780_pin_init_cb pin_init;
    hd44780_pin_deinit_cb pin_deinit;
    hd44780_set_pin_cb set_pin_state;
    hd44780_delay_us_cb delay_us;

    const uint8_t (*user_defined_char_tab)[8];
    uint8_t user_defined_char_tab_len;
};

struct hd44780_obj 
{
    hd44780_pin_init_cb pin_init;
    hd44780_pin_deinit_cb pin_deinit;
    hd44780_set_pin_cb set_pin_state;
    hd44780_delay_us_cb delay_us;
};

//------------------------------------------------------------------------------

/// @brief Initializes HD44780, IO and register callbacks
/// @note This driver does not read BUSY flag - RW pin can be connected to GND
/// @note User defined chars have to be placed in 2-dimensional array [ch_idx][byte_idx]
/// @param obj given LCD object @ref struct hd44780_obj
/// @param cfg given LCD configuration @ref struct hd4470_cfg
/// @return true if initialized correctly, otherwise false
bool hd44780_init(struct hd44780_obj *obj, struct hd44780_cfg *cfg);

/// @brief Prints given null-terminated string after last string's end
/// @param obj given LCD object @ref struct hd44780_obj
/// @param str null terminated string
void hd44780_print(struct hd44780_obj *obj, const char* str);

/// @brief Prints given char after last string's end
/// @param obj given LCD object @ref struct hd44780_obj
/// @param ch selected characer code
void hd44780_putc(struct hd44780_obj *obj, const char ch);

/// @brief Clears display content
/// @param obj given LCD object @ref struct hd44780_obj
void hd44780_clear(struct hd44780_obj *obj);

/// @brief Shifts cursor LCD content in selected direction 
/// @param obj given LCD object @ref struct hd44780_obj
/// @param disp true for display content, false for cursor
/// @param right true for right, false for left
void hd44780_shift(struct hd44780_obj *obj, bool disp, bool right);

/// @brief Sets cursor position 
/// @param obj given LCD object @ref struct hd44780_obj
/// @param line selected line
/// @param pos selected position
void hd44780_set_pos(struct hd44780_obj *obj, uint8_t line, uint8_t pos);

/// @brief Deinitializes HD44780, IO and reset callbacks
/// @param obj given LCD object @ref struct hd44780_obj
void hd44780_deinit(struct hd44780_obj *obj);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* HD44780_H_ */

//------------------------------------------------------------------------------
