//------------------------------------------------------------------------------

/// @file communication_manager.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "communication_manager.h"

#include <event.h>

#include <hal.h>

//------------------------------------------------------------------------------

bool communication_manager_init(void)
{
    return true;
}

void communication_manager_process(void)
{
    if (event_get() & EVENT_SEND_TIME_INFO_REQ)
    {
        event_send_time_req_data_t *time = event_get_data(EVENT_SEND_TIME_INFO_REQ);

        hal_send_time_info(time);

        event_clear(EVENT_SEND_TIME_INFO_REQ);
    }
}

//------------------------------------------------------------------------------
