//------------------------------------------------------------------------------

/// @file ui_manager.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "ui_manager.h"

#include <stddef.h>
#include <string.h>

#include <event.h>

#include <hal.h>

//------------------------------------------------------------------------------

#define UI_MANAGER_CHAR_BUF_LEN                         (15)

//------------------------------------------------------------------------------

#define UI_ALARM_BPM                                    (1000UL)
#define UI_ALARM_INTERVAL_MS                            (1000UL)
    
#define UI_ITEM_POS_TIME_H                              (3)
#define UI_ITEM_POS_TIME_M                              (6)
#define UI_ITEM_POS_DATE_D                              (9)
#define UI_ITEM_POS_DATE_M                              (12)
#define UI_ITEM_POS_DATE_Y                              (15)
#define UI_ITEM_POS_ALARM_EN                            (16)
#define UI_ITEM_POS_ALARM_H                             (19)
#define UI_ITEM_POS_ALARM_M                             (22)
#define UI_ITEM_POS_TIMEZONE                            (26)
#define UI_ITEM_POS_SYNC                                (28)
#define UI_ITEM_POS_ESC                                 (30)
#define UI_ITEM_POS_OK                                  (31)
    
#define UI_ITEM_TIME_H_MIN                              (0)
#define UI_ITEM_TIME_H_MAX                              (23)
    
#define UI_ITEM_TIME_M_MIN                              (0)
#define UI_ITEM_TIME_M_MAX                              (59)
    
#define UI_ITEM_DATE_D_MIN                              (1)
#define UI_ITEM_DATE_D_MAX                              (31)
    
#define UI_ITEM_DATE_M_MIN                              (1)
#define UI_ITEM_DATE_M_MAX                              (12)
    
#define UI_ITEM_DATE_Y_MIN                              (0)
#define UI_ITEM_DATE_Y_MAX                              (99)
    
#define UI_ITEM_ALARM_EN_MIN                            (0)
#define UI_ITEM_ALARM_EN_MAX                            (1)
    
#define UI_ITEM_ALARM_H_MIN                             (0)
#define UI_ITEM_ALARM_H_MAX                             (23)
    
#define UI_ITEM_ALARM_M_MIN                             (0)
#define UI_ITEM_ALARM_M_MAX                             (59)
    
#define UI_ITEM_TIMEZONE_MIN                            (-12)
#define UI_ITEM_TIMEZONE_MAX                            (14)
    
#define UI_ITEM_DUMMY_MIN                               (0)
#define UI_ITEM_DUMMY_MAX                               (0)

#define UI_ITEM_OFFSET_TIME_H                           offsetof(event_set_time_req_data_t, hours)
#define UI_ITEM_OFFSET_TIME_M                           offsetof(event_set_time_req_data_t, minutes)
#define UI_ITEM_OFFSET_DATE_D                           offsetof(event_set_time_req_data_t, date)
#define UI_ITEM_OFFSET_DATE_M                           offsetof(event_set_time_req_data_t, month)
#define UI_ITEM_OFFSET_DATE_Y                           offsetof(event_set_time_req_data_t, year)
#define UI_ITEM_OFFSET_ALARM_EN                         offsetof(event_set_alarm_req_data_t, is_enabled)
#define UI_ITEM_OFFSET_ALARM_H                          offsetof(event_set_alarm_req_data_t, hours)
#define UI_ITEM_OFFSET_ALARM_M                          offsetof(event_set_alarm_req_data_t, minutes)
#define UI_ITEM_OFFSET_TIMEZONE                         0
#define UI_ITEM_OFFSET_SYNC                             0
#define UI_ITEM_OFFSET_ESC                              0
#define UI_ITEM_OFFSET_OK                               0

#define UI_ITEM_POS_CLOCK_ICON_ROW                      (0)
#define UI_ITEM_POS_CLOCK_ICON_COL                      (0)
    
#define UI_ITEM_POS_ANTENNA_ICON_ROW                    (1)
#define UI_ITEM_POS_ANTENNA_ICON_COL                    (12)
    
#define UI_ITEM_POS_TIME_STRING_ROW                     (0)
#define UI_ITEM_POS_TIME_STRING_COL                     (2)
    
#define UI_ITEM_POS_ALARM_STRING_ROW                    (1)
#define UI_ITEM_POS_ALARM_STRING_COL                    (0)
    
