/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information.
 * ---
 * Copyright (C) 2011, Lukas Weber <laochailan@web.de>
 * Copyright (C) 2012, Alexeyew Andrew <http://akari.thebadasschoobs.org/>
 */

#include "replay.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "global.h"
#include "paths/native.h"

static uint8_t replay_magic_header[] = REPLAY_MAGIC_HEADER;

void replay_init(Replay *rpy) {
	memset(rpy, 0, sizeof(Replay));
	stralloc(&rpy->playername, config_get_str(CONFIG_PLAYERNAME));
	log_debug("Replay at %p initialized for writing", (void*)rpy);
}

ReplayStage* replay_create_stage(Replay *rpy, StageInfo *stage, uint64_t seed, Difficulty diff, uint32_t points, Player *plr) {
	ReplayStage *s;

	rpy->stages = (ReplayStage*)realloc(rpy->stages, sizeof(ReplayStage) * (++rpy->numstages));
	s = rpy->stages + rpy->numstages - 1;
	memset(s, 0, sizeof(ReplayStage));

	s->capacity = REPLAY_ALLOC_INITIAL;
	s->events = (ReplayEvent*)malloc(sizeof(ReplayEvent) * s->capacity);

	s->stage = stage->id;
	s->seed	= seed;
	s->diff	= diff;
	s->points = points;

	s->plr_pos_x = floor(creal(plr->pos));
	s->plr_pos_y = floor(cimag(plr->pos));

	s->plr_focus = plr->focus;
	s->plr_char	= plr->cha;
	s->plr_shot	= plr->shot;
	s->plr_lifes = plr->lifes;
	s->plr_bombs = plr->bombs;
	s->plr_power = plr->power;
	s->plr_inputflags = plr->inputflags;

	log_debug("Created a new stage %p in replay %p", (void*)s, (void*)rpy);
	return s;
}

void replay_stage_sync_player_state(ReplayStage *stg, Player *plr) {
	plr->points = stg->points;
	plr->shot = stg->plr_shot;
	plr->cha = stg->plr_char;
	plr->pos = stg->plr_pos_x + I * stg->plr_pos_y;
	plr->focus = stg->plr_focus;
	plr->lifes = stg->plr_lifes;
	plr->bombs = stg->plr_bombs;
	plr->power = stg->plr_power;
	plr->inputflags = stg->plr_inputflags;
}

static void replay_destroy_stage(ReplayStage *stage) {
	free(stage->events);
	memset(stage, 0, sizeof(ReplayStage));
}

void replay_destroy_events(Replay *rpy) {
	if(!rpy) {
		return;
	}

	if(rpy->stages) {
		for(int i = 0; i < rpy->numstages; ++i) {
			ReplayStage *stg = rpy->stages + i;
			free(stg->events);
			stg->events = NULL;
		}
	}
}

void replay_destroy(Replay *rpy) {
	if(!rpy) {
		return;
	}

	if(rpy->stages) {
		for(int i = 0; i < rpy->numstages; ++i) {
			replay_destroy_stage(rpy->stages + i);
		}

		free(rpy->stages);
	}

	free(rpy->playername);

	memset(rpy, 0, sizeof(Replay));
	log_debug("Replay at %p destroyed", (void*)rpy);
}

void replay_stage_event(ReplayStage *stg, uint32_t frame, uint8_t type, int16_t value) {
	if(!stg) {
		return;
	}

	ReplayStage *s = stg;
	ReplayEvent *e = s->events + s->numevents;
	e->frame = frame;
	e->type = type;
	e->value = (uint16_t)value;
	s->numevents++;

	if(s->numevents >= s->capacity) {
		log_debug("Replay stage reached its capacity of %d, reallocating", s->capacity);
		s->capacity *= 2;
		s->events = (ReplayEvent*)realloc(s->events, sizeof(ReplayEvent) * s->capacity);
		log_debug("The new capacity is %d", s->capacity);
	}

	if(type == EV_OVER) {
		log_debug("The replay is OVER");
	}
}

static void replay_write_string(SDL_RWops *file, char *str) {
	SDL_WriteLE16(file, strlen(str));
	SDL_RWwrite(file, str, 1, strlen(str));
}

static int replay_write_stage_event(ReplayEvent *evt, SDL_RWops *file) {
	SDL_WriteLE32(file, evt->frame);
	SDL_WriteU8(file, evt->type);
	SDL_WriteLE16(file, evt->value);

	return true;
}

