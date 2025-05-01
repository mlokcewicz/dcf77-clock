//------------------------------------------------------------------------------

/// @file timer.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef TIMER_H_
#define TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>

#include <util/delay.h>

//------------------------------------------------------------------------------

#define MS_TO_TICKS(ms, presc) (ms * (F_CPU / 1000UL) / presc)

//------------------------------------------------------------------------------

typedef void (*timer_ovrf_cb)(void);
typedef void (*timer_comp_cb)(void);
typedef void (*timer_capt_cb)(uint16_t icr);

//------------------------------------------------------------------------------

enum timer_id
{
    TIMER_ID_0,
    TIMER_ID_1,
    TIMER_ID_2,
};

enum timer_clock
{
    TIMER_CLOCK_STOP = 0,
    TIMER_CLOCK_NO_PRESC,
    TIMER_CLOCK_PRESC_8,
    TIMER_CLOCK_PRESC_64,
    TIMER_CLOCK_PRESC_256,
    TIMER_CLOCK_PRESC_1024,
    TIMER_CLOCK_EXT_FALLING_EDGE,
    TIMER_CLOCK_EXT_RISING_EDGE,
};

/* Valid only for TIMER2 */
enum timer_async_clock
{
    TIMER_ASYNC_CLOCK_DISABLED = 0b00,
    TIMER_ASYNC_CLOCK_ASYNC_OSC = 0b01,
    TIMER_ASYNC_CLOCK_ASYNC_EXT = 0b11,
};

enum timer_mode
{
    TIMER_MODE_NORMAL = 0,
    TIMER_MODE_PWM_PHASE_CORRECT_TOP_0xFF,
    TIMER_MODE_CTC,
    TIMER_MODE_FAST_PWM_TOP_0xFF,
    TIMER_MODE_RESERVED_1,
    TIMER_MODE_PWM_PHASE_CORRECT_TOP_OCRA,
    TIMER_MODE_RESERVED_2,
    TIMER_MODE_FAST_PWM_TOP_OCRA,

    /* Valid only for TIMER1 */
    TIMER_MODE_16_BIT_NORMAL = 1 << 4,
    TIMER_MODE_16_BIT_PWM_PHASE_CORRECT_8_BIT,
    TIMER_MODE_16_BIT_PWM_PHASE_CORRECT_9_BIT,
    TIMER_MODE_16_BIT_PWM_PHASE_CORRECT_10_BIT,
    TIMER_MODE_16_BIT_CTC_OCRA,
    TIMER_MODE_16_BIT_FAST_PWM_8_BIT,
    TIMER_MODE_16_BIT_FAST_PWM_9_BIT,
    TIMER_MODE_16_BIT_FAST_PWM_10_BIT,
    TIMER_MODE_16_BIT_PWM_CORR_PHASE_CORRECT_FREQ_ICR,
    TIMER_MODE_16_BIT_PWM_CORR_PHASE_CORRECT_FREQ_OCRA,
    TIMER_MODE_16_BIT_PWM_CORR_PHASE_ICR,
    TIMER_MODE_16_BIT_PWM_CORR_PHASE_OCRA,
    TIMER_MODE_16_BIT_CTC_ICR,
    TIMER_MODE_16_BIT_RESERVED,
    TIMER_MODE_16_BIT_FAST_PWM_ICR,
    TIMER_MODE_16_BIT_FAST_PWM_OCRA,
};

enum timer_compare_match_config
{
    TIMER_CM_DISABLED = 0,
    TIMER_CM_CHANGE_PIN_STATE,  // For Fast PWM and PWM with phase correction - if WGM02=0 - nothing 
    TIMER_CM_CLEAR_PIN,         // For Fast PWM - clear if CM, set if counter value is minimal, for PWM with phase coorection - clear if counting up, set if counting down
    TIMER_CM_SET_PIN,           // For Fast PWM - set if CM, clear if counter value is minimal, for PWM with phase coorection - set if counting up,  clear if counting down
};

//------------------------------------------------------------------------------

struct timer_obj
{
    enum timer_id id;
    enum timer_clock clock;

    timer_ovrf_cb ovrfv_cb;
    timer_comp_cb out_comp_a_cb;
    timer_comp_cb comp_b_cb;
    timer_capt_cb in_capt_cb;
};

struct timer_cfg
{  
    enum timer_id id;
    enum timer_clock clock;
    enum timer_async_clock async_clock;
    enum timer_mode mode;
    enum timer_compare_match_config com_a_cfg;
    enum timer_compare_match_config com_b_cfg;

    uint16_t counter_val;
    timer_ovrf_cb ovrfv_cb;

    uint16_t out_comp_a_val;
    uint16_t out_comp_b_val;
    timer_comp_cb out_comp_a_cb;
    timer_comp_cb out_comp_b_cb;
    
    uint16_t input_capture_val;
    bool input_capture_pullup;
    bool input_capture_noise_canceler;
    bool input_capture_rising_edge;
    timer_capt_cb in_capt_cb;
};

//------------------------------------------------------------------------------

/// @brief Initializes TIMER peripheral selected in cfg
/// @note When input capture callback is provided, ICP1 pin is configured as input
/// @note When output compare pin driving is enabled, OCxA/B pin is configured as output
/// @note PWM frequency to value calculation: CLK_MCU / (2 * TIMER_CLK_PRESCALER * freq )) - 1
/// @param obj specific timer object structure @ref timer_obj
/// @param cfg configuration structure @ref timer_cfg
/// @return true if initialized successfully, otherwise false
bool timer_init(struct timer_obj *obj, struct timer_cfg *cfg);

/// @brief Starts or stops TIMER peripheral selected by obj
/// @note Each function call restarts timer
/// @param obj specific timer object structure @ref timer_obj
/// @param start true for start, false for stop
void timer_start(struct timer_obj *obj, bool start);

/// @brief Gets current TIMER peripheral counter value selected in obj
/// @param obj specific timer object structure @ref timer_obj
/// @return current counter value
uint16_t timer_get_val(struct timer_obj *obj);

/// @brief Restores registers to default state for TIMER peripheral selected in obj and resets object pointer in internal context
/// @param obj specific timer object structure @ref timer_obj
void timer_deinit(struct timer_obj *obj);

//------------------------------------------------------------------------------

/// @brief Initializes system_timer based on Timer 0 with 1 ms tick
void system_timer_init(void);

/// @brief Gets current system timer value
/// @return current system timer value [ms]
uint32_t system_timer_get(void);

/// @brief Checks if current tickstamp is older compared to timeout and previous tickstamp
/// @param tickstamp precious timestamp
/// @param timeout timeout
/// @return true if timeout passed
bool system_timer_timeout_passed(uint32_t tickstamp, uint32_t timeout);

//------------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif

#endif /* TIMER_H_ */

//------------------------------------------------------------------------------
