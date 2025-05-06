//------------------------------------------------------------------------------

/// @file clock_manager.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "clock_manager.h"

//------------------------------------------------------------------------------

/* RTC */

#include <twi.h>
#include <ds1307.h>
#include <avr/interrupt.h>
#include <exti.h>
#include <gpio.h>   
// #include <time.h>

extern bool new_sec;

static struct twi_cfg twi1_cfg = 
{
    .pull_up_en = false,
    .frequency = 100,
    .irq_mode = false,
};

bool ds1307_io_init_cb1(void)
{
    return twi_init(&twi1_cfg);
}

bool ds1307_serial_send_cb1(uint8_t device_addr, uint8_t *data, uint16_t len)
{
    return twi_send(device_addr, data, len, true);
}

bool ds1307_serial_receive_cb1(uint8_t device_addr, uint8_t *data, uint16_t len)
{
    return twi_receive(device_addr, data, len);
}

static struct ds1307_cfg rtc_cfg = 
{
    .io_init = ds1307_io_init_cb1,
    .serial_send = ds1307_serial_send_cb1,
    .serial_receive = ds1307_serial_receive_cb1,

    .sqw_en = true,
    .rs = DS1307_RS_1HZ,
};

struct ds1307_obj rtc_obj;

static void exti_sqw_cb(void)
{
    if (!gpio_get(GPIO_PORT_C, GPIO_PIN_2))
        new_sec = true;
}

//------------------------------------------------------------------------------

bool clock_manager_init(void)
{

    exti_init(EXTI_ID_PCINT10, EXTI_TRIGGER_CHANGE, exti_sqw_cb);
    exti_enable(EXTI_ID_PCINT10, true);

    ds1307_init(&rtc_obj, &rtc_cfg);



    return true;
}

void clock_manager_process(void)
{

}

//------------------------------------------------------------------------------
