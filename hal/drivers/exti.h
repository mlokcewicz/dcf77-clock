//------------------------------------------------------------------------------

/// @file exti.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef EXTI_H_
#define EXTI_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>

//------------------------------------------------------------------------------

typedef void (*exti_cb)(void);

//------------------------------------------------------------------------------

enum exti_id
{
    EXTI_ID_INT0,
    EXTI_ID_INT1,
    EXTI_ID_PCINT0,
    EXTI_ID_PCINT1,
    EXTI_ID_PCINT2,
    EXTI_ID_PCINT3,
    EXTI_ID_PCINT4,
    EXTI_ID_PCINT5,
    EXTI_ID_PCINT6,
    EXTI_ID_PCINT7,
    EXTI_ID_PCINT8,
    EXTI_ID_PCINT9,
    EXTI_ID_PCINT10,
    EXTI_ID_PCINT11,
    EXTI_ID_PCINT12,
    EXTI_ID_PCINT13,
    EXTI_ID_PCINT14,
    EXTI_ID_RESERVED,
    EXTI_ID_PCINT16,
    EXTI_ID_PCINT17,
    EXTI_ID_PCINT18,
    EXTI_ID_PCINT19,
    EXTI_ID_PCINT20,
    EXTI_ID_PCINT21,
    EXTI_ID_PCINT22,
    EXTI_ID_PCINT23,
};

enum exti_trigger
{
    EXTI_TRIGGER_LOW,
    EXTI_TRIGGER_CHANGE,
    EXTI_TRIGGER_FALLING_EDGE,
    EXTI_TRIGGER_RISING_EDGE,
};

//------------------------------------------------------------------------------

/// @brief Configures external interrupt trigger and assign selected callback
/// @note After calling that function for PCINT from the same group, callback is overwritten
/// @param id selected interrupt id @ref enum exti_id  
/// @param trigger selected interrupt trigger configuration @ref enum exti_tirgger (valid only for INTx)
/// @param cb interrupt callback pointer @ref exti_cb
/// @return true if initialized successfully, otherwise false
bool exti_init(enum exti_id id, enum exti_trigger trigger, exti_cb cb);

/// @brief Enables or disables sellected interrupt
/// @param id selected interrupt id @ref enum exti_id 
/// @param enable true for enble, false for disable
void exti_enable(enum exti_id id, bool enable);

/// @brief Disables selected interrupt and reset callback and trigger configuration
/// @param id selected interrupt id @ref enum exti_id 
/// @return true if deinitialized successfully, otherwise false
bool exti_deinit(enum exti_id id);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* EXTI_H_ */

//------------------------------------------------------------------------------
