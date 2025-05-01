//------------------------------------------------------------------------------

/// @file core.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef CORE_H_
#define CORE_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>

#include <avr/sleep.h>
#include <avr/power.h>

//------------------------------------------------------------------------------

enum core_reset_source
{
    CORE_RST_SRC_POR = 1 << 0,
    CORE_RST_SRC_EXT = 1 << 1,
    CORE_RST_SRC_BOR = 1 << 2,
    CORE_RST_SRC_WDG = 1 << 3
};

enum core_sleep_mode
{
    CORE_SLEEP_MODE_IDLE = SLEEP_MODE_IDLE,
    CORE_SLEEP_MODE_PWR_DOWN = SLEEP_MODE_PWR_DOWN,
    CORE_SLEEP_MODE_PWR_SAVE = SLEEP_MODE_PWR_SAVE,
    CORE_SLEEP_MODE_ADC = SLEEP_MODE_ADC,
    CORE_SLEEP_MODE_STANDBY = SLEEP_MODE_STANDBY,
    // CORE_SLEEP_MODE_EXT_STANDBY = SLEEP_MODE_EXT_STANDBY // not available in ATmega88
};

enum core_power_down_periph
{
    CORE_POWER_DOWN_PERIPH_ADC = 1 << 0,
    CORE_POWER_DOWN_PERIPH_USART0 = 1 << 1,
    CORE_POWER_DOWN_PERIPH_SPI = 1 << 2,
    CORE_POWER_DOWN_PERIPH_TIMER1 = 1 << 3,
    CORE_POWER_DOWN_PERIPH_TIMER0 = 1 << 5,
    CORE_POWER_DOWN_PERIPH_TIMER2 = 1 << 6,
    CORE_POWER_DOWN_PERIPH_TWI = 1 << 7,
    CORE_POWER_DOWN_PERIPH_ALL = 1 << 8,
};

enum core_clock_presc
{
    CORE_CLOCK_PRESC_1 = clock_div_1,
    CORE_CLOCK_PRESC_2 = clock_div_2,
    CORE_CLOCK_PRESC_4 = clock_div_4,
    CORE_CLOCK_PRESC_8 = clock_div_8,
    CORE_CLOCK_PRESC_16 = clock_div_16,
    CORE_CLOCK_PRESC_32 = clock_div_32,
    CORE_CLOCK_PRESC_64 = clock_div_64,
    CORE_CLOCK_PRESC_128 = clock_div_128,
    CORE_CLOCK_PRESC_256 = clock_div_256
};

//------------------------------------------------------------------------------

/// @brief Returns last reset source
/// @note function uses extern variable mcusr_mirror defined in wdg.c
/// @return reset source @ref core_reset_source 
enum core_reset_source core_get_reset_source(void);

/// @brief Enters sleep mode
/// @param mode sleep mode @ref core_sleep_mode
/// @param disable_bod ture if BOD has to be disabled 
void core_enter_sleep_mode(enum core_sleep_mode mode, bool disable_bod);

/// @brief Disables or enables selected peripheral
/// @note Busy peripherals cannot be turned off - their functions have to be disabled first
/// @param periph peripheral to enable or disable @ref core_power_down_periph
/// @param enable ture if selected peripheral has to be enabled, otehrwise false
/// @return true if operation succeeded 
bool core_peripheral_power_down(enum core_power_down_periph periph, bool enable);

/// @brief Returns current clock prescaler value
/// @return current clock prescaler value @ref core_clock_presc
enum core_clock_presc core_get_clk_prec(void);

/// @brief Sets clock prescaler value
/// @param presc desired clock prescaler value @ref core_clock_presc
void core_set_clk_prec(enum core_clock_presc presc);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* CORE_H_ */

//------------------------------------------------------------------------------
