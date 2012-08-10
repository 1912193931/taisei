/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information. 
 * ---
 * Copyright (C) 2011, Lukas Weber <laochailan@web.de>
 */

#ifndef INGAMEMENU_H
#define INGAMEMENU_H

#include "menu.h"

MenuData *create_ingame_menu(void);
void draw_ingame_menu(MenuData *menu);
void ingame_menu_logic(MenuData **menu);

#endif
