//------------------------------------------------------------------------------

/// @file radio_manager.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "radio_manager.h"

#include <stddef.h>

#include <hal.h>

#include <simple_stdio.h>
#include <dcf77_decoder.h>

#include <gpio.h>
#include <mas6181b.h>

//------------------------------------------------------------------------------

/* From Clock Manager */
extern struct ds1307_time unix_time;

/* From UI Manager */
extern void print_time(uint8_t line);

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

//------------------------------------------------------------------------------

bool radio_manager_init(void)
{
    return true;
}

void radio_manager_process(void)
{
    static bool dcf_prev_val = false;

    bool dcf_val = hal_dcf_get_state();

    if (dcf_val != dcf_prev_val)
    {
        hal_led_set(!dcf_val);
        dcf_prev_val = dcf_val;
    }

    static bool prev_triggered_on_bit = false;

    if (prev_triggered_on_bit != last_triggered_on_bit)
    {
        static char buf[8];
        simple_stdio_uint16_to_str(last_time_ms, buf);

        uint8_t pos = last_triggered_on_bit ? 0 : 8;

        hal_lcd_print("       ", 0, pos);
        hal_lcd_print(buf, 0, pos);

        if (decoder_status == DCF77_DECODER_STATUS_FRAME_STARTED)
        {
            hal_lcd_print("S", 0, 15);
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

            hal_set_time(&unix_time);

            print_time(0);

            synced = true;
        }

        prev_triggered_on_bit = last_triggered_on_bit;
    }
}

//------------------------------------------------------------------------------