#define UI_ITEM_POS_TIMEZONE_STRING_ROW                 (1)
#define UI_ITEM_POS_TIMEZONE_STRING_COL                 (8)
    
#define UI_ITEM_POS_SYNC_STATUS_IS_SYNCED_ROW           (1)
#define UI_ITEM_POS_SYNC_STATUS_IS_SYNCED_COL           (14)
    
#define UI_ITEM_POS_SYNC_STATUS_TIME_ROW                (0)
#define UI_ITEM_POS_SYNC_STATUS_BIT_TIME_COL            (3)
#define UI_ITEM_POS_SYNC_STATUS_BREAK_TIME_COL          (8)
    
#define UI_ITEM_POS_SYNC_STATUS_BIT_NUMBER_ROW          (1)
#define UI_ITEM_POS_SYNC_STATUS_BIT_NUMBER_COL          (3)
    
#define UI_ITEM_POS_SYNC_STATUS_STATE_ROW               (1)
#define UI_ITEM_POS_SYNC_STATUS_STATER_COL              (9)
    
#define UI_ITEM_POS_SYNC_STATUS_TIME_STRING_ROW         (0)
#define UI_ITEM_POS_SYNC_STATUS_TIME_STRING_COL         (0)

#define UI_ITEM_POS_SYNC_STATUS_BIT_STATE_STRING_ROW    (1)
#define UI_ITEM_POS_SYNC_STATUS_BIT_STATE_STRING_COL    (0)

//------------------------------------------------------------------------------

enum ui_manager_icon_id 
{
    UI_MANAGER_CHAR_ID_CLOCK,
    UI_MANAGER_CHAR_ID_ANTENNA,  
    UI_MANAGER_CHAR_ID_ALARM_SET,
    UI_MANAGER_CHAR_ID_ALARM_UNSET,
    UI_MANAGER_CHAR_ID_ESCAPE,
    UI_MANAGER_CHAR_ID_OK,
};

enum ui_manager_item_id
{
    UI_MANAGER_ITEM_ID_TIME_H,
    UI_MANAGER_ITEM_ID_TIME_M,
    UI_MANAGER_ITEM_ID_DATE_D,
    UI_MANAGER_ITEM_ID_DATE_M,
    UI_MANAGER_ITEM_ID_DATE_Y,
    UI_MANAGER_ITEM_ID_ALARM_EN,
    UI_MANAGER_ITEM_ID_ALARM_H,
    UI_MANAGER_ITEM_ID_ALARM_M,
    UI_MANAGER_ITEM_ID_TIMEZONE,
    UI_MANAGER_ITEM_ID_SYNC,
    UI_MANAGER_ITEM_ID_ESC,
    UI_MANAGER_ITEM_ID_OK,

    UI_MANAGER_ITEM_ID_MAX,
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
    UI_MANAGER_STATE_ALARM,
};

//------------------------------------------------------------------------------

struct ui_manager_ctx 
{
    enum ui_manager_state state;
    enum ui_manager_item_id item_id;
    bool time_was_changed;
    char buf[UI_MANAGER_CHAR_BUF_LEN];
};

static struct ui_manager_ctx ctx; 

