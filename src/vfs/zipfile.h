/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2019, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2019, Andrei Alexeyev <akari@alienslab.net>.
 */

#ifndef IGUARD_vfs_zipfile_h
#define IGUARD_vfs_zipfile_h

#include "taisei.h"

#include "private.h"

bool vfs_zipfile_init(VFSNode *node, VFSNode *source);

#endif // IGUARD_vfs_zipfile_h
