//------------------------------------------------------------------------------

/// @file ui_manager.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "ui_manager.h"

#include <stddef.h>
#include <string.h>

#include <event.h>

#include <simple_stdio.h>

#include <hal.h>

//------------------------------------------------------------------------------

enum ui_manager_icon_id 
{
    UI_MANAGER_CHAR_ID_CLOCK,
    UI_MANAGER_CHAR_ID_CALENDAR,
    UI_MANAGER_CHAR_ID_ALARM,
    UI_MANAGER_CHAR_ID_ANTENNA,  
};

enum ui_manager_item_id
{
    UI_MANAGER_ITEM_ID_TIME_H,
    UI_MANAGER_ITEM_ID_TIME_M,
    UI_MANAGER_ITEM_ID_TIME_S,
    UI_MANAGER_ITEM_ID_DATE_D,
    UI_MANAGER_ITEM_ID_DATE_M,
    UI_MANAGER_ITEM_ID_ALARM_H,
    UI_MANAGER_ITEM_ID_ALARM_M,

    UI_MANAGER_ITEM_ID_MAX,
};

enum ui_manager_item_property
{
    UI_MANAGER_ITEM_PROPERTY_POS,
    UI_MANAGER_ITEM_PROPERTY_MAX_VALUE,
    UI_MANAGER_ITEM_PROPERTY_VALUE,

    UI_MANAGER_ITEM_PROPERTY_MAX,   
};

enum ui_manager_state
{
    UI_MANAGER_STATE_TIME_DISPLAY,
    UI_MANAGER_STATE_TIME_SET,
    UI_MANAGER_STATE_VALUE_SELECT,
};

//------------------------------------------------------------------------------

static int8_t items[UI_MANAGER_ITEM_ID_MAX][UI_MANAGER_ITEM_PROPERTY_MAX] = 
{
    [UI_MANAGER_ITEM_ID_TIME_H] = {2, 23, 0},
    [UI_MANAGER_ITEM_ID_TIME_M] = {5, 59, 0},
    [UI_MANAGER_ITEM_ID_TIME_S] = {8, 59, 0},
    [UI_MANAGER_ITEM_ID_DATE_D] = {11, 31, 0},
    [UI_MANAGER_ITEM_ID_DATE_M] = {14, 12, 0},
    [UI_MANAGER_ITEM_ID_ALARM_H] = {18, 23, 0},
    [UI_MANAGER_ITEM_ID_ALARM_M] = {21, 59, 0},
};

static enum ui_manager_item_id item_id = 0;
static enum ui_manager_state state = UI_MANAGER_STATE_TIME_DISPLAY;

static struct buzzer_note alarm_beep[] = 
{
	{BUZZER_TONE_C6, BUZZER_NOTE_QUARTER},
	{BUZZER_TONE_STOP, BUZZER_NOTE_QUARTER},
	{BUZZER_TONE_C6, BUZZER_NOTE_QUARTER},
	{BUZZER_TONE_STOP, BUZZER_NOTE_QUARTER},
	{BUZZER_TONE_C6, BUZZER_NOTE_QUARTER},
	{BUZZER_TONE_STOP, BUZZER_NOTE_QUARTER},
	{BUZZER_TONE_C6, BUZZER_NOTE_QUARTER},
	{BUZZER_TONE_STOP, BUZZER_NOTE_QUARTER},
	{BUZZER_TONE_STOP, 1000UL * 800},
}; 

//------------------------------------------------------------------------------

/* HAL callbacks and structures */

const uint8_t hal_user_defined_char_tab[4][8] = 
{
    [UI_MANAGER_CHAR_ID_CLOCK] = {0x0E, 0x0E, 0x15, 0x15, 0x13, 0x0E, 0x0E, 0x0E}, // CLOCK
    [UI_MANAGER_CHAR_ID_CALENDAR] = {0x00, 0x15, 0x1F, 0x11, 0x15, 0x11, 0x1F, 0x00}, // CALENDAR
    [UI_MANAGER_CHAR_ID_ALARM] = {0x00, 0x1B, 0x0E, 0x15, 0x15, 0x13, 0x0E, 0x00}, // ALARM
    [UI_MANAGER_CHAR_ID_ANTENNA] = {0x00, 0x15, 0x0A, 0x04, 0x04, 0x04, 0x04, 0x00}, // ANTENNA
};

void hal_button_pressed_cb(void)
{
    // hal_lcd_clear();

    // memset(event_get_data(EVENT_SET_TIME_REQ), 0x00, sizeof(struct ds1307_time));

    // hal_set_time(event_get_data(EVENT_SET_TIME_REQ));
    
    // event_set(EVENT_SYNC_TIME_REQ);

    // hal_audio_set_pattern(NULL, 0, 0);

    switch (state)
    {
        case UI_MANAGER_STATE_TIME_DISPLAY:

            hal_lcd_set_cursor_mode(true, false);
            hal_lcd_set_cursor(0,0);
            state = UI_MANAGER_STATE_TIME_SET;
            break;

        case UI_MANAGER_STATE_TIME_SET:

            hal_lcd_set_cursor_mode(true, true);

            state = UI_MANAGER_STATE_VALUE_SELECT;
            break;

        case UI_MANAGER_STATE_VALUE_SELECT:

            hal_lcd_set_cursor_mode(true, false);

            state = UI_MANAGER_STATE_TIME_SET;
            break;
        default:
            break;
    }
}