static uint32_t replay_calc_stageinfo_checksum(ReplayStage *stg) {
	uint32_t cs = 0;
	cs += stg->stage;
	cs += stg->seed;
	cs += stg->diff;
	cs += stg->points;
	cs += stg->plr_char;
	cs += stg->plr_shot;
	cs += stg->plr_pos_x;
	cs += stg->plr_pos_y;
	cs += stg->plr_focus;
	cs += stg->plr_power;
	cs += stg->plr_lifes;
	cs += stg->plr_bombs;
	cs += stg->plr_inputflags;
	cs += stg->numevents;
	return cs;
}

static int replay_write_stage(ReplayStage *stg, SDL_RWops *file) {
	SDL_WriteLE16(file, stg->stage);
	SDL_WriteLE32(file, stg->seed);
	SDL_WriteU8(file, stg->diff);
	SDL_WriteLE32(file, stg->points);
	SDL_WriteU8(file, stg->plr_char);
	SDL_WriteU8(file, stg->plr_shot);
	SDL_WriteLE16(file, stg->plr_pos_x);
	SDL_WriteLE16(file, stg->plr_pos_y);
	SDL_WriteU8(file, stg->plr_focus);
	SDL_WriteLE16(file, stg->plr_power);
	SDL_WriteU8(file, stg->plr_lifes);
	SDL_WriteU8(file, stg->plr_bombs);
	SDL_WriteU8(file, stg->plr_inputflags);
	SDL_WriteLE16(file, stg->numevents);
	SDL_WriteLE32(file, 1 + ~replay_calc_stageinfo_checksum(stg));

	return true;
}

int replay_write(Replay *rpy, SDL_RWops *file, bool compression) {
	uint8_t *u8_p;
	int i, j;

	for(u8_p = replay_magic_header; *u8_p; ++u8_p) {
		SDL_WriteU8(file, *u8_p);
	}

	uint16_t version = REPLAY_STRUCT_VERSION;

	if(compression) {
		version |= REPLAY_VERSION_COMPRESSION_BIT;
	}

	SDL_WriteLE16(file, version);

	void *buf;
	SDL_RWops *abuf = NULL;
	SDL_RWops *vfile = file;

	if(compression) {
		abuf = SDL_RWAutoBuffer(&buf, 64);
		vfile = SDL_RWWrapZWriter(abuf, REPLAY_COMPRESSION_CHUNK_SIZE, false);
	}

	replay_write_string(vfile, rpy->playername);
	SDL_WriteLE16(vfile, rpy->numstages);

	for(i = 0; i < rpy->numstages; ++i) {
		if(!replay_write_stage(rpy->stages + i, vfile)) {
			if(compression) {
				SDL_RWclose(vfile);
				SDL_RWclose(abuf);
			}

			return false;
		}
	}

	if(compression) {
		SDL_RWclose(vfile);
		SDL_WriteLE32(file, SDL_RWtell(file) + SDL_RWtell(abuf) + 4);
		SDL_RWwrite(file, buf, SDL_RWtell(abuf), 1);
		SDL_RWclose(abuf);
		vfile = SDL_RWWrapZWriter(file, REPLAY_COMPRESSION_CHUNK_SIZE, false);
	}

	for(i = 0; i < rpy->numstages; ++i) {
		ReplayStage *stg = rpy->stages + i;
		for(j = 0; j < stg->numevents; ++j) {
			if(!replay_write_stage_event(stg->events + j, vfile)) {
				if(compression) {
					SDL_RWclose(vfile);
				}

				return false;
			}
		}
	}

	if(compression) {
		SDL_RWclose(vfile);
	}

	// useless byte to simplify the premature EOF check, can be anything
	SDL_WriteU8(file, REPLAY_USELESS_BYTE);

	return true;
}

#ifdef REPLAY_LOAD_GARBAGE_TEST
#define PRINTPROP(prop,fmt) log_debug(#prop " = %" # fmt " [%li / %li]", prop, (long int)SDL_RWtell(file), (long int)filesize)
#else
#define PRINTPROP(prop,fmt) (void)(prop)
#endif

#define CHECKPROP(prop,fmt) PRINTPROP(prop,fmt); if(filesize > 0 && SDL_RWtell(file) == filesize) { log_warn("Premature EOF"); return false; }

static void replay_read_string(SDL_RWops *file, char **ptr) {
	size_t len = SDL_ReadLE16(file);

	*ptr = malloc(len + 1);
	memset(*ptr, 0, len + 1);

	SDL_RWread(file, *ptr, 1, len);
}

