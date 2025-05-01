//------------------------------------------------------------------------------

/// @file core.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "core.h"

#include <avr/interrupt.h>

//------------------------------------------------------------------------------

/* Defined in wdg.h - store MCUSR value before clearing needed by watchdog stop procedure */
extern uint8_t mcusr_mirror;

//------------------------------------------------------------------------------

enum core_reset_source core_get_reset_source(void)
{
    enum core_reset_source rst_src = 0;

    rst_src = mcusr_mirror & ((1 << PORF) | (1 << EXTRF) | (1 << BORF) | (1 << WDRF));

    return rst_src;
}

void core_enter_sleep_mode(enum core_sleep_mode mode, bool disable_bod)
{
    set_sleep_mode(mode);

    if (disable_bod)
    {
        cli();
        sleep_enable();
        // sleep_bod_disable(); // not available in ATmega88
        sei();
        sleep_cpu();
        sleep_disable();
    }
    else
        sleep_mode();
}

bool core_peripheral_power_down(enum core_power_down_periph periph, bool enable)
{
    if (periph > (2 * CORE_POWER_DOWN_PERIPH_ALL) - 1)
        return false;

    if (periph & CORE_POWER_DOWN_PERIPH_ALL)
    {
        enable ? power_all_enable() : power_all_disable();
        return true;
    }

    if (periph & CORE_POWER_DOWN_PERIPH_ADC)
        enable ? power_adc_enable() : power_adc_disable();
    if (periph & CORE_POWER_DOWN_PERIPH_USART0)
        enable ? power_usart0_enable() : power_usart0_disable();
    if (periph & CORE_POWER_DOWN_PERIPH_SPI)
        enable ? power_spi_enable() : power_spi_disable();
    if (periph & CORE_POWER_DOWN_PERIPH_TIMER1)
        enable ? power_timer1_enable() : power_timer1_disable();
    if (periph & CORE_POWER_DOWN_PERIPH_TIMER0)
        enable ? power_timer0_enable() : power_timer0_disable();
    if (periph & CORE_POWER_DOWN_PERIPH_TIMER2)
        enable ? power_timer2_enable() : power_timer2_disable();
    if (periph & CORE_POWER_DOWN_PERIPH_TWI)
        enable ? power_twi_enable() : power_twi_disable();

    return true;
}

enum core_clock_presc core_get_clk_prec(void)
{
    return clock_prescale_get();
}

void core_set_clk_prec(enum core_clock_presc presc)
{
    clock_prescale_set(presc);
}

//------------------------------------------------------------------------------