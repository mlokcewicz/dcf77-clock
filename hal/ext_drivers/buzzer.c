//------------------------------------------------------------------------------

/// @file buzzer.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "buzzer.h"

//------------------------------------------------------------------------------

bool buzzer_init(struct buzzer_obj *obj, struct buzzer_cfg *cfg)
{
	if (!cfg || !cfg->init || !cfg->play || !cfg->stop || !cfg->deinit)
		return false;

	obj->init = cfg->init;
	obj->play = cfg->play;
	obj->stop = cfg->stop;
	obj->deinit = cfg->deinit;

	return obj->init();
}

void buzzer_play_pattern(struct buzzer_obj *obj, const struct buzzer_note song[], uint16_t size, uint16_t bpm)
{
	for (uint16_t i = 0; i < (size / sizeof(struct buzzer_note)); i++)
	{
		obj->play(song[i].tone, song[i].note / bpm);
	}
}

void buzzer_stop_pattern(struct buzzer_obj *obj)
{
	obj->stop();
}

bool buzzer_deinit(struct buzzer_obj *obj)
{
	return obj->deinit();
}

//------------------------------------------------------------------------------
