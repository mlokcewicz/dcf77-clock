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
