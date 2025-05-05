//------------------------------------------------------------------------------

/// @file button.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "button.h"

//------------------------------------------------------------------------------

#ifndef BUTTON_USE_IRQ
#define BUTTON_USE_IRQ 0
#endif

#ifndef BUTTON_USE_POLLING
#define BUTTON_USE_POLLING 0
#endif

//------------------------------------------------------------------------------

#if BUTTON_USE_POLLING
static bool process_poll(struct button_obj *obj)
{
    bool state = obj->get_state();

    /* Check if debounce counter timed out */
    if (obj->debounce_counter_current_value < obj->debounce_counter_initial_value)
    {
        obj->debounce_counter_current_value++;
        return obj->prev_state;
    }
    
    /* Check if button state has been changed */
    if (state ^ obj->prev_state)
    {
         /* Check if button is in active state */
        if (!state == obj->active_low)
            obj->pressed();

        obj->prev_state = state;
        obj->debounce_counter_current_value = 0;
        obj->autopress_counter_current_value = 0;
    }
    /* Check if button state is still active */
    else if (!state == obj->active_low)
    {
        if (obj->autopress_counter_current_value < obj->autopress_counter_initial_value)
        {
            obj->autopress_counter_current_value++;
        }
        else
        {
            obj->autopress_counter_current_value = 0;
            obj->pressed();
        }
    }

    return state;
}
#endif

#if BUTTON_USE_IRQ
static bool process_irq(struct button_obj *obj)
{
    bool state = obj->get_state();

    /* Check if button is in active state */
    if (!state == obj->active_low)
        obj->pressed();

    return state;
}
#endif

//------------------------------------------------------------------------------

bool button_init(struct button_obj *obj, struct button_cfg *cfg)
{
    if (!cfg || !cfg->init || !cfg->get_state || !cfg->pressed)
        return false;

    obj->init = cfg->init;
    obj->get_state = cfg->get_state;
    obj->pressed = cfg->pressed;
    obj->deinit = cfg->deinit;

    obj->active_low = cfg->active_low;
    obj->irq_cfg = cfg->irq_cfg;
    obj->debounce_counter_initial_value = cfg->debounce_counter_initial_value;
    obj->debounce_counter_current_value = obj->debounce_counter_initial_value;
    obj->autopress_counter_initial_value = cfg->autopress_counter_initial_value;
    obj->autopress_counter_current_value = cfg->autopress_counter_initial_value;

    /* Set initial state to inactive to detect first press */
    obj->prev_state = cfg->active_low;

    if (obj->debounce_counter_initial_value == UINT16_MAX)
        obj->debounce_counter_initial_value--;

    if (obj->autopress_counter_initial_value == UINT16_MAX)
        obj->autopress_counter_initial_value--;

    return obj->init();
}

bool button_process(struct button_obj *obj)
{
    if (!obj)
        return false;

    if (obj->irq_cfg)
    {
    #if BUTTON_USE_IRQ
        return process_irq(obj);
    #endif
    }
    else
    {
    #if BUTTON_USE_POLLING
        return process_poll(obj);
    #endif
    }

    return false;
}

bool button_deinit(struct button_obj *obj)
{
    return obj->deinit();
}

//------------------------------------------------------------------------------
