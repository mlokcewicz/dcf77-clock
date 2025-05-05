//------------------------------------------------------------------------------

/// @file mas6181b.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

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

//------------------------------------------------------------------------------

struct mas6181b_cfg
{
    mas6181b_io_init_cb io_init;
    mas6181b_pwr_down_cb pwr_down;
};

struct mas6181b_obj
{
    mas6181b_io_init_cb io_init;
    mas6181b_pwr_down_cb pwr_down;
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

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* MAS6181B_H_ */

//------------------------------------------------------------------------------
