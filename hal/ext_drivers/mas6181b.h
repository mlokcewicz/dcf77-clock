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

#ifndef MAS6181B_H_
#define MAS6181B_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>

//------------------------------------------------------------------------------

typedef void (*mas6181b_io_init_cb)(void);
typedef void (*mas6181b_pwr_down_cb)(bool pwr_down);
typedef bool (*mas6181b_get_cb)(void);

//------------------------------------------------------------------------------

struct mas6181b_cfg
{
    mas6181b_io_init_cb io_init;
    mas6181b_pwr_down_cb pwr_down;
    mas6181b_get_cb get;
};

struct mas6181b_obj
{
    mas6181b_io_init_cb io_init;
    mas6181b_pwr_down_cb pwr_down;
    mas6181b_get_cb get;
};

//------------------------------------------------------------------------------

/// @brief Initializes MAS6181B module accroding to given configuration @ref struct mas6181b_cfg
/// @param obj given MAS6181B object pointer
/// @param cfg given configuration structure pointer
/// @return true if initialized correctly, otherwiste false
bool mas6181b_init(struct mas6181b_obj *obj, struct mas6181b_cfg *cfg);

/// @brief Turns MAS6181B into power down state
/// @param obj given MAS6181B object pointer
/// @param power_down true for power down
/// @return true if powered down succesfully 
bool mas6181b_power_down(struct mas6181b_obj *obj, bool power_down);

/// @brief Gets MAS6181B output state
/// @param obj given MAS6181B object pointer
/// @return MAS6181B output state    
bool mas6181b_get_state(struct mas6181b_obj *obj);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* MAS6181B_H_ */

//------------------------------------------------------------------------------
