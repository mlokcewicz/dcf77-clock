//------------------------------------------------------------------------------

/// @file hal.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "hal.h"

#include <stddef.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h> 
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/eeprom.h> 

#include <button.h>
#include <buzzer.h> 
#include <rotary_encoder.h>
#include <hd44780.h>
#include <ds1307.h>
#include <mas6181b.h>

#include <gpio.h>
#include <exti.h>  
#include <twi.h>
#include <timer.h>

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

#if 0
ISR(BADISR_vect)
{
    /* Add error handling */
}
#endif

//------------------------------------------------------------------------------

/* Application layer callbacks TODO: Consider callback registration */

__attribute__((weak)) void hal_exti_sqw_cb(void); 
__attribute__((weak)) void hal_button_pressed_cb(void); 
__attribute__((weak)) void hal_encoder_rotation_cb(int8_t dir); 
__attribute__((weak)) void hal_dcf_cb(uint16_t ms, bool triggred_on_bit); 
__attribute__((weak)) const uint8_t hal_user_defined_char_tab[6][8];

/* Pin assignement */

#define HAL_LED_PIN GPIO_PIN_6
#define HAL_LED_PORT GPIO_PORT_D

#define HAL_BUZZER_PIN GPIO_PIN_3
#define HAL_BUZZER_PORT GPIO_PORT_D

#define HAL_LCD_RS_PIN GPIO_PIN_3
#define HAL_LCD_RS_PORT GPIO_PORT_C

#define HAL_LCD_E_PIN GPIO_PIN_4
#define HAL_LCD_E_PORT GPIO_PORT_D

#define HAL_LCD_D4_PIN GPIO_PIN_6
#define HAL_LCD_D4_PORT GPIO_PORT_B

#define HAL_LCD_D5_PIN GPIO_PIN_7
#define HAL_LCD_D5_PORT GPIO_PORT_B

#define HAL_LCD_D6_PIN GPIO_PIN_7
#define HAL_LCD_D6_PORT GPIO_PORT_D

#define HAL_LCD_D7_PIN GPIO_PIN_5  
#define HAL_LCD_D7_PORT GPIO_PORT_D

#define HAL_BUTTON_PIN GPIO_PIN_2
#define HAL_BUTTON_PORT GPIO_PORT_B

#define HAL_ENCODER_A_PIN GPIO_PIN_2
#define HAL_ENCODER_A_PORT GPIO_PORT_D  
#define HAL_ENCODER_B_PIN GPIO_PIN_3
#define HAL_ENCODER_B_PORT GPIO_PORT_D

#define HAL_ENCODER_EXTI_ID EXTI_ID_INT0
#define HAL_ENCODER_EXTI_TRIGGER EXTI_TRIGGER_FALLING_EDGE

#define HAL_SQW_PIN GPIO_PIN_2
#define HAL_SQW_PORT GPIO_PORT_C
#define HAL_SQW_EXTI_ID EXTI_ID_PCINT10
#define HAL_SQW_EXTI_TRIGGER EXTI_TRIGGER_CHANGE

#define HAL_MAS6181B_PWR_DOWN_PIN GPIO_PIN_1
#define HAL_MAS6181B_PWR_DOWN_PORT GPIO_PORT_B

#define HAL_MAS6181B_OUT_PIN GPIO_PIN_0
#define HAL_MAS6181B_OUT_PORT GPIO_PORT_B

#define HAL_MAS6181B_EXTI_ID EXTI_ID_PCINT0
#define HAL_MAS6181B_EXTI_TRIGGER EXTI_TRIGGER_CHANGE   

/* Buzzer */

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

/* LCD */

struct gpio_tuple
{
    enum gpio_port port;
    enum gpio_pin pin;
};

static const struct gpio_tuple lcd_pins[] =
{
    [LCD_RS] = {HAL_LCD_RS_PORT, HAL_LCD_RS_PIN},
    [LCD_E]  = {HAL_LCD_E_PORT, HAL_LCD_E_PIN},
    [LCD_D4] = {HAL_LCD_D4_PORT, HAL_LCD_D4_PIN},
    [LCD_D5] = {HAL_LCD_D5_PORT, HAL_LCD_D5_PIN},
    [LCD_D6] = {HAL_LCD_D6_PORT, HAL_LCD_D6_PIN},
    [LCD_D7] = {HAL_LCD_D7_PORT, HAL_LCD_D7_PIN},
};

static void lcd_set_pin_cb(uint8_t pin, bool state)
{
    gpio_set(lcd_pins[pin].port, lcd_pins[pin].pin, state);
}

