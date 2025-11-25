//------------------------------------------------------------------------------

/*
 * Copyright 2025 Michal Lokcewicz
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
volatile uint8_t *dcf77_get_frame(void);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* DCF77_DECODER_H_ */

//------------------------------------------------------------------------------
