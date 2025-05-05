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
            memset(ctx.frame[0], 0x00, sizeof(ctx.frame[0]));

            ctx.frame_started = false;
            ctx.bit_cnt = 0;

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
