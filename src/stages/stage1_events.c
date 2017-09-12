/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2017, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2017, Andrei Alexeyev <akari@alienslab.net>.
 */

#include "stage1_events.h"
#include "global.h"

Dialog *stage1_dialog(void) {
	Dialog *d = create_dialog(global.plr.cha == Marisa ? "dialog/marisa" : "dialog/youmu", "dialog/cirno");

	dadd_msg(d, Right, "Hey! Who’s there?");

	if(global.plr.cha == Marisa)
		dadd_msg(d, Left, "It’s me!");
	else
		dadd_msg(d, Left, "Just someone?");

	dadd_msg(d, Right, "How dare you pass the lake of the fairies?!\nIt’s a dangerous place for weak humans!");

	if(global.plr.cha == Marisa) {
		dadd_msg(d, Left, "You call me weak?");
		dadd_msg(d, Right, "I do!");
	} else {
		dadd_msg(d, Left, "I’m just passing by. Got a problem with that?");
		dadd_msg(d, Right, "Of course! You can’t do that!");
	}

	dadd_msg(d, Right, "I’ll freeze you where you stand!");
	dadd_msg(d, BGM, "bgm_stage1boss");

	return d;
}

void cirno_intro(Boss *c, int time) {
	if(time < 0)
		return;

	GO_TO(c, VIEWPORT_W/2.0 + 100.0*I, 0.035);
}

void cirno_icy(Boss *c, int time) {
	int t = time % 280;
	TIMER(&t);

	if(time < 0)
		return;

	FROM_TO(0, 200, 5-global.diff) {
		Color clr = rgb(0.1,0.2,0.4+0.4*frand());
		for(int d = 0; d < 2; d++) {
			int sign = 1 - 2*d;
			tsrand_fill(2);
			create_projectile2c("crystal", VIEWPORT_W/2.0 + 5*_i*sign*afrand(0)/(float)global.diff + cimag(c->pos)*I, clr, accelerated, 1.7*cexp(I*_i*sign/10.0)*(1-2*(_i&1)), 0.0001*I*_i + 2.*global.diff*global.diff/D_Lunatic/D_Lunatic*(0.0025 - 0.005*afrand(1)));
		}
	}
}

void cirno_mid_outro(Boss *c, int time) {
	if(time < 0)
		return;

	GO_TO(c, -300.0*I, 0.035);
}


int cirno_pfreeze_frogs(Projectile *p, int t) {
	if(t < 0)
		return 1;

	Boss *parent = global.boss;

	if(parent == NULL)
		return ACTION_DESTROY;

	int boss_t = (global.frames - parent->current->starttime) % 320;

	if(boss_t < 110)
		linear(p, t);
	else if(boss_t == 110) {
		p->clr = rgb(0.7,0.7,0.7);
	}

	if(t == 240) {
		p->pos0 = p->pos;
		p->args[0] = (1.8+0.2*global.diff)*cexp(I*2*M_PI*frand());
	}

	if(t > 240)
		linear(p, t-240);

	return 1;
}

void cirno_perfect_freeze(Boss *c, int time) {
	int t = time % 320;
	TIMER(&t);

	if(time < 0)
		return;

	FROM_TO(-40, 0, 1)
		GO_TO(c, VIEWPORT_W/2.0 + 100.0*I, 0.04);

	FROM_TO(20,80,1) {
		float r = frand();
		float g = frand();
		float b = frand();

		int i;
		int n = global.diff;
		for(i = 0; i < n; i++)
			create_projectile1c("ball", c->pos, rgb(r, g, b), cirno_pfreeze_frogs, 4*cexp(I*tsrand()));
	}

	GO_AT(c, 160, 190, 2 + 1.0*I);

	int d = max(0, global.diff - D_Normal);

	FROM_TO(160 - 50*d, 220 + 30*d, 6-global.diff/2) {
		float r1, r2;

		if(global.diff > D_Normal) {
			r1 = sin(time/M_PI*5.3) * cos(2*time/M_PI*5.3);
			r2 = cos(time/M_PI*5.3) * sin(2*time/M_PI*5.3);
		} else {
			r1 = nfrand();
			r2 = nfrand();
		}

		create_projectile2c("rice", c->pos + 60, rgb(0.3, 0.4, 0.9), asymptotic, (2.+0.4*global.diff)*cexp(I*(carg(global.plr.pos - c->pos) + 0.5*r1)), 2.5);
		create_projectile2c("rice", c->pos - 60, rgb(0.3, 0.4, 0.9), asymptotic, (2.+0.4*global.diff)*cexp(I*(carg(global.plr.pos - c->pos) + 0.5*r2)), 2.5);
	}

	GO_AT(c, 190, 220, -2);

	FROM_TO(280, 320, 1)
		GO_TO(c, VIEWPORT_W/2.0 + 100.0*I, 0.04);
}