static const uint8_t items[UI_MANAGER_ITEM_ID_MAX][UI_MANAGER_ITEM_PROPERTY_MAX] = 
{
    [UI_MANAGER_ITEM_ID_TIME_H]       = {UI_ITEM_POS_TIME_H,      UI_ITEM_TIME_H_MIN,        UI_ITEM_TIME_H_MAX,        UI_ITEM_OFFSET_TIME_H},
    [UI_MANAGER_ITEM_ID_TIME_M]       = {UI_ITEM_POS_TIME_M,      UI_ITEM_TIME_M_MIN,        UI_ITEM_TIME_M_MAX,        UI_ITEM_OFFSET_TIME_M},
    [UI_MANAGER_ITEM_ID_DATE_D]       = {UI_ITEM_POS_DATE_D,      UI_ITEM_DATE_D_MIN,        UI_ITEM_DATE_D_MAX,        UI_ITEM_OFFSET_DATE_D},
    [UI_MANAGER_ITEM_ID_DATE_M]       = {UI_ITEM_POS_DATE_M,      UI_ITEM_DATE_M_MIN,        UI_ITEM_DATE_M_MAX,        UI_ITEM_OFFSET_DATE_M},
    [UI_MANAGER_ITEM_ID_DATE_Y]       = {UI_ITEM_POS_DATE_Y,      UI_ITEM_DATE_Y_MIN,        UI_ITEM_DATE_Y_MAX,        UI_ITEM_OFFSET_DATE_Y},
    [UI_MANAGER_ITEM_ID_ALARM_EN] =     {UI_ITEM_POS_ALARM_EN,    UI_ITEM_ALARM_EN_MIN,      UI_ITEM_ALARM_EN_MAX,      UI_ITEM_OFFSET_ALARM_EN},
    [UI_MANAGER_ITEM_ID_ALARM_H]      = {UI_ITEM_POS_ALARM_H,     UI_ITEM_ALARM_H_MIN,       UI_ITEM_ALARM_H_MAX,       UI_ITEM_OFFSET_ALARM_H},
    [UI_MANAGER_ITEM_ID_ALARM_M]      = {UI_ITEM_POS_ALARM_M,     UI_ITEM_ALARM_M_MIN,       UI_ITEM_ALARM_M_MAX,       UI_ITEM_OFFSET_ALARM_M},
    [UI_MANAGER_ITEM_ID_TIMEZONE]     = {UI_ITEM_POS_TIMEZONE,    UI_ITEM_TIMEZONE_MIN,      UI_ITEM_TIMEZONE_MAX,      UI_ITEM_OFFSET_TIMEZONE},
    [UI_MANAGER_ITEM_ID_SYNC]         = {UI_ITEM_POS_SYNC,        UI_ITEM_DUMMY_MIN,         UI_ITEM_DUMMY_MAX,         UI_ITEM_OFFSET_SYNC},
    [UI_MANAGER_ITEM_ID_ESC]          = {UI_ITEM_POS_ESC,         UI_ITEM_DUMMY_MIN,         UI_ITEM_DUMMY_MAX,         UI_ITEM_OFFSET_ESC},
    [UI_MANAGER_ITEM_ID_OK]           = {UI_ITEM_POS_OK,          UI_ITEM_DUMMY_MIN,         UI_ITEM_DUMMY_MAX,         UI_ITEM_OFFSET_OK},
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
	{BUZZER_TONE_STOP, UI_ALARM_INTERVAL_MS * UI_ALARM_BPM},
}; 

//------------------------------------------------------------------------------

static void item_id_update(int8_t diff)
{
    ctx.item_id += diff;
    ctx.item_id = (ctx.item_id + UI_MANAGER_ITEM_ID_MAX) % UI_MANAGER_ITEM_ID_MAX;
}

static int8_t item_limit_value(int8_t val, int8_t min, int8_t max)
{
    if (val < min) 
        return max;
    if (val > max) 
        return min;
    
    return val;
}

static void item_update(enum ui_manager_item_id item_id, int8_t val)
{
    event_set_time_req_data_t *time = (event_set_time_req_data_t *)event_get_data(EVENT_SET_TIME_REQ);
    event_set_alarm_req_data_t *alarm = (event_set_alarm_req_data_t *)event_get_data(EVENT_SET_ALARM_REQ);
    int8_t *timezone = event_get_data(EVENT_SET_TIMEZONE_REQ);

    uint8_t *item_buf_ptrs[] = {(uint8_t*)time, (uint8_t*)alarm, (uint8_t*)timezone};

    uint8_t buf_idx = (item_id >= UI_MANAGER_ITEM_ID_ALARM_EN) + (item_id == UI_MANAGER_ITEM_ID_TIMEZONE);

    item_buf_ptrs[buf_idx][items[item_id][UI_MANAGER_ITEM_PROPERTY_OFFSET]] = item_limit_value((item_buf_ptrs[buf_idx][items[item_id][UI_MANAGER_ITEM_PROPERTY_OFFSET]]) + val, items[item_id][UI_MANAGER_ITEM_PROPERTY_MIN_VALUE], items[item_id][UI_MANAGER_ITEM_PROPERTY_MAX_VALUE]);
}

//------------------------------------------------------------------------------

static void ui_alarm_play(bool play)
{
    if (!play) 
        hal_audio_stop(); 

    hal_audio_set_pattern(play ? alarm_beep : NULL, sizeof(alarm_beep), UI_ALARM_BPM);
    hal_led_set(play);
}

static void ui_print_cursor(void)
{
    hal_lcd_set_cursor(items[ctx.item_id][UI_MANAGER_ITEM_PROPERTY_POS] / 16, items[ctx.item_id][UI_MANAGER_ITEM_PROPERTY_POS] % 16);
}

