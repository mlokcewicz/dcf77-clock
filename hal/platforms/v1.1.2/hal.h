//------------------------------------------------------------------------------

/// @file hal.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef HAL_H_
#define HAL_H_

//------------------------------------------------------------------------------

#include <stdint.h>

#include <buzzer.h> /* Do not duplicate struct buzzer_note (flash consumption reduction > clean code) */
#include <ds1307.h> /* Do not duplicate struct ds1307_time */

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

/// @brief Sets audio pattern for buzzer
/// @param pattern selected audio pattern array @ref struct buzzer_note
/// @param pattern_len selected audio pattern array size in bytes
/// @param bpm selected audio pattern BPM
void hal_audio_set_pattern(struct buzzer_note *pattern, uint16_t pattern_len, uint16_t bpm);

/// @brief Processes audio pattern for buzzer
void hal_audio_process(void);

/// @brief Sets time on RTC
/// @param time pointer to time structure @ref struct ds1307_time
void hal_set_time(struct ds1307_time *time);

/// @brief  Gets time from RTC
/// @param time pointer to time structure @ref struct ds1307_time
void hal_get_time(struct ds1307_time *time);

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
bool hal_system_timer_timeout_passed(uint32_t timestamp, uint32_t timeout);

/// @brief  Gets the current system timer tickstamp
/// @return Current system timer tickstamp
uint32_t hal_system_timer_get(void);

//------------------------------------------------------------------------------

#endif /* HAL_H_ */
