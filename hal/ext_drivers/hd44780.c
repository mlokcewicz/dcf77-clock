//------------------------------------------------------------------------------

/// @file hd44780.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "hd44780.h"

//------------------------------------------------------------------------------

#include <stdio.h>
#include <unistd.h>

//------------------------------------------------------------------------------

enum hd44780_mode
{
    MODE_DATA,
    MODE_COMMAND
};

enum hd44780_ommand
{
    HD44780_CMD_CLEAR_DISPLAY = 1 << 0,

    HD44780_CMD_RETURN_HOME = 1 << 1,

    HD44780_CMD_ENTRY_MODE_SET_DISP_SHIFT = 1 << 2 | 1 << 0, 
    HD44780_CMD_ENTRY_MODE_SET_DISP_NOSHIFT = 1 << 2, 
    HD44780_CMD_ENTRY_MODE_SET_INC = 1 << 2 | 1 << 1, 
    HD44780_CMD_ENTRY_MODE_SET_DEC = 1 << 2, 

    HD44780_CMD_DISPLAY_ON_OFF_CONTROL_CURSOR_BLINK_ON = 1 << 3 | 1 << 0, 
    HD44780_CMD_DISPLAY_ON_OFF_CONTROL_CURSOR_BLINK_OFF = 1 << 3, 
    HD44780_CMD_DISPLAY_ON_OFF_CONTROL_CURSOR_ON = 1 << 3 | 1 << 1, 
    HD44780_CMD_DISPLAY_ON_OFF_CONTROL_CURSOR_OFF = 1 << 3, 
    HD44780_CMD_DISPLAY_ON_OFF_CONTROL_DISP_ON = 1 << 3 | 1 << 2, 
    HD44780_CMD_DISPLAY_ON_OFF_CONTROL_DISP_OFF = 1 << 3, 

    HD44780_CMD_CURSOR_DISPLAY_SHIFT_RIGHT = 1 << 4 | 1 << 2, 
    HD44780_CMD_CURSOR_DISPLAY_SHIFT_LEFT = 1 << 4, 
    HD44780_CMD_CURSOR_DISPLAY_SHIFT_DISP = 1 << 4 | 1 << 3, 
    HD44780_CMD_CURSOR_DISPLAY_SHIFT_CURSOR = 1 << 4, 

    HD44780_CMD_FUNCTION_SET_FONT_5x10 = 1 << 5 | 1 << 2,
    HD44780_CMD_FUNCTION_SET_FONT_5x8 = 1 << 5,
    HD44780_CMD_FUNCTION_SET_LINES_2 = 1 << 5 | 1 << 3,
    HD44780_CMD_FUNCTION_SET_LINES_1 = 1 << 5,
    HD44780_CMD_FUNCTION_SET_DL_8_BITS = 1 << 5 | 1 << 4,
    HD44780_CMD_FUNCTION_SET_DL_4_BITS = 1 << 5,

    HD44780_CMD_SET_CGRAM_ADDRESS = 1 << 6,

    HD44780_CMD_SET_DDRAM_ADDRESS = 1 << 7,

    HD44780_CMD_READ_BUSY_FLAG = 1 << 8,
    HD44780_CMD_READ_BUSY_FLAG_SET = 1 << 7,
};

//------------------------------------------------------------------------------

static void send_nibble(struct hd44780_obj *obj, uint8_t nibble)
{
    /* Send nibble */
    for (uint8_t i = 0; i < 4; i++)
    {
        obj->set_pin_state(LCD_D4 + i, nibble & 1);
        nibble >>= 1;
    }

    /* Toggle enable pin */
    obj->set_pin_state(LCD_E, true);
    obj->delay_us(5);
    obj->set_pin_state(LCD_E, false);
    obj->delay_us(5);
}

static void send_byte(struct hd44780_obj *obj, uint8_t byte, enum hd44780_mode mode)
{
    /* Set mode for data pins */
    obj->set_pin_state(LCD_RS, !mode);
    
    /* Send first and second nibble */
    send_nibble(obj, byte >> 4);
    send_nibble(obj, byte);

    obj->delay_us(40); 

    if (mode == MODE_COMMAND && (byte == HD44780_CMD_CLEAR_DISPLAY || byte == HD44780_CMD_RETURN_HOME))
        obj->delay_us(1600); 
}

