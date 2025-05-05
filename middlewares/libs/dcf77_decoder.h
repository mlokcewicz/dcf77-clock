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

enum dcf77_decoder_status
{
    DCF77_DECODER_STATUS_WAITING,
    DCF77_DECODER_STATUS_FRAME_STARTED,
    DCF77_DECODER_STATUS_BIT_RECEIVED,
    DCF77_DECODER_STATUS_BREAK_RECEIVED,
    DCF77_DECODER_STATUS_ERROR,
    DCF77_DECODER_STATUS_SYNCED,
};

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

/// @brief Decodes given pulse (not re-entrant)
/// @param ms time in ms of detected pulse
/// @param triggered_on_bit true if given pulse is considered as a bit value (not break)
/// @return current status @ref enum dcf77_decoder_status
enum dcf77_decoder_status dcf77_decode(uint16_t ms, bool triggered_on_bit);

/// @brief Returns pointer do last received time frame
/// @return last received frame pointer @ref struct dcf77_frame
struct dcf77_frame *dcf77_get_frame(void);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* DCF77_DECODER_H_ */

//------------------------------------------------------------------------------
