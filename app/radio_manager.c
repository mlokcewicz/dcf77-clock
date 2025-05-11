//------------------------------------------------------------------------------

/// @file radio_manager.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "radio_manager.h"

#include <stddef.h>

#include <event.h>

#include <simple_stdio.h>
#include <dcf77_decoder.h>

#include <hal.h>

//------------------------------------------------------------------------------

bool synced = false;

static enum dcf77_decoder_status decoder_status = DCF77_DECODER_STATUS_WAITING;
static enum dcf77_decoder_status prev_decoder_status = DCF77_DECODER_STATUS_WAITING;
static bool last_triggered_on_bit;
static bool prev_triggered_on_bit = false;
static uint16_t last_time_ms;

void hal_dcf_cb(uint16_t ms, bool rising_edge)
{
    if (synced)
        return;

    last_triggered_on_bit = rising_edge;
    last_time_ms = ms;

    prev_decoder_status = decoder_status;

    decoder_status = dcf77_decode(ms, rising_edge); 
};

//------------------------------------------------------------------------------

bool radio_manager_init(void)
{
    return true;
}

void radio_manager_process(void)
{
    static bool dcf_prev_val = false;

    bool dcf_val = hal_dcf_get_state();

    if (dcf_val != dcf_prev_val)
    {
        hal_led_set(!dcf_val);
        dcf_prev_val = dcf_val;
    }

    if (prev_triggered_on_bit != last_triggered_on_bit)
    {
        struct event_sync_time_status_data *sync_time_status_data = event_get_data(EVENT_SYNC_TIME_STATUS);
        sync_time_status_data->rising_edge = last_triggered_on_bit;
        sync_time_status_data->time_ms = last_time_ms;
        sync_time_status_data->frame_started = (decoder_status == DCF77_DECODER_STATUS_FRAME_STARTED);
        sync_time_status_data->error = (decoder_status == DCF77_DECODER_STATUS_ERROR);
        
        event_set(EVENT_SYNC_TIME_STATUS);

        if (decoder_status == DCF77_DECODER_STATUS_FRAME_STARTED && prev_decoder_status == DCF77_DECODER_STATUS_SYNCED && !synced)
        {
            struct dcf77_frame *dcf_frame = dcf77_get_frame();
            
            struct ds1307_time *set_time_req_data = event_get_data(EVENT_SET_TIME_REQ);

            set_time_req_data->seconds_units = 0;
            set_time_req_data->seconds_tens = 0;    
            set_time_req_data->minutes_units = dcf_frame->minutes_units;
            set_time_req_data->minutes_tens = dcf_frame->minutes_tens;
            set_time_req_data->hours_units = dcf_frame->hours_units;
            set_time_req_data->hours_tens = dcf_frame->hours_tens;
            set_time_req_data->hour_mode = 0;
            set_time_req_data->date_units = dcf_frame->month_day_units;
            set_time_req_data->date_tens = dcf_frame->month_day_tens;
            set_time_req_data->month_units = dcf_frame->months_units;
            set_time_req_data->month_tens = dcf_frame->months_tens;
            set_time_req_data->year_units = dcf_frame->years_units;
            set_time_req_data->year_tens = dcf_frame->years_tens;
            set_time_req_data->clock_halt = 0;

            event_set(EVENT_SET_TIME_REQ);

            synced = true;
        }

        prev_triggered_on_bit = last_triggered_on_bit;
    }
}

//------------------------------------------------------------------------------
