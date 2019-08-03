/*
 * This software is licensed under the terms of the MIT License.
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2019, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2019, Andrei Alexeyev <akari@taisei-project.org>.
 */

#include "taisei.h"

#include <time.h>

#include "systime.h"

void get_system_time(SystemTime *systime) {
	#if defined(TAISEI_BUILDCONF_HAVE_TIMESPEC)
	timespec_get(systime, TIME_UTC);
	#else
	systime->tv_sec = time(NULL);
	systime->tv_nsec = 0;
	#endif
}
