//------------------------------------------------------------------------------

/// @file clock_manager.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "clock_manager.h"

#include <event.h>

#include <hal.h>

//------------------------------------------------------------------------------

struct clock_manager_ctx
{
     bool new_sec;
};

static struct clock_manager_ctx ctx;

//------------------------------------------------------------------------------

/* HAL callbacks */

void hal_exti_sqw_cb(void)
{
    ctx.new_sec = true;
}

//------------------------------------------------------------------------------

// struct clock_manager_timestamp
// {
//     uint8_t minutes_units : 4;
//     uint8_t minutes_tens : 3;
//     uint8_t hours_units : 4;
//     uint8_t hours_tens : 2;
// };

// static bool time_is_equal(struct event_update_time_req_data_t *time, struct clock_manager_timestamp *timestamp)
// {
//     return (time->seconds_units == 0 && 
//             time->seconds_tens == 0 && 
//             time->minutes_units == timestamp->minutes_units && 
//             time->minutes_tens == timestamp->minutes_tens &&
//             time->hours_units == timestamp->hours_units &&
//             time->hours_tens == timestamp->hours_tens);
// }

//------------------------------------------------------------------------------

bool clock_manager_init(void)
{
    if (hal_time_is_reset())
        event_set(EVENT_SYNC_TIME_REQ);

    return true;
}

void clock_manager_process(void)
{
    if (ctx.new_sec)
    {
        event_update_time_req_data_t *time = event_get_data(EVENT_UPDATE_TIME_REQ);

        hal_get_time(time);

        event_set(EVENT_UPDATE_TIME_REQ);

        if (time->seconds_units == 0 && 
            time->seconds_tens == 0 && 
            time->minutes_units == 3 && 
            time->minutes_tens == 2 &&
            time->hours_units == 0 &&
            time->hours_tens == 0)
        {
            event_set(EVENT_SYNC_TIME_REQ);
        }

        ctx.new_sec = false;
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
