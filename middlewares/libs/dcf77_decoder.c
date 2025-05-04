//------------------------------------------------------------------------------

/// @file dcf77_decoder.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "dcf77_decoder.h"

#include <stddef.h>

#include <../../hal/ext_drivers/ds1307.h> // out

extern struct ds1307_obj rtc_obj;

//------------------------------------------------------------------------------

enum dcf77_bit_val
{
    DCF77_BIT_VAL_0,
    DCF77_BIT_VAL_1,
    DCF77_BIT_VAL_NONE,
    DCF77_BIT_VAL_ERROR,
};

//------------------------------------------------------------------------------

struct dcf77_ctx 
{
    bool frame_started;
    uint8_t frame[8];  
    uint8_t bit_cnt;
    struct dcf77_status status;
};

static struct dcf77_ctx ctx;

//------------------------------------------------------------------------------

static bool is_in_range(uint16_t ms, uint16_t min, uint16_t max)
{
    return (ms >= min) && (ms < max);
}

static enum dcf77_bit_val get_bit_val(uint16_t ms)
{
    if (is_in_range(ms, 40, 130))
        return DCF77_BIT_VAL_0;
    if (is_in_range(ms, 140, 250))
        return DCF77_BIT_VAL_1;
    if (is_in_range(ms, 1500, 2200))
        return DCF77_BIT_VAL_NONE;

    return DCF77_BIT_VAL_ERROR;
}

//------------------------------------------------------------------------------

void dcf77_decode(uint16_t ms, bool triggered_on_bit)
{
    ctx.status.last_triggered_on_bit = triggered_on_bit;
    ctx.status.last_time_ms = ms;

    if (!ctx.frame_started)
    {
        if (get_bit_val(ms) == DCF77_BIT_VAL_NONE)
        {
            ctx.frame_started = true;
            ctx.status.last_frame_started = true;
        }

        return;
    }

    enum dcf77_bit_val val = 0;

    if (triggered_on_bit) // Bit transmission
    {
        val = get_bit_val(ms);

        if (val == DCF77_BIT_VAL_ERROR || val == DCF77_BIT_VAL_NONE)
        {
            ctx.status.last_error = true;
            ctx.frame_started = 0;
            ctx.bit_cnt = 0;
            return;
        }

        uint8_t byte_idx = ctx.bit_cnt / 8;
        uint8_t bit_idx = ctx.bit_cnt % 8;
    
        ctx.frame[byte_idx] |= (val << bit_idx);

        ctx.bit_cnt++;

        if (ctx.bit_cnt >= 59)
        {
            struct dcf77_frame *frame_ptr = (struct dcf77_frame*)ctx.frame;
            
            // static struct ds1307_time unix_time_dcf;

            // unix_time_dcf.clock_halt = 0;
            // unix_time_dcf.hour_mode = 0;
            // unix_time_dcf.hours_tens = frame_ptr->hours_tens;
            // unix_time_dcf.hours_units = frame_ptr->hours_units;
            // unix_time_dcf.minutes_tens = frame_ptr->minutes_tens;
            // unix_time_dcf.minutes_units = frame_ptr->minutes_units;
            // unix_time_dcf.date_tens = frame_ptr->month_day_tens;
            // unix_time_dcf.date_units = frame_ptr->month_days_units;
            // unix_time_dcf.month_tens = frame_ptr->months_tens;
            // unix_time_dcf.month_units = frame_ptr->months_units;
            // unix_time_dcf.year_tens = frame_ptr->years_tens;
            // unix_time_dcf.year_units = frame_ptr->years_units;
            // unix_time_dcf.seconds_tens = 0;
            // unix_time_dcf.seconds_units = 0;

            // ds1307_set_time(&rtc_obj, &unix_time_dcf);

            ctx.frame_started = false;
            ctx.bit_cnt = 0;

            ctx.status.last_frame = frame_ptr;
            ctx.status.last_synced = true;
    
            return;
        }
    }
    else // Break transmission
    {
        // TODO: Add breaks validation
        // TODO: take into account ignoring short pulses - it affects break time
    }
}

bool dcf77_ignore_short_pulse(uint16_t ms, bool triggered_on_bit)
{
    return triggered_on_bit && (ms < 35);
}

struct dcf77_status *dcf77_get_status(void)
{
    return &ctx.status;
}

//------------------------------------------------------------------------------