static void ui_print_static_icons(void)
{
    hal_lcd_set_cursor(UI_ITEM_POS_CLOCK_ICON_ROW, UI_ITEM_POS_CLOCK_ICON_COL);
    hal_lcd_putc(UI_MANAGER_CHAR_ID_CLOCK);

    hal_lcd_set_cursor(UI_ITEM_POS_ANTENNA_ICON_ROW, UI_ITEM_POS_ANTENNA_ICON_COL);
    hal_lcd_putc(UI_MANAGER_CHAR_ID_ANTENNA);
}

static void ui_print_time(struct ds1307_time *unix_time)
{
    uint8_t i = 0;

    ctx.buf[i++] = (unix_time->hours / 10 + '0');
    ctx.buf[i++] = (unix_time->hours % 10 + '0');
    ctx.buf[i++] = ((unix_time->seconds & 1) ? ':' : ' ');
    ctx.buf[i++] = (unix_time->minutes / 10 + '0');
    ctx.buf[i++] = (unix_time->minutes % 10 + '0');
    ctx.buf[i++] = (' ');
    ctx.buf[i++] = (unix_time->date / 10 + '0');
    ctx.buf[i++] = (unix_time->date % 10 + '0');
    ctx.buf[i++] = ('/');
    ctx.buf[i++] = (unix_time->month / 10 + '0');
    ctx.buf[i++] = (unix_time->month % 10 + '0');
    ctx.buf[i++] = ('/');
    ctx.buf[i++] = (unix_time->year / 10 + '0');
    ctx.buf[i++] = (unix_time->year % 10 + '0');
    ctx.buf[i++] = 0;

    hal_lcd_print(ctx.buf, UI_ITEM_POS_TIME_STRING_ROW, UI_ITEM_POS_TIME_STRING_COL);
}

static void ui_print_alarm(struct hal_timestamp *alarm)
{
    uint8_t i = 0;

    ctx.buf[i++] = (alarm->is_enabled ? UI_MANAGER_CHAR_ID_ALARM_SET : UI_MANAGER_CHAR_ID_ALARM_UNSET);
    ctx.buf[i++] = (' ');
    ctx.buf[i++] = (alarm->hours / 10 + '0');
    ctx.buf[i++] = (alarm->hours % 10 + '0');
    ctx.buf[i++] = (':');
    ctx.buf[i++] = (alarm->minutes / 10 + '0');
    ctx.buf[i++] = (alarm->minutes % 10 + '0');
    ctx.buf[i++] = 0;

    hal_lcd_print(ctx.buf, UI_ITEM_POS_ALARM_STRING_ROW, UI_ITEM_POS_ALARM_STRING_COL);
}

void ui_print_timezone(const int8_t *tz)
{
    int8_t val = *tz;

    hal_lcd_set_cursor(UI_ITEM_POS_TIMEZONE_STRING_ROW, UI_ITEM_POS_TIMEZONE_STRING_COL);
    
    hal_lcd_putc(val >= 0 ? '+' : '-');

    if (val < 0) 
        val = -val;
    
    hal_lcd_putc((val / 10) + '0');
    hal_lcd_putc((val % 10) + '0');
}

static void ui_print_sync_status(event_sync_time_status_data_t *sync_time_status_data, bool full)
{
    if (!full)
    {
        hal_lcd_print(sync_time_status_data->synced ? "OK" : "--", UI_ITEM_POS_SYNC_STATUS_IS_SYNCED_ROW, UI_ITEM_POS_SYNC_STATUS_IS_SYNCED_COL);
        return;
    }

    uint8_t i = 0;
    ctx.buf[i++] = (sync_time_status_data->time_ms / 1000 + '0');
    ctx.buf[i++] = ((sync_time_status_data->time_ms / 100) % 10 + '0');
    ctx.buf[i++] = ((sync_time_status_data->time_ms / 10) % 10 + '0');
    ctx.buf[i++] = (sync_time_status_data->time_ms % 10 + '0');
    ctx.buf[i++] = 0;

    hal_lcd_print(ctx.buf, UI_ITEM_POS_SYNC_STATUS_TIME_ROW, sync_time_status_data->triggred_on_bit ? UI_ITEM_POS_SYNC_STATUS_BIT_TIME_COL : UI_ITEM_POS_SYNC_STATUS_BREAK_TIME_COL);

    i = 0;
    ctx.buf[i++] = (sync_time_status_data->bit_number / 10 + '0');
    ctx.buf[i++] = (sync_time_status_data->bit_number % 10 + '0');
    ctx.buf[i++] = 0;

    hal_lcd_print(ctx.buf, UI_ITEM_POS_SYNC_STATUS_BIT_NUMBER_ROW, UI_ITEM_POS_SYNC_STATUS_BIT_NUMBER_COL);

    if (sync_time_status_data->frame_started)
        hal_lcd_print("STARTED", UI_ITEM_POS_SYNC_STATUS_STATE_ROW, UI_ITEM_POS_SYNC_STATUS_STATER_COL);
    else if (sync_time_status_data->error)
        hal_lcd_print("ERROR  ", UI_ITEM_POS_SYNC_STATUS_STATE_ROW, UI_ITEM_POS_SYNC_STATUS_STATER_COL);

    hal_led_set(!sync_time_status_data->dcf_output);
}