void cirno_pfreeze_bg(Boss *c, int time) {
	glColor4f(0.5,0.5,0.5,1);
	fill_screen(time/700.0, time/700.0, 1, "stage1/cirnobg");
	glColor4f(0.7,0.7,0.7,0.5);
	glBlendFunc(GL_ZERO, GL_SRC_COLOR);
	fill_screen(-time/700.0 + 0.5, time/700.0+0.5, 0.4, "stage1/cirnobg");
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	fill_screen(0, -time/100.0, 0, "stage1/snowlayer");
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1,1,1,1);
}

void cirno_mid_flee(Boss *c, int time) {
	if(time >= 0) {
		GO_TO(c, -250 + 30 * I, 0.02)
	}
}

Boss *create_cirno_mid(void) {
	Boss* cirno = create_boss("Cirno", "cirno", "dialog/cirno", VIEWPORT_W + 220 + 30.0*I);
	boss_add_attack(cirno, AT_Move, "Introduction", 2, 0, cirno_intro, NULL);
	boss_add_attack(cirno, AT_Normal, "Icy Storm", 20, 20000, cirno_icy, NULL);
	boss_add_attack_from_info(cirno, stage1_spells+0, false);
	boss_add_attack(cirno, AT_Move, "Flee", 5, 0, cirno_mid_flee, NULL);

	start_attack(cirno, cirno->attacks);
	return cirno;
}

void cirno_intro_boss(Boss *c, int time) {
	if(time < 0)
		return;
	TIMER(&time);
	GO_TO(c, VIEWPORT_W/2.0 + 100.0*I, 0.035);

	AT(100)
		global.dialog = stage1_dialog();
}

void cirno_iceplosion0(Boss *c, int time) {
	int t = time % 350;
	TIMER(&t);

	if(time < 0)
		return;

	FROM_TO(20,30,2) {
		int i;
		int n = 8+global.diff;
		for(i = 0; i < n; i++) {
			create_projectile2c("plainball", c->pos, rgb(0,0,0.5), asymptotic, (3+_i/3.0)*cexp(I*(2*M_PI/n*i + carg(global.plr.pos-c->pos))), _i*0.7);
		}
	}

	FROM_TO(40,100,1+2*(global.diff<D_Hard)) {
		create_projectile2c("crystal", c->pos, rgb(0.3,0.3,0.8), accelerated, global.diff/4.*cexp(2.0*I*M_PI*frand()) + 2.0*I, 0.002*cexp(I*(M_PI/10.0*(_i%20))));
	}

	FROM_TO(150, 300, 30-5*global.diff) {
		float dif = M_PI*2*frand();
		int i;
		for(i = 0; i < 20; i++) {
			create_projectile2c("plainball", c->pos, rgb(0.04*_i,0.04*_i,0.4+0.04*_i), asymptotic, (3+_i/4.0)*cexp(I*(2*M_PI/8.0*i + dif)), 2.5);
		}
	}
}

