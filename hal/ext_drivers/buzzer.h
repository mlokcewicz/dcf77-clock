//------------------------------------------------------------------------------

/// @file buzzer.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef BUZZER_H_
#define BUZZER_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>

//------------------------------------------------------------------------------

#define BUZZER_NOTE_WHOLE 			((4) * (BUZZER_NOTE_QUARTER))
#define BUZZER_NOTE_HALF 			((2) * (BUZZER_NOTE_QUARTER))
#define BUZZER_NOTE_QUARTER 		(60000UL)
#define BUZZER_NOTE_EIGHTH 			((BUZZER_NOTE_QUARTER) / (2))
#define BUZZER_NOTE_NINETH 			((BUZZER_NOTE_WHOLE) / (9))
#define BUZZER_NOTE_TWELEVE 		((BUZZER_NOTE_WHOLE) / (12))
#define BUZZER_NOTE_SIXTEENTH 		((BUZZER_NOTE_QUARTER) / (4))
#define BUZZER_NOTE_THIRTY_SECOND 	((BUZZER_NOTE_QUARTER) / (8))

#define BUZZER_TONE_B0    		31
#define BUZZER_TONE_C1    		33
#define BUZZER_TONE_CS1   		35
#define BUZZER_TONE_D1    		37
#define BUZZER_TONE_DS1   		39
#define BUZZER_TONE_E1    		41
#define BUZZER_TONE_F1    		44
#define BUZZER_TONE_FS1   		46
#define BUZZER_TONE_G1    		49
#define BUZZER_TONE_GS1   		52
#define BUZZER_TONE_A1    		55
#define BUZZER_TONE_AS1   		58
#define BUZZER_TONE_B1    		62
#define BUZZER_TONE_C2    		65
#define BUZZER_TONE_CS2   		69
#define BUZZER_TONE_D2    		73
#define BUZZER_TONE_DS2   		78
#define BUZZER_TONE_E2    		82
#define BUZZER_TONE_F2    		87
#define BUZZER_TONE_FS2   		93
#define BUZZER_TONE_G2    		98
#define BUZZER_TONE_GS2   		104
#define BUZZER_TONE_A2    		110
#define BUZZER_TONE_AS2   		117
#define BUZZER_TONE_B2    		123
#define BUZZER_TONE_C3    		131
#define BUZZER_TONE_CS3   		139
#define BUZZER_TONE_D3    		147
#define BUZZER_TONE_DS3   		156
#define BUZZER_TONE_E3    		165
#define BUZZER_TONE_F3    		175
#define BUZZER_TONE_FS3   		185
#define BUZZER_TONE_G3    		196
#define BUZZER_TONE_GS3   		208
#define BUZZER_TONE_A3    		220
#define BUZZER_TONE_AS3   		233
#define BUZZER_TONE_B3    		247
#define BUZZER_TONE_C4    		262
#define BUZZER_TONE_CS4   		277
#define BUZZER_TONE_D4    		294
#define BUZZER_TONE_DS4   		311
#define BUZZER_TONE_E4    		330
#define BUZZER_TONE_F4    		349
#define BUZZER_TONE_FS4   		370
#define BUZZER_TONE_G4    		392
#define BUZZER_TONE_GS4   		415
#define BUZZER_TONE_A4    		440
#define BUZZER_TONE_AS4   		466
#define BUZZER_TONE_B4    		494
#define BUZZER_TONE_C5    		523
#define BUZZER_TONE_CS5   		554
#define BUZZER_TONE_D5    		587
#define BUZZER_TONE_DS5   		622
#define BUZZER_TONE_E5    		659
#define BUZZER_TONE_F5    		698
#define BUZZER_TONE_FS5   		740
#define BUZZER_TONE_G5    		784
#define BUZZER_TONE_GS5   		831
#define BUZZER_TONE_A5    		880
#define BUZZER_TONE_AS5   		932
#define BUZZER_TONE_B5    		988
#define BUZZER_TONE_C6    		1047
#define BUZZER_TONE_CS6   		1109
#define BUZZER_TONE_D6    		1175
#define BUZZER_TONE_DS6   		1245
#define BUZZER_TONE_E6    		1319
#define BUZZER_TONE_F6    		1397
#define BUZZER_TONE_FS6   		1480
#define BUZZER_TONE_G6    		1568
#define BUZZER_TONE_GS6   		1661
#define BUZZER_TONE_A6    		1760
#define BUZZER_TONE_AS6   		1865
#define BUZZER_TONE_B6    		1976
#define BUZZER_TONE_C7    		2093
#define BUZZER_TONE_CS7   		2217
#define BUZZER_TONE_D7    		2349
#define BUZZER_TONE_DS7   		2489
#define BUZZER_TONE_E7    		2637
#define BUZZER_TONE_F7    		2794
#define BUZZER_TONE_FS7   		2960
#define BUZZER_TONE_G7  		3136
#define BUZZER_TONE_GS7 		3322
#define BUZZER_TONE_A7  		3520
#define BUZZER_TONE_AS7 		3729
#define BUZZER_TONE_B7  		3951
#define BUZZER_TONE_C8  		4186
#define BUZZER_TONE_CS8 		4435
#define BUZZER_TONE_D8  		4699
#define BUZZER_TONE_DS8 		4978

