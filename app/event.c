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

#include "event.h"

#include <stdint.h>
#include <stddef.h>

//------------------------------------------------------------------------------

struct event_ctx
{
    uint8_t event_buf;
    event_sync_time_status_data_t sync_time_status_data;
    event_update_time_req_data_t update_time_data;
    event_set_time_req_data_t set_time_data;
    event_set_alarm_req_data_t set_alarm_data;
    event_set_timezone_req_data_t timezone_buf;
};

static struct event_ctx ctx;

//------------------------------------------------------------------------------

void event_set(enum event_type event)
{
    ctx.event_buf |= event; // Set the event in the event buffer
}

enum event_type event_get(void)
{
    return ctx.event_buf; // Return the current event buffer
}

void *event_get_data(enum event_type event)
{
    if (event == EVENT_SYNC_TIME_STATUS)    
        return &ctx.sync_time_status_data;

    if (event == EVENT_UPDATE_TIME_REQ)
        return &ctx.update_time_data;

    if (event == EVENT_SET_TIME_REQ)
        return &ctx.set_time_data;

    if (event == EVENT_SET_TIMEZONE_REQ)
        return &ctx.timezone_buf;

    if (event == EVENT_SET_ALARM_REQ)
        return &ctx.set_alarm_data;

    if (event == EVENT_SEND_TIME_INFO_REQ)
        return &ctx.update_time_data;

    return NULL; // No data available for the event
}

void event_clear(enum event_type event)
{
    ctx.event_buf &= ~event; // Clear the event in the event buffer
}

//------------------------------------------------------------------------------
