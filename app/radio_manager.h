//------------------------------------------------------------------------------

/// @file radio_manager.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef RADIO_MANAGER_H_
#define RADIO_MANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>

//------------------------------------------------------------------------------

/// @brief Initializes Radio Manager
/// @return true if initialization was successful, false otherwise
bool radio_manager_init(void);

/// @brief Processes Radio Manager
/// @note This function should be called in the main loop
void radio_manager_process(void);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* RADIO_MANAGER_H_ */

//------------------------------------------------------------------------------
