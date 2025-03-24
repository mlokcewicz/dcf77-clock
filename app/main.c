//------------------------------------------------------------------------------

/// @file main.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include <stdio.h>

#include <util/delay.h>
#include <avr/io.h>
#include <avr/fuse.h>
#include <avr/interrupt.h>

//------------------------------------------------------------------------------

/* Fuse and lock bits are active-low so to program given bit use & operator */

LOCKBITS = 0xFF;

FUSES =
{
    .low = LFUSE_DEFAULT,
    .high = HFUSE_DEFAULT,
    .extended = EFUSE_DEFAULT,
};

//------------------------------------------------------------------------------

/* Non implemented ISR handling */

ISR(BADISR_vect)
{
    /* Add error handling */
}

//------------------------------------------------------------------------------

#include <hd44780.h>

/* LCD */
static void lcd_set_pin_cb(uint8_t pin, bool state) 
{
    int pin_to_pin_val[] = 
    {
        [LCD_RS] = PC3,
        [LCD_E] = PD4,
        [LCD_D4] = PB6,
        [LCD_D5] = PB7,
        [LCD_D6] = PD7,
        [LCD_D7] = PD5,
    };

    if (pin == LCD_RW)
        return;

    if (pin == LCD_RS)
    {
        if (state)
            PORTC |= (1 << pin_to_pin_val[pin]);
        else
            PORTC &= ~(1 << pin_to_pin_val[pin]);

        return;
    }
    
    if ((pin == LCD_D4) || (pin == LCD_D5))
    {
        if (state)
            PORTB |= (1 << pin_to_pin_val[pin]);
        else
            PORTB &= ~(1 << pin_to_pin_val[pin]);

        return;
    }

    if (state)
        PORTD |= (1 << pin_to_pin_val[pin]);
    else
        PORTD &= ~(1 << pin_to_pin_val[pin]);
}

static void lcd_delay_cb(uint32_t us) 
{
    for (uint32_t i = us; i > 0; i--)
        _delay_us(1);
}

static void lcd_pin_init_cb(void)
{

    DDRC |= 1 << PC3;
    DDRB |= (1 << PB6) | (1 << PB7);
    DDRD |= (1 << PD4) | (1 << PD5) | (1 << PD7);
}

static void lcd_pin_deinit_cb(void)
{
    
}

static struct hd44780_cfg lcd_cfg = 
{
    .set_pin_state = lcd_set_pin_cb,
    .delay_us = lcd_delay_cb,
    .pin_init = lcd_pin_init_cb,
    .pin_deinit = lcd_pin_deinit_cb,
};

static struct hd44780_obj lcd_obj;

/* DCF77 */
#include <timer.h>
#include <stdlib.h>

static void dcf77_decode(uint16_t ticks, bool rising_edge);

struct dcf77_frame
{
    uint8_t frame_start_always_zero : 1;
    uint16_t weather_info : 14;
    uint8_t auxiliary_antenna : 1;
    uint8_t time_change_announcement : 1;
    uint8_t winter_time : 2;
    uint8_t leap_second : 1;
    uint8_t time_start_always_zero : 1;
    uint8_t minutes_units : 4;
    uint8_t minutes_tens : 3;
    uint8_t minutes_parity : 1;
    uint8_t hours_units : 4;
    uint8_t hours_tens : 2;
    uint8_t hours_parity : 1;
    uint8_t month_days_units : 4;
    uint8_t month_day_tens : 2;
    uint8_t weekday : 3;
    uint8_t months_units : 4;
    uint8_t months_tens : 1;
    uint8_t years_units : 4;
    uint8_t years_tens: 4;
    uint8_t date_parity: 1;
} __attribute__((__packed__));

struct timer_obj timer1_obj;

static uint64_t tick_to_ms(uint16_t ticks, uint16_t presc)
{
    return (uint64_t)ticks * presc * 1000 / (F_CPU);
}

