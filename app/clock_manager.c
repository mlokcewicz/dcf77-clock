//------------------------------------------------------------------------------

/// @file clock_manager.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "clock_manager.h"

#include <hal.h>

//------------------------------------------------------------------------------

/* From Radio Manager */
extern bool synced;

/* From UI Manager */
extern void print_time(uint8_t line);

/* RTC */
bool new_sec = false;

void hal_exti_sqw_cb(void)
{
    new_sec = true;
}

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

//------------------------------------------------------------------------------

bool clock_manager_init(void)
{
    // set_system_time(1744458473);

    if (hal_time_is_reset())
        synced = false;

    return true;
}

void clock_manager_process(void)
{
    if (new_sec)
    {
        hal_get_time(&unix_time);
        print_time(1);
        new_sec = false;
    }

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
}

//------------------------------------------------------------------------------
