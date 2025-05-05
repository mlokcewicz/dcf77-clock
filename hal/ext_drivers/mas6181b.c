//------------------------------------------------------------------------------

/// @file mas6181b.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "mas6181b.h"

//------------------------------------------------------------------------------

bool mas6181b_init(struct mas6181b_obj *obj, struct mas6181b_cfg *cfg)
{
    if (!obj || !cfg || !cfg->io_init || !cfg->pwr_down)
        return false;

    obj->io_init = cfg->io_init;
    obj->pwr_down = cfg->pwr_down;

    obj->io_init();

    obj->pwr_down(false);

    return true;
}

bool mas6181b_power_down(struct mas6181b_obj *obj, bool power_down)
{
    if (!obj || !obj->pwr_down)
        return false;

    obj->pwr_down(power_down);

    return true;
}

//------------------------------------------------------------------------------