static int replay_read_header(Replay *rpy, SDL_RWops *file, int64_t filesize, size_t *ofs) {
	for(uint8_t *u8_p = replay_magic_header; *u8_p; ++u8_p) {
		++(*ofs);
		if(SDL_ReadU8(file) != *u8_p) {
			log_warn("Incorrect header");
			return false;
		}
	}

	CHECKPROP(rpy->version = SDL_ReadLE16(file), u);
	(*ofs) += 2;

	if((rpy->version & ~REPLAY_VERSION_COMPRESSION_BIT) != REPLAY_STRUCT_VERSION) {
		log_warn("Incorrect version");
		return false;
	}

	if(rpy->version & REPLAY_VERSION_COMPRESSION_BIT) {
		CHECKPROP(rpy->fileoffset = SDL_ReadLE32(file), u);
	}

	(*ofs) += 4;
	return true;
}

static int replay_read_meta(Replay *rpy, SDL_RWops *file, int64_t filesize) {
	replay_read_string(file, &rpy->playername);
	PRINTPROP(rpy->playername, s);

	CHECKPROP(rpy->numstages = SDL_ReadLE16(file), u);

	if(!rpy->numstages) {
		log_warn("No stages in replay");
		return false;
	}

	rpy->stages = malloc(sizeof(ReplayStage) * rpy->numstages);
	memset(rpy->stages, 0, sizeof(ReplayStage) * rpy->numstages);

	for(int i = 0; i < rpy->numstages; ++i) {
		ReplayStage *stg = rpy->stages + i;

		CHECKPROP(stg->stage = SDL_ReadLE16(file), u);
		CHECKPROP(stg->seed = SDL_ReadLE32(file), u);
		CHECKPROP(stg->diff = SDL_ReadU8(file), u);
		CHECKPROP(stg->points = SDL_ReadLE32(file), u);
		CHECKPROP(stg->plr_char = SDL_ReadU8(file), u);
		CHECKPROP(stg->plr_shot = SDL_ReadU8(file), u);
		CHECKPROP(stg->plr_pos_x = SDL_ReadLE16(file), u);
		CHECKPROP(stg->plr_pos_y = SDL_ReadLE16(file), u);
		CHECKPROP(stg->plr_focus = SDL_ReadU8(file), u);
		CHECKPROP(stg->plr_power = SDL_ReadLE16(file), u);
		CHECKPROP(stg->plr_lifes = SDL_ReadU8(file), u);
		CHECKPROP(stg->plr_bombs = SDL_ReadU8(file), u);
		CHECKPROP(stg->plr_inputflags = SDL_ReadU8(file), u);
		CHECKPROP(stg->numevents = SDL_ReadLE16(file), u);

		if(replay_calc_stageinfo_checksum(stg) + SDL_ReadLE32(file)) {
			log_warn("Stageinfo is corrupt");
			return false;
		}
	}

	return true;
}

static int replay_read_events(Replay *rpy, SDL_RWops *file, int64_t filesize) {
	for(int i = 0; i < rpy->numstages; ++i) {
		ReplayStage *stg = rpy->stages + i;

		if(!stg->numevents) {
			log_warn("No events in stage");
			return false;
		}

		stg->events = malloc(sizeof(ReplayEvent) * stg->numevents);
		memset(stg->events, 0, sizeof(ReplayEvent) * stg->numevents);

		for(int j = 0; j < stg->numevents; ++j) {
			ReplayEvent *evt = stg->events + j;

			CHECKPROP(evt->frame = SDL_ReadLE32(file), u);
			CHECKPROP(evt->type = SDL_ReadU8(file), u);
			CHECKPROP(evt->value = SDL_ReadLE16(file), u);
		}
	}

	return true;
}

