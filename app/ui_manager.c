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
    UI_MANAGER_ITEM_PROPERTY_MIN_VALUE,
    UI_MANAGER_ITEM_PROPERTY_MAX_VALUE,
    UI_MANAGER_ITEM_PROPERTY_OFFSET,

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

struct ui_manager_ctx 
{
    enum ui_manager_state state;
    enum ui_manager_item_id item_id;
};

static struct ui_manager_ctx ctx; 

static const int8_t items[UI_MANAGER_SETTABLE_ITEM_ID_MAX][UI_MANAGER_ITEM_PROPERTY_MAX] = 
{
    [UI_MANAGER_ITEM_ID_TIME_H] = {3, 0, 23, offsetof(event_set_time_req_data_t, hours)},
    [UI_MANAGER_ITEM_ID_TIME_M] = {6, 0, 59, offsetof(event_set_time_req_data_t, minutes)},
    [UI_MANAGER_ITEM_ID_TIME_S] = {9, 0, 59, offsetof(event_set_time_req_data_t, seconds)},
    [UI_MANAGER_ITEM_ID_DATE_D] = {12, 1, 31, offsetof(event_set_time_req_data_t, date)},
    [UI_MANAGER_ITEM_ID_DATE_M] = {15, 1, 12, offsetof(event_set_time_req_data_t, month)},
    [UI_MANAGER_ITEM_ID_ALARM_H] = {19, 0, 23, offsetof(event_set_alarm_req_data_t, hours)},
    [UI_MANAGER_ITEM_ID_ALARM_M] = {22, 0, 59, offsetof(event_set_alarm_req_data_t, minutes)},
    [UI_MANAGER_ITEM_ID_SYNC] = {24, 0, 0, 0},
    [UI_MANAGER_ITEM_ID_OK] = {26, 0, 0, 0},
    [UI_MANAGER_ITEM_ID_ESC] = {29, 0, 0, 0},
    [UI_MANAGER_ITEM_ID_NEXT] = {31, 0, 0, 0},
    // [UI_MANAGER_ITEM_ID_PREV] = {16, 0, 0},
};

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

static char buf[16]; 

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

static void print_time(struct ds1307_time *unix_time)
{
    uint8_t i = 0;
    buf[i++] = (unix_time->hours / 10 + '0');
    buf[i++] = (unix_time->hours % 10 + '0');
    buf[i++] = (':');
    buf[i++] = (unix_time->minutes / 10 + '0');
    buf[i++] = (unix_time->minutes % 10 + '0');
    buf[i++] = (':');
    buf[i++] = (unix_time->seconds / 10 + '0');
    buf[i++] = (unix_time->seconds % 10 + '0');
    buf[i++] = (' ');
    buf[i++] = (unix_time->date / 10 + '0');
    buf[i++] = (unix_time->date % 10 + '0');
    buf[i++] = ('.');
    buf[i++] = (unix_time->month / 10 + '0');
    buf[i++] = (unix_time->month % 10 + '0');
    buf[i++] = 0;
    hal_lcd_print(buf, 0, 2);
}

static void print_alarm(struct hal_timestamp *alarm)
{
    uint8_t i = 0;
    buf[i++] = (alarm->hours / 10 + '0');
    buf[i++] = (alarm->hours % 10 + '0');
    buf[i++] = (':');
    buf[i++] = (alarm->minutes / 10 + '0');
    buf[i++] = (alarm->minutes % 10 + '0');
    buf[i++] = 0;
    hal_lcd_print(buf, 1, 2);
}

static void print_sync_status(event_sync_time_status_data_t *sync_time_status_data)
{
    simple_stdio_uint16_to_str(sync_time_status_data->time_ms, buf);

    uint8_t pos = sync_time_status_data->triggred_on_bit ? 0 : 8;

    hal_lcd_print("       ", 0, pos);
    hal_lcd_print(buf, 0, pos);

    if (sync_time_status_data->frame_started)
        hal_lcd_print("S", 0, 15);
    else if (sync_time_status_data->error)
        hal_lcd_print("E", 0, 15);

    hal_led_set(!sync_time_status_data->dcf_output);
}

static void print_alarm_date_display_screen(void)
{
    hal_lcd_clear();

    print_static_icons();
    print_time(event_get_data(EVENT_UPDATE_TIME_REQ));
    print_alarm(event_get_data(EVENT_SET_ALARM_REQ));
}

static void print_alarm_date_set_screen(void)
{
    hal_lcd_clear();

    print_static_icons();
    print_time(event_get_data(EVENT_UPDATE_TIME_REQ));
    print_alarm(event_get_data(EVENT_SET_ALARM_REQ));

    hal_lcd_print("OK", 1, 10);
    hal_lcd_print("E", 1, 13);
    hal_lcd_print(">", 1, 15);

    hal_lcd_set_cursor_mode(true, false);
    hal_lcd_set_cursor(items[0][UI_MANAGER_ITEM_PROPERTY_POS] / 16, items[0][UI_MANAGER_ITEM_PROPERTY_POS] % 16);
}

