//------------------------------------------------------------------------------

/// @file hal.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "hal.h"

#include <stddef.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include <core.h>
#include <wdg.h>
#include <gpio.h>
#include <exti.h>  
#include <twi.h>
#include <timer.h>

#include <button.h>
#include <buzzer.h> 
#include <rotary_encoder.h>
#include <hd44780.h>
#include <ds1307.h>
#include <mas6181b.h>

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

/* Application layer callbacks */

extern void hal_exti_sqw_cb(void); // TODO: Register callback
extern void hal_button1_pressed_cb(void); // TODO: Register callback
extern void hal_encoder1_rotation_cb(enum rotary_encoder_direction dir, int8_t step_cnt); // TODO: Register callback
extern void hal_dcf_cb(uint16_t ms, bool rising_edge); // TODO: Register callback

/* BUZZER */

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

static bool buzzer1_init_cb(void)
{
    return true;
}

static bool buzzer1_play_cb(uint16_t tone, uint16_t time_ms)
{
    static uint32_t start_tickstamp = 0;

    if (start_tickstamp == 0) // note not started
    {
        if (tone != BUZZER_TONE_STOP)
        {
            timer2_cfg.out_comp_a_val = (F_CPU / (2 * 8 * tone)) - 1;
            timer_init(&timer2_obj, &timer2_cfg);
            timer_start(&timer2_obj, true);
        }
        else
            timer_start(&timer2_obj, false);

        start_tickstamp = system_timer_get();
        
        return true;
    }

    /* Note is playing */
    if (!system_timer_timeout_passed(start_tickstamp, time_ms))
        return true;

    start_tickstamp = 0;

    return false; // note is played
}

static void buzzer1_stop_cb(void)
{
    timer_start(&timer2_obj, false);
}

static struct buzzer_cfg buzzer1_cfg = 
{
	.init = buzzer1_init_cb,
	.play = buzzer1_play_cb,
	.stop = buzzer1_stop_cb,
	.deinit = NULL,
};

static struct buzzer_obj buzzer1_obj;

static struct buzzer_note alarm_beep[] = // TODO: Move to app layer
{
	{BUZZER_TONE_C6, BUZZER_NOTE_QUARTER},
	{BUZZER_TONE_STOP, BUZZER_NOTE_QUARTER},
	{BUZZER_TONE_C6, BUZZER_NOTE_QUARTER},
	{BUZZER_TONE_STOP, BUZZER_NOTE_QUARTER},
	{BUZZER_TONE_C6, BUZZER_NOTE_QUARTER},
	{BUZZER_TONE_STOP, BUZZER_NOTE_QUARTER},
	{BUZZER_TONE_C6, BUZZER_NOTE_QUARTER},
	{BUZZER_TONE_STOP, BUZZER_NOTE_QUARTER},
	{BUZZER_TONE_STOP, 1000UL * 800},
};

/* LCD */

static void lcd_set_pin_cb(uint8_t pin, bool state)
{
    struct gpio_tuple
    {
        enum gpio_port port;
        enum gpio_pin pin;
    };

    static const struct gpio_tuple lcd_pins[] =
    {
        [LCD_RS] = {GPIO_PORT_C, GPIO_PIN_3},
        [LCD_E] = {GPIO_PORT_D, GPIO_PIN_4},
        [LCD_D4] = {GPIO_PORT_B, GPIO_PIN_6},
        [LCD_D5] = {GPIO_PORT_B, GPIO_PIN_7},
        [LCD_D6] = {GPIO_PORT_D, GPIO_PIN_7},
        [LCD_D7] = {GPIO_PORT_D, GPIO_PIN_5},
    };

    gpio_set(lcd_pins[pin].port, lcd_pins[pin].pin, state);
}

static void lcd_delay_cb(uint16_t us) 
{
    while (us--)
        _delay_us(1);
}

static void lcd_pin_init_cb(void)
{
    gpio_init(GPIO_PORT_C, GPIO_PIN_3, true, false);
    gpio_init(GPIO_PORT_B, GPIO_PIN_6, true, false);
    gpio_init(GPIO_PORT_B, GPIO_PIN_7, true, false);
    gpio_init(GPIO_PORT_D, GPIO_PIN_4, true, false);
    gpio_init(GPIO_PORT_D, GPIO_PIN_5, true, false);
    gpio_init(GPIO_PORT_D, GPIO_PIN_7, true, false);
}

static struct hd44780_cfg lcd_cfg = 
{
    .set_pin_state = lcd_set_pin_cb,
    .delay_us = lcd_delay_cb,
    .pin_init = lcd_pin_init_cb,
    .pin_deinit = NULL,
};

struct hd44780_obj lcd_obj;

/* BUTTON */

static bool button1_init_cb(void)
{
    return gpio_init(GPIO_PORT_B, GPIO_PIN_2, false, false);
}