static void lcd_delay_cb(uint16_t us) 
{
    while (us--)
        _delay_us(1);
}

static void lcd_pin_init_cb(void)
{
    for (uint8_t i = 0; i < sizeof(lcd_pins) / sizeof(lcd_pins[0]); i++)
    {
        gpio_init(lcd_pins[i].port, lcd_pins[i].pin, true, false);
    }
}

static struct hd44780_cfg lcd_cfg = 
{
    .set_pin_state = lcd_set_pin_cb,
    .delay_us = lcd_delay_cb,
    .pin_init = lcd_pin_init_cb,
    .pin_deinit = NULL,
    .user_defined_char_tab = hal_user_defined_char_tab,
    .user_defined_char_tab_len = sizeof(hal_user_defined_char_tab) / sizeof(hal_user_defined_char_tab[0]),
};

static struct hd44780_obj lcd_obj;

/* Button */

static bool button1_init_cb(void)
{
    return gpio_init(HAL_BUTTON_PORT, HAL_BUTTON_PIN, false, false);
}

static bool button1_get_state_cb(void)
{
    return gpio_get(HAL_BUTTON_PORT, HAL_BUTTON_PIN);
}

static struct button_cfg button1_cfg = 
{
	.init = button1_init_cb,
	.get_state = button1_get_state_cb,
	.pressed = hal_button_pressed_cb,
	.deinit = NULL,
    
    .active_low = true,
    .irq_cfg = false,

	.debounce_counter_initial_value = 255,
	.autopress_counter_initial_value = 0,
};

static struct button_obj button1_obj;

/* Rotary encoder */

static bool encoder1_get_a_cb(void)
{
    return gpio_get(HAL_ENCODER_A_PORT, HAL_ENCODER_A_PIN);
};

static bool encoder1_get_b_cb(void)
{
    return gpio_get(HAL_ENCODER_B_PORT, HAL_ENCODER_B_PIN);
}

static bool encoder1_init_cb(void)
{
    gpio_init(HAL_ENCODER_A_PORT, HAL_ENCODER_A_PIN, false, false);
    gpio_init(HAL_ENCODER_B_PORT, HAL_ENCODER_B_PIN, false, false);

    return true;
}

static void encoder1_rotation_cb(enum rotary_encoder_direction dir, int8_t step_cnt)
{
    (void)step_cnt;
    hal_encoder_rotation_cb(dir);
}

static struct rotary_encoder_cfg encoder1_cfg = 
{
    .get_a_cb = encoder1_get_a_cb,
    .get_b_cb = encoder1_get_b_cb,
    .init_cb = encoder1_init_cb,
    .deinit_cb = NULL,
    .rotation_cb = encoder1_rotation_cb,
    .sub_steps_count = 4,
    .irq_cfg = ROTARY_ENCODER_IRQ_CONFIG_NONE,
    .debounce_counter_initial_value = 0,
};

static struct rotary_encoder_obj encoder1_obj;

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
    if (!gpio_get(HAL_SQW_PORT, HAL_SQW_PIN))
        hal_exti_sqw_cb();  
}

/* MAS6181B */

static void mas6181b1_io_init_cb(void)
{
    gpio_init(HAL_MAS6181B_PWR_DOWN_PORT, HAL_MAS6181B_PWR_DOWN_PIN, true, false);
    gpio_init(HAL_MAS6181B_OUT_PORT, HAL_MAS6181B_OUT_PIN, false, false);
}

static void mas6181b1_pwr_down_cb(bool pwr_down)
{
    /* Open collector */
    gpio_init(HAL_MAS6181B_PWR_DOWN_PORT, HAL_MAS6181B_PWR_DOWN_PIN, !pwr_down, false);
    gpio_set(HAL_MAS6181B_PWR_DOWN_PORT, HAL_MAS6181B_PWR_DOWN_PIN, pwr_down);
}

static bool mas6181b1_get_cb(void)
{
    return gpio_get(HAL_MAS6181B_OUT_PORT, HAL_MAS6181B_OUT_PIN);
}

static struct mas6181b_cfg mas6181b1_cfg = 
{
    .io_init = mas6181b1_io_init_cb,
    .pwr_down = mas6181b1_pwr_down_cb,
    .get = mas6181b1_get_cb,
};

static struct mas6181b_obj mas6181b1_obj;

/* DCF77 Decoder Interrupt */

static void exti_mas6181B_cb(void)
{
    static uint16_t last_time = 0;
    uint16_t current_time = system_timer_get();
    uint16_t time_diff = current_time - last_time;
    last_time = current_time;

    hal_dcf_cb(time_diff, gpio_get(HAL_MAS6181B_OUT_PORT, HAL_MAS6181B_OUT_PIN));
}

