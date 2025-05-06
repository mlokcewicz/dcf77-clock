//------------------------------------------------------------------------------

/// @file radio_manager.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "radio_manager.h"

#include "stddef.h"

//------------------------------------------------------------------------------

#include <gpio.h>

/* MAS6181B */

#include <mas6181b.h>
#include <timer.h>
#include <dcf77_decoder.h>

#include <ds1307.h>
#include <hd44780.h>

#include <avr/io.h>

extern struct hd44780_obj lcd_obj;
extern struct ds1307_obj rtc_obj;
extern struct ds1307_time unix_time;

bool synced = false;
bool new_sec = false;



/* DCF77 */

#include <dcf77_decoder.h>

static void timer1_capt_cb(uint16_t icr);

static struct timer_cfg timer1_cfg = 
{  
    .id = TIMER_ID_1,
    .clock = TIMER_CLOCK_PRESC_256, 
    .async_clock = TIMER_ASYNC_CLOCK_DISABLED,
    .mode = TIMER_MODE_16_BIT_NORMAL,
    .com_a_cfg = TIMER_CM_DISABLED,
    .com_b_cfg = TIMER_CM_DISABLED,

    .counter_val = 0,
    .ovrfv_cb = NULL,

    .out_comp_a_val = 0,
    .out_comp_b_val = 0,
    .out_comp_a_cb = NULL,
    .out_comp_b_cb = NULL,
    
    .input_capture_val = 0,
    .input_capture_pullup = false,
    .input_capture_noise_canceler = false,
    .input_capture_rising_edge = false,
    .in_capt_cb = timer1_capt_cb,
};

static struct timer_obj timer1_obj;

static enum dcf77_decoder_status decoder_status = DCF77_DECODER_STATUS_WAITING;
static enum dcf77_decoder_status prev_decoder_status = DCF77_DECODER_STATUS_WAITING;
static bool last_triggered_on_bit;
static uint16_t last_time_ms;

#define PRESC 256
#define TICKS_TO_MS(ticks, presc) ((uint32_t)ticks * 1000UL / (F_CPU / presc)) // out

static void timer1_capt_cb(uint16_t icr)
{
    if (synced)
        return;

    /* Now trigger on RISING edge is on BIT edge */

    bool rising_edge = (TCCR1B & (1 << ICES1)); 
    
    TCCR1B ^= (1 << ICES1); // Change trigger edge
    TCNT1 = 0;

    last_triggered_on_bit = rising_edge;
    last_time_ms = TICKS_TO_MS(icr, PRESC);

    prev_decoder_status = decoder_status;

    decoder_status = dcf77_decode(TICKS_TO_MS(icr, PRESC), rising_edge); 
};

static char buf[16]; 
static void print_time(uint8_t line)
{
    uint8_t i = 0;
    buf[i++] = (unix_time.hours_tens + '0');
    buf[i++] = (unix_time.hours_units + '0');
    buf[i++] = (':');
    buf[i++] = (unix_time.minutes_tens + '0');
    buf[i++] = (unix_time.minutes_units + '0');
    buf[i++] = (' ');
    buf[i++] = (unix_time.date_tens + '0');
    buf[i++] = (unix_time.date_units + '0');
    buf[i++] = ('.');
    buf[i++] = (unix_time.month_tens + '0');
    buf[i++] = (unix_time.month_units + '0');
    buf[i++] = (' ');
    buf[i++] = (unix_time.seconds_tens + '0');
    buf[i++] = (unix_time.seconds_units + '0');
    buf[i++] = 0;
    hd44780_set_pos(&lcd_obj, line, 0);
    hd44780_print(&lcd_obj, buf);
}

static void uint16_to_str(uint16_t value, char *buffer) 
{
    uint8_t i = 0;
    do 
    {
        buffer[i++] = (value % 10) + '0';
        value /= 10;
    } while (value > 0);
    
    buffer[i] = '\0';

    /* Reverse the string */
    for (uint8_t j = 0; j < i / 2; j++) 
    {
        char temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }
}

static void mas6181b1_io_init_cb(void)
{
    gpio_init(GPIO_PORT_B, GPIO_PIN_1, true, false);
    gpio_init(GPIO_PORT_B, GPIO_PIN_0, false, false);

    timer_init(&timer1_obj, &timer1_cfg);
    timer_start(&timer1_obj, true);
}

static void mas6181b1_pwr_down_cb(bool pwr_down)
{
    gpio_set(GPIO_PORT_B, GPIO_PIN_1, pwr_down);
}

static struct mas6181b_cfg mas6181b1_cfg = 
{
    .io_init = mas6181b1_io_init_cb,
    .pwr_down = mas6181b1_pwr_down_cb,
};

static struct mas6181b_obj mas6181b1_obj;

//------------------------------------------------------------------------------


bool radio_manager_init(void)
{
    /* DCF */
    mas6181b_init(&mas6181b1_obj, &mas6181b1_cfg);


    return true;
}

void radio_manager_process(void)
{
    static bool dcf_prev_val = false;

    bool dcf_val = gpio_get(GPIO_PORT_B, GPIO_PIN_0);

    if (dcf_val != dcf_prev_val)
    {
        gpio_set(GPIO_PORT_D, GPIO_PIN_6, !dcf_val);
        dcf_prev_val = dcf_val;
    }

    if (new_sec)
    {
        ds1307_get_time(&rtc_obj, &unix_time);
        print_time(1);
        new_sec = false;
    }

    static bool prev_triggered_on_bit = false;

    if (prev_triggered_on_bit != last_triggered_on_bit)
    {
        uint16_to_str(last_time_ms, buf);

        uint8_t pos = last_triggered_on_bit ? 0 : 8;

        hd44780_set_pos(&lcd_obj, 0, pos);
        hd44780_print(&lcd_obj, "       ");
        hd44780_set_pos(&lcd_obj, 0, pos);
        hd44780_print(&lcd_obj, buf);

        if (decoder_status == DCF77_DECODER_STATUS_FRAME_STARTED)
        {
            hd44780_set_pos(&lcd_obj, 0, 15);
            hd44780_print(&lcd_obj, "S");
        }
        else if (decoder_status == DCF77_DECODER_STATUS_ERROR)
        {
            hd44780_set_pos(&lcd_obj, 0, 15);
            hd44780_print(&lcd_obj, "E");
        }
        if (decoder_status == DCF77_DECODER_STATUS_FRAME_STARTED && prev_decoder_status == DCF77_DECODER_STATUS_SYNCED && !synced)
        {
            struct dcf77_frame *dcf_frame = dcf77_get_frame();

            unix_time.clock_halt = 0;
            unix_time.hour_mode = 0;
            unix_time.hours_tens = dcf_frame->hours_tens;
            unix_time.hours_units = dcf_frame->hours_units;
            unix_time.minutes_tens = dcf_frame->minutes_tens;
            unix_time.minutes_units = dcf_frame->minutes_units;
            unix_time.date_tens = dcf_frame->month_day_tens;
            unix_time.date_units = dcf_frame->month_day_units;
            unix_time.month_tens = dcf_frame->months_tens;
            unix_time.month_units = dcf_frame->months_units;
            unix_time.year_tens = dcf_frame->years_tens;
            unix_time.year_units = dcf_frame->years_units;
            unix_time.seconds_tens = 0;
            unix_time.seconds_units = 0;

            ds1307_set_time(&rtc_obj, &unix_time);

            print_time(0);

            synced = true;
        }

        prev_triggered_on_bit = last_triggered_on_bit;
    }
}

//------------------------------------------------------------------------------
