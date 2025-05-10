//------------------------------------------------------------------------------

/// @file clock_manager.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "clock_manager.h"

//------------------------------------------------------------------------------

/* RTC */

#include <twi.h>
#include <ds1307.h>
#include <exti.h>
#include <gpio.h>   

bool new_sec = false;

void hal_exti_sqw_cb(void)
{
    new_sec = true;
}

//------------------------------------------------------------------------------

bool clock_manager_init(void)
{

    return true;
}

void clock_manager_process(void)
{

}

//------------------------------------------------------------------------------
