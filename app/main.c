//------------------------------------------------------------------------------

/// @file main.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include <stddef.h>

#include <util/delay.h>
#include <avr/io.h>
#include <avr/fuse.h>
#include <avr/interrupt.h>

//------------------------------------------------------------------------------

/* Fuse and lock bits are active-low so to program given bit use & operator */

LOCKBITS = 0xFF;

FUSES =
{
    .low = LFUSE_DEFAULT,
    .high = HFUSE_DEFAULT,
    .extended = EFUSE_DEFAULT,
};

//------------------------------------------------------------------------------

/* Non implemented ISR handling */

ISR(BADISR_vect)
{
    /* Add error handling */
}

//------------------------------------------------------------------------------

/* Common */
#include <stdbool.h>

#include <core.h>
#include <wdg.h>
// #include <avr/wdt.h>

#include <ds1307.h>
#include <gpio.h>

#include <radio_manager.h>
#include <clock_manager.h>
#include <ui_manager.h>

#include <timer.h>


struct ds1307_time unix_time = 
{
    .clock_halt = 0,
    .hour_mode = 0,
    .seconds_units = 2,
    .seconds_tens = 3,
    .minutes_units = 5,
    .minutes_tens = 4,
    .hours_units = 8,
    .hours_tens = 1,
    .date_units = 1,
    .date_tens = 0,
    .day = 5,
    .month_units = 5,
    .month_tens = 0,
    .year_units = 5,
    .year_tens = 2,
};

int main()
{
    wdg_init(WDG_MODE_RST, WDG_PERIOD_8S, NULL);
    // wdt_enable(WDTO_8S);
    
    system_timer_init();

    // set_system_time(1744458473);

    radio_manager_init();
    clock_manager_init();
    ui_manager_init();

    
    sei();

    // if (!ds1307_is_running(&rtc_obj))
    //     ds1307_set_time(&rtc_obj, &unix_time);

    while (1)
    {
        radio_manager_process();
        clock_manager_process();
        ui_manager_process();


        // buzzer_play_pattern(&buzzer1_obj, alarm_beep, sizeof(alarm_beep), 800);
        // _delay_ms(1000);
        // buzzer_process(&buzzer1_obj);
        
        // uint8_t *str = "DUPA";
        // ds1307_save_to_ram(&rtc_obj, 5, str, 4);
        // char buf2[10] = {0};
        // ds1307_read_from_ram(&rtc_obj, 4, buf2, 4);
        // hd44780_set_pos(&lcd_obj, 0, 8);
        // hd44780_print(&lcd_obj, buf2);
        // _delay_ms(5000);

        // hd44780_set_pos(&lcd_obj, 1, 0);
        // char buff[16] = {0};
        // hd44780_print(&lcd_obj, ltoa(system_timer_get(), buff, 10));

        // uint32_t unix_time = time(NULL);
        // hd44780_set_pos(&lcd_obj, 1, 0);
        // hd44780_print(&lcd_obj, ctime(&unix_time) + 4);
        
        wdg_feed();
        // wdt_reset();
        core_enter_sleep_mode(CORE_SLEEP_MODE_IDLE, false);
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
