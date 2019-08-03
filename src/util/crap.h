/*
 * This software is licensed under the terms of the MIT License.
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2019, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2019, Andrei Alexeyev <akari@taisei-project.org>.
 */

#ifndef IGUARD_util_crap_h
#define IGUARD_util_crap_h

#include "taisei.h"

#include <SDL.h>

void* memdup(const void *src, size_t size) attr_returns_allocated attr_nonnull(1);
void inherit_missing_pointers(uint num, void *dest[num], void *const base[num]) attr_nonnull(2, 3);
bool is_main_thread(void);

INLINE uint32_t float_to_bits(float f) {
	union { uint32_t i; float f; } u;
	u.f = f;
	return u.i;
}

INLINE float bits_to_float(uint32_t i) {
	union { uint32_t i; float f; } u;
	u.i = i;
	return u.f;
}

extern SDL_threadID main_thread_id;

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof(*(arr)))

#endif // IGUARD_util_crap_h
