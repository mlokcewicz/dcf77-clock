//------------------------------------------------------------------------------

/// @file rotary_encoder.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef ROTARY_ENCODER_H_
#define ROTARY_ENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>

//------------------------------------------------------------------------------

enum rotary_encoder_direction
{
    ROTARY_ENCODER_DIR_LEFT = -1,
    ROTARY_ENCODER_DIR_RIGHT = 1,
    ROTARY_ENCODER_DIR_NONE = 0,
};

enum rotary_encoder_irq_config
{
    ROTARY_ENCODER_IRQ_CONFIG_A = 0, 
    ROTARY_ENCODER_IRQ_CONFIG_B,
    ROTARY_ENCODER_IRQ_CONFIG_NONE,
};

//------------------------------------------------------------------------------

typedef bool (*rotary_encoder_get_a_cb)(void);
typedef bool (*rotary_encoder_get_b_cb)(void);
typedef void (*rotary_encoder_rotation_cb)(enum rotary_encoder_direction dir, int8_t step_cnt);
typedef bool (*rotary_encoder_init_cb)(void);
typedef bool (*rotary_encoder_deinit_cb)(void);

//------------------------------------------------------------------------------

struct rotary_encoder_cfg
{
    rotary_encoder_get_a_cb get_a_cb;
    rotary_encoder_get_b_cb get_b_cb;
    rotary_encoder_rotation_cb rotation_cb;
    rotary_encoder_init_cb init_cb;
    rotary_encoder_deinit_cb deinit_cb;

    enum rotary_encoder_irq_config irq_cfg;
    uint8_t sub_steps_count;
    volatile uint32_t debounce_counter_initial_value;
};

struct rotary_encoder_obj
{
    rotary_encoder_get_a_cb get_a_cb;
    rotary_encoder_get_b_cb get_b_cb;
    rotary_encoder_rotation_cb rotation_cb;
    rotary_encoder_init_cb init_cb;
    rotary_encoder_deinit_cb deinit_cb;

    enum rotary_encoder_irq_config irq_cfg;
    uint8_t sub_steps_count;

    volatile uint8_t prev_pos;
    volatile int8_t step_cnt;

    volatile uint32_t debounce_counter_initial_value;
    volatile uint32_t debounce_counter_current_value;

};

//------------------------------------------------------------------------------

/// @brief Initializes rotary encoder object
/// @note For 2=step encoder in interrupt mode, use both edges trigger
/// @param obj - rotary encoder object structure pointer
/// @param cfg - rotary encoder object configuration structure pointer
/// @return true if initialized properly, otherwise false
bool rotary_encoder_init(struct rotary_encoder_obj *obj, struct rotary_encoder_cfg *cfg);

/// @brief Updates rotary encoder state and call callbacks when full transition occured
/// @note It has to be called periodically without long delays or call inside ISR triggered by rising or falling edge of selected pin (A / B)
/// @param obj - rotary encoder object structure pointer
/// @return last full transition direction according to @ref enum rotary_encoder_direction
enum rotary_encoder_direction rotary_encoder_process(struct rotary_encoder_obj *obj);

/// @param obj - rotary encoder object structure pointer
/// @return true if deinitialized properly, otherwise false
bool rotary_encoder_deinit(struct rotary_encoder_obj *obj);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* ROTARY_ENCODER_H_ */

//------------------------------------------------------------------------------

