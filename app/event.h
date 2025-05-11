//------------------------------------------------------------------------------

/// @file event.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef EVENT_H_
#define EVENT_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdint.h>

//------------------------------------------------------------------------------

enum event_type
{
    EVENT_SYNC_TIME_REQ = 1 << 0,
    EVENT_SYNC_TIME_STATUS = 1 << 1,
    EVENT_SET_TIME_REQ = 1 << 2,
    EVENT_SET_ALARM_REQ = 1 << 3,
    EVENT_ALARM_REQ = 1 << 4,
    EVENT_TIME_UPDATE_REQ = 1 << 5, 
};

struct event_sync_time_status_data
{
    uint8_t error : 1;
    uint8_t frame_started : 1;
    uint8_t rising_edge : 1;
    uint16_t time_ms;
}__attribute__((packed));

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
