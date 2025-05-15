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
    UI_MANAGER_ITEM_ID_SYNC,
    UI_MANAGER_ITEM_ID_OK,
    UI_MANAGER_ITEM_ID_ESC,
    UI_MANAGER_ITEM_ID_NEXT,
    // UI_MANAGER_ITEM_ID_PREV,

    UI_MANAGER_SETTABLE_ITEM_ID_MAX,
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
    UI_MANAGER_STATE_TIME_DATE_ALARM_DISPLAY,
    UI_MANAGER_STATE_TIME_DATE_ALARM_SET,
    UI_MANAGER_STATE_VALUE_SELECT,
    UI_MANAGER_STATE_SYNC_SATUS_DISPLAY,
};

//------------------------------------------------------------------------------

static int8_t items[UI_MANAGER_SETTABLE_ITEM_ID_MAX][UI_MANAGER_ITEM_PROPERTY_MAX] = 
{
    [UI_MANAGER_ITEM_ID_TIME_H] = {2, 23, 0},
    [UI_MANAGER_ITEM_ID_TIME_M] = {5, 59, 0},
    [UI_MANAGER_ITEM_ID_TIME_S] = {8, 59, 0},
    [UI_MANAGER_ITEM_ID_DATE_D] = {11, 31, 0},
    [UI_MANAGER_ITEM_ID_DATE_M] = {14, 12, 0},
    [UI_MANAGER_ITEM_ID_ALARM_H] = {18, 23, 0},
    [UI_MANAGER_ITEM_ID_ALARM_M] = {21, 59, 0},
    [UI_MANAGER_ITEM_ID_SYNC] = {23, 0, 0},
    [UI_MANAGER_ITEM_ID_OK] = {26, 0, 0},
    [UI_MANAGER_ITEM_ID_ESC] = {29, 0, 0},
    [UI_MANAGER_ITEM_ID_NEXT] = {31, 0, 0},
    // [UI_MANAGER_ITEM_ID_PREV] = {16, 0, 0},
};

static enum ui_manager_item_id item_id = 0;
static enum ui_manager_state state = UI_MANAGER_STATE_TIME_DATE_ALARM_DISPLAY;

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

static void copy_item_to_time(event_set_time_req_data_t *time)
{
    time->clock_halt = 0;
    time->hour_mode = 0;
    time->hours_tens = items[UI_MANAGER_ITEM_ID_TIME_H][UI_MANAGER_ITEM_PROPERTY_VALUE] / 10;
    time->hours_units = items[UI_MANAGER_ITEM_ID_TIME_H][UI_MANAGER_ITEM_PROPERTY_VALUE] % 10;
    time->minutes_tens = items[UI_MANAGER_ITEM_ID_TIME_M][UI_MANAGER_ITEM_PROPERTY_VALUE] / 10;
    time->minutes_units = items[UI_MANAGER_ITEM_ID_TIME_M][UI_MANAGER_ITEM_PROPERTY_VALUE] % 10;
    time->seconds_tens = items[UI_MANAGER_ITEM_ID_TIME_S][UI_MANAGER_ITEM_PROPERTY_VALUE] / 10;
    time->seconds_units = items[UI_MANAGER_ITEM_ID_TIME_S][UI_MANAGER_ITEM_PROPERTY_VALUE] % 10;
    time->date_tens = items[UI_MANAGER_ITEM_ID_DATE_D][UI_MANAGER_ITEM_PROPERTY_VALUE] / 10;
    time->date_units = items[UI_MANAGER_ITEM_ID_DATE_D][UI_MANAGER_ITEM_PROPERTY_VALUE] % 10;
    time->month_tens = items[UI_MANAGER_ITEM_ID_DATE_M][UI_MANAGER_ITEM_PROPERTY_VALUE] / 10;
    time->month_units = items[UI_MANAGER_ITEM_ID_DATE_M][UI_MANAGER_ITEM_PROPERTY_VALUE] % 10;
}

