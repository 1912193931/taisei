/*
 * This software is licensed under the terms of the MIT License.
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2019, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2019, Andrei Alexeyev <akari@taisei-project.org>.
 */

#ifndef IGUARD_renderer_glescommon_gles_h
#define IGUARD_renderer_glescommon_gles_h

#include "taisei.h"

#include "../glcommon/vtable.h"

void gles_init(RendererBackend *gles_backend, int major, int minor);
void gles_init_context(SDL_Window *w);
bool gles_screenshot(Pixmap *out);

#endif // IGUARD_renderer_glescommon_gles_h
