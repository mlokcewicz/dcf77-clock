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
		[GPIO_PORT_B] = {&PORTB, &DDRB, &PINB},
		[GPIO_PORT_C] = {&PORTC, &DDRC, &PINC},
		[GPIO_PORT_D] = {&PORTD, &DDRD, &PIND},
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
		*(ctx.port_obj_map[port].ddr_reg) &= ~(1 << pin);
		*(ctx.port_obj_map[port].ddr_reg) |= (dir << pin);

		/* Set pull up if GPIO is configured as input */
		if (!dir)
		{
			*(ctx.port_obj_map[port].port_reg) &= ~(1 << pin);
			*(ctx.port_obj_map[port].port_reg) |= (pull_up << pin);

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
		*(ctx.port_obj_map[port].port_reg) |= (value << pin);
	else
		*(ctx.port_obj_map[port].port_reg) &= ~(1 << pin);

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
			*(ctx.port_obj_map[port].port_reg) |= (value << pin);
		else 
			*(ctx.port_obj_map[port].port_reg) &= ~(1 << pin);
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
	*(ctx.port_obj_map[port].pin_reg) = (1 << pin);

	return true;
}

bool gpio_get(enum gpio_port port, enum gpio_pin pin)
{
	return !!(*(ctx.port_obj_map[port].pin_reg) & (1 << pin));
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

