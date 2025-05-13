//------------------------------------------------------------------------------

/// @file main.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include <stddef.h>

#include <hal.h>

#include <radio_manager.h>
#include <clock_manager.h>
#include <ui_manager.h>

#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <event.h>

//------------------------------------------------------------------------------

int main()
{
    hal_init();

    radio_manager_init();
    clock_manager_init();
    ui_manager_init();

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_mode();
    
    while (1)
    {
        /* Watchdog, sleep mode handling */

        cli();
        if (!event_get())
        {
            sleep_enable();
            sei();
        // here if interupt occurs, it shoud have sleep_disable() in handler
            sleep_cpu();
            sleep_disable();
        }
        else
        {
            sei();
        }

        hal_process();

        /* Main logic */
        radio_manager_process();
        clock_manager_process();
        ui_manager_process();
    }
}

//------------------------------------------------------------------------------
