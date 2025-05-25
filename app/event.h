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

#ifndef EVENT_H_
#define EVENT_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdint.h>

#include <hal.h> /* Do not duplicate struct ds1307_time and hal_timestamp (in this case flash consumption reduction > clean code) */

//------------------------------------------------------------------------------

enum event_type
{
    EVENT_SYNC_TIME_REQ = 1 << 0,
    EVENT_SYNC_TIME_STATUS = 1 << 1,
    EVENT_UPDATE_TIME_REQ = 1 << 2, 
    EVENT_SET_TIME_REQ = 1 << 3,
    EVENT_SET_TIMEZONE_REQ = 1 << 4, 
    EVENT_SET_ALARM_REQ = 1 << 5,
    EVENT_ALARM_REQ = 1 << 6,
    EVENT_SEND_TIME_INFO_REQ = 1 << 7,
};

enum event_sync_time_status
{
    EVENT_SYNC_TIME_STATUS_WAITING,
    EVENT_SYNC_TIME_STATUS_FRAME_STARTED,
    EVENT_SYNC_TIME_STATUS_ERROR,
    EVENT_SYNC_TIME_STATUS_SYNCED,
};

struct event_sync_time_status_data
{
    uint8_t triggred_on_bit;
    uint8_t bit_number;
    uint16_t time_ms;
    uint8_t dcf_output;
    enum event_sync_time_status status;
};

typedef struct event_sync_time_status_data event_sync_time_status_data_t;
typedef struct ds1307_time event_update_time_req_data_t;
typedef struct ds1307_time event_set_time_req_data_t;
typedef int8_t event_set_timezone_req_data_t; 
typedef struct hal_timestamp event_set_alarm_req_data_t;
typedef struct ds1307_time event_send_time_req_data_t;

//------------------------------------------------------------------------------

/// @brief Sets event in the event buffer
/// @param event event to set
void event_set(enum event_type event);

/// @brief Gets event buffer
/// @return events from the event buffer
enum event_type event_get(void);    

/// @brief Gets event data
/// @param event event to get data for
/// @return pointer to event data
void *event_get_data(enum event_type event);

/// @brief Clears event in the event buffer
/// @param event event to clear
void event_clear(enum event_type event);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* EVENT_H_ */

//------------------------------------------------------------------------------
