/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2017, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2017, Andrei Alexeyev <akari@alienslab.net>.
 */

#include <string.h>
#include <stdio.h>

#include "audio.h"
#include "resource/resource.h"
#include "global.h"

static char *saved_bgm;
static Hashtable *bgm_descriptions;
CurrentBGM current_bgm = { .name = NULL };

static void play_sound_internal(const char *name, bool unconditional, int cooldown) {
	if(!audio_backend_initialized() || global.frameskip) {
		return;
	}

	Sound *snd = get_sound(name);

	if(!snd || (!unconditional && snd->lastplayframe + 3 >= global.frames) || snd->islooping) {
		return;
	}

	snd->lastplayframe = global.frames + cooldown;
	audio_backend_sound_play(snd->impl);
}

void play_sound(const char *name) {
	play_sound_internal(name, false, 0);
}

void play_sound_cooldown(const char *name, int cooldown) {
	play_sound_internal(name, false, cooldown);
}

void play_ui_sound(const char *name) {
	play_sound_internal(name, true, 0);
}

void play_loop(const char *name) {
	if(!audio_backend_initialized() || global.frameskip) {
		return;
	}

	Sound *snd = get_sound(name);

	if(!snd) {
		return;
	}
	snd->lastplayframe = global.frames;
	if(!snd->islooping) {
		audio_backend_sound_loop(snd->impl);
		snd->islooping = true;
	}
}


void reset_sounds(void) {
	Resource *snd;
	for(HashtableIterator *i = hashtable_iter(resources.handlers[RES_SFX].mapping);
			hashtable_iter_next(i, 0, (void**)&snd);) {
		snd->sound->lastplayframe = 0;
		if(snd->sound->islooping) {
			audio_backend_sound_stop_loop(snd->sound->impl);
			snd->sound->islooping = false;
		}
	}
}

void update_sounds(void) {
	Resource *snd;
	for(HashtableIterator *i = hashtable_iter(resources.handlers[RES_SFX].mapping);
			hashtable_iter_next(i, 0, (void**)&snd);) {
		if(snd->sound->islooping && global.frames > snd->sound->lastplayframe + LOOPTIMEOUTFRAMES) {
			snd->sound->islooping = false;
			audio_backend_sound_stop_loop(snd->sound->impl);
			log_debug("channel stopped");
		}
	}
}

Sound* get_sound(const char *name) {
	Resource *res = get_resource(RES_SFX, name, RESF_OPTIONAL);
	return res ? res->sound : NULL;
}

Music* get_music(const char *name) {
	Resource *res = get_resource(RES_BGM, name, RESF_OPTIONAL);
	return res ? res->music : NULL;
}

static void sfx_cfg_volume_callback(ConfigIndex idx, ConfigValue v) {
	audio_backend_set_sfx_volume(config_set_float(idx, v.f));
}

static void bgm_cfg_volume_callback(ConfigIndex idx, ConfigValue v) {
	audio_backend_set_bgm_volume(config_set_float(idx, v.f));
}

static void load_bgm_descriptions(void) {
	bgm_descriptions = parse_keyvalue_file(BGM_PATH_PREFIX "bgm.conf", HT_DYNAMIC_SIZE);
	return;
}

static inline char* get_bgm_desc(char *name) {
	return bgm_descriptions ? (char*)hashtable_get_string(bgm_descriptions, name) : NULL;
}

void resume_bgm(void) {
	start_bgm(current_bgm.name); // In most cases it just unpauses existing music.
}

void stop_bgm(bool force) {
	if(!current_bgm.name) {
		return;
	}

	if(audio_backend_music_is_playing() && !audio_backend_music_is_paused()) {
		if(force) {
			audio_backend_music_stop();
		} else {
			audio_backend_music_pause();
		}

		log_info("BGM stopped");
	} else {
		log_info("No BGM was playing");
	}
}

void save_bgm(void) {
	// Deal with consequent saves without restore.
	stralloc(&saved_bgm, current_bgm.name);
}

void restore_bgm(void) {
	start_bgm(saved_bgm);
	free(saved_bgm);
	saved_bgm = NULL;
}

void start_bgm(const char *name) {
	if(!name || !*name) {
		stop_bgm(false);
		return;
	}

	// if BGM has changed, change it and start from beginning
	if(!current_bgm.name || strcmp(name, current_bgm.name)) {
		audio_backend_music_stop();

		stralloc(&current_bgm.name, name);

		if((current_bgm.music = get_music(name)) == NULL) {
			log_warn("BGM '%s' does not exist", current_bgm.name);
			stop_bgm(true);
			free(current_bgm.name);
			current_bgm.name = NULL;
			return;
		}
	}

	if(audio_backend_music_is_paused()) {
		audio_backend_music_resume();
	}

	if(audio_backend_music_is_playing()) {
		return;
	}

	if(!audio_backend_music_play(current_bgm.music->impl)) {
		return;
	}

	// Support drawing BGM title in game loop (only when music changed!)
	if((current_bgm.title = get_bgm_desc(current_bgm.name)) != NULL) {
		current_bgm.started_at = global.frames;
		// Boss BGM title color may differ from the one at beginning of stage
		current_bgm.isboss = strendswith(current_bgm.name, "boss");
	} else {
		current_bgm.started_at = -1;
	}

	log_info("Started %s", (current_bgm.title ? current_bgm.title : current_bgm.name));
}

void audio_init(void) {
	audio_backend_init();
	load_bgm_descriptions();
	config_set_callback(CONFIG_SFX_VOLUME, sfx_cfg_volume_callback);
	config_set_callback(CONFIG_BGM_VOLUME, bgm_cfg_volume_callback);
}

void audio_shutdown(void) {
	audio_backend_shutdown();

	if(bgm_descriptions) {
		hashtable_foreach(bgm_descriptions, hashtable_iter_free_data, NULL);
		hashtable_free(bgm_descriptions);
	}
}