static void copy_item_to_alarm(event_set_alarm_req_data_t *alarm)
{
    alarm->hours_tens = items[UI_MANAGER_ITEM_ID_ALARM_H][UI_MANAGER_ITEM_PROPERTY_VALUE] / 10;
    alarm->hours_units = items[UI_MANAGER_ITEM_ID_ALARM_H][UI_MANAGER_ITEM_PROPERTY_VALUE] % 10;
    alarm->minutes_tens = items[UI_MANAGER_ITEM_ID_ALARM_M][UI_MANAGER_ITEM_PROPERTY_VALUE] / 10;
    alarm->minutes_units = items[UI_MANAGER_ITEM_ID_ALARM_M][UI_MANAGER_ITEM_PROPERTY_VALUE] % 10;
    alarm->is_enabled = true;
}

static void copy_time_to_item(event_set_time_req_data_t *time)
{
    items[UI_MANAGER_ITEM_ID_TIME_H][UI_MANAGER_ITEM_PROPERTY_VALUE] = time->hours_tens * 10 + time->hours_units;
    items[UI_MANAGER_ITEM_ID_TIME_M][UI_MANAGER_ITEM_PROPERTY_VALUE] = time->minutes_tens * 10 + time->minutes_units;
    items[UI_MANAGER_ITEM_ID_TIME_S][UI_MANAGER_ITEM_PROPERTY_VALUE] = time->seconds_tens * 10 + time->seconds_units;
    items[UI_MANAGER_ITEM_ID_DATE_D][UI_MANAGER_ITEM_PROPERTY_VALUE] = time->date_tens * 10 + time->date_units;
    items[UI_MANAGER_ITEM_ID_DATE_M][UI_MANAGER_ITEM_PROPERTY_VALUE] = time->month_tens * 10 + time->month_units;
}

static void copy_alarm_to_item(event_set_alarm_req_data_t *alarm)
{
    items[UI_MANAGER_ITEM_ID_ALARM_H][UI_MANAGER_ITEM_PROPERTY_VALUE] = alarm->hours_tens * 10 + alarm->hours_units;
    items[UI_MANAGER_ITEM_ID_ALARM_M][UI_MANAGER_ITEM_PROPERTY_VALUE] = alarm->minutes_tens * 10 + alarm->minutes_units;
}

static char buf[16]; 

static void print_time(struct ds1307_time *unix_time)
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
    hal_lcd_print(buf, 0, 2);
}

static void print_alarm(struct hal_timestamp *alarm)
{
    uint8_t i = 0;
    buf[i++] = (alarm->hours_tens + '0');
    buf[i++] = (alarm->hours_units + '0');
    buf[i++] = (':');
    buf[i++] = (alarm->minutes_tens + '0');
    buf[i++] = (alarm->minutes_units + '0');
    buf[i++] = 0;
    hal_lcd_print(buf, 1, 2);
}

static void print_static_icons(void)
{
    hal_lcd_set_cursor(0, 0);
    hal_lcd_putc(UI_MANAGER_CHAR_ID_CLOCK);
    
    hal_lcd_set_cursor(1, 0);
    hal_lcd_putc(UI_MANAGER_CHAR_ID_ALARM);

    hal_lcd_set_cursor(1, 8);
    hal_lcd_putc(UI_MANAGER_CHAR_ID_ANTENNA);

    hal_lcd_set_cursor(1, 15);
    hal_lcd_putc('>');
}

static void print_sync_status( event_sync_time_status_data_t *sync_time_status_data)
{
    simple_stdio_uint16_to_str(sync_time_status_data->time_ms, buf);

    uint8_t pos = sync_time_status_data->triggred_on_bit ? 0 : 8;

    hal_lcd_print("       ", 0, pos);
    hal_lcd_print(buf, 0, pos);

    if (sync_time_status_data->frame_started)
    {
        hal_lcd_print("S", 0, 15);
    }
    else if (sync_time_status_data->error)
    {
        hal_lcd_print("E", 0, 15);
    }

    hal_led_set(!sync_time_status_data->dcf_output);
}

#include "ui_manager.h" // Upewnij się, że enum ui_manager_item_id jest dostępny

