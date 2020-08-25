/*
 * This software is licensed under the terms of the MIT License.
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2019, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2019, Andrei Alexeyev <akari@taisei-project.org>.
*/

#ifndef IGUARD_stages_stage6_spells_spells_h
#define IGUARD_stages_stage6_spells_spells_h

#include "stages/stage6/elly.h"
#include "taisei.h"

#include "boss.h"

void elly_curvature(Boss*, int);
void elly_lhc(Boss*, int);
void elly_eigenstate(Boss*, int);
void elly_newton(Boss*, int);
void elly_eigenstate(Boss*, int);
void elly_broglie(Boss*, int);
void elly_maxwell(Boss*, int);
void elly_kepler(Boss*, int);
void elly_ricci(Boss*, int);
void elly_theory(Boss*, int);

#endif // IGUARD_stages_stage6_spells_spells_h
