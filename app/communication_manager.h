//------------------------------------------------------------------------------

/// @file communication_manager.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef COMMUNICATION_MANAGER_H_
#define COMMUNICATION_MANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>

//------------------------------------------------------------------------------

/// @brief Initializes Communication Manager
/// @return true if initialization was successful, false otherwise
bool communication_manager_init(void);

/// @brief Processes Communication Manager
/// @note This function should be called in the main loop
void communication_manager_process(void);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* COMMUNICATION_MANAGER_H_ */

//------------------------------------------------------------------------------
