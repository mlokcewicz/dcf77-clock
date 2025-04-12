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

/* BUZZER */

void buzzer_beep(uint16_t frequency_hz, uint16_t duration_ms) 
{
    uint16_t delay_us = 1000000UL / (frequency_hz * 2); 
    uint16_t cycles = (frequency_hz * duration_ms) / 1000;

    DDRB |= (1 << PB3);  

    for (uint16_t i = 0; i < cycles; i++) 
    {
        PORTB |= (1 << PB3);  
        lcd_delay_cb(delay_us);
        PORTB &= ~(1 << PB3); 
        lcd_delay_cb(delay_us);
    }
}

/* ENCODER */

#include <rotary_encoder.h>

bool rot_encoder_get_a_cb(void)
{
    return (PIND & (1 << PD2));
};
bool rot_encoder_get_b_cb(void)
{
    return (PIND & (1 << PD3));
}
void rot_encoder_rotation_cb(enum rotary_encoder_direction dir, int8_t step_cnt)
{
    (void)step_cnt;

    if (dir == ROTARY_ENCODER_DIR_LEFT)
        buzzer_beep(3000, 500);  // 1 kHz, 500 ms
        // hd44780_shift(&lcd_obj, false, false);
    else
        buzzer_beep(1000, 500);  // 1 kHz, 500 ms
        // hd44780_shift(&lcd_obj, false, true);

}

bool rot_encoder_init_cb(void)
{
    DDRD &= ~(1 << PD2); // input 
    DDRD &= ~(1 << PD3); // input 

    return true;
}
bool rot_encoder_deinit_cb(void)
{
    return true;
}

struct rotary_encoder_cfg cf = 
{
    .get_a_cb = rot_encoder_get_a_cb,
    .get_b_cb = rot_encoder_get_b_cb,
    .init_cb = rot_encoder_init_cb,
    .deinit_cb = rot_encoder_deinit_cb,
    .rotation_cb = rot_encoder_rotation_cb,
    .sub_steps_count = 4,
    .irq_cfg = ROTARY_ENCODER_IRQ_CONFIG_NONE,
};

