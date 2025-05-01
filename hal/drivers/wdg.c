//------------------------------------------------------------------------------

/// @file wdg.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "wdg.h"

#include <stdint.h>
#include <stddef.h>

#include <avr/wdt.h>
#include <avr/interrupt.h>

#include <util/atomic.h>

//------------------------------------------------------------------------------

struct wdg_context
{
    wdg_int_cb int_cb;
};

static struct wdg_context ctx;

//------------------------------------------------------------------------------

uint8_t mcusr_mirror __attribute__ ((section (".noinit")));

void get_mcusr(void) __attribute__((naked)) __attribute__((section(".init3")));

void get_mcusr(void)
{
    /* Turn off WDG on start-up in case of accidental turning on. WDG after reset is set to 15 ms timeout (even if configured otherwise) */
    mcusr_mirror = MCUSR;
    MCUSR = 0;
    wdt_disable();
}

//------------------------------------------------------------------------------

ISR(WDT_vect)
{
    if (ctx.int_cb)
        ctx.int_cb();
}

//------------------------------------------------------------------------------

bool wdg_init(enum wdg_mode mode, enum wdg_period period, wdg_int_cb int_cb)
{
    if (mode > (WDG_MODE_INT | WDG_MODE_RST) || period > WDG_PERIOD_8S)
        return false;

    wdt_reset();

    /* WDRF overrides WDE bit so it has to be cleared before switchong WDG off */
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    ctx.int_cb = int_cb;

    uint8_t wdtcsr_val = 0;

    /* Set restart or interrupt mode */
    if (mode & WDG_MODE_INT)
        wdtcsr_val |= (1 << WDIE);
    if (mode & WDG_MODE_RST)
        wdtcsr_val |= (1 << WDE);

    /* Set new prescaler (time-out) value */ 
    wdtcsr_val |= period & 0x07;
    wdtcsr_val |= (!!(period & 0x08) << WDP3);

    /* Timing sequence is used ONLY to clear WDE bit or set/clear WDP bits. Setting of WDE bit can be done directly.
       After timing sequence enabling, WDTCSR must be set during next 4 clock cycles - so it can be set by &= ect. Must be overwritten directly. */
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        /* Start timed equence */
        WDTCSR |= (1 << WDCE) | (1 << WDE);
        WDTCSR = wdtcsr_val;
    }

    return true;
}

void wdg_feed(void)
{
    wdt_reset();
}

void wdg_system_reset(void)
{
    wdt_enable(WDTO_15MS);
    cli();

    while (1);
}

void wdg_deinit(void)
{
    /* WDRF overrides WDE bit so it has to be cleared before switchong WDG off */
    MCUSR &= ~(1 << WDRF);
    
    wdt_disable();

    ctx.int_cb = NULL;
}

//------------------------------------------------------------------------------
