/*
 * This software is licensed under the terms of the MIT License.
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2019, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2019, Andrei Alexeyev <akari@taisei-project.org>.
 */

#ifndef IGUARD_vfs_union_public_h
#define IGUARD_vfs_union_public_h

#include "taisei.h"

bool vfs_create_union_mountpoint(const char *mountpoint)
	attr_nonnull(1);

#endif // IGUARD_vfs_union_public_h
