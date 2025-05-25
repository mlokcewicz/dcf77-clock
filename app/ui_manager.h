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

#ifndef UI_MANAGER_H_
#define UI_MANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>

//------------------------------------------------------------------------------

/// @brief Initializes UI Manager
/// @return true if initialization was successful, false otherwise 
bool ui_manager_init(void);

/// @brief Processes UI Manager
/// @note This function should be called in the main loop
void ui_manager_process(void);  

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* UI_MANAGER_H_ */

//------------------------------------------------------------------------------
