/*
 * This software is licensed under the terms of the MIT License.
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2019, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2019, Andrei Alexeyev <akari@taisei-project.org>.
 */

#ifndef IGUARD_stages_stage2_h
#define IGUARD_stages_stage2_h

#include "taisei.h"

#include "stage.h"

extern struct stage2_spells_s {
	// this struct must contain only fields of type AttackInfo
	// order of fields affects the visual spellstage number, but not its real internal ID

	struct {
		AttackInfo amulet_of_harm;
		AttackInfo bad_pick;
		AttackInfo wheel_of_fortune;
	} boss;

	struct {
		AttackInfo monty_hall_danmaku;
	} extra;

	// required for iteration
	AttackInfo null;
} stage2_spells;

extern StageProcs stage2_procs;
extern StageProcs stage2_spell_procs;

#endif // IGUARD_stages_stage2_h