static void timer1_capt_cb(uint16_t icr)
{
    static bool rising_edge = false; // This has to be the same as initialization

    if (rising_edge) // Triggered on rising edge, break was measuered, change to falling
        TCCR1B &= ~(1 << ICES1);
    else // Triggered on falling edge, bit was measured, change to rising
        TCCR1B |= (1 << ICES1); 
    
    rising_edge ^= 1;

    /* Ignore short pulses (triggered on falling edge) */
    if (rising_edge && (tick_to_ms(icr, 256) < 35))
        return;

    // /* Ignore short pauses (triggered on rising edge) */
    // if (!rising_edge && (tick_to_ms(icr, 256) < 600))
    //     return;

    // TODO: Check pauses since last ignored short pulse (static timestapm) or ignore everything for at least 600 ms

    TCNT1 = 0;

    dcf77_decode(icr, !rising_edge); // restore real trigger source edge
};

// static void timer1_ovr_cb(void)
// {
//     uint16_t tcnt = timer_get_val(&timer1_obj);
    
//     char buf[10] = {0};
//     utoa(tcnt, buf, 10);

//     // sprintf(buf, "%u", tcnt);
//     hd44780_set_pos(&lcd_obj, 1, 0);
//     hd44780_print(&lcd_obj,"          ");
//     hd44780_set_pos(&lcd_obj, 1, 0);

//     hd44780_print(&lcd_obj, "OVR ");
//     hd44780_print(&lcd_obj, buf);
// }

struct timer_cfg timer1_cfg = 
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

static bool is_in_range(uint16_t ms, uint16_t min, uint16_t max)
{
    return (ms >= min) && (ms < max);
}

enum dcf77_bit_val
{
    DCF77_BIT_VAL_0,
    DCF77_BIT_VAL_1,
    DCF77_BIT_VAL_NONE,
    DCF77_BIT_VAL_ERROR,
};

#define EXTENDEND_TOLERANCE 1

static enum dcf77_bit_val get_bit_val(uint16_t ms)
{
#if EXTENDEND_TOLERANCE

    if (is_in_range(ms, 40, 140))
        return DCF77_BIT_VAL_0;
    if (is_in_range(ms, 141, 400))
        return DCF77_BIT_VAL_1;
    if (is_in_range(ms, 1500, 2200))
        return DCF77_BIT_VAL_NONE;

    return DCF77_BIT_VAL_ERROR;

#endif

    if (is_in_range(ms, 40, 130))
        return DCF77_BIT_VAL_0;
    if (is_in_range(ms, 140, 250))
        return DCF77_BIT_VAL_1;
    if (is_in_range(ms, 1500, 2200))
        return DCF77_BIT_VAL_NONE;

    return DCF77_BIT_VAL_ERROR;
}

