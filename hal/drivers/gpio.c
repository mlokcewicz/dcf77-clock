//------------------------------------------------------------------------------

/// @file gpio.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "gpio.h"

#include <avr/io.h>

#include <util/atomic.h>

//------------------------------------------------------------------------------

struct port_object
{
	uint8_t pin_map[GPIO_PIN_7 + 1];
	volatile uint8_t *port_reg;
	volatile uint8_t *ddr_reg;
	volatile uint8_t *pin_reg;
};

struct gpio_context
{
	const struct port_object port_obj_map[GPIO_PORT_D + 1];
};

static struct gpio_context ctx = 
{
	.port_obj_map = 
	{
		[GPIO_PORT_B] = {{PB0, PB1, PB2, PB3, PB4, PB5, PB6, PB7}, &PORTB, &DDRB, &PINB},
		[GPIO_PORT_C] = {{PC0, PC1, PC2, PC3, PC4, PC5, PC6, 0xFF}, &PORTC, &DDRC, &PINC},
		[GPIO_PORT_D] = {{PD0, PD1, PD2, PD3, PD4, PD5, PD6, PD7}, &PORTD, &DDRD, &PIND},
	}
};

//------------------------------------------------------------------------------

bool gpio_init(enum gpio_port port, enum gpio_pin pin, bool dir, bool pull_up)
{
	if (port > GPIO_PORT_D || pin > GPIO_PIN_7)
		return false;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		/* Set direction */
		*(ctx.port_obj_map[port].ddr_reg) &= ~(1 << ctx.port_obj_map[port].pin_map[pin]);
		*(ctx.port_obj_map[port].ddr_reg) |= (dir << ctx.port_obj_map[port].pin_map[pin]);

		/* Set pull up if GPIO is configured as input */
		if (!dir)
		{
			*(ctx.port_obj_map[port].port_reg) &= ~(1 << ctx.port_obj_map[port].pin_map[pin]);
			*(ctx.port_obj_map[port].port_reg) |= (pull_up << ctx.port_obj_map[port].pin_map[pin]);

			if (pull_up)
				MCUCR &= ~(1 << PUD);
		}

	}

	/* Wait 1 clock cycle for synchronizer */
	asm volatile("NOP"::); 

	return true;
}

bool gpio_set(enum gpio_port port, enum gpio_pin pin, bool value)
{
	if (port > GPIO_PORT_D || pin > GPIO_PIN_7)
		return false;

	/* Set value */
	if (value)
		*(ctx.port_obj_map[port].port_reg) |= (value << ctx.port_obj_map[port].pin_map[pin]);
	else
		*(ctx.port_obj_map[port].port_reg) &= ~(1 << ctx.port_obj_map[port].pin_map[pin]);

	/* Wait 1 clock cycle for synchronizer */
	asm volatile("NOP"::); 

	return true;
}

bool gpio_set_atomic(enum gpio_port port, enum gpio_pin pin, bool value)
{
	if (port > GPIO_PORT_D || pin > GPIO_PIN_7)
		return false;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		/* Set value */
		if (value)
			*(ctx.port_obj_map[port].port_reg) |= (value << ctx.port_obj_map[port].pin_map[pin]);
		else 
			*(ctx.port_obj_map[port].port_reg) &= ~(1 << ctx.port_obj_map[port].pin_map[pin]);
	}

	/* Wait 1 clock cycle for synchronizer */
	asm volatile("NOP"::); 

	return true;
}

bool gpio_toggle(enum gpio_port port, enum gpio_pin pin)
{
	if (port > GPIO_PORT_D || pin > GPIO_PIN_7)
		return false;

	/* Toggle value (atomic) */
	*(ctx.port_obj_map[port].pin_reg) = (1 << ctx.port_obj_map[port].pin_map[pin]);

	return true;
}

bool gpio_get(enum gpio_port port, enum gpio_pin pin)
{
	return !!(*(ctx.port_obj_map[port].pin_reg) & (1 << ctx.port_obj_map[port].pin_map[pin]));
}

bool gpio_set_port(enum gpio_port port, uint8_t value)
{
	if (port > GPIO_PORT_D)
		return false;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		/* Set value */
		*(ctx.port_obj_map[port].port_reg) = value;
	}

	/* Wait 1 clock cycle for synchronizer */
	asm volatile("NOP"::); 

	return true;
}

uint8_t gpio_get_port(enum gpio_port port)
{
	return (*(ctx.port_obj_map[port].pin_reg));
}

//------------------------------------------------------------------------------
