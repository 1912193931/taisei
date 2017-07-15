/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information.
 * ---
 * Copyright (C) 2011, Lukas Weber <laochailan@web.de>
 */

#ifndef SFX_H
#define SFX_H

#include <stdbool.h>

typedef struct Sound {
	int lastplayframe;
	void *impl;
} Sound;

char* sound_path(const char *name);
bool check_sound_path(const char *path);
void* load_sound_begin(const char *path, unsigned int flags);
void* load_sound_end(void *opaque, const char *path, unsigned int flags);
void unload_sound(void *snd);

#define SFX_PATH_PREFIX "res/sfx/"

#endif
