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
// Macros for extracting DCF77 frame fields from a uint8_t* frame (bit 0 = LSB of frame[0])

// MOJE 96% - Program:    7868 bytes (96.0% Full)

// #define DCF77_DECODER_FRAME_GET_FRAME_START(frame)         (((frame[0] >> 0) & 0x01))
// #define DCF77_DECODER_FRAME_GET_WEATHER_INFO(frame)        ((((uint16_t)(frame[0] >> 1)) & 0x7F) | (((uint16_t)(frame[1] & 0x3F)) << 7))
// #define DCF77_DECODER_FRAME_GET_AUX_ANTENNA(frame)         (((frame[1] >> 7) & 0x01))
// #define DCF77_DECODER_FRAME_GET_TIME_CHANGE_ANN(frame)     (((frame[2] >> 0) & 0x01))
// #define DCF77_DECODER_FRAME_GET_WINTER_TIME(frame)         (((frame[2] >> 1) & 0x03))
// #define DCF77_DECODER_FRAME_GET_LEAP_SECOND(frame)         (((frame[2] >> 3) & 0x01))
// #define DCF77_DECODER_FRAME_GET_TIME_START(frame)          (((frame[2] >> 4) & 0x01))

// #define DCF77_DECODER_FRAME_GET_MINUTES_UNITS(frame)       (((frame[3] >> 0) & 0x01) << 3 | ((frame[2] >> 5) & 0x07))
// #define DCF77_DECODER_FRAME_GET_MINUTES_TENS(frame)        (((frame[3] >> 1) & 0x07))
// #define DCF77_DECODER_FRAME_GET_MINUTES_PARITY(frame)      (((frame[3] >> 4) & 0x01))
// #define DCF77_DECODER_FRAME_GET_HOURS_UNITS(frame)         (((frame[4] >> 0) & 0x01) << 3 | ((frame[3] >> 5) & 0x07))
// #define DCF77_DECODER_FRAME_GET_HOURS_TENS(frame)          (((frame[4] >> 1) & 0x03))
// #define DCF77_DECODER_FRAME_GET_HOURS_PARITY(frame)        (((frame[4] >> 3) & 0x01))
// #define DCF77_DECODER_FRAME_GET_DAY_UNITS(frame)           (((frame[4] >> 4) & 0x0F))
// #define DCF77_DECODER_FRAME_GET_DAY_TENS(frame)            (((frame[5] >> 0) & 0x03))
// #define DCF77_DECODER_FRAME_GET_WEEKDAY(frame)             (((frame[5] >> 2) & 0x07))
// #define DCF77_DECODER_FRAME_GET_MONTH_UNITS(frame)         (((frame[6] >> 0) & 0x01) << 3 | ((frame[5] >> 5) & 0x07))
// #define DCF77_DECODER_FRAME_GET_MONTH_TENS(frame)          (((frame[6] >> 1) & 0x01))
// #define DCF77_DECODER_FRAME_GET_YEAR_UNITS(frame)          (((frame[6] >> 2) & 0x0F))
// #define DCF77_DECODER_FRAME_GET_YEAR_TENS(frame)           (((frame[7] >> 0) & 0x03) << 2 | ((frame[6] >> 6) & 0x03))
// #define DCF77_DECODER_FRAME_GET_DATE_PARITY(frame)         (((frame[7] >> 2) & 0x01))

#define DCF77_GET_BITS(frame, bit_pos, mask) ((((((uint16_t)frame[((bit_pos) / 8) + 1] << 8) | (uint16_t)frame[(bit_pos) / 8]) >> ((bit_pos) % 8)) & (mask)))

#define DCF77_DECODER_FRAME_GET_FRAME_START(frame)          DCF77_GET_BITS(frame, 0, 0x01)
#define DCF77_DECODER_FRAME_GET_WEATHER_INFO(frame)         (((DCF77_GET_BITS(frame, 1, 0x7F)) | (DCF77_GET_BITS(frame, 8, 0x3F) << 7)))
#define DCF77_DECODER_FRAME_GET_AUX_ANTENNA(frame)          DCF77_GET_BITS(frame, 15, 0x01)
#define DCF77_DECODER_FRAME_GET_TIME_CHANGE_ANN(frame)      DCF77_GET_BITS(frame, 16, 0x01)
#define DCF77_DECODER_FRAME_GET_WINTER_TIME(frame)          DCF77_GET_BITS(frame, 17, 0x03)
#define DCF77_DECODER_FRAME_GET_LEAP_SECOND(frame)          DCF77_GET_BITS(frame, 19, 0x01)
#define DCF77_DECODER_FRAME_GET_TIME_START(frame)           DCF77_GET_BITS(frame, 20, 0x01)

#define DCF77_DECODER_FRAME_GET_MINUTES_UNITS(frame)        DCF77_GET_BITS(frame, 21, 0x0F)
#define DCF77_DECODER_FRAME_GET_MINUTES_TENS(frame)         DCF77_GET_BITS(frame, 25, 0x07)
#define DCF77_DECODER_FRAME_GET_MINUTES_PARITY(frame)       DCF77_GET_BITS(frame, 28, 0x01)

#define DCF77_DECODER_FRAME_GET_HOURS_UNITS(frame)          DCF77_GET_BITS(frame, 29, 0x0F)
#define DCF77_DECODER_FRAME_GET_HOURS_TENS(frame)           DCF77_GET_BITS(frame, 33, 0x03)
#define DCF77_DECODER_FRAME_GET_HOURS_PARITY(frame)         DCF77_GET_BITS(frame, 35, 0x01)

#define DCF77_DECODER_FRAME_GET_DAY_UNITS(frame)            DCF77_GET_BITS(frame, 36, 0x0F)
#define DCF77_DECODER_FRAME_GET_DAY_TENS(frame)             DCF77_GET_BITS(frame, 40, 0x03)
#define DCF77_DECODER_FRAME_GET_WEEKDAY(frame)              DCF77_GET_BITS(frame, 42, 0x07)
#define DCF77_DECODER_FRAME_GET_MONTH_UNITS(frame)          DCF77_GET_BITS(frame, 45, 0x0F)
#define DCF77_DECODER_FRAME_GET_MONTH_TENS(frame)           DCF77_GET_BITS(frame, 49, 0x01)
#define DCF77_DECODER_FRAME_GET_YEAR_UNITS(frame)           DCF77_GET_BITS(frame, 50, 0x0F)
#define DCF77_DECODER_FRAME_GET_YEAR_TENS(frame)            DCF77_GET_BITS(frame, 54, 0x0F)
#define DCF77_DECODER_FRAME_GET_DATE_PARITY(frame)          DCF77_GET_BITS(frame, 58, 0x01)

//------------------------------------------------------------------------------

/// @brief Decodes given pulse (not re-entrant)
/// @param ms time in ms of detected pulse
/// @param triggered_on_bit true if given pulse is considered as a bit value (not break)
/// @return current status @ref enum dcf77_decoder_status
enum dcf77_decoder_status dcf77_decode(uint16_t ms, bool triggered_on_bit);

/// @brief Returns pointer do last received time frame
/// @return last received frame pointer
uint8_t *dcf77_get_frame(void);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* DCF77_DECODER_H_ */

//------------------------------------------------------------------------------
