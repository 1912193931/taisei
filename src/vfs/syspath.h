/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2019, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2019, Andrei Alexeyev <akari@alienslab.net>.
 */

#ifndef IGUARD_vfs_syspath_h
#define IGUARD_vfs_syspath_h

#include "taisei.h"

#include "private.h"
#include "syspath_public.h"

extern char vfs_syspath_preferred_separator;
bool vfs_syspath_init(VFSNode *node, const char *path);
void vfs_syspath_normalize(char *buf, size_t bufsize, const char *path);

#endif // IGUARD_vfs_syspath_h
