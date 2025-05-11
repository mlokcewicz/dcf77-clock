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

/* HAL callbacks */

void hal_button_pressed_cb(void)
{
    hal_lcd_clear();

    memset(event_get_data(EVENT_SET_TIME_REQ), 0x00, sizeof(struct ds1307_time));

    hal_set_time(event_get_data(EVENT_SET_TIME_REQ));

    event_set(EVENT_SYNC_TIME_REQ);
}

void hal_encoder_rotation_cb(bool right)
{
    (void)right;
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
    buf[i++] = (' ');
    buf[i++] = (unix_time->date_tens + '0');
    buf[i++] = (unix_time->date_units + '0');
    buf[i++] = ('.');
    buf[i++] = (unix_time->month_tens + '0');
    buf[i++] = (unix_time->month_units + '0');
    buf[i++] = (' ');
    buf[i++] = (unix_time->seconds_tens + '0');
    buf[i++] = (unix_time->seconds_units + '0');
    buf[i++] = 0;
    hal_lcd_print(buf, line, 0);
}

//------------------------------------------------------------------------------

bool ui_manager_init(void)
{
    // hal_audio_set_pattern(alarm_beep, sizeof(alarm_beep), 800);

    return true;
}

void ui_manager_process(void)
{
    hal_audio_process();

    if (event_get() & EVENT_SYNC_TIME_STATUS)
    {
        event_sync_time_status_data_t *sync_time_status_data = event_get_data(EVENT_SYNC_TIME_STATUS);

        static char buf2[8];
        simple_stdio_uint16_to_str(sync_time_status_data->time_ms, buf2);
    
        uint8_t pos = sync_time_status_data->rising_edge ? 0 : 8;
    
        hal_lcd_print("       ", 0, pos);
        hal_lcd_print(buf2, 0, pos);
    
        if (sync_time_status_data->frame_started)
        {
            hal_lcd_print("S", 0, 15);
        }
        else if (sync_time_status_data->error)
        {
            hal_lcd_print("E", 0, 15);
        }

        hal_led_set(!sync_time_status_data->dcf_output);

        event_clear(EVENT_SYNC_TIME_STATUS);
    }
    if (event_get() & EVENT_UPDATE_TIME_REQ)
    {
        print_time(1, event_get_data(EVENT_UPDATE_TIME_REQ));
        event_clear(EVENT_UPDATE_TIME_REQ);
    }
}

//------------------------------------------------------------------------------
