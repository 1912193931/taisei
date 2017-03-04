/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information.
 * ---
 * Copyright (C) 2011, Lukas Weber <laochailan@web.de>
 */

#ifndef AUDIO_H
#define AUDIO_H

#include "resource/sfx.h"
#include "resource/bgm.h"

typedef struct CurrentBGM {
    char *name;
    char *title;
    int isboss;
    int started_at;
    Music *music;
} CurrentBGM;

extern CurrentBGM current_bgm;

void audio_backend_init(void);
void audio_backend_shutdown(void);
bool audio_backend_initialized(void);
void audio_backend_set_sfx_volume(float gain);
void audio_backend_set_bgm_volume(float gain);
bool audio_backend_music_is_paused(void);
bool audio_backend_music_is_playing(void);
void audio_backend_music_resume(void);
void audio_backend_music_stop(void);
void audio_backend_music_pause(void);
bool audio_backend_music_play(void *impl);
bool audio_backend_sound_play(void *impl);

void audio_init(void);
void audio_shutdown(void);

void play_sound(const char *name);
void play_ui_sound(const char *name);
void reset_sounds(void);

Sound* get_sound(const char *name);
Music* get_music(const char *music);

void start_bgm(const char *name);
void stop_bgm(void);
void resume_bgm(void);
void save_bgm(void);
void restore_bgm(void);

#endif