// static void update_time(enum ui_manager_item_id item_id, int8_t val)
// {
//     event_update_time_req_data_t *time = (event_update_time_req_data_t *)event_get_data(EVENT_UPDATE_TIME_REQ);

//     switch (item_id)
//     {
//         case UI_MANAGER_ITEM_ID_TIME_H:
//             {
//                 int8_t h = time->hours_tens * 10 + time->hours_units + val;
//                 if (h < 0) h = 23;
//                 if (h > 23) h = 0;
//                 time->hours_tens = h / 10;
//                 time->hours_units = h % 10;
//             }
//             break;
//         case UI_MANAGER_ITEM_ID_TIME_M:
//             {
//                 int8_t m = time->minutes_tens * 10 + time->minutes_units + val;
//                 if (m < 0) m = 59;
//                 if (m > 59) m = 0;
//                 time->minutes_tens = m / 10;
//                 time->minutes_units = m % 10;
//             }
//             break;
//         case UI_MANAGER_ITEM_ID_TIME_S:
//             {
//                 int8_t s = time->seconds_tens * 10 + time->seconds_units + val;
//                 if (s < 0) s = 59;
//                 if (s > 59) s = 0;
//                 time->seconds_tens = s / 10;
//                 time->seconds_units = s % 10;
//             }
//             break;
//         case UI_MANAGER_ITEM_ID_DATE_D:
//             {
//                 int8_t d = time->date_tens * 10 + time->date_units + val;
//                 if (d < 1) d = 31;
//                 if (d > 31) d = 1;
//                 time->date_tens = d / 10;
//                 time->date_units = d % 10;
//             }
//             break;
//         case UI_MANAGER_ITEM_ID_DATE_M:
//             {
//                 int8_t m = time->month_tens * 10 + time->month_units + val;
//                 if (m < 1) m = 12;
//                 if (m > 12) m = 1;
//                 time->month_tens = m / 10;
//                 time->month_units = m % 10;
//             }
//             break;
//         case UI_MANAGER_ITEM_ID_ALARM_H:
//             {
//                 int8_t h = time->hours_tens * 10 + time->hours_units + val;
//                 if (h < 0) h = 23;
//                 if (h > 23) h = 0;
//                 time->hours_tens = h / 10;
//                 time->hours_units = h % 10;
//             }
//             break;
//         case UI_MANAGER_ITEM_ID_ALARM_M:
//             {
//                 int8_t m = time->minutes_tens * 10 + time->minutes_units + val;
//                 if (m < 0) m = 59;
//                 if (m > 59) m = 0;
//                 time->minutes_tens = m / 10;
//                 time->minutes_units = m % 10;
//             }
//             break;
//         default:
//             break;
//     }
// }

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
    hal_audio_set_pattern(NULL, 0, 0);

    switch (state)
    {
        case UI_MANAGER_STATE_TIME_DATE_ALARM_DISPLAY:

            // copy_alarm_to_item(event_get_data(EVENT_SET_ALARM_REQ));
            // copy_time_to_item(event_get_data(EVENT_UPDATE_TIME_REQ));

            hal_lcd_print("OK", 1, 10);
            hal_lcd_print("E", 1, 13);
            hal_lcd_print(">", 1, 15);

            hal_lcd_set_cursor_mode(true, false);
            hal_lcd_set_cursor(items[0][UI_MANAGER_ITEM_PROPERTY_POS] / 16, items[0][UI_MANAGER_ITEM_PROPERTY_POS] % 16);

            state = UI_MANAGER_STATE_TIME_DATE_ALARM_SET;
            break;

        case UI_MANAGER_STATE_TIME_DATE_ALARM_SET:

            if (item_id == UI_MANAGER_ITEM_ID_ESC)
            {
                hal_lcd_set_cursor_mode(false, false);
                state = UI_MANAGER_STATE_TIME_DATE_ALARM_DISPLAY;
            }
            else if (item_id == UI_MANAGER_ITEM_ID_OK)
            {
                hal_lcd_set_cursor_mode(false, false);

                // copy_item_to_time(event_get_data(EVENT_SET_TIME_REQ));

                event_set(EVENT_SET_TIME_REQ);

                // copy_item_to_alarm(event_get_data(EVENT_SET_ALARM_REQ));

                event_set(EVENT_SET_ALARM_REQ);

                state = UI_MANAGER_STATE_TIME_DATE_ALARM_DISPLAY;
            }
            else if (item_id == UI_MANAGER_ITEM_ID_SYNC)
            {
                event_set(EVENT_SYNC_TIME_REQ);
            }
            else if (item_id == UI_MANAGER_ITEM_ID_NEXT)
            {
                state = UI_MANAGER_STATE_SYNC_SATUS_DISPLAY;
                hal_lcd_clear();
                hal_lcd_set_cursor(1,0);
                hal_lcd_putc('<');
                hal_lcd_set_cursor(1,0);
            }
            else
            {
                hal_lcd_set_cursor_mode(true, true);

                state = UI_MANAGER_STATE_VALUE_SELECT;
            }
            break;

        case UI_MANAGER_STATE_VALUE_SELECT:

            hal_lcd_set_cursor_mode(true, false);

            state = UI_MANAGER_STATE_TIME_DATE_ALARM_SET;
            break;

        case UI_MANAGER_STATE_SYNC_SATUS_DISPLAY:
            
            // if (item_id == UI_MANAGER_ITEM_ID_PREV)
            {
                hal_lcd_print("OK", 1, 10);
                hal_lcd_print("E", 1, 13);
                hal_lcd_print(">", 1, 15);

                print_alarm(event_get_data(EVENT_SET_ALARM_REQ));
                print_static_icons();
                print_time(event_get_data(EVENT_UPDATE_TIME_REQ));
                state = UI_MANAGER_STATE_TIME_DATE_ALARM_SET;
            }

        default:
            break;
    }
}