int replay_read(Replay *rpy, SDL_RWops *file, ReplayReadMode mode) {
	int64_t filesize; // must be signed
	SDL_RWops *vfile = file;

	mode &= REPLAY_READ_ALL;

	if(!mode) {
		log_fatal("Called with invalid read mode");
	}

	filesize = SDL_RWsize(file);

	if(filesize < 0) {
		log_warn("SDL_RWsize() failed: %s", SDL_GetError());
	} else {
		log_debug("%li bytes", (long int)filesize);
	}

	if(mode & REPLAY_READ_META) {
		memset(rpy, 0, sizeof(Replay));

		if(filesize > 0 && filesize <= sizeof(replay_magic_header) + 2) {
			log_warn("Replay file is too short (%li)", (long int)filesize);
			return false;
		}

		size_t ofs = 0;

		if(!replay_read_header(rpy, file, filesize, &ofs)) {
			return false;
		}

		bool compression = false;

		if(rpy->version & REPLAY_VERSION_COMPRESSION_BIT) {
			if(rpy->fileoffset < SDL_RWtell(file)) {
				log_warn("Invalid offset %li", (long int)rpy->fileoffset);
				return false;
			}

			vfile = SDL_RWWrapZReader(SDL_RWWrapSegment(file, ofs, rpy->fileoffset, false),
									  REPLAY_COMPRESSION_CHUNK_SIZE, true);
			filesize = -1;
			compression = true;
		}

		if(!replay_read_meta(rpy, vfile, filesize)) {
			if(compression) {
				SDL_RWclose(vfile);
			}

			return false;
		}

		if(compression) {
			SDL_RWclose(vfile);
			vfile = file;
		} else {
			rpy->fileoffset = SDL_RWtell(file);
		}
	}

	if(mode & REPLAY_READ_EVENTS) {
		if(!(mode & REPLAY_READ_META)) {
			if(!rpy->fileoffset) {
				log_fatal("Tried to read events before reading metadata");
			}

			for(int i = 0; i < rpy->numstages; ++i) {
				if(rpy->stages[i].events) {
					log_warn("Reading events into a replay that already had events, call replay_destroy_events() if this is intended");
					replay_destroy_events(rpy);
					break;
				}
			}

			if(SDL_RWseek(file, rpy->fileoffset, RW_SEEK_SET) < 0) {
				log_warn("SDL_RWseek() failed: %s", SDL_GetError());
				return false;
			}
		}

		bool compression = false;

		if(rpy->version & REPLAY_VERSION_COMPRESSION_BIT) {
			vfile = SDL_RWWrapZReader(file, REPLAY_COMPRESSION_CHUNK_SIZE, false);
			filesize = -1;
			compression = true;
		}

		if(!replay_read_events(rpy, vfile, filesize)) {
			if(compression) {
				SDL_RWclose(vfile);
			}

			replay_destroy_events(rpy);
			return false;
		}

		if(compression) {
			SDL_RWclose(vfile);
		}

		// useless byte to simplify the premature EOF check, can be anything
		SDL_ReadU8(file);
	}

	return true;
}

#undef CHECKPROP
#undef PRINTPROP

char* replay_getpath(const char *name, bool ext) {
	char *p = (char*)malloc(strlen(get_replays_path()) + strlen(name) + strlen(REPLAY_EXTENSION) + 3);

	if(ext) {
		sprintf(p, "%s/%s.%s", get_replays_path(), name, REPLAY_EXTENSION);
	} else {
		sprintf(p, "%s/%s", get_replays_path(), name);
	}

	return p;
}

int replay_save(Replay *rpy, const char *name) {
	char *p = replay_getpath(name, !strendswith(name, REPLAY_EXTENSION));
	log_info("Saving %s", p);

	SDL_RWops *file = SDL_RWFromFile(p, "wb");
	free(p);

	if(!file) {
		log_warn("SDL_RWFromFile() failed: %s", SDL_GetError());
		return false;
	}

	int result = replay_write(rpy, file, REPLAY_WRITE_COMPRESSED);
	SDL_RWclose(file);
	return result;
}

int replay_load(Replay *rpy, const char *name, ReplayReadMode mode) {
	char *p;

	if(mode & REPLAY_READ_RAWPATH) {
		p = (char*)name;
	} else {
		p = replay_getpath(name, !strendswith(name, REPLAY_EXTENSION));
	}

	log_info("replay_load(): loading %s (mode %i)", p, mode);

	SDL_RWops *file = SDL_RWFromFile(p, "rb");

	if(!(mode & REPLAY_READ_RAWPATH)) {
		free(p);
	}

	if(!file) {
		log_warn("SDL_RWFromFile() failed: %s", SDL_GetError());
		return false;
	}

	int result = replay_read(rpy, file, mode);

	if(!result) {
		replay_destroy(rpy);
	}

	SDL_RWclose(file);
	return result;
}