void cirno_crystal_rain(Boss *c, int time) {
	int t = time % 500;
	TIMER(&t);

	if(time < 0)
		return;

	int hdiff = max(0, (int)global.diff - D_Normal);

	if(frand() > 0.95-0.1*global.diff) {
		tsrand_fill(2);
		create_projectile2c("crystal", VIEWPORT_W*afrand(0), rgb(0.2,0.2,0.4), accelerated, 1.0*I, 0.01*I + (-0.005+0.005*global.diff)*anfrand(1));
	}

	FROM_TO(100, 400, 120-20*global.diff - 10 * hdiff) {
		float i;
		bool odd = (hdiff? (_i&1) : 0);
		float n = (global.diff-1+hdiff*4 + odd)/2.0;

		for(i = -n; i <= n; i++) {
			create_projectile2c(odd? "plainball" : "bigball", c->pos, rgb(0.2,0.2,0.9), asymptotic, 2*cexp(I*carg(global.plr.pos-c->pos)+0.3*I*i), 2.3);
		}
	}

	GO_AT(c, 20, 70, 1+0.6*I);
	GO_AT(c, 120, 170, -1+0.2*I);
	GO_AT(c, 230, 300, -1+0.6*I);

	FROM_TO(400, 500, 1)
		GO_TO(c, VIEWPORT_W/2.0 + 100.0*I, 0.01);
}

void cirno_iceplosion1(Boss *c, int time) {
	int t = time % 360;
	TIMER(&t);

	if(time < 0)
		GO_TO(c, VIEWPORT_W/2.0 + 100.0*I, 0.02);

	FROM_TO(20,30,2) {
		int i;
		for(i = 0; i < 15+global.diff; i++) {
			create_projectile2c("plainball", c->pos, rgb(0,0,0.5), asymptotic, (3+_i/3.0)*cexp(I*((2)*M_PI/8.0*i + (0.1+0.03*global.diff)*(1 - 2*frand()))), _i*0.7);
		}
	}

	FROM_TO(40,100,2+2*(global.diff<D_Hard)) {
		create_projectile2c("crystal", c->pos + 100, rgb(0.3,0.3,0.8), accelerated, 1.5*cexp(2.0*I*M_PI*frand()) - 0.4 + 2.0*I*global.diff/4., 0.002*cexp(I*(M_PI/10.0*(_i%20))));
		create_projectile2c("crystal", c->pos - 100, rgb(0.3,0.3,0.8), accelerated, 1.5*cexp(2.0*I*M_PI*frand()) + 0.4 + 2.0*I*global.diff/4., 0.002*cexp(I*(M_PI/10.0*(_i%20))));
	}

	FROM_TO(150, 300, 30) {
		float dif = M_PI*2*frand();
		int i;
		for(i = 0; i < 20; i++) {
			create_projectile2c("plainball", c->pos, rgb(0.04*_i,0.04*_i,0.4+0.04*_i), asymptotic, (3+_i/3.0)*cexp(I*(2*M_PI/8.0*i + dif)), 2.5);
		}
	}
}

int cirno_icicles(Projectile *p, int t) {
	int turn = 60;

	if(t < 0)
		return 1;

	if(t < turn) {
		p->pos += p->args[0]*pow(0.9,t);
	} else if(t == turn) {
		p->args[0] = 2.5*cexp(I*(carg(p->args[0])-M_PI/2.0+M_PI*(creal(p->args[0]) > 0)));
		if(global.diff > D_Normal)
			p->args[0] += 0.05*nfrand();
	} else if(t > turn) {
		p->pos += p->args[0];
	}

	p->angle = carg(p->args[0]);

	return 1;
}

