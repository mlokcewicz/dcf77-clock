//------------------------------------------------------------------------------

/// @file main.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include <stddef.h>

#include <hal.h>

#include <radio_manager.h>
#include <clock_manager.h>
#include <communication_manager.h>
#include <ui_manager.h>

//------------------------------------------------------------------------------

int main()
{
    hal_init();

    radio_manager_init();
    clock_manager_init();
    communication_manager_init();
    ui_manager_init();
    
    while (1)
    {
        /* Watchdog, sleep mode handling and polling mode handling */
        hal_process();

        /* Main logic */
        radio_manager_process();
        clock_manager_process();
        communication_manager_process();
        ui_manager_process();
    }
}

//------------------------------------------------------------------------------