static int8_t item_limit_value(int8_t val, int8_t min, int8_t max)
{
    if (val < min) return max;
    if (val > max) return min;
    return val;
}

static void item_update(enum ui_manager_item_id item_id, int8_t val)
{
    event_set_time_req_data_t *time = (event_set_time_req_data_t *)event_get_data(EVENT_SET_TIME_REQ);
    event_set_alarm_req_data_t *alarm = (event_set_alarm_req_data_t *)event_get_data(EVENT_SET_ALARM_REQ);

    int8_t *item_buf_ptrs[] = {(int8_t*)time, (int8_t*)alarm};

    item_buf_ptrs[item_id > UI_MANAGER_ITEM_ID_DATE_M][items[item_id][UI_MANAGER_ITEM_PROPERTY_OFFSET]] = item_limit_value(item_buf_ptrs[item_id > UI_MANAGER_ITEM_ID_DATE_M][items[item_id][UI_MANAGER_ITEM_PROPERTY_OFFSET]] + val, items[item_id][UI_MANAGER_ITEM_PROPERTY_MIN_VALUE], items[item_id][UI_MANAGER_ITEM_PROPERTY_MAX_VALUE]);
}

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

    switch (ctx.state)
    {
        case UI_MANAGER_STATE_TIME_DATE_ALARM_DISPLAY:

            memcpy(event_get_data(EVENT_SET_TIME_REQ), event_get_data(EVENT_UPDATE_TIME_REQ), sizeof(event_set_time_req_data_t));

            print_alarm_date_set_screen();

            ctx.state = UI_MANAGER_STATE_TIME_DATE_ALARM_SET;
            break;

        case UI_MANAGER_STATE_TIME_DATE_ALARM_SET:

            if (ctx.item_id == UI_MANAGER_ITEM_ID_ESC)
            {
                print_alarm_date_display_screen();
                hal_lcd_set_cursor_mode(false, false);
                ctx.state = UI_MANAGER_STATE_TIME_DATE_ALARM_DISPLAY;
            }
            else if (ctx.item_id == UI_MANAGER_ITEM_ID_OK)
            {
                print_alarm_date_display_screen();
                hal_lcd_set_cursor_mode(false, false);

                event_set(EVENT_SET_TIME_REQ);
                event_set(EVENT_SET_ALARM_REQ);

                ctx.state = UI_MANAGER_STATE_TIME_DATE_ALARM_DISPLAY;
            }
            else if (ctx.item_id == UI_MANAGER_ITEM_ID_SYNC)
            {
                event_set(EVENT_SYNC_TIME_REQ);
            }
            else if (ctx.item_id == UI_MANAGER_ITEM_ID_NEXT)
            {
                ctx.state = UI_MANAGER_STATE_SYNC_SATUS_DISPLAY;
                hal_lcd_clear();
                hal_lcd_set_cursor_mode(false, false);
                hal_lcd_set_cursor(1,0);
                hal_lcd_putc('<');
            }
            else
            {
                hal_lcd_set_cursor_mode(true, true);

                ctx.state = UI_MANAGER_STATE_VALUE_SELECT;
            }
            break;

        case UI_MANAGER_STATE_VALUE_SELECT:

            hal_lcd_set_cursor_mode(true, false);

            ctx.state = UI_MANAGER_STATE_TIME_DATE_ALARM_SET;
            break;

        case UI_MANAGER_STATE_SYNC_SATUS_DISPLAY:

            print_alarm_date_set_screen();
            
            ctx.state = UI_MANAGER_STATE_TIME_DATE_ALARM_SET;
            break;

        default:
            break;
    }
}

void hal_encoder_rotation_cb(uint8_t dir)
{
    switch (ctx.state)
    {
        case UI_MANAGER_STATE_TIME_DATE_ALARM_DISPLAY:
            break;

        case UI_MANAGER_STATE_TIME_DATE_ALARM_SET:

            ctx.item_id += dir;
            ctx.item_id = (ctx.item_id + UI_MANAGER_SETTABLE_ITEM_ID_MAX) % UI_MANAGER_SETTABLE_ITEM_ID_MAX;

            hal_lcd_set_cursor(items[ctx.item_id][UI_MANAGER_ITEM_PROPERTY_POS] / 16, items[ctx.item_id][UI_MANAGER_ITEM_PROPERTY_POS] % 16);
            break;

        case UI_MANAGER_STATE_VALUE_SELECT:

            item_update(ctx.item_id, dir);
            print_time(event_get_data(EVENT_SET_TIME_REQ));
            print_alarm(event_get_data(EVENT_SET_ALARM_REQ));

            hal_lcd_set_cursor(items[ctx.item_id][UI_MANAGER_ITEM_PROPERTY_POS] / 16, items[ctx.item_id][UI_MANAGER_ITEM_PROPERTY_POS] % 16);

            break;
        default:
            break;
    }
}

//------------------------------------------------------------------------------

bool ui_manager_init(void)
{
    print_alarm_date_display_screen();

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

    switch (ctx.state)
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
