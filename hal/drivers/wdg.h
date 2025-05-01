//------------------------------------------------------------------------------

/// @file wdg.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef WDG_H_
#define WDG_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>

//------------------------------------------------------------------------------

enum wdg_mode
{
    WDG_MODE_INT = 1 << 0,
    WDG_MODE_RST = 1 << 1,
};

enum wdg_period
{
    WDG_PERIOD_15MS = 0,
    WDG_PERIOD_30MS,
    WDG_PERIOD_60MS,
    WDG_PERIOD_120MS, 
    WDG_PERIOD_250MS, 
    WDG_PERIOD_500MS, 
    WDG_PERIOD_1S, 
    WDG_PERIOD_2S, 
    WDG_PERIOD_4S, 
    WDG_PERIOD_8S
};

//------------------------------------------------------------------------------

typedef void (*wdg_int_cb)(void);

//------------------------------------------------------------------------------

/// @brief Configures and starts wachdog timer
/// @note to use interrupt mode, interrupts have to be enabled globally 
/// @param mode selected mode @ref wdg_mode 
/// @param period selected restart or interrupt period @ref wdg_period
/// @param int_cb interrupt callback pointer
bool wdg_init(enum wdg_mode mode, enum wdg_period period, wdg_int_cb int_cb);

/// @brief Reloads watchdog timer
void wdg_feed(void);

/// @brief Performs system restart (never returns to caller)
void wdg_system_reset(void);

/// @brief Switch off watchdog timer and resets internal context
void wdg_deinit(void);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* WDG_H_ */

//------------------------------------------------------------------------------