struct rotary_encoder_obj rot;

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

    /* Ignore short pulses (triggered on falling edge) REVERSED */
    if (!rising_edge && (tick_to_ms(icr, 256) < 35))
        return;

    // /* Ignore short pauses (triggered on rising edge) REVERSED */
    // if (rising_edge && (tick_to_ms(icr, 256) < 600))
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

    uint8_t pos = !rising_edge ? 8 : 0;

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

    if (rising_edge) // Bit transmission // REVERSED NOW
    {
        val = get_bit_val(ticks);

        if (val == DCF77_BIT_VAL_ERROR || val == DCF77_BIT_VAL_NONE)
        {
            hd44780_set_pos(&lcd_obj, 1, 0);
            // sprintf(buf, "ERROR: %u           ", ticks);

            hd44780_print(&lcd_obj, "ERROR         ");
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

			// uint8_t hours = 10 * frame_ptr->hours_tens + frame_ptr->hours_units;
			// uint8_t minutes = 10 * frame_ptr->minutes_tens + frame_ptr->minutes_units;
			
			// uint8_t day = 10 * frame_ptr->month_day_tens + frame_ptr->month_days_units;
			// uint8_t month = 10 * frame_ptr->months_tens + frame_ptr->months_units;
			// uint8_t year = 10 * frame_ptr->years_tens + frame_ptr->years_units;

            
            uint8_t i = 0;
            buf[i++] = (frame_ptr->hours_tens + '0');
            buf[i++] = (frame_ptr->hours_units + '0');
            buf[i++] = (':');
            buf[i++] = (frame_ptr->minutes_tens + '0');
            buf[i++] = (frame_ptr->minutes_units + '0');
            buf[i++] = (' ');
            buf[i++] = (frame_ptr->month_day_tens + '0');
            buf[i++] = (frame_ptr->month_days_units + '0');
            buf[i++] = ('.');
            buf[i++] = (frame_ptr->months_tens + '0');
            buf[i++] = (frame_ptr->months_units + '0');
            buf[i++] = ('.');
            buf[i++] = (frame_ptr->years_tens + '0');
            buf[i++] = (frame_ptr->years_units + '0');
            buf[i++] = 0;
            
            // sprintf(buf, "%02u:%02u %02u.%02u.%02u", hours, minutes, day, month, year);
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

#include <twi.h>
#include <avr/interrupt.h>

static struct twi_cfg twi1_cfg = 
{
    .pull_up_en = false,
    .frequency = 100,
    .irq_mode = false,
};

int main()
{
    /* LED */
    DDRD |= (1 << PD6);
    PORTD &= ~(1 << PD6);

    /* DCF */
    DDRB |= (1 << PB1);
    PORTB &= ~(1 << PB1); // SEL

    DDRB &= ~(1 << PB0); // input 
    MCUCR &= ~(1 << PUD);

    /* SW */
    DDRB &= ~(1 << PB2); // input 

    hd44780_init(&lcd_obj, &lcd_cfg);
    hd44780_print(&lcd_obj, "TEST");
    hd44780_set_pos(&lcd_obj, 1, 0);

    timer_init(&timer1_obj, &timer1_cfg);

    timer_start(&timer1_obj, true);

    rotary_encoder_init(&rot, &cf);

    twi_init(&twi1_cfg);
    uint8_t sec_config[] = {0x00, 0x09};
    twi_send(0b11010000, sec_config, 2, true);
    uint8_t sqw_config[] = {0x07, 0b00010000};
    twi_send(0b11010000, sqw_config, 2, true);

    sei();
    
    // set_system_time(1742839880);

    while (1)
    {
        // PORTD ^= (1 << PD6);

        while (!(PINB & (1 << PB2)))
        {
            // PORTD |= 1 << PD6;

            if (!(PINC & (1 << PC2)))
                PORTD |= 1 << PD6;
            else
                PORTD &= ~(1 << PD6);
        }

        if (PINB & (1 << PB0))
            PORTD &= ~(1 << PD6);
        else 
            PORTD |= 1 << PD6;

        rotary_encoder_process(&rot);

        // uint8_t sec_buf = 6;
        // uint8_t sec_addr = 0x00;

        // twi_send(0b11010001, &sec_addr, 1, true);
        // twi_receive(0b11010001, &sec_buf, 1);

        // hd44780_set_pos(&lcd_obj, 1, 15);
        // hd44780_putc(&lcd_obj, (sec_buf & 0x0F) + '0');

        // _delay_ms(1000);
        // system_tick();
        // uint32_t unix_time = time(NULL);
        // hd44780_set_pos(&lcd_obj, 1, 0);
        // hd44780_print(&lcd_obj, ctime(&unix_time) + 4);
    }
}

//------------------------------------------------------------------------------

// Basic threads / processess:
// * radio_manager (waits for request of sync, then enables dcf receiver and TIM1 irq, and set new last_sync timestamp, and waits for sync_finished and disables dcf and TIM1 irq)
// * clock_manager (waits for SQW and send event to UI, wait for time from TIM1 irq or from UI)
// * ui_manager (waits for INT0, PCINT interrupts from encoder / button and driver LCD in thread, waits for SWQ irq to update time, checks for alert)

// Events:
// * **ENC+ (to ui_manager)
// * **ENC- (to ui_manager)
// * **SW (to ui_manager)
// * **TIM (to radio_manager/clock_manager)
// * **SEC (to clock_manager/ui_manager)
// * Sync REQ (from ui_manager and radio_manager) (to radio_manager)
// * Clock UPDT REQ (from ui_manager and radio_manager) (to clock_manager and ui_manager)
// * Alarm (from clock_manager) (to ui_manager)
