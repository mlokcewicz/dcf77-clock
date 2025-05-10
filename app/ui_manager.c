//------------------------------------------------------------------------------

/// @file ui_manager.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "ui_manager.h"

#include <string.h>

#include <util/delay.h>

#include <hal.h>

//------------------------------------------------------------------------------
 
#include <ds1307.h>
#include <rotary_encoder.h>
extern struct ds1307_time unix_time;
extern struct ds1307_obj rtc_obj;
extern bool synced;

void hal_button1_pressed_cb(void)
{
    hal_lcd_clear();
   
    memset(&unix_time, 0x00, sizeof(unix_time));

    ds1307_set_time(&rtc_obj, &unix_time);

    synced = false;
}

void hal_encoder1_rotation_cb(enum rotary_encoder_direction dir, int8_t step_cnt)
{
    (void)step_cnt;

    if (dir == ROTARY_ENCODER_DIR_LEFT)
    {

    }
    else
    {
        
    }

}


//------------------------------------------------------------------------------

bool ui_manager_init(void)
{
    return true;
}

void ui_manager_process(void)
{

}

//------------------------------------------------------------------------------
