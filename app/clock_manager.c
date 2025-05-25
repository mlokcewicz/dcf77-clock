//------------------------------------------------------------------------------

/*
 * Copyright 2025 Michal Lokcewicz
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//------------------------------------------------------------------------------

#include "clock_manager.h"

#include <event.h>

#include <hal.h>

//------------------------------------------------------------------------------

#define CLOCK_MANAGER_TIMESTAMP(h, m) (struct hal_timestamp){.hours = h, .minutes = m}

#define CLOCK_MANAGER_SYNC_TIMESTAMP CLOCK_MANAGER_TIMESTAMP(4, 0)  /* Auto synchronization at 04:00 */
#define CLOCK_MANAGER_DCF77_TIME_ZONE 1                             /* DCF77 sends time signal in UTC+1 time zone (UTC+2 for DST) */

//------------------------------------------------------------------------------

struct clock_manager_ctx
{
    volatile bool new_sec;
    struct hal_timestamp alarm;
    int8_t timezone;
};

static struct clock_manager_ctx ctx;

//------------------------------------------------------------------------------

/* HAL callbacks */

void hal_exti_sqw_cb(void) // Called from ISR
{
    ctx.new_sec = true;
}

//------------------------------------------------------------------------------

static bool timestamp_is_reached(event_update_time_req_data_t *time, struct hal_timestamp *timestamp)
{
#if 1
    return (time->seconds == 0 &&  time->minutes == timestamp->minutes && time->hours == timestamp->hours);
#else 
    /* Safe version of timestamp check - robust against interrupt loss / rtc communication failure */ 
    /* Due to value editing, timestamp struct cannot be temporary object created in-place */ 
    if ((time->minutes + 60 * time->hours) >= (timestamp->minutes + 60 * timestamp->hours))
    {
        if (!timestamp->is_handled)
        {
            timestamp->is_handled = true;
            return true;
        }
    }
    else
        timestamp->is_handled = false;

    return false;
#endif
}

static const uint8_t days[] = {31,28,31,30,31,30,31,31,30,31,30,31};

static bool is_leap_year(uint8_t year)
{
    year += 2000;
    return ((year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0)));
}

static uint8_t days_in_month(uint8_t m, uint8_t y)
{
    if (m == 2 && is_leap_year(y)) 
        return 29;

    if (m >= 1 && m <= 12) 
        return days[m-1];
    
    return 31;
}

static void shift_time(event_set_time_req_data_t *t, int8_t tz)
{
    int8_t h = t->hours + tz;
    int8_t d = t->date; 
    int8_t m = t->month; 
    int8_t y = t->year;

    if (h < 0)
    {
        h += 24;
        d--;
    }
    else if (h >= 24)
    {
        h -= 24;
        d++;
    }

    t->hours = h;

    uint8_t dim = days_in_month(m, y);

    if (d < 1)
    {
        if (--m < 1)
        {
            m = 12; 
            // y = (y ? y - 1 : 99); 
        }

        d = days_in_month(m, y);
    }
    else if (d > dim)
    {
        d = 1;
        
        if (++m > 12)
        {
            m = 1; 
            // y = (y < 99 ? y + 1 : 0);
        }
    }

    t->date = d;
    t->month = m;
    // t->year = y;
}

//------------------------------------------------------------------------------

bool clock_manager_init(void)
{
    /* Sync time every startup */
    event_set(EVENT_SYNC_TIME_REQ);

    hal_get_alarm(&ctx.alarm);
    hal_get_timezone(&ctx.timezone);

    return true;
}

void clock_manager_process(void)
{
    if (event_get() & EVENT_SET_TIME_REQ)
    {
        event_set_time_req_data_t *time = event_get_data(EVENT_SET_TIME_REQ);

        /* Set time request without set time zone request means DCF77 sync request - shift from UTC+01 needed */
        if (!(event_get() & EVENT_SET_TIMEZONE_REQ))
            shift_time(time, ctx.timezone - CLOCK_MANAGER_DCF77_TIME_ZONE); 

        hal_set_time(event_get_data(EVENT_SET_TIME_REQ));

        event_clear(EVENT_SET_TIME_REQ);
    }

    if (event_get() & EVENT_SET_TIMEZONE_REQ)
    {
        hal_set_timezone(event_get_data(EVENT_SET_TIMEZONE_REQ));
        hal_get_timezone(&ctx.timezone);

        event_clear(EVENT_SET_TIMEZONE_REQ);
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

        if (timestamp_is_reached(time, &CLOCK_MANAGER_SYNC_TIMESTAMP))
            event_set(EVENT_SYNC_TIME_REQ);

        if (ctx.alarm.is_enabled && timestamp_is_reached(time, &ctx.alarm))
            event_set(EVENT_ALARM_REQ);

        ctx.new_sec = false;
    }
}

//------------------------------------------------------------------------------