void cirno_icicle_fall(Boss *c, int time) {
	int t = time % 400;
	TIMER(&t);

	if(time < 0)
		return;

	GO_TO(c, VIEWPORT_W/2.0+120.0*I, 0.01);

	FROM_TO(20,200,30-3*global.diff) {
		for(float i = 2-0.2*global.diff; i < 5; i+=1./(1+global.diff)) {
			create_projectile1c("crystal", c->pos, rgb(0.3,0.3,0.9), cirno_icicles, 6*i*cexp(I*(-0.1+0.1*_i)));
			create_projectile1c("crystal", c->pos, rgb(0.3,0.3,0.9), cirno_icicles, 6*i*cexp(I*(M_PI+0.1-0.1*_i)));
		}
	}

	if(global.diff > D_Easy) {
		FROM_TO(120,200,3) {
			float f = frand()*_i;

			create_projectile2c("ball", c->pos, rgb(0.,0.,0.3), accelerated, 0.2*(-2*I-1.5+f),-0.02*I);
			create_projectile2c("ball", c->pos, rgb(0.,0.,0.3), accelerated, 0.2*(-2*I+1.5-f),-0.02*I);
		}
	}
	if(global.diff > D_Normal) {
		FROM_TO(300,400,10) {
			float x = VIEWPORT_W/2+VIEWPORT_W/2*(0.3+_i/10.);
			float angle1 = M_PI/10*frand();
			float angle2 = M_PI/10*frand();
			for(float i = 1; i < 5; i++) {
				create_projectile2c("ball", x, rgb(0.,0.,0.3), accelerated, i*I*0.5*cexp(I*angle1),0.001*I-(global.diff == D_Lunatic)*0.001*frand());
				create_projectile2c("ball", VIEWPORT_W-x, rgb(0.,0.,0.3), accelerated, i*I*0.5*cexp(-I*angle2),0.001*I+(global.diff == D_Lunatic)*0.001*frand());
			}
		}
	}


}

int cirno_crystal_blizzard_proj(Projectile *p, int time) {
	if(!(time % 7))
		create_particle1c("stain", p->pos, 0, GrowFadeAdd, timeout, 20)->angle = global.frames * 15;

	if(time > 100 + global.diff * 100)
		p->args[0] *= 1.03;

	return asymptotic(p, time);
}

void cirno_crystal_blizzard(Boss *c, int time) {
	int t = time % 700;
	TIMER(&t);

	if(time < 0) {
		GO_TO(c, VIEWPORT_W/2.0+300*I, 0.1);
		return;
	}

	FROM_TO(60, 360, 10) {
		int i, cnt = 14 + global.diff * 3;
		for(i = 0; i < cnt; ++i) {
			create_projectile2c("crystal", i*VIEWPORT_W/cnt, i % 2? rgb(0.2,0.2,0.4) : rgb(0.5,0.5,0.5), accelerated, 0, 0.02*I + 0.01*I * (i % 2? 1 : -1) * sin((i*3+global.frames)/30.0));
		}
	}

	FROM_TO(330, 700, 1) {
		GO_TO(c, global.plr.pos, 0.01);

		if(!(time % (1 + D_Lunatic - global.diff))) {
			tsrand_fill(2);
			create_projectile2c("wave", c->pos, rgb(0.2, 0.2, 0.4), cirno_crystal_blizzard_proj,
				20 * (0.1 + 0.1 * anfrand(0)) * cexp(I*(carg(global.plr.pos - c->pos) + anfrand(1) * 0.2)), 5
			)->draw = ProjDrawAdd;
		}

		if(!(time % 7)) {
			int i, cnt = global.diff - 1;
			for(i = 0; i < cnt; ++i)
				create_projectile2c("ball", c->pos, rgb(0.1, 0.1, 0.5), accelerated, 0, 0.01 * cexp(I*(global.frames/20.0 + 2*i*M_PI/cnt)))->draw = ProjDrawAdd;
		}
	}
}

void cirno_superhardspellcard(Boss *c, int t) {
	// HOWTO: create a super hard spellcard in a few seconds

	cirno_iceplosion0(c, t);
	cirno_iceplosion1(c, t);
	cirno_crystal_rain(c, t);
	cirno_icicle_fall(c, t);
	cirno_icy(c, t);
	cirno_perfect_freeze(c, t);
}

