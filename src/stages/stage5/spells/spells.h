/*
 * This software is licensed under the terms of the MIT License.
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2019, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2019, Andrei Alexeyev <akari@taisei-project.org>.
*/

#ifndef IGUARD_stages_stage5_spells_spells_h
#define IGUARD_stages_stage5_spells_spells_h

#include "stages/stage5/iku.h"
#include "taisei.h"

#include "boss.h"

void iku_atmospheric(Boss *, int);
void iku_lightning(Boss *, int);
void iku_induction(Boss *, int);
void iku_cathode(Boss *, int);
void iku_overload(Boss *, int);

#endif // IGUARD_stages_stage5_spells_spells_h
