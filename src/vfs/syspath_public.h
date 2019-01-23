/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2019, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2019, Andrei Alexeyev <akari@alienslab.net>.
 */

#ifndef IGUARD_vfs_syspath_public_h
#define IGUARD_vfs_syspath_public_h

#include "taisei.h"

enum {
	VFS_SYSPATH_MOUNT_READONLY = (1 << 0),
	VFS_SYSPATH_MOUNT_MKDIR = (1 << 1),
};

bool vfs_mount_syspath(const char *mountpoint, const char *fspath, uint flags)
	attr_nonnull(1, 2) attr_nodiscard;

#endif // IGUARD_vfs_syspath_public_h