Boss *create_cirno(void) {
	Boss* cirno = create_boss("Cirno", "cirno", "dialog/cirno", -230 + 100.0*I);
	boss_add_attack(cirno, AT_Move, "Introduction", 2, 0, cirno_intro_boss, NULL);
	boss_add_attack(cirno, AT_Normal, "Iceplosion 0", 20, 20000, cirno_iceplosion0, NULL);
	boss_add_attack_from_info(cirno, stage1_spells+1, false);
	boss_add_attack(cirno, AT_Normal, "Iceplosion 1", 20, 20000, cirno_iceplosion1, NULL);
	boss_add_attack_from_info(cirno, stage1_spells+2, false);
	boss_set_extra_spell(cirno, stage1_spells+3);

	start_attack(cirno, cirno->attacks);
	return cirno;
}

int stage1_burst(Enemy *e, int time) {
	TIMER(&time);
	AT(EVENT_DEATH) {
		spawn_items(e->pos, Point, 3, NULL);
		return 1;
	}

	FROM_TO(0, 60, 1)
		e->pos += 2.0*I;

	AT(60) {
		int i = 0;
		int n = 1.5*global.diff-1;

		for(i = -n; i <= n; i++) {
			create_projectile2c("crystal", e->pos, rgb(0.2, 0.3, 0.5), asymptotic, (2+0.1*global.diff)*cexp(I*(carg(global.plr.pos - e->pos) + 0.2*i)), 5);
		}

		e->moving = true;
		e->dir = creal(e->args[0]) < 0;

		e->pos0 = e->pos;
	}


	FROM_TO(70, 900, 1)
		e->pos = e->pos0 + (0.04*e->args[0])*_i*_i;

	return 1;
}

int stage1_circletoss(Enemy *e, int time) {
	TIMER(&time);
	AT(EVENT_DEATH) {
		spawn_items(e->pos, Point, 2, Power, 1, NULL);
		return 1;
	}

	e->pos += e->args[0];

	FROM_TO(60,100,2+(global.diff<D_Hard)) {
		e->args[0] = 0.5*e->args[0];
		create_projectile2c("rice", e->pos, rgb(0.6, 0.2, 0.7), asymptotic, 2*cexp(I*M_PI/10*_i), _i/2.0);
	}


	if(global.diff > D_Easy) {
		FROM_TO_INT(90,500,150,5+7*global.diff,1) {
			tsrand_fill(2);
			create_projectile2c("thickrice", e->pos, rgb(0.2, 0.4, 0.8), asymptotic, (1+afrand(0)*2)*cexp(I*carg(global.plr.pos - e->pos)+0.05*I*global.diff*anfrand(1)), 3);
		}
	}

	FROM_TO(global.diff > D_Easy ? 500 : 240, 900, 1)
		e->args[0] += 0.03*e->args[1] - 0.04*I;

	return 1;
}

int stage1_sinepass(Enemy *e, int time) {
	TIMER(&time);
	AT(EVENT_DEATH) {
		tsrand_fill(2);
		spawn_items(e->pos, Point, afrand(0)>0.5, Power, afrand(1)>0.8, NULL);
		return 1;
	}

	e->args[1] -= cimag(e->pos-e->pos0)*0.03*I;
	e->pos += e->args[1]*0.4 + e->args[0];

	if(frand() > 0.997-0.005*(global.diff-1))
		create_projectile1c("ball", e->pos, rgb(0.8,0.8,0.4), linear, (1+0.2*global.diff+frand())*cexp(I*carg(global.plr.pos - e->pos)));

	return 1;
}

int stage1_drop(Enemy *e, int t) {
	TIMER(&t);
	AT(EVENT_DEATH) {
		spawn_items(e->pos, Point, 2, Power, frand()>0.8, NULL);
		return 1;
	}
	if(t < 0)
		return 1;

	e->pos = e->pos0 + e->args[0]*t + e->args[1]*t*t;

	FROM_TO(10,1000,1)
		if(frand() > 0.997-0.007*(global.diff-1))
			create_projectile1c("ball", e->pos, rgb(0.8,0.8,0.4), linear, (1+0.3*global.diff+frand())*cexp(I*carg(global.plr.pos - e->pos)));

	return 1;
}

