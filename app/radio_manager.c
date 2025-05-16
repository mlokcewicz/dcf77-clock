//------------------------------------------------------------------------------

/// @file radio_manager.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "radio_manager.h"

#include <stddef.h>

#include <event.h>

#include <dcf77_decoder.h>

#include <hal.h>

//------------------------------------------------------------------------------

struct radio_manager_ctx
{
    bool synced;
    enum dcf77_decoder_status decoder_status;
    enum dcf77_decoder_status prev_decoder_status;
    bool triggered_on_bit;
    bool prev_triggered_on_bit;
    uint16_t last_time_ms;
};

static struct radio_manager_ctx ctx;

//------------------------------------------------------------------------------

/* HAL callbacks */

void hal_dcf_cb(uint16_t ms, bool triggred_on_bit)
{
    if (ctx.synced)
        return;

    ctx.triggered_on_bit = triggred_on_bit;
    ctx.last_time_ms = ms;

    ctx.prev_decoder_status = ctx.decoder_status;

    ctx.decoder_status = dcf77_decode(ms, triggred_on_bit); 
};

//------------------------------------------------------------------------------

bool radio_manager_init(void)
{
    ctx.synced = true;
    
    return true;
}

void radio_manager_process(void)
{
    if (event_get() & EVENT_SYNC_TIME_REQ)
    {
        ctx.synced = false;

        hal_dcf_power_down(false);

        event_clear(EVENT_SYNC_TIME_REQ);
    }

    if (!ctx.synced && ctx.prev_triggered_on_bit != ctx.triggered_on_bit)
    {
        event_sync_time_status_data_t *sync_time_status_data = event_get_data(EVENT_SYNC_TIME_STATUS);
        
        sync_time_status_data->triggred_on_bit = ctx.triggered_on_bit;
        sync_time_status_data->time_ms = ctx.last_time_ms;
        sync_time_status_data->frame_started = (ctx.decoder_status == DCF77_DECODER_STATUS_FRAME_STARTED);
        sync_time_status_data->error = (ctx.decoder_status == DCF77_DECODER_STATUS_ERROR);
        sync_time_status_data->dcf_output = hal_dcf_get_state();
        
        event_set(EVENT_SYNC_TIME_STATUS);

        if (ctx.decoder_status == DCF77_DECODER_STATUS_FRAME_STARTED && ctx.prev_decoder_status == DCF77_DECODER_STATUS_SYNCED)
        {
            sync_time_status_data->dcf_output = true;

            struct dcf77_frame *dcf_frame = dcf77_get_frame();
            
            struct ds1307_time *set_time_req_data = event_get_data(EVENT_SET_TIME_REQ);

            // set_time_req_data->clock_halt = 0;
            // set_time_req_data->hour_mode = 0;
            // set_time_req_data->seconds_units = 0;
            // set_time_req_data->seconds_tens = 0;    
            // set_time_req_data->minutes_units = dcf_frame->minutes_units;
            // set_time_req_data->minutes_tens = dcf_frame->minutes_tens;
            // set_time_req_data->hours_units = dcf_frame->hours_units;
            // set_time_req_data->hours_tens = dcf_frame->hours_tens;
            // set_time_req_data->date_units = dcf_frame->month_day_units;
            // set_time_req_data->date_tens = dcf_frame->month_day_tens;
            // set_time_req_data->month_units = dcf_frame->months_units;
            // set_time_req_data->month_tens = dcf_frame->months_tens;
            // set_time_req_data->year_units = dcf_frame->years_units;
            // set_time_req_data->year_tens = dcf_frame->years_tens;

            set_time_req_data->seconds = 0;
            set_time_req_data->minutes = 10 * dcf_frame->minutes_tens + dcf_frame->minutes_units;
            set_time_req_data->hours = 10 * dcf_frame->hours_tens + dcf_frame->hours_units;
            set_time_req_data->date = 10 * dcf_frame->month_day_tens + dcf_frame->month_day_units;
            set_time_req_data->month = 10 * dcf_frame->months_tens + dcf_frame->months_units;
            set_time_req_data->year = 10 * dcf_frame->years_tens + dcf_frame->years_units;

            event_set(EVENT_SET_TIME_REQ);

            hal_dcf_power_down(true);

            ctx.synced = true;
        }

        ctx.prev_triggered_on_bit = ctx.triggered_on_bit;
    }
}

//------------------------------------------------------------------------------
