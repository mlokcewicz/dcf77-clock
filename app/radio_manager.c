//------------------------------------------------------------------------------

/// @file radio_manager.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "radio_manager.h"

#include <stddef.h>

#include <hal.h>

#include <dcf77_decoder.h>

#include <gpio.h>
#include <timer.h>

#include <mas6181b.h>
#include <ds1307.h>

//------------------------------------------------------------------------------

extern struct ds1307_obj rtc_obj;
extern struct ds1307_time unix_time;
extern bool new_sec;

bool synced = false;

static enum dcf77_decoder_status decoder_status = DCF77_DECODER_STATUS_WAITING;
static enum dcf77_decoder_status prev_decoder_status = DCF77_DECODER_STATUS_WAITING;
static bool last_triggered_on_bit;
static uint16_t last_time_ms;

void hal_dcf_cb(uint16_t ms, bool rising_edge)
{
    if (synced)
        return;

    last_triggered_on_bit = rising_edge;
    last_time_ms = ms;

    prev_decoder_status = decoder_status;

    decoder_status = dcf77_decode(ms, rising_edge); 
};

static char buf[16]; 
static void print_time(uint8_t line)
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

static void uint16_to_str(uint16_t value, char *buffer) 
{
    uint8_t i = 0;
    do 
    {
        buffer[i++] = (value % 10) + '0';
        value /= 10;
    } while (value > 0);
    
    buffer[i] = '\0';

    /* Reverse the string */
    for (uint8_t j = 0; j < i / 2; j++) 
    {
        char temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }
}

//------------------------------------------------------------------------------

bool radio_manager_init(void)
{
    return true;
}

void radio_manager_process(void)
{
    static bool dcf_prev_val = false;

    bool dcf_val = gpio_get(GPIO_PORT_B, GPIO_PIN_0);

    if (dcf_val != dcf_prev_val)
    {
        gpio_set(GPIO_PORT_D, GPIO_PIN_6, !dcf_val);
        dcf_prev_val = dcf_val;
    }

    if (new_sec)
    {
        ds1307_get_time(&rtc_obj, &unix_time);
        print_time(1);
        new_sec = false;
    }

    static bool prev_triggered_on_bit = false;

    if (prev_triggered_on_bit != last_triggered_on_bit)
    {
        uint16_to_str(last_time_ms, buf);

        uint8_t pos = last_triggered_on_bit ? 0 : 8;

        hal_lcd_print("       ", 0, pos);
        hal_lcd_print(buf, 0, pos);

        if (decoder_status == DCF77_DECODER_STATUS_FRAME_STARTED)
        {
            hal_lcd_print("F", 0, 15);
        }
        else if (decoder_status == DCF77_DECODER_STATUS_ERROR)
        {
            hal_lcd_print("E", 0, 15);
        }
        if (decoder_status == DCF77_DECODER_STATUS_FRAME_STARTED && prev_decoder_status == DCF77_DECODER_STATUS_SYNCED && !synced)
        {
            struct dcf77_frame *dcf_frame = dcf77_get_frame();

            unix_time.clock_halt = 0;
            unix_time.hour_mode = 0;
            unix_time.hours_tens = dcf_frame->hours_tens;
            unix_time.hours_units = dcf_frame->hours_units;
            unix_time.minutes_tens = dcf_frame->minutes_tens;
            unix_time.minutes_units = dcf_frame->minutes_units;
            unix_time.date_tens = dcf_frame->month_day_tens;
            unix_time.date_units = dcf_frame->month_day_units;
            unix_time.month_tens = dcf_frame->months_tens;
            unix_time.month_units = dcf_frame->months_units;
            unix_time.year_tens = dcf_frame->years_tens;
            unix_time.year_units = dcf_frame->years_units;
            unix_time.seconds_tens = 0;
            unix_time.seconds_units = 0;

            ds1307_set_time(&rtc_obj, &unix_time);

            print_time(0);

            synced = true;
        }

        prev_triggered_on_bit = last_triggered_on_bit;
    }
}

//------------------------------------------------------------------------------
