//------------------------------------------------------------------------------

/// @file hal.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef HAL_H_
#define HAL_H_

//------------------------------------------------------------------------------

#include <stdint.h>

#include <buzzer.h> /* Do not duplicate struct buzzer_note (in this case FLASH consumption reduction > clean code) */
#include <ds1307.h> /* Do not duplicate struct ds1307_time */

//------------------------------------------------------------------------------

struct hal_timestamp
{
    uint8_t hours;
    uint8_t minutes;
    uint8_t is_enabled;
}__attribute__((packed));

//------------------------------------------------------------------------------

/// @brief Initializes hardware abstraction layer
/// @note This function initializes all low level drivers and sets up the system
void hal_init(void);

/// @brief Handles hardware abstraction layer internal processes (watchdog, sleep mode)
/// @note This function should be called in the main loop   
void hal_process(void);

/// @brief Sets LED state
void hal_led_set(bool state);

/// @brief Clears LCD display
void hal_lcd_clear(void);

/// @brief Prints string on LCD display
void hal_lcd_print(const char* str, uint8_t row, uint8_t col);

/// @brief Sets cursor position on LCD display
/// @param row selected row
/// @param col selected column
void hal_lcd_set_cursor(uint8_t row, uint8_t col);

/// @brief Sets cursor mode on LCD display
/// @param visible true to set visible cursor, otherwise false
/// @param blinking true to set blinking cursor, otherwise false
void hal_lcd_set_cursor_mode(bool visible, bool blinking);

/// @brief Prints character on LCD display
/// @param ch selected character code
void hal_lcd_putc(const char ch);

/// @brief Sets audio pattern for buzzer
/// @param pattern selected audio pattern array @ref struct buzzer_note
/// @param pattern_len selected audio pattern array size in bytes
/// @param bpm selected audio pattern BPM
void hal_audio_set_pattern(struct buzzer_note *pattern, uint16_t pattern_len, uint16_t bpm);

/// @brief Stops audio pattern for buzzer
void hal_audio_stop(void);

/// @brief Processes audio pattern for buzzer
void hal_audio_process(void);

/// @brief Processes button polling
void hal_button_process(void);

/// @brief Processes encoder polling
void hal_rotary_encoder_process(void);

/// @brief Sets time on RTC
/// @param time pointer to time structure @ref struct ds1307_time
void hal_set_time(struct ds1307_time *time);

/// @brief  Gets time from RTC
/// @param time pointer to time structure @ref struct ds1307_time
void hal_get_time(struct ds1307_time *time);

/// @brief Sets alarm on RTC
/// @param alarm pointer to alarm structure @ref struct hal_timestamp
void hal_set_alarm(struct hal_timestamp *alarm);

/// @brief  Gets alarm from RTC
/// @param alarm pointer to alarm structure @ref struct hal_timestamp
void hal_get_alarm(struct hal_timestamp *alarm);

/// @brief Sets timezone
/// @param tz pointer to timezone value
void hal_set_timezone(int8_t *tz);

/// @brief Gets timezone
/// @param tz pointer to timezone value
void hal_get_timezone(int8_t *tz);

/// @brief Checks if RTC is running
/// @return true if RTC is running, otherwise false
bool hal_time_is_reset(void);

/// @brief Gets actual DCF77 receiver output state
/// @return true if DCF77 receiver output is high, otherwise false
bool hal_dcf_get_state(void);

/// @brief Sets DCF77 receiver power down state
/// @param pwr_down true to power down DCF77 receiver, otherwise false
void hal_dcf_power_down(bool pwr_down);

/// @brief Checks if the timeout has passed
/// @param timestamp start tickstamp
/// @param timeout timeout in milliseconds
/// @return True if the timeout has passed, otherwise false
bool hal_system_timer_timeout_passed(uint16_t timestamp, uint16_t timeout);

/// @brief  Gets the current system timer tickstamp
/// @return Current system timer tickstamp
uint16_t hal_system_timer_get(void);

//------------------------------------------------------------------------------

#endif /* HAL_H_ */
