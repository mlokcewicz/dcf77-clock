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
        [LCD_RS] = PC4,
        [LCD_RW] = PC5,
        [LCD_E] = PB7,
        [LCD_D4] = PC0,
        [LCD_D5] = PC1,
        [LCD_D6] = PC2,
        [LCD_D7] = PC3,
    };

    if (pin == LCD_E)
    {
        if (state)
            PORTB |= (1 << pin_to_pin_val[pin]);
        else
            PORTB &= ~(1 << pin_to_pin_val[pin]);

        return;
    }

    if (state)
        PORTC |= (1 << pin_to_pin_val[pin]);
    else
        PORTC &= ~(1 << pin_to_pin_val[pin]);
}

static void lcd_delay_cb(uint32_t us) 
{
    for (uint32_t i = us; i > 0; i--)
        _delay_us(1);
}

static void lcd_pin_init_cb(void)
{
    DDRC = 0xFF;
    DDRB |= 1 << PB7;
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
    uint8_t minutes_tens : 4;
    uint8_t minutes_parity : 1;
    uint8_t hours_units : 4;
    uint8_t hours_tens : 4;
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
    static bool rising_edge = false;

    if (rising_edge)
        TCCR1B |= (1 << ICES1);
    else
        TCCR1B &= ~(1 << ICES1);

    rising_edge ^= 1;

    TCNT1 = 0;

    dcf77_decode(icr, rising_edge);
};

static void timer1_ovr_cb(void)
{
    uint16_t tcnt = timer_get_val(&timer1_obj);
    
    char buf[10] = {0};
    utoa(tcnt, buf, 10);

    // sprintf(buf, "%u", tcnt);
    hd44780_set_pos(&lcd_obj, 1, 0);
    hd44780_print(&lcd_obj,"          ");
    hd44780_set_pos(&lcd_obj, 1, 0);

    hd44780_print(&lcd_obj, "OVR ");
    hd44780_print(&lcd_obj, buf);
}

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
    return (ms > min) && (ms < max);
}

enum dcf77_bit_val
{
    DCF77_BIT_VAL_0,
    DCF77_BIT_VAL_1,
    DCF77_BIT_VAL_NONE,
    DCF77_BIT_VAL_ERROR,
};

static enum dcf77_bit_val get_bit_val(uint16_t ms)
{
    if (is_in_range(ms, 40, 130))
        return DCF77_BIT_VAL_0;
    if (is_in_range(ms, 140, 250))
        return DCF77_BIT_VAL_1;
    if (is_in_range(ms, 1600, 2200))
        return DCF77_BIT_VAL_NONE;

    return DCF77_BIT_VAL_ERROR;
}

static void dcf77_decode(uint16_t ticks, bool rising_edge)
{
    static bool frame_started = false;
    static uint64_t frame = 0;
    static uint64_t bit_cnt = 0;


    ticks = tick_to_ms(ticks, 256);
    char buf[10] = {0};
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
    }

    enum dcf77_bit_val val = 0;

    if (rising_edge) // Time shold be bit
    {
        val = get_bit_val(ticks);

        if (val == DCF77_BIT_VAL_ERROR || DCF77_BIT_VAL_NONE)
        {
            hd44780_set_pos(&lcd_obj, 1, 0);
            hd44780_print(&lcd_obj, "ERROR          ");
            frame = 0;
            frame_started = 0;
            bit_cnt = 0;
            return;
        }

        frame = (frame << 1) | val;
        bit_cnt++;

        if (bit_cnt > 36)
        {
            struct dcf77_frame *frame_ptr = (struct dcf77_frame*)&frame;

            uint8_t minutes = frame_ptr->minutes_units + 10 * frame_ptr->minutes_tens;
            uint8_t hours = frame_ptr->hours_units + 10 * frame_ptr->hours_tens;

            sprintf(buf, "%u:%u OK", hours, minutes);
            hd44780_set_pos(&lcd_obj, 1, 0);
            hd44780_print(&lcd_obj, buf);
            cli();
            return;
        }

    }
    else // should be break
    {
        //TODO
    }

}

int main()
{
    DDRD |= (1 << PD0);
    PORTD &= ~(1 << PD0);

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
        // PORTD ^= (1 << PD0);

        if (PINB & (1 << PB0))
            PORTD |= 1 << PD0;
        else 
            PORTD &= ~(1 << PD0);

        // _delay_ms(1000);
    }
}

//------------------------------------------------------------------------------
