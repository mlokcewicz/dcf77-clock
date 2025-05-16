//------------------------------------------------------------------------------

/// @file clock_manager.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "clock_manager.h"

#include <event.h>

#include <hal.h>

//------------------------------------------------------------------------------

// #define CLOCK_MANAGER_TIMESTAMP(h_tens, h_units, m_tens, m_units) \
// { \
//     .hours_tens = h_tens, \
//     .hours_units = h_units, \
//     .minutes_tens = m_tens, \
//     .minutes_units = m_units \
// }

// #define CLOCK_MANAGER_SYNC_TIMESTAMP (struct hal_timestamp)CLOCK_MANAGER_TIMESTAMP(0, 4, 0, 0)
#define CLOCK_MANAGER_TIMESTAMP(h, m) \
{ \
    .hours = h, \
    .minutes = m \
}

#define CLOCK_MANAGER_SYNC_TIMESTAMP (struct hal_timestamp)CLOCK_MANAGER_TIMESTAMP(4, 0)

//------------------------------------------------------------------------------

struct clock_manager_ctx
{
    bool new_sec;
    struct hal_timestamp alarm;
};

static struct clock_manager_ctx ctx;

//------------------------------------------------------------------------------

/* HAL callbacks */

void hal_exti_sqw_cb(void)
{
    ctx.new_sec = true;
}

//------------------------------------------------------------------------------

static bool time_is_equal(event_update_time_req_data_t *time, struct hal_timestamp *timestamp)
{
    return (time->seconds == 0 && 
            time->minutes == timestamp->minutes && 
            time->hours == timestamp->hours);
    // return (time->seconds == 0 && 
    //         time->minutes / 10 == timestamp->minutes_units && 
    //         time->minutes % 10 == timestamp->minutes_tens &&
    //         time->hours / 10 == timestamp->hours_units &&
    //         time->hours % 10 == timestamp->hours_tens);
    // // return (time->seconds_units == 0 && 
    //         time->seconds_tens == 0 && 
    //         time->minutes_units == timestamp->minutes_units && 
    //         time->minutes_tens == timestamp->minutes_tens &&
    //         time->hours_units == timestamp->hours_units &&
    //         time->hours_tens == timestamp->hours_tens);
}

//------------------------------------------------------------------------------

bool clock_manager_init(void)
{
    // if (hal_time_is_reset())
    //     event_set(EVENT_SYNC_TIME_REQ);

    hal_get_alarm(&ctx.alarm);
    hal_get_alarm(event_get_data(EVENT_SET_ALARM_REQ));

    return true;
}

void clock_manager_process(void)
{
    if (event_get() & EVENT_SET_TIME_REQ)
    {
        hal_set_time(event_get_data(EVENT_SET_TIME_REQ));

        event_clear(EVENT_SET_TIME_REQ);
    }

    if (event_get() & EVENT_SET_ALARM_REQ)
    {
        hal_set_alarm(event_get_data(EVENT_SET_ALARM_REQ));
        hal_get_alarm(&ctx.alarm);

        event_clear(EVENT_SET_ALARM_REQ);
    }

    if (ctx.new_sec)
    {
        event_update_time_req_data_t *time = event_get_data(EVENT_UPDATE_TIME_REQ);

        hal_get_time(time);

        event_set(EVENT_UPDATE_TIME_REQ);

        if (time_is_equal(time, &CLOCK_MANAGER_SYNC_TIMESTAMP))
            event_set(EVENT_SYNC_TIME_REQ);

        if (ctx.alarm.is_enabled && time_is_equal(time, &ctx.alarm))
        {
            event_set(EVENT_ALARM_REQ);
            ctx.alarm.is_enabled = false;
        }

        ctx.new_sec = false;
    }
}

//------------------------------------------------------------------------------
