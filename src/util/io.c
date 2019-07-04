/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2019, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2019, Andrei Alexeyev <akari@taisei-project.org>.
 */

#include "taisei.h"

#include "io.h"
#include "log.h"
#include "vfs/public.h"
#include "assert.h"
#include "stringops.h"
#include "rwops/rwops_autobuf.h"
#include "util/crap.h"

char* read_all(const char *filename, size_t *outsize) {
	SDL_RWops *file = vfs_open(filename, VFS_MODE_READ);

	if(!file) {
		log_error("VFS error: %s", vfs_get_error());
		return NULL;
	}

	char *result, buf[BUFSIZ];
	SDL_RWops *abuf = SDL_RWAutoBuffer((void**)&result, sizeof(buf));
	assume(abuf != NULL);

	size_t size = 0;

	for(;;) {
		size_t read = SDL_RWread(file, buf, 1, sizeof(buf));

		if(read == 0) {
			break;
		}

		SDL_RWwrite(abuf, buf, 1, read);
		size += read;
	}

	SDL_RWclose(file);
	SDL_WriteU8(abuf, 0);
	result = memdup(result, size + 1);
	SDL_RWclose(abuf);

	if(outsize) {
		*outsize = size;
	}

	return result;
}

char* SDL_RWgets(SDL_RWops *rwops, char *buf, size_t bufsize) {
	char c, *ptr = buf, *end = buf + bufsize - 1;
	assert(end > ptr);

	while((c = SDL_ReadU8(rwops)) && ptr <= end) {
		if((*ptr++ = c) == '\n')
			break;
	}

	if(ptr == buf)
		return NULL;

	if(ptr > end) {
		*end = 0;
		log_warn("Line too long (%zu bytes max): %s", bufsize, buf);
	} else {
		*ptr = 0;
	}

	return buf;
}

char* SDL_RWgets_realloc(SDL_RWops *rwops, char **buf, size_t *bufsize) {
	char c, *ptr = *buf, *end = *buf + *bufsize - 1;
	assert(end >= ptr);

	while((c = SDL_ReadU8(rwops))) {
		*ptr++ = c;

		if(ptr > end) {
			ptrdiff_t ofs = ptr - *buf;
			*bufsize *= 2;
			*buf = realloc(*buf, *bufsize);
			end = *buf + *bufsize - 1;
			ptr = *buf + ofs;
			*end = 0;
		}

		if(c == '\n') {
			break;
		}
	}

	if(ptr == *buf)
		return NULL;

	assert(ptr <= end);
	*ptr = 0;

	return *buf;
}

size_t SDL_RWprintf(SDL_RWops *rwops, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char *str = vstrfmt(fmt, args);
	va_end(args);

	size_t ret = SDL_RWwrite(rwops, str, 1, strlen(str));
	free(str);

	return ret;
}

void tsfprintf(FILE *out, const char *restrict fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vfprintf(out, fmt, args);
	va_end(args);
}

char* try_path(const char *prefix, const char *name, const char *ext) {
	char *p = strjoin(prefix, name, ext, NULL);

	if(vfs_query(p).exists) {
		return p;
	}

	free(p);
	return NULL;
}
