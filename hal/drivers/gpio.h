//------------------------------------------------------------------------------

/// @file gpio.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef GPIO_H_
#define GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>

//------------------------------------------------------------------------------

enum gpio_port
{
    GPIO_PORT_B,
    GPIO_PORT_C,
    GPIO_PORT_D
};

enum gpio_pin
{
    GPIO_PIN_0,
    GPIO_PIN_1,
    GPIO_PIN_2,
    GPIO_PIN_3,
    GPIO_PIN_4,
    GPIO_PIN_5,
    GPIO_PIN_6,
    GPIO_PIN_7
};

//------------------------------------------------------------------------------

/// @brief Iinitializes selected GPIO pin
/// @param port selected GPIO port @ref enum gpio_port
/// @param pin selected GPIO pin @ref enum gpio_pin
/// @param dir true for output, false for input
/// @param pull_up true for pull-up resistor enabling (valid only for input)
/// @return true if initialized properly, otherwise false
bool gpio_init(enum gpio_port port, enum gpio_pin pin, bool dir, bool pull_up);

/// @brief Set given output value
/// @param port selected GPIO port @ref enum gpio_port
/// @param pin selected GPIO pin @ref enum gpio_pin
/// @param value true for high stare, false for low state
/// @return true if set properly, otherwise false
bool gpio_set(enum gpio_port port, enum gpio_pin pin, bool value);

/// @brief Toggles output value to opposite than actual
/// @param port selected GPIO port @ref enum gpio_port
/// @param pin selected GPIO pin @ref enum gpio_pin
/// @return true if toggled properly, otherwise false
bool gpio_toggle(enum gpio_port port, enum gpio_pin pin);

/// @brief Gets actual pin state
/// @param port selected GPIO port @ref enum gpio_port
/// @param pin selected GPIO pin @ref enum gpio_pin
/// @return actual pin state
bool gpio_get(enum gpio_port port, enum gpio_pin pin);

/// @brief Set given output value for whole port
/// @param port selected GPIO port @ref enum gpio_port
/// @param value 8-bit port value
/// @return true if set properly, otherwise false
bool gpio_set_port(enum gpio_port port, uint8_t value);

/// @brief Gets actual port value
/// @param port selected GPIO port @ref enum gpio_port
/// @return actual 8-bit port value
uint8_t gpio_get_port(enum gpio_port port);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* GPIO_H_ */

//------------------------------------------------------------------------------