static void dcf77_decode(uint16_t ticks, bool rising_edge)
{
    static bool frame_started = false;
    static uint64_t frame = 0;
    static uint64_t bit_cnt = 0;

    ticks = tick_to_ms(ticks, 256);
    char buf[25] = {0};
    utoa(ticks, buf, 10);

    uint8_t pos = rising_edge ? 8 : 0;

    hd44780_set_pos(&lcd_obj, 0, pos);
    hd44780_print(&lcd_obj,"        ");
    hd44780_set_pos(&lcd_obj, 0, pos);

    hd44780_print(&lcd_obj, buf);

    if (!frame_started)
    {
        if (get_bit_val(ticks) == DCF77_BIT_VAL_NONE)
        {
            frame_started = true;
            hd44780_set_pos(&lcd_obj, 1, 0);
            hd44780_print(&lcd_obj,"          ");
            hd44780_set_pos(&lcd_obj, 1, 0);
            hd44780_print(&lcd_obj, "FRAME STARTED");

            return;
        }
        return;
    }

    enum dcf77_bit_val val = 0;

    if (!rising_edge) // Bit transmission
    {
        val = get_bit_val(ticks);

        if (val == DCF77_BIT_VAL_ERROR || val == DCF77_BIT_VAL_NONE)
        {
            hd44780_set_pos(&lcd_obj, 1, 0);
            sprintf(buf, "ERROR: %u           ", ticks);

            hd44780_print(&lcd_obj, buf);
            frame = 0;
            frame_started = 0;
            bit_cnt = 0;
            return;
        }

        frame |= ((uint64_t)val << bit_cnt); 
        bit_cnt++;

        if (bit_cnt >= 59)
        {
            struct dcf77_frame *frame_ptr = (struct dcf77_frame*)&frame;

			uint8_t hours = 10 * frame_ptr->hours_tens + frame_ptr->hours_units;
			uint8_t minutes = 10 * frame_ptr->minutes_tens + frame_ptr->minutes_units;
			
			uint8_t day = 10 * frame_ptr->month_day_tens + frame_ptr->month_days_units;
			uint8_t month = 10 * frame_ptr->month_day_tens + frame_ptr->months_units;
			uint8_t year = 10 * frame_ptr->years_tens + frame_ptr->years_units;

            sprintf(buf, "%02u:%02u %02u.%02u.%02u", hours, minutes, day, month, year);
            hd44780_set_pos(&lcd_obj, 1, 0);
            hd44780_print(&lcd_obj, buf);
            cli();

            while(1);
            return;
        }

    }
    else // should be break
    {
        //TODO
        // take into account ignoring short pulses - it affects break time
    }

}

/* RTC */

int main()
{
    /* LED */
    DDRD |= (1 << PD6);
    PORTD &= ~(1 << PD6);

    /* DCF */
    DDRB |= (1 << PB1);
    PORTB &= ~(1 << PB1); // SEL

    DDRB &= ~(1 << PB0); // input 
    // PORTB |= (1 << PB0); /* Pull up for EXTI */
    // PORTB &= ~(1 << PB0); /* Pull down for EXTI */
    MCUCR &= ~(1 << PUD);

    hd44780_init(&lcd_obj, &lcd_cfg);
    hd44780_print(&lcd_obj, "TEST");
    hd44780_set_pos(&lcd_obj, 1, 0);

    timer_init(&timer1_obj, &timer1_cfg);

    timer_start(&timer1_obj, true);

    sei();

    while (1)
    {
        // PORTD ^= (1 << PD6);

        if (PINB & (1 << PB0))
            PORTD |= 1 << PD6;
        else 
            PORTD &= ~(1 << PD6);

        // _delay_ms(1000);
    }
}

//------------------------------------------------------------------------------

// DCF77 - PB0 (TIM1 ICP)
// DS1307 - TWI + PCINT
// LCD - dowolnie + podstwietlenie pod PWM timer0
// BUZZER - Timer2
// ENCODER - INT0 i INT1
// BUTTON - PCINT
// LED - dodowlnie
// USART - pod USART
// ISP - pod ISP

// Basic threads / processess:
// * radio_manager (waits for request of sync, then enables dcf receiver and TIM1 irq, and set new last_sync timestamp, and waits for sync_finished and disables dcf and TIM1 irq)
// * clock_manager (waits for SQW and send event to UI, wait for time from TIM1 irq or from UI)
// * ui_manager (waits for INT0, PCINT interrupts from encoder / button and driver LCD in thread, waits for SWQ irq to update time, checks for alert)

// Events:
// * **ENC+ (to ui_manager)
// * **ENC- (to ui_manager)
// * **SW (to ui_manager)
// * **TIM (to clock_manager/radio_manager)
// * **SEC (to clock_manager/ui_manager)
// * Sync REQ (from ui_manager and radio_manager) (to radio_manager)
// * Clock UPDT REQ (from ui_manager and radio_manager) (to clock_manager and ui_manager)
// * Alarm (from clock_manager) (to ui_manager)
