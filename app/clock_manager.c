//------------------------------------------------------------------------------

/// @file clock_manager.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "clock_manager.h"

#include <event.h>

#include <hal.h>

//------------------------------------------------------------------------------

/* From Radio Manager */
extern bool synced;

/* RTC */
static bool new_sec = false;

void hal_exti_sqw_cb(void)
{
    new_sec = true;
}

//------------------------------------------------------------------------------

bool clock_manager_init(void)
{
    if (hal_time_is_reset())
        synced = false;

    return true;
}

void clock_manager_process(void)
{
    if (new_sec)
    {
        hal_get_time(event_get_data(EVENT_TIME_UPDATE_REQ));

        event_set(EVENT_TIME_UPDATE_REQ);

        new_sec = false;
    }

    if (event_get() & EVENT_SET_TIME_REQ)
    {
        hal_set_time(event_get_data(EVENT_SET_TIME_REQ));

        event_clear(EVENT_SET_TIME_REQ);
    }

    // uint8_t *str = "DUPA";
    // ds1307_save_to_ram(&rtc_obj, 5, str, 4);
    // char buf2[10] = {0};
    // ds1307_read_from_ram(&rtc_obj, 4, buf2, 4);
    // hd44780_set_pos(&lcd_obj, 0, 8);
    // hd44780_print(&lcd_obj, buf2);
    // _delay_ms(5000);
}

//------------------------------------------------------------------------------
