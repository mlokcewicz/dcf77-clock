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
    uint8_t bit_number;
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
    
    if (ctx.decoder_status == DCF77_DECODER_STATUS_BIT_RECEIVED)
        ctx.bit_number++;
    else if (ctx.decoder_status != DCF77_DECODER_STATUS_BREAK_RECEIVED)
        ctx.bit_number = 0;
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
        
        sync_time_status_data->bit_number = ctx.bit_number;
        sync_time_status_data->triggred_on_bit = ctx.triggered_on_bit;
        sync_time_status_data->time_ms = ctx.last_time_ms;
        sync_time_status_data->dcf_output = hal_dcf_get_state();

        if (ctx.decoder_status == DCF77_DECODER_STATUS_WAITING)
            sync_time_status_data->status = EVENT_SYNC_TIME_STATUS_WAITING; 
        else if (ctx.decoder_status == DCF77_DECODER_STATUS_FRAME_STARTED)
            sync_time_status_data->status = EVENT_SYNC_TIME_STATUS_FRAME_STARTED;
        else if (ctx.decoder_status == DCF77_DECODER_STATUS_ERROR)
            sync_time_status_data->status = EVENT_SYNC_TIME_STATUS_ERROR;  

        event_set(EVENT_SYNC_TIME_STATUS);

        if (ctx.decoder_status == DCF77_DECODER_STATUS_FRAME_STARTED && ctx.prev_decoder_status == DCF77_DECODER_STATUS_SYNCED)
        {
            sync_time_status_data->dcf_output = true;
            sync_time_status_data->status = EVENT_SYNC_TIME_STATUS_SYNCED;
            
            event_set_time_req_data_t *set_time_req_data = event_get_data(EVENT_SET_TIME_REQ);
            
            uint8_t *dcf_frame = dcf77_get_frame();
            
            set_time_req_data->seconds = 0;
            set_time_req_data->minutes = 10 * DCF77_DECODER_FRAME_GET_MINUTES_TENS(dcf_frame) + DCF77_DECODER_FRAME_GET_MINUTES_UNITS(dcf_frame);
            set_time_req_data->hours = 10 * DCF77_DECODER_FRAME_GET_HOURS_TENS(dcf_frame) + DCF77_DECODER_FRAME_GET_HOURS_UNITS(dcf_frame);
            set_time_req_data->date = 10 * DCF77_DECODER_FRAME_GET_DAY_TENS(dcf_frame) + DCF77_DECODER_FRAME_GET_DAY_UNITS(dcf_frame);
            set_time_req_data->month = 10 * DCF77_DECODER_FRAME_GET_MONTH_TENS(dcf_frame) + DCF77_DECODER_FRAME_GET_MONTH_UNITS(dcf_frame);
            set_time_req_data->year = 10 * DCF77_DECODER_FRAME_GET_YEAR_TENS(dcf_frame) + DCF77_DECODER_FRAME_GET_YEAR_UNITS(dcf_frame);

            event_set(EVENT_SET_TIME_REQ);

            hal_dcf_power_down(true);

            ctx.synced = true;
        }

        ctx.prev_triggered_on_bit = ctx.triggered_on_bit;
    }
}

//------------------------------------------------------------------------------
