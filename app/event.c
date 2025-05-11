//------------------------------------------------------------------------------

/// @file event.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "event.h"

#include <stdint.h>
#include <stddef.h>

#include <hal.h> /* Flash consumption reduction - do not duplicate structure types */

//------------------------------------------------------------------------------

// Events:
// * SYNC_TIME_REQ (UM --> RM)
// * SYNC_TIME_STATUS + data (RM --> UM)
// * SET_TIME_REQ + data (UM / RM --> CM)
// * SET_ALARM_REQ + data (UM --> CM)
// * ALARM_REQ (CM --> UM)
// * TIME_UPDATE_REQ (CM --> UM)

static uint8_t event_buf;

static struct event_sync_time_status_data sync_time_status_data;

static struct ds1307_time time_alarm_data;

//------------------------------------------------------------------------------

void event_set(enum event_type event)
{
    event_buf |= event; // Set the event in the event buffer
}

enum event_type event_get(void)
{
    return event_buf; // Return the current event buffer
}

void *event_get_data(enum event_type event)
{
    if (event == EVENT_SYNC_TIME_STATUS)    
        return &sync_time_status_data;

    if (event == EVENT_SET_TIME_REQ)
        return &time_alarm_data;

    if (event == EVENT_SET_ALARM_REQ)
        return &time_alarm_data;

    if (event == EVENT_TIME_UPDATE_REQ)
        return &time_alarm_data;

    return NULL; // No data available for the event
}

void event_clear(enum event_type event)
{
    event_buf &= ~event; // Clear the event in the event buffer
}

//------------------------------------------------------------------------------