static bool button1_get_state_cb(void)
{
    return gpio_get(GPIO_PORT_B, GPIO_PIN_2);
}

static struct button_cfg button1_cfg = 
{
	.init = button1_init_cb,
	.get_state = button1_get_state_cb,
	.pressed = hal_button1_pressed_cb,
	.deinit = NULL,
    
    .active_low = true,
    .irq_cfg = true,

	.debounce_counter_initial_value = 0,
	.autopress_counter_initial_value = 0,
};

static struct button_obj button1_obj;

static void exti_button1_cb(void)
{
    button_process(&button1_obj);
}

/* ENCODER */

static bool encoder1_get_a_cb(void)
{
    return gpio_get(GPIO_PORT_D, GPIO_PIN_2);
};

static bool encoder1_get_b_cb(void)
{
    return gpio_get(GPIO_PORT_D, GPIO_PIN_3);
}

static bool encoder1_init_cb(void)
{
    gpio_init(GPIO_PORT_D, GPIO_PIN_2, false, false);
    gpio_init(GPIO_PORT_D, GPIO_PIN_3, false, false);

    return true;
}

static struct rotary_encoder_cfg encoder1_cfg = 
{
    .get_a_cb = encoder1_get_a_cb,
    .get_b_cb = encoder1_get_b_cb,
    .init_cb = encoder1_init_cb,
    .deinit_cb = NULL,
    .rotation_cb = hal_encoder1_rotation_cb,
    .sub_steps_count = 4,
    .irq_cfg = ROTARY_ENCODER_IRQ_CONFIG_A,
};

static struct rotary_encoder_obj encoder1_obj;

static void exti_encoder1_cb(void)
{
    rotary_encoder_process(&encoder1_obj);
}

/* TWI */

static struct twi_cfg twi1_cfg = 
{
    .pull_up_en = false,
    .frequency = 100,
    .irq_mode = false,
};

/* RTC - DS1307 */

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
        hal_exti_sqw_cb();  
}

/* MAS6181B */

static void mas6181b1_io_init_cb(void)
{
    gpio_init(GPIO_PORT_B, GPIO_PIN_1, true, false);
    gpio_init(GPIO_PORT_B, GPIO_PIN_0, false, false);
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

/* DCF77 */

#define DCF_TIMER_PRESC 256

void timer1_capt_cb(uint16_t icr)
{
    /* Now trigger on RISING edge is on BIT edge */

    bool rising_edge = (TCCR1B & (1 << ICES1)); // TODO: Use timer driver
    
    TCCR1B ^= (1 << ICES1); // Change trigger edge
    TCNT1 = 0;

    hal_dcf_cb(TICKS_TO_MS(icr, DCF_TIMER_PRESC), rising_edge);
};

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

//------------------------------------------------------------------------------

void hal_init(void)
{
    wdg_init(WDG_MODE_RST, WDG_PERIOD_8S, NULL);
    
    system_timer_init();
    
    /* LED */
    gpio_init(GPIO_PORT_D, GPIO_PIN_6, true, false);
    gpio_set(GPIO_PORT_D, GPIO_PIN_6, false);

    /* BUZZER */
    buzzer_init(&buzzer1_obj, &buzzer1_cfg);
    buzzer_set_pattern(&buzzer1_obj, alarm_beep, sizeof(alarm_beep), 800);

    /* LCD */
    hd44780_init(&lcd_obj, &lcd_cfg);
    hd44780_print(&lcd_obj, "TEST");
    hd44780_set_pos(&lcd_obj, 1, 0);

    /* BUTTON */
    button_init(&button1_obj, &button1_cfg);

    /* ROTARY ENCODER */
    rotary_encoder_init(&encoder1_obj, &encoder1_cfg);

    /* EXTI */
    exti_init(EXTI_ID_PCINT2, EXTI_TRIGGER_CHANGE, exti_button1_cb);
    exti_enable(EXTI_ID_PCINT2, true);
    exti_init(EXTI_ID_INT0, EXTI_TRIGGER_FALLING_EDGE, exti_encoder1_cb);
    exti_enable(EXTI_ID_INT0, true);
    exti_init(EXTI_ID_PCINT10, EXTI_TRIGGER_CHANGE, exti_sqw_cb);
    exti_enable(EXTI_ID_PCINT10, true);

    /* DS1307 */
    ds1307_init(&rtc_obj, &rtc_cfg);

    /* MAS6181B */
    mas6181b_init(&mas6181b1_obj, &mas6181b1_cfg);
    
    /* DCF */
    timer_init(&timer1_obj, &timer1_cfg);
    timer_start(&timer1_obj, true);

    sei();
}

void hal_lcd_clear(void)
{
    hd44780_clear(&lcd_obj);
}

void hal_lcd_print(const char* str, uint8_t row, uint8_t col)
{
    hd44780_set_pos(&lcd_obj, row, col);
    hd44780_print(&lcd_obj, str);
}

//------------------------------------------------------------------------------
