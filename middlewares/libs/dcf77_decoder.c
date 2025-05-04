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

// #define PRESC 256 // out

// #define MS_TO_TICKS(ms, presc) (ms * (F_CPU / 1000UL) / presc) // out

// #define TICKS_TO_MS(ticks, presc) ((uint32_t)ticks * presc * 1000UL / F_CPU) // out




static bool is_in_range(uint16_t ms, uint16_t min, uint16_t max)
{
    return (ms >= min) && (ms < max);
}

static enum dcf77_bit_val get_bit_val(uint16_t ms)
{
    // if (is_in_range(ticks, MS_TO_TICKS(40, PRESC), MS_TO_TICKS(130, PRESC)))
    //     return DCF77_BIT_VAL_0;
    // if (is_in_range(ticks, MS_TO_TICKS(140, PRESC), MS_TO_TICKS(250, PRESC)))
    //     return DCF77_BIT_VAL_1;
    // if (is_in_range(ticks, MS_TO_TICKS(1500, PRESC), MS_TO_TICKS(2200, PRESC)))
    //     return DCF77_BIT_VAL_NONE;

    if (is_in_range(ms, 40, 130))
        return DCF77_BIT_VAL_0;
    if (is_in_range(ms, 140, 250))
        return DCF77_BIT_VAL_1;
    if (is_in_range(ms, 1500, 2200))
        return DCF77_BIT_VAL_NONE;

    return DCF77_BIT_VAL_ERROR;
}

bool last_edge_rising = false;
uint16_t last_ticks = 0;
bool last_error = false;
bool last_frame_started = false;
bool last_synced = false;
bool synced = false;

struct dcf77_frame *last_frame = NULL;

void dcf77_decode(uint16_t ticks, bool rising_edge)
{
    static bool frame_started = false;
    static uint8_t frame[8] = {0};  
    static uint8_t bit_cnt = 0;


    // ticks = TICKS_TO_MS(ticks, PRESC);
    last_edge_rising = rising_edge;
    last_ticks = ticks;

    if (!frame_started)
    {
        if (get_bit_val(ticks) == DCF77_BIT_VAL_NONE)
        {
            frame_started = true;
            last_frame_started = true;
        }

        return;
    }

    enum dcf77_bit_val val = 0;

    if (rising_edge) // Bit transmission // REVERSED NOW
    {
        val = get_bit_val(ticks);

        if (val == DCF77_BIT_VAL_ERROR || val == DCF77_BIT_VAL_NONE)
        {
            last_error = true;
            frame_started = 0;
            bit_cnt = 0;
            return;
        }

        uint8_t byte_idx = bit_cnt / 8;
        uint8_t bit_idx = bit_cnt % 8;
    
        frame[byte_idx] |= (val << bit_idx);

        bit_cnt++;

        if (bit_cnt >= 59)
        {
            struct dcf77_frame *frame_ptr = (struct dcf77_frame*)frame;
            
            static struct ds1307_time unix_time_dcf;

            unix_time_dcf.clock_halt = 0;
            unix_time_dcf.hour_mode = 0;
            unix_time_dcf.hours_tens = frame_ptr->hours_tens;
            unix_time_dcf.hours_units = frame_ptr->hours_units;
            unix_time_dcf.minutes_tens = frame_ptr->minutes_tens;
            unix_time_dcf.minutes_units = frame_ptr->minutes_units;
            unix_time_dcf.date_tens = frame_ptr->month_day_tens;
            unix_time_dcf.date_units = frame_ptr->month_days_units;
            unix_time_dcf.month_tens = frame_ptr->months_tens;
            unix_time_dcf.month_units = frame_ptr->months_units;
            unix_time_dcf.year_tens = frame_ptr->years_tens;
            unix_time_dcf.year_units = frame_ptr->years_units;
            unix_time_dcf.seconds_tens = 0;
            unix_time_dcf.seconds_units = 0;

            ds1307_set_time(&rtc_obj, &unix_time_dcf);

            synced = true;
            frame_started = false;
            bit_cnt = 0;

            last_synced = true;
            last_frame = frame_ptr;
    
            return;
        }
    }
    else // should be break
    {
        // TODO: Add breaks validation
        // TODO: take into account ignoring short pulses - it affects break time
    }

}

//------------------------------------------------------------------------------