static void ui_print_alarm_date_screen(bool set)
{
    hal_lcd_clear();
    
    ui_print_static_icons();
    ui_print_time(event_get_data(set ? EVENT_SET_TIME_REQ : EVENT_UPDATE_TIME_REQ));
    ui_print_alarm(event_get_data(EVENT_SET_ALARM_REQ));
    ui_print_timezone(event_get_data(EVENT_SET_TIMEZONE_REQ));

    if (set)
    {
        hal_lcd_set_cursor(1, 14);
        hal_lcd_putc(UI_MANAGER_CHAR_ID_ESCAPE);
        hal_lcd_putc(UI_MANAGER_CHAR_ID_OK);
    }

    hal_lcd_set_cursor_mode(set, false);
    
    ui_print_cursor();
}

static void ui_print_time_sync_status_screen(void)
{
    hal_lcd_clear();
    hal_lcd_set_cursor_mode(false, false);

    hal_lcd_print("T:     /     ms", UI_ITEM_POS_SYNC_STATUS_TIME_STRING_ROW, UI_ITEM_POS_SYNC_STATUS_TIME_STRING_COL);
    hal_lcd_print("B:    S:", UI_ITEM_POS_SYNC_STATUS_BIT_STATE_STRING_ROW, UI_ITEM_POS_SYNC_STATUS_BIT_STATE_STRING_COL);
}

static void ui_print_value_select_screen(void)
{
    hal_lcd_set_cursor_mode(true, true);
}

//------------------------------------------------------------------------------
/* HAL callbacks and structures */

const uint8_t hal_user_defined_char_tab[6][8] = 
{
    [UI_MANAGER_CHAR_ID_CLOCK] = {0x0E, 0x0E, 0x15, 0x15, 0x13, 0x0E, 0x0E, 0x0E},
    [UI_MANAGER_CHAR_ID_ANTENNA] = {0x00, 0x15, 0x0A, 0x04, 0x04, 0x04, 0x04, 0x00},
    [UI_MANAGER_CHAR_ID_ALARM_SET] = {0x04, 0x0E, 0x0E, 0x0E, 0x0E, 0x1F, 0x1F, 0x04},
    [UI_MANAGER_CHAR_ID_ALARM_UNSET] = {0x04, 0x0E,0x0A, 0x0A, 0x0A, 0x11, 0x1F, 0x04},
    [UI_MANAGER_CHAR_ID_OK] = {0x00, 0x01, 0x03, 0x16, 0x1C, 0x08, 0x00, 0x00},
    [UI_MANAGER_CHAR_ID_ESCAPE] = {0x00, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x00, 0x00}
};

