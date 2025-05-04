//------------------------------------------------------------------------------

/// @file dcf77_decoder.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef DCF77_DECODER_H_
#define DCF77_DECODER_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>

//------------------------------------------------------------------------------

struct dcf77_frame
{
    uint8_t frame_start_always_zero : 1;
    uint16_t weather_info : 14;
    uint8_t auxiliary_antenna : 1;
    uint8_t time_change_announcement : 1;
    uint8_t winter_time : 2;
    uint8_t leap_second : 1;
    uint8_t time_start_always_zero : 1;
    uint8_t minutes_units : 4;
    uint8_t minutes_tens : 3;
    uint8_t minutes_parity : 1;
    uint8_t hours_units : 4;
    uint8_t hours_tens : 2;
    uint8_t hours_parity : 1;
    uint8_t month_days_units : 4;
    uint8_t month_day_tens : 2;
    uint8_t weekday : 3;
    uint8_t months_units : 4;
    uint8_t months_tens : 1;
    uint8_t years_units : 4;
    uint8_t years_tens: 4;
    uint8_t date_parity: 1;
} __attribute__((__packed__));

//------------------------------------------------------------------------------

extern bool last_edge_rising ;
extern uint16_t last_ticks;
extern bool last_error;
extern bool last_frame_started;
extern bool last_synced;
extern bool synced;
extern struct dcf77_frame *last_frame;

void dcf77_decode(uint16_t ticks, bool rising_edge);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* DCF77_DECODER_H_ */

//------------------------------------------------------------------------------
