//------------------------------------------------------------------------------

/// @file ui_manager.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "ui_manager.h"

#include <string.h>

#include <hal.h>

//------------------------------------------------------------------------------

extern struct ds1307_time unix_time;
extern bool synced;

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

void hal_button_pressed_cb(void)
{
    hal_lcd_clear();

    memset(&unix_time, 0x00, sizeof(unix_time));

    hal_set_time(&unix_time);

    synced = false;
}

void hal_encoder_rotation_cb(bool right)
{
    (void)right;
}

static char buf[16]; 
void print_time(uint8_t line)
{
    uint8_t i = 0;
    buf[i++] = (unix_time.hours_tens + '0');
    buf[i++] = (unix_time.hours_units + '0');
    buf[i++] = (':');
    buf[i++] = (unix_time.minutes_tens + '0');
    buf[i++] = (unix_time.minutes_units + '0');
    buf[i++] = (' ');
    buf[i++] = (unix_time.date_tens + '0');
    buf[i++] = (unix_time.date_units + '0');
    buf[i++] = ('.');
    buf[i++] = (unix_time.month_tens + '0');
    buf[i++] = (unix_time.month_units + '0');
    buf[i++] = (' ');
    buf[i++] = (unix_time.seconds_tens + '0');
    buf[i++] = (unix_time.seconds_units + '0');
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
}

//------------------------------------------------------------------------------