int stage1_circle(Enemy *e, int t) {
	TIMER(&t);
	AT(EVENT_DEATH) {
		spawn_items(e->pos, Point, 3, Power, 2, NULL);
		return 1;
	}

	FROM_TO(0, 150, 1)
		e->pos += (e->args[0] - e->pos)*0.02;

	FROM_TO_INT(150, 550, 40, 40, 2+2*(global.diff<D_Hard))
		create_projectile2c("rice", e->pos, rgb(0.6, 0.2, 0.7), asymptotic, (1.7+0.2*global.diff)*cexp(I*M_PI/10*_ni), _ni/2.0);

	FROM_TO(560,1000,1)
		e->pos += e->args[1];

	return 1;
}

int stage1_multiburst(Enemy *e, int t) {
	TIMER(&t);
	AT(EVENT_DEATH) {
		spawn_items(e->pos, Point, 3, Power, 2, NULL);
		return 1;
	}

	FROM_TO(0, 50, 1)
		e->pos += 2.0*I;

	FROM_TO_INT(60, 300, 70, 40, 18-2*global.diff) {
		int i;
		int n = global.diff-1;
		for(i = -n; i <= n; i++)
			create_projectile1c("crystal", e->pos, rgb(0.2, 0.3, 0.5), linear, 2.5*cexp(I*(carg(global.plr.pos - e->pos) + i/5.0)));
	}

	FROM_TO(320, 700, 1) {
		e->args[1] += 0.03;
		e->pos += e->args[0]*e->args[1] + 1.4*I;
	}

	return 1;
}

int stage1_instantcircle(Enemy *e, int t) {
	TIMER(&t);
	AT(EVENT_DEATH) {
		spawn_items(e->pos, Point, 2, Power, 4, NULL);
		return 1;
	}

	FROM_TO(0, 110, 1) {
		e->pos += e->args[0];
	}

	int i;

	AT(150) {
		for(i = 0; i < 20+2*global.diff; i++)
			create_projectile2c("rice", e->pos, rgb(0.6, 0.2, 0.7), asymptotic, 1.5*cexp(I*2*M_PI/(20.0+global.diff)*i), 2.0);
	}

	AT(170) {
		if(global.diff > D_Easy) {
			for(i = 0; i < 20+3*global.diff; i++)
				create_projectile2c("rice", e->pos, rgb(0.6, 0.2, 0.7), asymptotic, 3*cexp(I*2*M_PI/(20.0+global.diff)*i), 3.0);
		}
	}

	if(t > 200)
		e->pos += e->args[1];

	return 1;
}

int stage1_tritoss(Enemy *e, int t) {
	TIMER(&t);
	AT(EVENT_DEATH) {
		spawn_items(e->pos, Point, 5, Power, 2, NULL);
		return 1;
	}

	FROM_TO(0, 100, 1) {
		e->pos += e->args[0];
	}

	FROM_TO(120, 800,8-global.diff) {
		float a = M_PI/30.0*((_i/7)%30)+0.1*nfrand();
		int i;
		int n = 3+global.diff/2;

		for(i = 0; i < n; i++)
			create_projectile2c("thickrice", e->pos, rgb(0.2, 0.4, 0.8), asymptotic, 2*cexp(I*a+2.0*I*M_PI/n*i), 3);
	}

	FROM_TO(480, 800, 300) {
		int i, n = 15 + global.diff*3;
		for(i = 0; i < n; i++) {
			create_projectile2c("rice", e->pos, rgb(0.6, 0.2, 0.7), asymptotic, 1.5*cexp(I*2*M_PI/n*i), 2.0);
			if(global.diff > D_Easy)
				create_projectile2c("rice", e->pos, rgb(0.6, 0.2, 0.7), asymptotic, 3*cexp(I*2*M_PI/n*i), 3.0);
		}
	}

	if(t > 820)
		e->pos += e->args[1];

	return 1;
}

