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

#include "buzzer.h"

//------------------------------------------------------------------------------

bool buzzer_init(struct buzzer_obj *obj, struct buzzer_cfg *cfg)
{
	if (!cfg || !cfg->init || !cfg->play || !cfg->stop)
		return false;

	obj->init = cfg->init;
	obj->play = cfg->play;
	obj->stop = cfg->stop;
	obj->deinit = cfg->deinit;

	return obj->init();
}

void buzzer_play_pattern(struct buzzer_obj *obj, const struct buzzer_note pattern[], uint16_t size, uint16_t bpm)
{
	for (uint16_t i = 0; i < (size / sizeof(struct buzzer_note)); i++)
	{
		obj->play(pattern[i].tone, pattern[i].note / bpm);
	}
}

void buzzer_stop_pattern(struct buzzer_obj *obj)
{
	obj->stop();
}

void buzzer_set_pattern(struct buzzer_obj *obj, const struct buzzer_note pattern[], uint16_t size, uint16_t bpm)
{
	obj->current_pattern = pattern;
	obj->current_pattern_size = size;
	obj->current_pattern_bpm = bpm;
	obj->current_step = 0;
}

void buzzer_process(struct buzzer_obj *obj)
{
	if (!obj || !obj->current_pattern)
		return;

	if (obj->play(obj->current_pattern[obj->current_step].tone, obj->current_pattern[obj->current_step].note / obj->current_pattern_bpm))
		return; // Note is still played

	/* Callback returned false - note is already played, go to next note */
	obj->current_step == (obj->current_pattern_size / sizeof(struct buzzer_note)) - 1 ? obj->current_step = 0 : obj->current_step++;
}

bool buzzer_deinit(struct buzzer_obj *obj)
{
	return obj->deinit();
}

//------------------------------------------------------------------------------