//------------------------------------------------------------------------------

void hal_init(void)
{
    /* Watchdog */
    wdt_enable(WDTO_8S);

    /* Power down */
    power_adc_disable();
    power_usart0_disable();
    power_spi_disable();
    
    /* System timer */
    system_timer_init();

    /* LED */
    gpio_init(HAL_LED_PORT, HAL_LED_PIN, true, false);
    gpio_set(HAL_LED_PORT, HAL_LED_PIN, false);

    /* Buzzer */
    buzzer_init(&buzzer1_obj, &buzzer1_cfg);

    /* LCD */
    hd44780_init(&lcd_obj, &lcd_cfg);

    /* Button */
    button_init(&button1_obj, &button1_cfg);

    /* Rotary encoder */
    rotary_encoder_init(&encoder1_obj, &encoder1_cfg);

    /* External interrupts */
    exti_init(HAL_MAS6181B_EXTI_ID, HAL_MAS6181B_EXTI_TRIGGER, exti_mas6181B_cb);
    exti_enable(HAL_MAS6181B_EXTI_ID, true); 

    exti_init(HAL_SQW_EXTI_ID, HAL_SQW_EXTI_TRIGGER, exti_sqw_cb);
    exti_enable(HAL_SQW_EXTI_ID, true);

    /* DS1307 */
    ds1307_init(&rtc_obj, &rtc_cfg);

    /* MAS6181B */
    mas6181b_init(&mas6181b1_obj, &mas6181b1_cfg);
    
    sei();
}

void hal_process(void)
{
    buzzer_process(&buzzer1_obj);
    button_process(&button1_obj);
    rotary_encoder_process(&encoder1_obj);

    wdt_reset();

    set_sleep_mode(SLEEP_MODE_IDLE);
    sleep_mode();
}

void hal_led_set(bool state)
{
    gpio_set(HAL_LED_PORT, HAL_LED_PIN, state);
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

void hal_lcd_set_cursor(uint8_t row, uint8_t col)
{
    hd44780_set_pos(&lcd_obj, row, col);
}

void hal_lcd_set_cursor_mode(bool visible, bool blinking)
{
    hd44780_set_cursor_mode(&lcd_obj, visible, blinking);
}

void hal_lcd_putc(const char ch)
{
    hd44780_putc(&lcd_obj, ch);
}

void hal_audio_set_pattern(struct buzzer_note *pattern, uint16_t pattern_len, uint16_t bpm)
{
    buzzer_set_pattern(&buzzer1_obj, pattern, pattern_len, bpm);
}

void hal_audio_stop(void)
{
    buzzer_stop_pattern(&buzzer1_obj);
}

void hal_audio_process(void)
{
    buzzer_process(&buzzer1_obj);
}

void hal_button_process(void)
{
    button_process(&button1_obj);
}

void hal_rotary_encoder_process(void)
{
    rotary_encoder_process(&encoder1_obj);
}

void hal_set_time(struct ds1307_time *time)
{
    ds1307_set_time(&rtc_obj, time);
}

void hal_get_time(struct ds1307_time *time)
{
    ds1307_get_time(&rtc_obj, time);
}

void hal_set_alarm(struct hal_timestamp *alarm)
{
    eeprom_write_block((const void *)alarm, (void *)0x00, sizeof(struct hal_timestamp));
}

void hal_get_alarm(struct hal_timestamp *alarm)
{
    eeprom_read_block((void *)alarm, (const void *)0x00, sizeof(struct hal_timestamp));
}

void hal_set_timezone(int8_t *tz)
{
    eeprom_write_block((const void *)tz, (void *)(0x00 + sizeof(struct hal_timestamp)), sizeof(int8_t));
}

void hal_get_timezone(int8_t *tz)
{
    eeprom_read_block((void *)tz, (const void *)(0x00 + sizeof(struct hal_timestamp)), sizeof(int8_t));
}

bool hal_time_is_reset(void)
{
    return ds1307_is_running(&rtc_obj);
}

bool hal_dcf_get_state(void)
{
    return mas6181b_get_state(&mas6181b1_obj);
}

void hal_dcf_power_down(bool pwr_down)
{
    mas6181b_power_down(&mas6181b1_obj, pwr_down);
}

bool hal_system_timer_timeout_passed(uint16_t timestamp, uint16_t timeout)
{
    return system_timer_timeout_passed(timestamp, timeout);
}

uint16_t hal_system_timer_get(void)
{
    return system_timer_get();
}

//------------------------------------------------------------------------------
