//------------------------------------------------------------------------------

/// @file rotary_encoder.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "rotary_encoder.h"

#include <stddef.h>

//------------------------------------------------------------------------------

static enum rotary_encoder_direction process_poll(struct rotary_encoder_obj *obj)
{
    /* Check if debounce counter timed out */
    if (obj->debounce_counter_current_value < obj->debounce_counter_initial_value)
    {
        obj->debounce_counter_current_value++;
        return ROTARY_ENCODER_DIR_NONE;
    }

    bool a = obj->get_a_cb();
    bool b = obj->get_b_cb();

    /* LEFT             RIGHT           */
    /* AB               AB              */                
    /* GREY  BIN  DEC   GREY  BIN  DEC  */                
    /* 00    00   0     10    11   3    */          
    /* 01    01   1     11    10   2    */    
    /* 11    10   2     01    01   1    */    
    /* 10    11   3     00    00   0    */       

    /* Convert to binary */
    uint8_t current_pos = a << 1 | (a ^ b); 

    /* Negative value means left direction */
    int8_t delta = (int8_t)obj->prev_pos - (int8_t)current_pos; 
    enum rotary_encoder_direction dir = ROTARY_ENCODER_DIR_NONE;
    
    if (delta != 0)
        obj->debounce_counter_current_value = 0;

    obj->prev_pos = current_pos;

    dir = (delta & (1 << 1)) ? ROTARY_ENCODER_DIR_LEFT : ROTARY_ENCODER_DIR_RIGHT; 

    /* delta = -1, -2,  3 - left  (1)11  (1)10  11 */
    /* delta =  1,  2, -3 - right  01    10     (1)01 */

    /* Detect full step transition (from 3 to 0 or 0 to 3) */
    if (delta == obj->sub_steps_count - 1 || delta == -obj->sub_steps_count + 1)
    {
        obj->step_cnt += dir;

        obj->rotation_cb(dir, obj->step_cnt);
        return dir;    
    }

    return ROTARY_ENCODER_DIR_NONE;
}

static enum rotary_encoder_direction process_irq(struct rotary_encoder_obj *obj)
{
    /* Detect edge and determine second pin state (other than interrupt trigger) */
    bool rising_edge = (obj->irq_cfg == ROTARY_ENCODER_IRQ_CONFIG_B) ? obj->get_b_cb() : obj->get_a_cb();
    bool second_pin_state = (obj->irq_cfg == ROTARY_ENCODER_IRQ_CONFIG_B) ? obj->get_a_cb() : obj->get_b_cb();
    
    enum rotary_encoder_direction dir = ROTARY_ENCODER_DIR_NONE;

    /* According to classic 4 - sub-steps encoder transistion table */
    if (obj->irq_cfg == ROTARY_ENCODER_IRQ_CONFIG_A)
        dir = (rising_edge == second_pin_state) ? ROTARY_ENCODER_DIR_LEFT : ROTARY_ENCODER_DIR_RIGHT;
    if (obj->irq_cfg == ROTARY_ENCODER_IRQ_CONFIG_B)
        dir = (rising_edge != second_pin_state) ? ROTARY_ENCODER_DIR_LEFT : ROTARY_ENCODER_DIR_RIGHT;

    obj->step_cnt += dir;

    obj->rotation_cb(dir, obj->step_cnt);

    return dir;
}

//------------------------------------------------------------------------------

bool rotary_encoder_init(struct rotary_encoder_obj *obj, struct rotary_encoder_cfg *cfg)
{
    if (!cfg || ! cfg->get_a_cb || !cfg->get_b_cb || !cfg->init_cb || !cfg->deinit_cb || !cfg->rotation_cb)
        return false;

    obj->get_a_cb = cfg->get_a_cb;
    obj->get_b_cb = cfg->get_b_cb;
    obj->init_cb = cfg->init_cb;
    obj->deinit_cb = cfg->deinit_cb;
    obj->rotation_cb = cfg->rotation_cb;

    obj->irq_cfg = cfg->irq_cfg;
    obj->sub_steps_count = cfg->sub_steps_count;

    obj->debounce_counter_initial_value = cfg->debounce_counter_initial_value;
    obj->debounce_counter_current_value = obj->debounce_counter_initial_value;

    if (obj->debounce_counter_initial_value == UINT32_MAX)
        obj->debounce_counter_initial_value--;

    /* Set initial positon (for polling mode) */
    bool a = obj->get_a_cb();
    bool b = obj->get_b_cb();

    obj->prev_pos = a << 1 | (a ^ b);

    obj->step_cnt = 0;

    return obj->init_cb();
}

enum rotary_encoder_direction rotary_encoder_process(struct rotary_encoder_obj *obj)
{
    if (!obj)
        return ROTARY_ENCODER_DIR_NONE;

    if (obj->irq_cfg == ROTARY_ENCODER_IRQ_CONFIG_NONE)
        return process_poll(obj);
    else
        return process_irq(obj);
}

bool rotary_encoder_deinit(struct rotary_encoder_obj *obj)
{
    bool res = obj->deinit_cb();

    obj->get_a_cb = NULL;
    obj->get_b_cb = NULL;
    obj->init_cb = NULL;
    obj->deinit_cb = NULL;
    obj->rotation_cb = NULL;

    obj->sub_steps_count = 0;

    return res;
}

//------------------------------------------------------------------------------