void replay_copy(Replay *dst, Replay *src, bool steal_events) {
	int i;

	replay_destroy(dst);
	memcpy(dst, src, sizeof(Replay));

	dst->playername = (char*)malloc(strlen(src->playername)+1);
	strcpy(dst->playername, src->playername);

	dst->stages = (ReplayStage*)malloc(sizeof(ReplayStage) * src->numstages);
	memcpy(dst->stages, src->stages, sizeof(ReplayStage) * src->numstages);

	for(i = 0; i < src->numstages; ++i) {
		ReplayStage *s, *d;
		s = src->stages + i;
		d = dst->stages + i;

		if(steal_events) {
			s->events = NULL;
		} else {
			d->capacity = s->numevents;
			d->events = (ReplayEvent*)malloc(sizeof(ReplayEvent) * d->capacity);
			memcpy(d->events, s->events, sizeof(ReplayEvent) * d->capacity);
		}
	}
}

void replay_stage_check_desync(ReplayStage *stg, int time, uint16_t check, ReplayMode mode) {
	if(!stg || time % (FPS * 5)) {
		return;
	}

	if(mode == REPLAY_PLAY) {
		if(stg->desync_check && stg->desync_check != check) {
			log_warn("Replay desync detected! %u != %u", stg->desync_check, check);
		} else {
			log_debug("%u OK", check);
		}
	}
#ifdef REPLAY_WRITE_DESYNC_CHECKS
	else {
		log_debug("%u", check);
		replay_stage_event(stg, time, EV_CHECK_DESYNC, (int16_t)check);
	}
#endif
}

int replay_test(void) {
#ifdef REPLAY_LOAD_GARBAGE_TEST
	int sz = getenvint("TAISEI_REPLAY_LOAD_GARBAGE_TEST");
	int headsz = sizeof(replay_magic_header) + 8; // 8 = version (uint16) + strlen (uint16) + plrname "test" (char[4])

	if(sz <= 0) {
		return 0;
	}

	uint8_t *u8_p, *buf = malloc(sz + headsz);
	SDL_RWops *handle = SDL_RWFromMem(buf, sz + headsz);

	// SDL_RWwrite(handle, replay_magic_header, 1, sizeof(replay_magic_header));

	for(u8_p = replay_magic_header; *u8_p; ++u8_p) {
		SDL_WriteU8(handle, *u8_p);
	}

	SDL_WriteLE16(handle, REPLAY_STRUCT_VERSION);
	SDL_WriteLE16(handle, 4);
	SDL_WriteU8(handle, 't');
	SDL_WriteU8(handle, 'e');
	SDL_WriteU8(handle, 's');
	SDL_WriteU8(handle, 't');

	log_info("Wrote a valid replay header");

	RandomState rnd;
	tsrand_init(&rnd, time(0));

	for(int i = 0; i < sz; ++i) {
		SDL_WriteU8(handle, tsrand_p(&rnd) & 0xFF);
	}

	log_info("Wrote %i bytes of garbage", sz);

	SDL_RWseek(handle, 0, RW_SEEK_SET);

	for(int i = 0; i < headsz; ++i) {
		tsfprintf(stdout, "%x ", buf[i]);
	}

	tsfprintf(stdout, "\n");

	Replay rpy;

	if(replay_read(&rpy, handle, REPLAY_READ_ALL)) {
		log_fatal("Succeeded loading garbage data as a replay... that shouldn't happen");
	}

	replay_destroy(&rpy);
	free(buf);
	return 1;
#else
	return 0;
#endif
}

void replay_play(Replay *rpy, int firststage) {
	if(rpy != &global.replay) {
		replay_copy(&global.replay, rpy, true);
	}

	global.replaymode = REPLAY_PLAY;

	if(global.replay.numstages == 1) {
		firststage = 0;
	}

	for(int i = firststage; i < global.replay.numstages; ++i) {
		ReplayStage *rstg = global.replay_stage = global.replay.stages+i;
		StageInfo *gstg = stage_get(rstg->stage);

		if(!gstg) {
			log_warn("Invalid stage %d in replay at %i skipped.\n", rstg->stage, i);
			continue;
		}

		stage_loop(gstg);

		if(global.game_over == GAMEOVER_ABORT) {
			break;
		}

		global.game_over = 0;
	}

	global.game_over = 0;
	global.replaymode = REPLAY_RECORD;
	replay_destroy(&global.replay);
	global.replay_stage = NULL;
	free_resources(false);
}

void replay_play_path(const char *path, int firststage) {
	replay_destroy(&global.replay);

	if(!replay_load(&global.replay, path, REPLAY_READ_ALL | REPLAY_READ_RAWPATH)) {
		return;
	}

	replay_play(&global.replay, firststage);
}
