/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2019, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2019, Andrei Alexeyev <akari@taisei-project.org>.
 */

#include "taisei.h"

#include "gles30.h"
#include "../glescommon/gles.h"
#include "../glescommon/texture.h"

static void gles30_init(void) {
	gles_init(&_r_backend_gles30, 3, 0);
}

RendererBackend _r_backend_gles30 = {
	.name = "gles30",
	.funcs = {
		.init = gles30_init,
		.screenshot = gles_screenshot,
	},
	.custom = &(GLBackendData) {
		.vtable = {
			.texture_type_info = gles_texture_type_info,
			.texture_format_caps = gles_texture_format_caps,
			.init_context = gles_init_context,
		}
	},
};
