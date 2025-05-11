//------------------------------------------------------------------------------

/// @file main.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include <stddef.h>

#include <hal.h>

#include <radio_manager.h>
#include <clock_manager.h>
#include <ui_manager.h>

//------------------------------------------------------------------------------

int main()
{
    hal_init();

    radio_manager_init();
    clock_manager_init();
    ui_manager_init();
    
    while (1)
    {
        /* Watchdog, sleep mode handling */
        hal_process();

        /* Main logic */
        radio_manager_process();
        clock_manager_process();
        ui_manager_process();
    }
}

//------------------------------------------------------------------------------

// 3 managers + main logic

// Basic threads / processess:
// * radio_manager (waits for request of sync, then enables dcf receiver and TIM1 irq, and set new last_sync timestamp, and waits for sync_finished and disables dcf and TIM1 irq)
// * clock_manager (waits for SQW and send event to UI, wait for time from TIM1 irq or from UI)
// * ui_manager (waits for INT0, PCINT interrupts from encoder / button and driver LCD in thread, waits for SWQ irq to update time, checks for alert)

// Events:
// * **ENC+ (to ui_manager)
// * **ENC- (to ui_manager)
// * **SW (to ui_manager)
// * **TIM (to radio_manager/clock_manager)
// * **SEC (to clock_manager/ui_manager)
// * Sync REQ (from ui_manager and radio_manager) (to radio_manager)
// * Clock UPDT REQ (from ui_manager and radio_manager) (to clock_manager and ui_manager)
// * Alarm (from clock_manager) (to ui_manager)