void stage1_events(void) {
	TIMER(&global.timer);

	AT(0) {
		start_bgm("bgm_stage1");
	}

	// opening. projectile bursts
	FROM_TO(100, 160, 25) {
		create_enemy1c(VIEWPORT_W/2 + 70, 700, Fairy, stage1_burst, 1 + 0.6*I);
		create_enemy1c(VIEWPORT_W/2 - 70, 700, Fairy, stage1_burst, -1 + 0.6*I);
	}

	// more bursts. fairies move / \ like
	FROM_TO(240, 300, 30) {
		create_enemy1c(70 + _i*40, 700, Fairy, stage1_burst, -1 + 0.6*I);
		create_enemy1c(VIEWPORT_W - (70 + _i*40), 700, Fairy, stage1_burst, 1 + 0.6*I);
	}

	// big fairies, circle + projectile toss
	FROM_TO(400, 460, 50)
		create_enemy2c(VIEWPORT_W*_i + VIEWPORT_H/3*I, 1500, BigFairy, stage1_circletoss, 2-4*_i-0.3*I, 1-2*_i);


	// swirl, sine pass
	FROM_TO(380, 1000, 20) {
		tsrand_fill(2);
		create_enemy2c(VIEWPORT_W*(_i&1) + afrand(0)*100.0*I + 70.0*I, 100, Swirl, stage1_sinepass, 3.5*(1-2*(_i&1)), afrand(1)*7.0*I);
	}

	// swirl, drops
	FROM_TO(1100, 1600, 20)
		create_enemy2c(VIEWPORT_W/3, 100, Swirl, stage1_drop, 4.0*I, 0.06);

	FROM_TO(1500, 2000, 20)
		create_enemy2c(VIEWPORT_W+200.0*I, 100, Swirl, stage1_drop, -2, -0.04-0.03*I);

	// bursts
	FROM_TO(1250, 1800, 60) {
		tsrand_fill(2);
		create_enemy1c(VIEWPORT_W/2 + afrand(0)*500-250, 500, Fairy, stage1_burst, afrand(1)*2-1);
	}

	// circle - multi burst combo
	FROM_TO(1700, 2300, 300) {
		tsrand_fill(3);
		create_enemy2c(VIEWPORT_W/2, 1400, BigFairy, stage1_circle, VIEWPORT_W/4 + VIEWPORT_W/2*afrand(0)+200.0*I, 3-6*(afrand(1)>0.5)+afrand(2)*2.0*I);
	}

	FROM_TO(2000, 2500, 200) {
		int i, t = global.diff + 1;
		for(i = 0; i < t; i++)
			create_enemy1c(VIEWPORT_W/2 - 40*t + 80*i, 1000, Fairy, stage1_multiburst, i - 2.5);
	}

	AT(2700)
		global.boss = create_cirno_mid();

	// some chaotic swirls + instant circle combo
	FROM_TO(2760, 3800, 20) {
		tsrand_fill(2);
		create_enemy2c(VIEWPORT_W/2 - 200*anfrand(0), 250+40*global.diff, Swirl, stage1_drop, 1.0*I, 0.001*I + 0.02 + 0.06*anfrand(1));
	}

	FROM_TO(2900, 3750, 190-30*global.diff) {
		tsrand_fill(2);
		create_enemy2c(VIEWPORT_W*afrand(0), 1200, Fairy, stage1_instantcircle, 2.0*I, 3.0 - 6*afrand(1) - 1.0*I);
	}


	// multiburst + normal circletoss, later tri-toss
	FROM_TO(3900, 4800, 200) {
		tsrand_fill(2);
		create_enemy1c(VIEWPORT_W*afrand(0), 1000, Fairy, stage1_multiburst, 2.5*afrand(1));
	}

	FROM_TO(4000, 4100, 20)
		create_enemy2c(VIEWPORT_W*_i + VIEWPORT_H/3*I, 1700, Fairy, stage1_circletoss, 2-4*_i-0.3*I, 1-2*_i);

	AT(4200)
		create_enemy2c(VIEWPORT_W/2.0, 4000, BigFairy, stage1_tritoss, 2.0*I, -2.6*I);

	AT(5000)
		global.boss = create_cirno();

	AT(5400 - FADE_TIME) {
		stage_finish(GAMEOVER_WIN);
	}
}
