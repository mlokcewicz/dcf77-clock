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

#include "mas6181b.h"

//------------------------------------------------------------------------------

bool mas6181b_init(struct mas6181b_obj *obj, struct mas6181b_cfg *cfg)
{
    if (!obj || !cfg || !cfg->io_init || !cfg->pwr_down)
        return false;

    obj->io_init = cfg->io_init;
    obj->pwr_down = cfg->pwr_down;
    obj->get = cfg->get;

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

bool mas6181b_get_state(struct mas6181b_obj *obj)
{
    return obj && obj->get && obj->get();
}

//------------------------------------------------------------------------------
