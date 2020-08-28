/*
 * This software is licensed under the terms of the MIT License.
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2019, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2019, Andrei Alexeyev <akari@taisei-project.org>.
 */

#ifndef IGUARD_stages_stage5_iku_h
#define IGUARD_stages_stage5_iku_h

#include "taisei.h"

#include "boss.h"

void iku_intro(Boss*, int);
void iku_mid_intro(Boss*, int);
void iku_bolts(Boss*, int);
void iku_bolts2(Boss*, int);
void iku_bolts3(Boss*, int);
void iku_atmospheric(Boss*, int);
void iku_lightning(Boss*, int);
void iku_cathode(Boss*, int);
void iku_induction(Boss*, int);
void iku_extra(Boss*, int);

void iku_slave_visual(Enemy*, int, bool);
void iku_extra_slave_visual(Enemy*, int, bool);

void lightning_particle(cmplx, int);

Boss* stage5_spawn_iku(cmplx pos);
Boss* stage5_spawn_iku_mid(void);

#endif // IGUARD_stages_stage5_iku_h