void hal_encoder_rotation_cb(bool right)
{
    switch (state)
    {
        case UI_MANAGER_STATE_TIME_DISPLAY:
            break;

        case UI_MANAGER_STATE_TIME_SET:

            if (right) item_id++; else item_id--;
            item_id = (item_id + UI_MANAGER_ITEM_ID_MAX) % UI_MANAGER_ITEM_ID_MAX;

            hal_lcd_set_cursor(items[item_id][UI_MANAGER_ITEM_PROPERTY_POS] / 16, items[item_id][UI_MANAGER_ITEM_PROPERTY_POS] % 16);
            break;

        case UI_MANAGER_STATE_VALUE_SELECT:
            
            if (right) items[item_id][UI_MANAGER_ITEM_PROPERTY_VALUE]++; else items[item_id][UI_MANAGER_ITEM_PROPERTY_VALUE]--;
            items[item_id][UI_MANAGER_ITEM_PROPERTY_VALUE] = (items[item_id][UI_MANAGER_ITEM_PROPERTY_VALUE] + items[item_id][UI_MANAGER_ITEM_PROPERTY_MAX_VALUE] + 1) % (items[item_id][UI_MANAGER_ITEM_PROPERTY_MAX_VALUE] + 1);
            hal_lcd_putc(items[item_id][UI_MANAGER_ITEM_PROPERTY_VALUE] / 10 + '0');
            hal_lcd_putc(items[item_id][UI_MANAGER_ITEM_PROPERTY_VALUE] % 10 + '0');

            hal_lcd_set_cursor(items[item_id][UI_MANAGER_ITEM_PROPERTY_POS] / 16, items[item_id][UI_MANAGER_ITEM_PROPERTY_POS] % 16);

            break;
        default:
            break;
    }

    
}

//------------------------------------------------------------------------------

static char buf[16]; 

void print_time(uint8_t line, struct ds1307_time *unix_time)
{
    uint8_t i = 0;
    buf[i++] = (unix_time->hours_tens + '0');
    buf[i++] = (unix_time->hours_units + '0');
    buf[i++] = (':');
    buf[i++] = (unix_time->minutes_tens + '0');
    buf[i++] = (unix_time->minutes_units + '0');
    buf[i++] = (':');
    buf[i++] = (unix_time->seconds_tens + '0');
    buf[i++] = (unix_time->seconds_units + '0');
    buf[i++] = (' ');
    buf[i++] = (unix_time->date_tens + '0');
    buf[i++] = (unix_time->date_units + '0');
    buf[i++] = ('.');
    buf[i++] = (unix_time->month_tens + '0');
    buf[i++] = (unix_time->month_units + '0');
    buf[i++] = 0;
    hal_lcd_print(buf, line, 2);
}

//------------------------------------------------------------------------------

bool ui_manager_init(void)
{
    hal_lcd_set_cursor(0, 0);
    hal_lcd_putc(UI_MANAGER_CHAR_ID_CLOCK);
    hal_lcd_print(" 23:45:32", 0, 1);
    
    hal_lcd_set_cursor(0, 10);
    hal_lcd_print("13.05", 0, 11);
    
    hal_lcd_set_cursor(1, 0);
    hal_lcd_putc(UI_MANAGER_CHAR_ID_ALARM);
    hal_lcd_print(" 06:30", 1, 1);

    hal_lcd_set_cursor(1, 8);
    hal_lcd_putc(UI_MANAGER_CHAR_ID_ANTENNA);
    hal_lcd_print(" Synced", 1, 9);

    return true;
}

void ui_manager_process(void)
{
    hal_audio_process();
    return; // AAAAAAAAAAAAAAAAAAAA

    if (event_get() & EVENT_UPDATE_TIME_REQ)
    {
        print_time(0, event_get_data(EVENT_UPDATE_TIME_REQ));

        event_clear(EVENT_UPDATE_TIME_REQ);
    }

    if (event_get() & EVENT_ALARM_REQ)
    {
        hal_audio_set_pattern(alarm_beep, sizeof(alarm_beep), 800);

        event_clear(EVENT_ALARM_REQ);
    }


    if (event_get() & EVENT_SYNC_TIME_STATUS)
    {
        event_sync_time_status_data_t *sync_time_status_data = event_get_data(EVENT_SYNC_TIME_STATUS);

        simple_stdio_uint16_to_str(sync_time_status_data->time_ms, buf);
    
        uint8_t pos = sync_time_status_data->triggred_on_bit ? 0 : 8;
    
        hal_lcd_print("       ", 1, pos);
        hal_lcd_print(buf, 1, pos);
    
        if (sync_time_status_data->frame_started)
        {
            hal_lcd_print("S", 1, 15);
        }
        else if (sync_time_status_data->error)
        {
            hal_lcd_print("E", 1, 15);
        }

        hal_led_set(!sync_time_status_data->dcf_output);

        event_clear(EVENT_SYNC_TIME_STATUS);
    }
}

//------------------------------------------------------------------------------
