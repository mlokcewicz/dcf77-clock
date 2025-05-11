//------------------------------------------------------------------------------

/// @file clock_manager.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "clock_manager.h"

#include <event.h>

#include <hal.h>

//------------------------------------------------------------------------------

/* From Radio Manager */
extern bool synced;

/* From UI Manager */
extern void print_time(uint8_t line);

/* RTC */
bool new_sec = false;

void hal_exti_sqw_cb(void)
{
    new_sec = true;
}

// struct ds1307_time unix_time = 
// {
//     .clock_halt = 0,
//     .hour_mode = 0,
//     .seconds_units = 2,
//     .seconds_tens = 3,
//     .minutes_units = 5,
//     .minutes_tens = 4,
//     .hours_units = 8,
//     .hours_tens = 1,
//     .date_units = 1,
//     .date_tens = 0,
//     .day = 5,
//     .month_units = 5,
//     .month_tens = 0,
//     .year_units = 5,
//     .year_tens = 2,
// };

//------------------------------------------------------------------------------

bool clock_manager_init(void)
{
    // set_system_time(1744458473);

    if (hal_time_is_reset())
        synced = false;

    return true;
}

void clock_manager_process(void)
{
    if (new_sec)
    {
        hal_get_time(event_get_data(EVENT_TIME_UPDATE_REQ));
        event_set(EVENT_TIME_UPDATE_REQ);
        // print_time(1);

        // struct event_time_alarm_data *time_update_req_data = event_get_data(EVENT_TIME_UPDATE_REQ);
        // time_update_req_data->seconds_units = unix_time.seconds_units;
        // time_update_req_data->seconds_tens = unix_time.seconds_tens;
        // time_update_req_data->minutes_units = unix_time.minutes_units;
        // time_update_req_data->minutes_tens = unix_time.minutes_tens;
        // time_update_req_data->hours_units = unix_time.hours_units;
        // time_update_req_data->hours_tens = unix_time.hours_tens;
        // time_update_req_data->hour_mode = unix_time.hour_mode;
        // time_update_req_data->day = unix_time.day;
        // time_update_req_data->date_units = unix_time.date_units;
        // time_update_req_data->date_tens = unix_time.date_tens;
        // time_update_req_data->month_units = unix_time.month_units;
        // time_update_req_data->month_tens = unix_time.month_tens;
        // time_update_req_data->year_units = unix_time.year_units;
        // time_update_req_data->year_tens = unix_time.year_tens;
        
        // event_set(EVENT_TIME_UPDATE_REQ);

        new_sec = false;
    }

    if (event_get() & EVENT_SET_TIME_REQ)
    {
        // struct ds1307_time *set_time_req_data = event_get_data(EVENT_SET_TIME_REQ);

        // unix_time.seconds_units = set_time_req_data->seconds_units;
        // unix_time.seconds_tens = set_time_req_data->seconds_tens;
        // unix_time.minutes_units = set_time_req_data->minutes_units;
        // unix_time.minutes_tens = set_time_req_data->minutes_tens;
        // unix_time.hours_units = set_time_req_data->hours_units;
        // unix_time.hours_tens = set_time_req_data->hours_tens;
        // unix_time.hour_mode = set_time_req_data->hour_mode;
        // unix_time.day = set_time_req_data->day;
        // unix_time.date_units = set_time_req_data->date_units;
        // unix_time.date_tens = set_time_req_data->date_tens;
        // unix_time.month_units = set_time_req_data->month_units;
        // unix_time.month_tens = set_time_req_data->month_tens;
        // unix_time.year_units = set_time_req_data->year_units;
        // unix_time.year_tens = set_time_req_data->year_tens;

        hal_set_time(event_get_data(EVENT_SET_TIME_REQ));

        event_clear(EVENT_SET_TIME_REQ);

        // hal_set_system_timer(0);
    }

    // uint8_t *str = "DUPA";
    // ds1307_save_to_ram(&rtc_obj, 5, str, 4);
    // char buf2[10] = {0};
    // ds1307_read_from_ram(&rtc_obj, 4, buf2, 4);
    // hd44780_set_pos(&lcd_obj, 0, 8);
    // hd44780_print(&lcd_obj, buf2);
    // _delay_ms(5000);

    // hd44780_set_pos(&lcd_obj, 1, 0);
    // char buff[16] = {0};
    // hd44780_print(&lcd_obj, ltoa(system_timer_get(), buff, 10));

    // uint32_t unix_time = time(NULL);
    // hd44780_set_pos(&lcd_obj, 1, 0);
    // hd44780_print(&lcd_obj, ctime(&unix_time) + 4);
}

//------------------------------------------------------------------------------