void hal_button_pressed_cb(void)
{
    switch (ctx.state)
    {
    case UI_MANAGER_STATE_TIME_DATE_ALARM_DISPLAY:
    case UI_MANAGER_STATE_SYNC_SATUS_DISPLAY:

        memcpy(event_get_data(EVENT_SET_TIME_REQ), event_get_data(EVENT_UPDATE_TIME_REQ), sizeof(event_set_time_req_data_t));

        ui_print_alarm_date_screen(true);

        ctx.state = UI_MANAGER_STATE_TIME_DATE_ALARM_SET;

        break;

    case UI_MANAGER_STATE_TIME_DATE_ALARM_SET:

        if (ctx.item_id == UI_MANAGER_ITEM_ID_ESC)
        {
            /* Restore previous alarm and timezone value - could be replaced by copying using Clock Manager buffer pointer getters */
            hal_get_alarm(event_get_data(EVENT_SET_ALARM_REQ));
            hal_get_timezone(event_get_data(EVENT_SET_TIMEZONE_REQ));

            ui_print_alarm_date_screen(false);

            ctx.state = UI_MANAGER_STATE_TIME_DATE_ALARM_DISPLAY;
        }
        else if (ctx.item_id == UI_MANAGER_ITEM_ID_OK)
        {
            ui_print_alarm_date_screen(false);

            if (ctx.time_was_changed)
            {
                event_set(EVENT_SET_TIME_REQ);
                ctx.time_was_changed = false;
            }

            event_set(EVENT_SET_ALARM_REQ);
            event_set(EVENT_SET_TIMEZONE_REQ);

            ctx.state = UI_MANAGER_STATE_TIME_DATE_ALARM_DISPLAY;
        }
        else if (ctx.item_id == UI_MANAGER_ITEM_ID_SYNC)
        {
            event_set(EVENT_SYNC_TIME_REQ);
        }
        else
        {
            if (ctx.item_id < UI_MANAGER_ITEM_ID_ALARM_EN)
                ctx.time_was_changed = true;

            ui_print_value_select_screen();

            ctx.state = UI_MANAGER_STATE_VALUE_SELECT;
        }

        break;

    case UI_MANAGER_STATE_VALUE_SELECT:

        ui_print_alarm_date_screen(true);

        ctx.state = UI_MANAGER_STATE_TIME_DATE_ALARM_SET;

        break;
    
    case UI_MANAGER_STATE_ALARM:

        ui_alarm_play(false);

        ctx.state = UI_MANAGER_STATE_TIME_DATE_ALARM_DISPLAY;

        break;

    default:

        break;
    }
}

void hal_encoder_rotation_cb(int8_t dir)
{
    switch (ctx.state)
    {
    case UI_MANAGER_STATE_TIME_DATE_ALARM_DISPLAY:

        ui_print_time_sync_status_screen();

        ctx.state = UI_MANAGER_STATE_SYNC_SATUS_DISPLAY;

        break;

    case UI_MANAGER_STATE_TIME_DATE_ALARM_SET:

        item_id_update(dir);

        ui_print_cursor();

        break;

    case UI_MANAGER_STATE_VALUE_SELECT:

        item_update(ctx.item_id, dir);

        ui_print_time(event_get_data(EVENT_SET_TIME_REQ));
        ui_print_alarm(event_get_data(EVENT_SET_ALARM_REQ));
        ui_print_timezone(event_get_data(EVENT_SET_TIMEZONE_REQ));

        ui_print_cursor();

        break;

    case UI_MANAGER_STATE_SYNC_SATUS_DISPLAY:

        ui_print_alarm_date_screen(false);

        ctx.state = UI_MANAGER_STATE_TIME_DATE_ALARM_DISPLAY;

        break;

    default:

        break;
    }
}

//------------------------------------------------------------------------------

bool ui_manager_init(void)
{
    /* It could be replaced by using event / getter from Clock Manager to remove HAL dependency */
    hal_get_alarm(event_get_data(EVENT_SET_ALARM_REQ));
    hal_get_timezone(event_get_data(EVENT_SET_TIMEZONE_REQ));

    ui_print_alarm_date_screen(false);

    return true;
}

void ui_manager_process(void)
{
    if (event_get() & EVENT_ALARM_REQ)
    {
        ui_alarm_play(true);

        ctx.state = UI_MANAGER_STATE_ALARM;

        event_clear(EVENT_ALARM_REQ);
    }

    switch (ctx.state)
    {
    case UI_MANAGER_STATE_TIME_DATE_ALARM_DISPLAY:
    case UI_MANAGER_STATE_ALARM:

        if (event_get() & EVENT_UPDATE_TIME_REQ)
        {
            ui_print_time(event_get_data(EVENT_UPDATE_TIME_REQ));
            ui_print_sync_status(event_get_data(EVENT_SYNC_TIME_STATUS), false);

            event_clear(EVENT_UPDATE_TIME_REQ);
        }

        break;

    case UI_MANAGER_STATE_SYNC_SATUS_DISPLAY:

        if (event_get() & EVENT_SYNC_TIME_STATUS)
        {
            ui_print_sync_status(event_get_data(EVENT_SYNC_TIME_STATUS), true);

            event_clear(EVENT_SYNC_TIME_STATUS);
        }

        break;

    default: 
        
        break;
    }
}

//------------------------------------------------------------------------------