#define BUZZER_TONE_STOP 		0

//------------------------------------------------------------------------------

typedef bool (*buzzer_init_cb)(void);
typedef bool (*buzzer_play_cb)(uint16_t tone, uint16_t time_ms);
typedef void (*buzzer_stop_cb)(void);
typedef bool (*buzzer_deinit_cb)(void);

//------------------------------------------------------------------------------

struct buzzer_obj
{
	buzzer_init_cb init;
	buzzer_play_cb play;
	buzzer_stop_cb stop;
	buzzer_deinit_cb deinit;

	const struct buzzer_note *current_pattern;
	uint16_t current_pattern_size;
	uint16_t current_pattern_bpm;
	uint16_t current_step;
};

struct buzzer_cfg
{
	buzzer_init_cb init;
	buzzer_play_cb play;
	buzzer_stop_cb stop;
	buzzer_deinit_cb deinit;
};

struct buzzer_note
{
	uint16_t tone;
	uint32_t note;
};

//------------------------------------------------------------------------------

/// @brief Initializes buzzer low level driver
/// @param obj buzzer object structure pointer
/// @param cfg low level callbacks structure @ref struct buzzer_cfg
/// @return propagates buzzer_init_ll callback return value if cfg structure is valid
bool buzzer_init(struct buzzer_obj *obj, struct buzzer_cfg *cfg);

/// @brief Plays selected audio pattern
/// @param obj buzzer object structure pointer
/// @param pattern selected autio pattern array @ref struct note
/// @param size selected autio pattern array size in bytes
/// @param bpm selected autio pattern BPM
void buzzer_play_pattern(struct buzzer_obj *obj, const struct buzzer_note pattern[], uint16_t size, uint16_t bpm);

/// @brief Stops actual audio pattern
/// @param obj buzzer object structure pointer
void buzzer_stop_pattern(struct buzzer_obj *obj);

/// @brief Sets current pattern for non-blocking play mode (@ref buzzer_process())
/// @param obj buzzer object structure pointer
/// @param pattern selected autio pattern array @ref struct note
/// @param size selected autio pattern array size in bytes
/// @param bpm selected autio pattern BPM
void buzzer_set_pattern(struct buzzer_obj *obj, const struct buzzer_note pattern[], uint16_t size, uint16_t bpm);

/// @brief Processes current audio pattern
/// @param obj buzzer object structure pointer
void buzzer_process(struct buzzer_obj *obj);

/// @brief Deinitializes buzzer low level driver and resets context
/// @param obj buzzer object structure pointer
/// @return propagates buzzer_deinit_ll callback return value if cfg structure is valid
bool buzzer_deinit(struct buzzer_obj *obj);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* BUZZER_H_ */

//------------------------------------------------------------------------------