void hal_encoder_rotation_cb(uint8_t dir)
{
    switch (state)
    {
        case UI_MANAGER_STATE_TIME_DATE_ALARM_DISPLAY:
            break;

        case UI_MANAGER_STATE_TIME_DATE_ALARM_SET:

            item_id += dir;
            item_id = (item_id + UI_MANAGER_SETTABLE_ITEM_ID_MAX) % UI_MANAGER_SETTABLE_ITEM_ID_MAX;

            hal_lcd_set_cursor(items[item_id][UI_MANAGER_ITEM_PROPERTY_POS] / 16, items[item_id][UI_MANAGER_ITEM_PROPERTY_POS] % 16);
            break;

        case UI_MANAGER_STATE_VALUE_SELECT:
            
            items[item_id][UI_MANAGER_ITEM_PROPERTY_VALUE] += dir;
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

bool ui_manager_init(void)
{
    hal_lcd_clear();

    print_static_icons();
    print_alarm(event_get_data(EVENT_SET_ALARM_REQ));

    return true;
}

void ui_manager_process(void)
{
    hal_audio_process();

    if (event_get() & EVENT_ALARM_REQ)
    {
        hal_audio_set_pattern(alarm_beep, sizeof(alarm_beep), 800);

        event_clear(EVENT_ALARM_REQ);
    }

    switch (state)
    {
        case UI_MANAGER_STATE_TIME_DATE_ALARM_DISPLAY:
        {
            if (event_get() & EVENT_UPDATE_TIME_REQ)
            {
                print_time(event_get_data(EVENT_UPDATE_TIME_REQ));

                event_clear(EVENT_UPDATE_TIME_REQ);
            }

            break;
        }
        case UI_MANAGER_STATE_TIME_DATE_ALARM_SET:
        {
            break;
        }
        case UI_MANAGER_STATE_VALUE_SELECT:
        {
            break;
        }
        case UI_MANAGER_STATE_SYNC_SATUS_DISPLAY: 
        {
            if (event_get() & EVENT_SYNC_TIME_STATUS)
            {
                print_sync_status(event_get_data(EVENT_SYNC_TIME_STATUS));

                event_clear(EVENT_SYNC_TIME_STATUS);
            }
            break;
        }
    }
}

//------------------------------------------------------------------------------
