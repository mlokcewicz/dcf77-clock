//------------------------------------------------------------------------------

/// @file clock_manager.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef CLOCK_MANAGER_H_
#define CLOCK_MANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>

//------------------------------------------------------------------------------

/// @brief Initializes Clock Manager
/// @return true if initialization was successful, false otherwise
bool clock_manager_init(void);

/// @brief Processes Clock Manager
/// @note This function should be called in the main loop
void clock_manager_process(void);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* CLOCK_MANAGER_H_ */

//------------------------------------------------------------------------------
