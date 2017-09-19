/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2017, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2017, Andrei Alexeyev <akari@alienslab.net>.
 */

#include "events.h"
#include "config.h"
#include "global.h"
#include "video.h"
#include "gamepad.h"

struct sdl_custom_events_s sdl_custom_events;
static uint32_t keyrepeat_paused_until;

void events_init(void) {
	uint32_t *events = (uint32_t*)&sdl_custom_events;
	uint32_t id = SDL_RegisterEvents(NUM_CUSTOM_EVENTS);

	for(int i = 0; i < NUM_CUSTOM_EVENTS; ++i, ++id) {
		events[i] = id;
		log_debug("User event registered: %u", events[i]);
	}
}

void events_pause_keyrepeat(void) {
	// for whatever stupid bizarre reason, keyrepeat skips the delay after the window toggles fullscreen on some systems
	// this ugly hack is a workaround for that
	keyrepeat_paused_until = SDL_GetTicks() + 250;
}

void handle_events(EventHandler handler, EventFlags flags, void *arg) {
	SDL_Event event;

	bool kbd 	= flags & EF_Keyboard;
	bool text	= flags & EF_Text;
	bool menu	= flags & EF_Menu;
	bool game	= flags & EF_Game;
	bool video  = flags & EF_Video;

	// TODO: rewrite text input handling to properly support multibyte characters and IMEs

	if(text) {
		if(!SDL_IsTextInputActive()) {
			SDL_StartTextInput();
		}
	} else {
		if(SDL_IsTextInputActive()) {
			SDL_StopTextInput();
		}
	}

	while(SDL_PollEvent(&event)) {
		if(IS_CUSTOM_EVENT(event.type)) {
			log_debug("Custom event %u: %"PRIxMAX" %"PRIxMAX, event.type, (uintmax_t)event.user.data1, (uintmax_t)event.user.data2);

			if(event.type == sdl_custom_events.video_mode_changed) {
				if(video) {
					handler(E_VideoModeChanged, 0, arg);
				}
			}

			resource_sdl_event(&event);
			continue;
		}

		SDL_Scancode scan = event.key.keysym.scancode;
		SDL_Keymod mod = event.key.keysym.mod;
		bool repeat = event.key.repeat;
		uint32_t timenow;

		if((event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) && repeat && (timenow = SDL_GetTicks()) < keyrepeat_paused_until) {
			log_debug("Prevented a potentially bogus key repeat: %i %i %i %i %i", event.type - SDL_KEYDOWN, scan, mod, repeat, keyrepeat_paused_until - timenow);
			continue;
		}

		switch(event.type) {
			case SDL_KEYDOWN:
				if(text) {
					if(scan == SDL_SCANCODE_ESCAPE)
						handler(E_CancelText, 0, arg);
					else if(scan == SDL_SCANCODE_RETURN)
						handler(E_SubmitText, 0, arg);
					else if(scan == SDL_SCANCODE_BACKSPACE)
						handler(E_CharErased, 0, arg);
				} else if(!repeat) {
					if(scan == config_get_int(CONFIG_KEY_SCREENSHOT)) {
						video_take_screenshot();
						break;
					}

					if((scan == SDL_SCANCODE_RETURN && (mod & KMOD_ALT)) || scan == config_get_int(CONFIG_KEY_FULLSCREEN)) {
						config_set_int(CONFIG_FULLSCREEN, !config_get_int(CONFIG_FULLSCREEN));
						break;
					}
				}

				if(kbd) {
					handler(E_KeyDown, scan, arg);
				}

				if(menu && (!repeat || transition.state == TRANS_IDLE)) {
					struct eventmap_t { int scancode; int event; } map[] = {
						// order matters
						// handle all the hardcoded controls first to prevent accidentally overriding them with unusable ones

						{ SDL_SCANCODE_DOWN, E_CursorDown },
						{ SDL_SCANCODE_UP, E_CursorUp },
						{ SDL_SCANCODE_RIGHT, E_CursorRight },
						{ SDL_SCANCODE_LEFT, E_CursorLeft },
						{ SDL_SCANCODE_RETURN, E_MenuAccept },
						{ SDL_SCANCODE_ESCAPE, E_MenuAbort },

						{ config_get_int(CONFIG_KEY_DOWN), E_CursorDown },
						{ config_get_int(CONFIG_KEY_UP), E_CursorUp },
						{ config_get_int(CONFIG_KEY_RIGHT), E_CursorRight },
						{ config_get_int(CONFIG_KEY_LEFT), E_CursorLeft },
						{ config_get_int(CONFIG_KEY_SHOT), E_MenuAccept },
						{ config_get_int(CONFIG_KEY_BOMB), E_MenuAbort },

						{SDL_SCANCODE_UNKNOWN, -1}
					};

					for(struct eventmap_t *m = map; m->scancode != SDL_SCANCODE_UNKNOWN; ++m) {
						if(scan == m->scancode) {
							handler(m->event, 0, arg);
							break;
						}
					}
				}

				if(game && !repeat) {
					if(scan == config_get_int(CONFIG_KEY_PAUSE) || scan == SDL_SCANCODE_ESCAPE) {
						handler(E_Pause, 0, arg);
					} else {
						int key = config_key_from_scancode(scan);
						if(key >= 0)
							handler(E_PlrKeyDown, key, arg);
					}
				}

				break;

			case SDL_KEYUP:
				if(kbd) {
					handler(E_KeyUp, scan, arg);
				}

				if(game && !repeat) {
					int key = config_key_from_scancode(scan);
					if(key >= 0)
						handler(E_PlrKeyUp, key, arg);
				}

				break;

			case SDL_TEXTINPUT: {
				char *c;

				for(c = event.text.text; *c; ++c) {
					handler(E_CharTyped, *c, arg);
				}

				break;
			}

			case SDL_WINDOWEVENT:
				switch(event.window.event) {
					case SDL_WINDOWEVENT_RESIZED:
						video_resize(event.window.data1, event.window.data2);
						break;

					case SDL_WINDOWEVENT_FOCUS_LOST:
						if(game) {
							handler(E_Pause, 0, arg);
						}
						break;
				}
				break;

			case SDL_QUIT:
				exit(0);
				break;

			default:
				gamepad_event(&event, handler, flags, arg);
				break;
		}
	}
}

// Inputdevice-agnostic method of checking whether a game control is pressed.
// ALWAYS use this instead of SDL_GetKeyState if you need it.
bool gamekeypressed(KeyIndex key) {
	return SDL_GetKeyboardState(NULL)[config_get_int(KEYIDX_TO_CFGIDX(key))] || gamepad_gamekeypressed(key);
}
