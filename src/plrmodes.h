/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information. 
 * ---
 * Copyright (C) 2011, Lukas Weber <laochailan@web.de>
 */

#ifndef PLRMODES_H
#define PLRMODES_H

#include "enemy.h"
#include "projectile.h"
#include "player.h"

/* Youmu */

void youmu_shot(Player *plr);

void youmu_opposite_draw(Enemy *e, int t);
void youmu_opposite_logic(Enemy *e, int t);

int youmu_homing(Projectile *p, int t);

/* Marisa */

void marisa_shot(Player *plr);

#endif