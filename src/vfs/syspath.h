
#ifndef TAISEI_VFS_SYSPATH
#define TAISEI_VFS_SYSPATH

#include "private.h"

extern char vfs_syspath_prefered_separator;
bool vfs_syspath_init(VFSNode *node, const char *path);

#endif
