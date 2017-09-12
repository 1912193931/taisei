/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2017, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2017, Andrei Alexeyev <akari@alienslab.net>.
 */

#ifndef ENDING_H
#define ENDING_H

#include "resource/texture.h"

enum {
	ENDING_FADE_OUT = 200,
	ENDING_FADE_TIME = 60,
};

enum {
    // do not reorder these or change the values
    ENDING_BAD_1,
    ENDING_BAD_2,
    ENDING_GOOD_1,
    ENDING_GOOD_2,

    NUM_ENDINGS,
};

typedef struct EndingEntry EndingEntry;
struct EndingEntry {
	char *msg;
	Texture *tex;

	int time;
};

typedef struct Ending Ending;
struct Ending {
	EndingEntry *entries;
	int count;
	int duration;

	int pos;
};

void add_ending_entry(Ending *e, int time, char *msg, char *tex);

void create_ending(Ending *e);
void ending_loop(void);
void free_ending(Ending *e);
void ending_preload(void);

#endif
