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
    const uint8_t pin_to_pin_val[] = 
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

static void lcd_delay_cb(uint16_t us) 
{
    while (us--)
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

static void buzzer_beep(uint16_t frequency_hz, uint16_t duration_ms) 
{
    // return;
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

/* BUTTON */

#include <button.h>

bool button1_init_cb(void)
{
    /* SW */
    DDRB &= ~(1 << PB2); // input 

    return true;
}

bool button1_get_state_cb(void)
{
    return PINB & (1 << PB2);
}

void button1_pressed_cb(void)
{
    buzzer_beep(6000, 50);
}

bool button1_deinit_cb(void)
{
    return true;
}

static struct button_cfg button1_cfg = 
{
	.init = button1_init_cb,
	.get_state = button1_get_state_cb,
	.pressed = button1_pressed_cb,
	.deinit = button1_deinit_cb,
    
    .active_low = true,
    .irq_cfg = true,

	.debounce_counter_initial_value = 0,
	.autopress_counter_initial_value = 0,
};

static struct button_obj button1_obj;

/* ENCODER */

#include <rotary_encoder.h>

static bool encoder1_get_a_cb(void)
{
    return (PIND & (1 << PD2));
};

static bool encoder1_get_b_cb(void)
{
    return (PIND & (1 << PD3));
}

static void encoder1_rotation_cb(enum rotary_encoder_direction dir, int8_t step_cnt)
{
    (void)step_cnt;

    if (dir == ROTARY_ENCODER_DIR_LEFT)
        buzzer_beep(3000, 200);  // 1 kHz, 500 ms
    else
        buzzer_beep(1000, 200);  // 1 kHz, 500 ms
}

static bool encoder1_init_cb(void)
{
    DDRD &= ~(1 << PD2); // input 
    DDRD &= ~(1 << PD3); // input 

    return true;
}

static bool encoder1_deinit_cb(void)
{
    return true;
}

static struct rotary_encoder_cfg encoder1_cfg = 
{
    .get_a_cb = encoder1_get_a_cb,
    .get_b_cb = encoder1_get_b_cb,
    .init_cb = encoder1_init_cb,
    .deinit_cb = encoder1_deinit_cb,
    .rotation_cb = encoder1_rotation_cb,
    .sub_steps_count = 4,
    .irq_cfg = ROTARY_ENCODER_IRQ_CONFIG_A,
};

static struct rotary_encoder_obj encoder1_obj;

/* RTC */

#include <twi.h>
#include <avr/interrupt.h>
// #include <time.h>

static struct twi_cfg twi1_cfg = 
{
    .pull_up_en = false,
    .frequency = 100,
    .irq_mode = false,
};

/* EXTI */

#include <exti.h>

static void exti_button1_cb(void)
{
    button_process(&button1_obj);
}

static void exti_sqw_cb(void)
{
    // if (!(PINC & (1 << PC2)))
        // buzzer_beep(500, 50);
}

static void exti_encoder1_cb(void)
{
    rotary_encoder_process(&encoder1_obj);
}

/* Buzzer */

#include <timer.h>

#include <buzzer.h>

static struct timer_cfg timer2_cfg = 
{  
    .id = TIMER_ID_2,
    .clock = TIMER_CLOCK_PRESC_8,
    .async_clock = TIMER_ASYNC_CLOCK_DISABLED,
    .mode = TIMER_MODE_CTC,
    .com_a_cfg = TIMER_CM_CHANGE_PIN_STATE,
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
    .in_capt_cb = NULL,
};

static struct timer_obj timer2_obj;

bool buzzer1_init_cb(void)
{
    return true;
}

void buzzer1_play_cb(uint16_t tone, uint16_t time_ms)
{
    if (tone != BUZZER_TONE_STOP)
    {
        timer2_cfg.out_comp_a_val = (F_CPU / (2 * 8 * tone)) - 1;
        timer_init(&timer2_obj, &timer2_cfg);
        timer_start(&timer2_obj, true);
    }

    while (time_ms--)
        _delay_ms(1);
    
    timer_start(&timer2_obj, false);
}

void buzzer1_stop_cb(void)
{
    timer_start(&timer2_obj, false);
}

bool buzzer1_deinit_cb(void)
{
    return true;
}

static struct buzzer_cfg buzzer1_cfg = 
{
	.init = buzzer1_init_cb,
	.play = buzzer1_play_cb,
	.stop = buzzer1_stop_cb,
	.deinit = buzzer1_deinit_cb,
};

static struct buzzer_obj buzzer1_obj;

struct buzzer_note alarm_beep[] = 
{
	{BUZZER_TONE_C6, BUZZER_NOTE_QUARTER},
	{BUZZER_TONE_STOP, BUZZER_NOTE_QUARTER},
	{BUZZER_TONE_C6, BUZZER_NOTE_QUARTER},
	{BUZZER_TONE_STOP, BUZZER_NOTE_QUARTER},
	{BUZZER_TONE_C6, BUZZER_NOTE_QUARTER},
	{BUZZER_TONE_STOP, BUZZER_NOTE_QUARTER},
	{BUZZER_TONE_C6, BUZZER_NOTE_QUARTER},
	{BUZZER_TONE_STOP, BUZZER_NOTE_QUARTER},
};

/* DCF77 */

#include <stdlib.h>

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

static void dcf77_decode(uint16_t ticks, bool rising_edge);

enum dcf77_bit_val
{
    DCF77_BIT_VAL_0,
    DCF77_BIT_VAL_1,
    DCF77_BIT_VAL_NONE,
    DCF77_BIT_VAL_ERROR,
};

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

#define PRESC 256

static void timer1_capt_cb(uint16_t icr)
{
    static bool rising_edge = false; // This has to be the same as initialization

    if (rising_edge) // Triggered on rising edge, break was measuered, change to falling
        TCCR1B &= ~(1 << ICES1);
    else // Triggered on falling edge, bit was measured, change to rising
        TCCR1B |= (1 << ICES1); 
    
    rising_edge ^= 1;

    /* Ignore short pulses (triggered on falling edge) REVERSED */
    if (!rising_edge && (icr < MS_TO_TICKS(35, PRESC)))
        return;

    // /* Ignore short pauses (triggered on rising edge) REVERSED */
    // if (rising_edge && (tick_to_ms(icr, 256) < 600))
    //     return;

    // TODO: Check pauses since last ignored short pulse (static timestapm) or ignore everything for at least 600 ms

    TCNT1 = 0;

    dcf77_decode(icr, !rising_edge); // restore real trigger source edge
};

static bool is_in_range(uint16_t ms, uint16_t min, uint16_t max)
{
    return (ms >= min) && (ms < max);
}

static enum dcf77_bit_val get_bit_val(uint16_t ticks)
{
    if (is_in_range(ticks, MS_TO_TICKS(40, PRESC), MS_TO_TICKS(130, PRESC)))
        return DCF77_BIT_VAL_0;
    if (is_in_range(ticks, MS_TO_TICKS(140, PRESC), MS_TO_TICKS(250, PRESC)))
        return DCF77_BIT_VAL_1;
    if (is_in_range(ticks, MS_TO_TICKS(1500, PRESC), MS_TO_TICKS(2200, PRESC)))
        return DCF77_BIT_VAL_NONE;

    return DCF77_BIT_VAL_ERROR;
}

static void dcf77_decode(uint16_t ticks, bool rising_edge)
{
    static bool frame_started = false;
    static uint8_t frame[8] = {0};  
    static uint8_t bit_cnt = 0;

    // ticks = tick_to_ms(ticks, 256);
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
            hd44780_print(&lcd_obj, "FRAME STARTED");
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
            hd44780_print(&lcd_obj, "ERROR         ");
            frame_started = 0;
            bit_cnt = 0;
            return;
        }

        uint8_t byte_idx = bit_cnt / 8;
        uint8_t bit_idx = bit_cnt % 8;
    
        frame[byte_idx] |= (val << bit_idx);

        bit_cnt++;

        if (bit_cnt >= 59)
        {
            struct dcf77_frame *frame_ptr = (struct dcf77_frame*)frame;
            
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
            
            hd44780_set_pos(&lcd_obj, 1, 0);
            hd44780_print(&lcd_obj, buf);
            cli();

            while(1);
            return;
        }
    }
    else // should be break
    {
        // TODO: Add breaks validation
        // TODO: take into account ignoring short pulses - it affects break time
    }

}

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

    system_timer_init();

    hd44780_init(&lcd_obj, &lcd_cfg);
    hd44780_print(&lcd_obj, "TEST");
    hd44780_set_pos(&lcd_obj, 1, 0);

    timer_init(&timer1_obj, &timer1_cfg);
    timer_start(&timer1_obj, true);

    button_init(&button1_obj, &button1_cfg);
    rotary_encoder_init(&encoder1_obj, &encoder1_cfg);

    buzzer_init(&buzzer1_obj, &buzzer1_cfg);

    exti_init(EXTI_ID_PCINT10, EXTI_TRIGGER_CHANGE, exti_sqw_cb);
    exti_enable(EXTI_ID_PCINT10, true);
    exti_init(EXTI_ID_PCINT2, EXTI_TRIGGER_CHANGE, exti_button1_cb);
    exti_enable(EXTI_ID_PCINT2, true);
    exti_init(EXTI_ID_INT0, EXTI_TRIGGER_FALLING_EDGE, exti_encoder1_cb);
    exti_enable(EXTI_ID_INT0, true);

    twi_init(&twi1_cfg);
    uint8_t sec_config[] = {0x00, 0x09};
    twi_send(0b11010000, sec_config, 2, true);
    uint8_t sqw_config[] = {0x07, 0b00010000};
    twi_send(0b11010000, sqw_config, 2, true);

    // set_system_time(1744458473);
    
    sei();

    while (1)
    {
        // if (PINB & (1 << PB0))
        //     PORTD &= ~(1 << PD6);
        // else 
        //     PORTD |= 1 << PD6;


        // buzzer_play_pattern(&buzzer1_obj, alarm_beep, sizeof(alarm_beep), 800);
        // _delay_ms(1000);
        
        // timer2_cfg.out_comp_a_val = (F_CPU / (2 * 8 * 1000)) - 1;
        // timer_init(&timer2_obj, &timer2_cfg);
        // timer_start(&timer2_obj, true);
        // _delay_ms(200);
        // timer_start(&timer2_obj, false);
        // _delay_ms(200);

        // uint8_t sec_buf = 6;
        // uint8_t sec_addr = 0x00;

        // twi_send(0b11010001, &sec_addr, 1, true);
        // twi_receive(0b11010001, &sec_buf, 1);

        // hd44780_set_pos(&lcd_obj, 1, 15);
        // hd44780_putc(&lcd_obj, (sec_buf & 0x0F) + '0');

        // hd44780_set_pos(&lcd_obj, 1, 0);
        // char buff[16] = {0};
        // hd44780_print(&lcd_obj, ltoa(system_timer_get(), buff, 10));

        // uint32_t unix_time = time(NULL);
        // hd44780_set_pos(&lcd_obj, 1, 0);
        // hd44780_print(&lcd_obj, ctime(&unix_time) + 4);
    }
}

//------------------------------------------------------------------------------

// Components (added / to add)
// * HD44780
// * DCF77 Decoder
// * Rotary encoder
// * TWI
// * TIM 
// * EXTI
// * Button
// * Buzzer

// * DS1307

// * USART
// * GPIO
// * Core
// * WDG

// 3 managers + main logic

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
