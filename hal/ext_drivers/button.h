//------------------------------------------------------------------------------

/*
 * Copyright 2025 Michal Lokcewicz
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//------------------------------------------------------------------------------

#ifndef BUTTON_H_
#define BUTTON_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>

//------------------------------------------------------------------------------

typedef bool (*button_init_cb)(void);
typedef bool (*button_get_state_cb)(void);
typedef void (*button_pressed_cb)(void);
typedef bool (*button_deinit_cb)(void);

//------------------------------------------------------------------------------

struct button_cfg
{
	button_init_cb init;
	button_get_state_cb get_state;
	button_pressed_cb pressed;
	button_deinit_cb deinit;
    
    bool active_low;
    bool irq_cfg;

	uint16_t debounce_counter_initial_value;
	uint16_t autopress_counter_initial_value;
};

struct button_obj
{
	button_init_cb init;
	button_get_state_cb get_state;
	button_pressed_cb pressed;
	button_deinit_cb deinit;
    
    bool active_low;
    bool irq_cfg;
    
    volatile bool prev_state;
	uint16_t debounce_counter_initial_value;
	volatile uint16_t debounce_counter_current_value;
	uint16_t autopress_counter_initial_value;
	volatile uint16_t autopress_counter_current_value;
};

//------------------------------------------------------------------------------

/// @brief Initializes button low level driver
/// @param obj button object structure pointer
/// @param cfg low level callbacks structure @ref struct button_cfg
/// @return propagates button_init_cb callback return value if cfg structure is valid
bool button_init(struct button_obj *obj, struct button_cfg *cfg);

/// @brief Updates button state and call callback when it's pressed
/// @note Should be called periodically or inside ISR if configured
/// @param obj button object structure pointer
/// @return current button state
bool button_process(struct button_obj *obj);

/// @brief Deinitializes button low level driver and resets context
/// @param obj button object structure pointer
/// @return propagates button_deinit_cb callback return value if cfg structure is valid
bool button_deinit(struct button_obj *obj);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* BUTTON_H_ */

//------------------------------------------------------------------------------
