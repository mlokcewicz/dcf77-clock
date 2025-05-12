//------------------------------------------------------------------------------

/// @file ui_manager.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

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