bool hd44780_init(struct hd44780_obj *obj, struct hd44780_cfg *cfg)
{
    if (!obj || !cfg || !cfg->pin_init || !cfg->pin_deinit || !cfg->set_pin_state || !cfg->delay_us)
        return false;

    /* Assign callbacks */
    obj->pin_init = cfg->pin_init;
    obj->pin_deinit = cfg->pin_deinit;
    obj->set_pin_state = cfg->set_pin_state;
    obj->delay_us = cfg->delay_us;

    /* Initialize IO */
    obj->pin_init();

    for (uint8_t i = 0; i < 7; i++)
        obj->set_pin_state(LCD_RS + i, false);
    
    /* Initialize LCD */
    obj->delay_us(50000); // Done twice to reduce delay parameter size to uint16_t
    obj->delay_us(50000);

    /* LCD is initialized by default with 8-bit data bus so there is a need to double each nibble */
    /* Set 8-bit data bus command at least 3 times - specific for some displays */
    /* Then 4-bit but still in 8 bit mode - lower nibble has to be 4 bit set command */

    static const uint8_t initialization_bytes[] = 
    {
        HD44780_CMD_FUNCTION_SET_DL_8_BITS | (HD44780_CMD_FUNCTION_SET_DL_8_BITS >> 4),
        HD44780_CMD_FUNCTION_SET_DL_8_BITS | (HD44780_CMD_FUNCTION_SET_DL_4_BITS >> 4),
        HD44780_CMD_DISPLAY_ON_OFF_CONTROL_DISP_OFF | HD44780_CMD_DISPLAY_ON_OFF_CONTROL_CURSOR_OFF | HD44780_CMD_DISPLAY_ON_OFF_CONTROL_CURSOR_BLINK_OFF,
        HD44780_CMD_FUNCTION_SET_DL_4_BITS | HD44780_CMD_FUNCTION_SET_LINES_2 | HD44780_CMD_FUNCTION_SET_FONT_5x8,
        HD44780_CMD_ENTRY_MODE_SET_INC | HD44780_CMD_ENTRY_MODE_SET_DISP_NOSHIFT,
        HD44780_CMD_CLEAR_DISPLAY,
        HD44780_CMD_RETURN_HOME,
        HD44780_CMD_DISPLAY_ON_OFF_CONTROL_DISP_ON | HD44780_CMD_DISPLAY_ON_OFF_CONTROL_CURSOR_OFF | HD44780_CMD_DISPLAY_ON_OFF_CONTROL_CURSOR_BLINK_OFF,
    };

    for (uint8_t i = 0; i < sizeof(initialization_bytes); i++)
        send_byte(obj, initialization_bytes[i], MODE_COMMAND);

    /* Load user defined characters to CGRAM */
    if (cfg->user_defined_char_tab)
    {
        for (uint8_t char_idx = 0; char_idx < cfg->user_defined_char_tab_len; char_idx++)
        {
            /* Set char address in CGRAM */
            send_byte(obj, HD44780_CMD_SET_CGRAM_ADDRESS | (char_idx * 8), MODE_COMMAND);

            for (uint8_t byte_idx = 0; byte_idx < 8; byte_idx++)
            {
                send_byte(obj, cfg->user_defined_char_tab[char_idx][byte_idx], MODE_DATA);
            }
        }

        /* Return to DDRAM write mode */
        send_byte(obj, HD44780_CMD_SET_DDRAM_ADDRESS | 0, MODE_COMMAND);
    }

    return true;
}

void hd44780_print(struct hd44780_obj *obj, const char* str)
{
    /* Write null terminated string */
    while (*str)
        send_byte(obj, *str++, MODE_DATA);
}

void hd44780_putc(struct hd44780_obj *obj, const char ch)
{
    send_byte(obj, ch, MODE_DATA);
}

void hd44780_set_pos(struct hd44780_obj *obj, uint8_t line, uint8_t pos)
{
    uint8_t address = line * 0x40 + pos;
    
    /* Set the cursor position */
    send_byte(obj, HD44780_CMD_SET_DDRAM_ADDRESS | address, MODE_COMMAND);
}

void hd44780_shift(struct hd44780_obj *obj, bool disp, bool right)
{
    enum hd44780_ommand cmd = 0;
    cmd |= disp ? HD44780_CMD_CURSOR_DISPLAY_SHIFT_DISP : HD44780_CMD_CURSOR_DISPLAY_SHIFT_CURSOR;
    cmd |= right ? HD44780_CMD_CURSOR_DISPLAY_SHIFT_RIGHT : HD44780_CMD_CURSOR_DISPLAY_SHIFT_LEFT;

    send_byte(obj, cmd, MODE_COMMAND);
}

void hd44780_clear(struct hd44780_obj *obj)
{
    send_byte(obj, HD44780_CMD_CLEAR_DISPLAY, MODE_COMMAND);
}

void hd44780_deinit(struct hd44780_obj *obj)
{
    /* Deinitialize LCD */
    send_byte(obj, HD44780_CMD_CLEAR_DISPLAY, MODE_COMMAND);
    send_byte(obj, HD44780_CMD_RETURN_HOME, MODE_COMMAND);
    send_byte(obj, HD44780_CMD_DISPLAY_ON_OFF_CONTROL_DISP_OFF | HD44780_CMD_DISPLAY_ON_OFF_CONTROL_CURSOR_OFF | HD44780_CMD_DISPLAY_ON_OFF_CONTROL_CURSOR_BLINK_OFF, MODE_COMMAND);

    /* Enter power down state */
    for (uint8_t i = 0; i < 7; i++)
        obj->set_pin_state(LCD_RS + i, false);

    /* Deinitialize IO */
    obj->pin_deinit();

    /* Reset callbacks */
    obj->pin_init = NULL;
    obj->pin_deinit = NULL;
    obj->set_pin_state = NULL;
    obj->delay_us = NULL;
}

//------------------------------------------------------------------------------
