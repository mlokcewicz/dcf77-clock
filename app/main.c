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

int main()
{
    DDRD |= (1 << PD0);
    PORTD &= ~(1 << PD0);

    DDRB &= ~(1 << PB1); // input
    PORTB |= (1 << PB1); /* Pull up for EXTI */

    hd44780_init(&lcd_obj, &lcd_cfg);
    hd44780_print(&lcd_obj, "TEST");
    hd44780_set_pos(&lcd_obj, 1, 0);

    sei();

    while (1)
    {
        // PORTD ^= (1 << PD0);

        if (PINB & (1 << PB1))
            PORTD &= ~(1 << PD0);
        else 
            PORTD |= 1 << PD0;

        // _delay_ms(1000);
    }
}

//------------------------------------------------------------------------------
