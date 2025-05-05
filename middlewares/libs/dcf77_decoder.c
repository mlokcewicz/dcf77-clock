//------------------------------------------------------------------------------

/// @file dcf77_decoder.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "dcf77_decoder.h"

#include <stddef.h>
#include <string.h>

//------------------------------------------------------------------------------

#define DCF77_DECODER_BIT_VAL_0_MIN_TIME_MS 40
#define DCF77_DECODER_BIT_VAL_0_MAX_TIME_MS 130
#define DCF77_DECODER_BIT_VAL_1_MIN_TIME_MS 140
#define DCF77_DECODER_BIT_VAL_1_MAX_TIME_MS 250
#define DCF77_DECODER_BIT_VAL_NONE_MIN_TIME_MS 1500
#define DCF77_DECODER_BIT_VAL_NONE_MAX_TIME_MS 2200

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
    uint8_t frame[2][8];  
    uint8_t bit_cnt;
};

static struct dcf77_ctx ctx;

//------------------------------------------------------------------------------

static bool is_in_range(uint16_t ms, uint16_t min, uint16_t max)
{
    return (ms >= min) && (ms < max);
}

static enum dcf77_bit_val get_bit_val(uint16_t ms)
{
    if (is_in_range(ms, DCF77_DECODER_BIT_VAL_0_MIN_TIME_MS, DCF77_DECODER_BIT_VAL_0_MAX_TIME_MS))
        return DCF77_BIT_VAL_0;
    if (is_in_range(ms, DCF77_DECODER_BIT_VAL_1_MIN_TIME_MS, DCF77_DECODER_BIT_VAL_1_MAX_TIME_MS))
        return DCF77_BIT_VAL_1;
    if (is_in_range(ms, DCF77_DECODER_BIT_VAL_NONE_MIN_TIME_MS, DCF77_DECODER_BIT_VAL_NONE_MAX_TIME_MS))
        return DCF77_BIT_VAL_NONE;

    return DCF77_BIT_VAL_ERROR;
}

static bool get_parity_even_bit(uint32_t data)
{
    // uint8_t no_of_ones = 0;
    // uint8_t i = 32;

    // while(i--)
    // {
    //     no_of_ones += (data & 1);
    //     data >>= 1;
    // }

    // return (no_of_ones & 1);

    // data ^= data >> 16;
    // data ^= data >> 8;
    // data ^= data >> 4;
    // data ^= data >> 2;
    // data ^= data >> 1;
    // return data & 1;

    return __builtin_parity(data);
}

static bool validate_frame(void)
{
    struct dcf77_frame *dcf_frame = (struct dcf77_frame *)ctx.frame[0];

    if (dcf_frame->frame_start_always_zero != 0)
        return false;

    if (dcf_frame->time_start_always_one != 1)
        return false;


    // uint16_t minutes_with_parity;
    // memcpy(&minutes_with_parity, &(ctx.frame[0][2]), 2);
    // minutes_with_parity >>= 4;

    // if (get_parity_even_bit(minutes_with_parity))
    //     return false;

    if (get_parity_even_bit((dcf_frame->minutes_tens << 4) | dcf_frame->minutes_units) != dcf_frame->minutes_parity)
        return false;

    // uint16_t hours_with_parity;
    // memcpy(&hours_with_parity, &(ctx.frame[0][3]), 2);
    // hours_with_parity >>= 4;

    // if (get_parity_even_bit(hours_with_parity))
    //     return false;

    if (get_parity_even_bit((dcf_frame->hours_tens << 4) | dcf_frame->hours_units) != dcf_frame->hours_parity)
        return false;

        
    // uint32_t date;
    // memcpy(&date, &(ctx.frame[0][5]), 4);
    // date >>= 4;

    uint32_t date = ((uint32_t)dcf_frame->month_day_units << 18) | ((uint32_t)dcf_frame->month_day_tens << 16) | 
                    (dcf_frame->weekday << 13) | 
                    (dcf_frame->months_units << 9) | (dcf_frame->months_tens << 8) | 
                    (dcf_frame->years_units << 4) | dcf_frame->years_tens ;
       
    if (get_parity_even_bit(date) != dcf_frame->date_parity)
        return false;
    

    // uint8_t month_day_parity = get_parity_even_bit((dcf_frame->month_day_tens << 4) | dcf_frame->month_day_units);
    // uint8_t month_parity = get_parity_even_bit((dcf_frame->months_tens << 7) | (dcf_frame->months_units << 3) | dcf_frame->weekday);
    // uint8_t years_parity = get_parity_even_bit((dcf_frame->years_tens << 4) | dcf_frame->years_units);

    // if (get_parity_even_bit((month_day_parity << 2) | (month_parity << 1) | years_parity) != dcf_frame->date_parity)
    //     return false;

    return true;
}

//------------------------------------------------------------------------------

enum dcf77_decoder_status dcf77_decode(uint16_t ms, bool triggered_on_bit)
{
    enum dcf77_bit_val val = 0;
    struct dcf77_frame *dcf_frame = (struct dcf77_frame *)ctx.frame[0];

    if (!ctx.frame_started)
    {
        if (get_bit_val(ms) == DCF77_BIT_VAL_NONE)
        {
            ctx.frame_started = true;
            
            return DCF77_DECODER_STATUS_FRAME_STARTED;
        }

        return DCF77_DECODER_STATUS_WAITING;
    }

    if (triggered_on_bit) // Bit transmission
    {
        val = get_bit_val(ms);

        if (val == DCF77_BIT_VAL_ERROR || val == DCF77_BIT_VAL_NONE)
        {
            ctx.frame_started = 0;
            ctx.bit_cnt = 0;
            return DCF77_DECODER_STATUS_ERROR;
        }

        uint8_t byte_idx = ctx.bit_cnt / 8;
        uint8_t bit_idx = ctx.bit_cnt % 8;
    
        ctx.frame[0][byte_idx] |= (val << bit_idx);

        ctx.bit_cnt++;

        if (ctx.bit_cnt >= 59 + dcf_frame->leap_second)
        {
            memcpy(ctx.frame[1], ctx.frame[0], sizeof(ctx.frame[1]));

            ctx.frame_started = false;
            ctx.bit_cnt = 0;

            if (!validate_frame())
                return DCF77_DECODER_STATUS_ERROR;

            return DCF77_DECODER_STATUS_SYNCED;
        }

        return DCF77_DECODER_STATUS_BIT_RECEIVED;
    }
    else // Break transmission
    {
        // TODO: Add breaks validation
        // TODO: take into account ignoring short pulses - it affects break time

        return DCF77_DECODER_STATUS_BREAK_RECEIVED;
    }
}

struct dcf77_frame *dcf77_get_frame(void)
{
    return (struct dcf77_frame*)ctx.frame[1];
}

//------------------------------------------------------------------------------
