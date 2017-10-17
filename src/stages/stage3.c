/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2017, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2017, Andrei Alexeyev <akari@alienslab.net>.
 */

#include "stage3.h"
#include "stage3_events.h"

#include "global.h"
#include "stage.h"
#include "stageutils.h"

/*
 *	See the definition of AttackInfo in boss.h for information on how to set up the idmaps.
 *  To add, remove, or reorder spells, see this stage's header file.
 */

struct stage3_spells_s stage3_spells = {
	.mid = {
		.deadly_dance			= {{ 0,  1,  2,  3}, AT_Spellcard, "Venom Sign ~ Deadly Dance", 13, 40000,
									scuttle_deadly_dance, scuttle_spellbg, BOSS_DEFAULT_GO_POS},
	},

	.boss = {
		.moonlight_rocket		= {{ 6,  7,  8,  9}, AT_Spellcard, "Firefly Sign ~ Moonlight Rocket", 30, 40000,
									wriggle_moonlight_rocket, wriggle_spellbg, BOSS_DEFAULT_GO_POS},
		.wriggle_night_ignite	= {{10, 11, 12, 13}, AT_Spellcard, "Light Source ~ Wriggle Night Ignite", 30, 46000,
									wriggle_night_ignite, wriggle_spellbg, BOSS_DEFAULT_GO_POS},
		.firefly_storm			= {{14, 15, 16, 17}, AT_Spellcard, "Bug Sign ~ Firefly Storm", 45, 45000,
									wriggle_firefly_storm, wriggle_spellbg, BOSS_DEFAULT_GO_POS},
	},

	.extra.light_singularity	= {{ 0,  1,  2,  3}, AT_ExtraSpell, "Lamp Sign ~ Light Singularity", 75, 45000,
									wriggle_light_singularity, wriggle_spellbg, BOSS_DEFAULT_GO_POS},
};

static Stage3D bgcontext;
static struct {
	float clr_r;
	float clr_g;
	float clr_b;

	float fog_exp;

	float tunnel_angle;
	float tunnel_avel;
	float tunnel_updn;
	float tunnel_side;
} stgstate;

Vector **stage3_bg_pos(Vector pos, float maxrange) {
	//Vector p = {100 * cos(global.frames / 52.0), 100, 50 * sin(global.frames / 50.0)};
	Vector p = {
		stgstate.tunnel_side * cos(global.frames / 52.0),
		0,
		stgstate.tunnel_updn * sin(global.frames / 50.0)
	};
	Vector r = {0, 3000, 0};

	return linear3dpos(pos, maxrange, p, r);
}

void stage3_bg_tunnel_draw(Vector pos) {
	int n = 6;
	float r = 300;
	int i;

	glEnable(GL_TEXTURE_2D);

	glPushMatrix();
	glTranslatef(pos[0], pos[1], pos[2]);

	glBindTexture(GL_TEXTURE_2D, get_tex("stage3/border")->gltex);
	for(i = 0; i < n; i++) {
		glPushMatrix();
		glRotatef(360.0/n*i + stgstate.tunnel_angle, 0, 1, 0);
		glTranslatef(0,0,-r);
		glScalef(2*r/tan((n-2)*M_PI/n), 3000, 1);
		draw_quad();
		glPopMatrix();
	}

	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}

void stage3_tunnel(FBO *fbo) {
	Shader *shader = get_shader("tunnel");

	glColor4f(1,1,1,1);
	glUseProgram(shader->prog);
	glUniform3f(uniloc(shader, "color"),stgstate.clr_r,stgstate.clr_g,stgstate.clr_b);
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, fbo->depth);
	glActiveTexture(GL_TEXTURE0);

	draw_fbo_viewport(fbo);
	glUseProgram(0);
}

void stage3_fog(FBO *fbo) {
	Shader *shader = get_shader("zbuf_fog");

	glColor4f(1,1,1,1);
	glUseProgram(shader->prog);
	glUniform1i(uniloc(shader, "depth"), 2);
	glUniform4f(uniloc(shader, "fog_color"), .5, .5, .5, 1.0);
	glUniform1f(uniloc(shader, "start"), 0.2);
	glUniform1f(uniloc(shader, "end"), 0.8);
	glUniform1f(uniloc(shader, "exponent"), stgstate.fog_exp/2);
	glUniform1f(uniloc(shader, "sphereness"),0);
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, fbo->depth);
	glActiveTexture(GL_TEXTURE0);

	draw_fbo_viewport(fbo);
	glUseProgram(0);
}

void stage3_glitch(FBO *fbo) {
	Shader *shader = get_shader("glitch");

	glColor4f(1,1,1,1);
	float strength;

	if(global.boss && global.boss->current && ATTACK_IS_SPELL(global.boss->current->type) && !strcmp(global.boss->name, "Scuttle")) {
		strength = 0.05 * pow(max(0, (global.frames - global.boss->current->starttime) / (double)global.boss->current->timeout), 2.0);
	} else {
		strength = 0.0;
	}

	if(strength > 0) {
		glUseProgram(shader->prog);
		glUniform1f(uniloc(shader, "strength"), strength);
		glUniform1i(uniloc(shader, "frames"), global.frames + tsrand() % 30);
	} else {
		glUseProgram(0);
	}

	draw_fbo_viewport(fbo);
	glUseProgram(0);
}

void stage3_start(void) {
	init_stage3d(&bgcontext);

	bgcontext.cx[2] = -10;
	bgcontext.crot[0] = -95;
	bgcontext.cv[1] = 10;

	add_model(&bgcontext, stage3_bg_tunnel_draw, stage3_bg_pos);

	memset(&stgstate, 0, sizeof(stgstate));
	stgstate.clr_r = 1.0;
	stgstate.clr_g = 0.0;
	stgstate.clr_b = 0.5;
}

void stage3_preload(void) {
	preload_resources(RES_BGM, RESF_OPTIONAL, "stage3", "stage3boss", NULL);
	preload_resources(RES_TEXTURE, RESF_DEFAULT,
		"stage3/border",
		"stage3/spellbg1",
		"stage3/spellbg2",
		"stage3/wspellbg",
		"stage3/wspellclouds",
		"stage3/wspellswarm",
		"dialog/wriggle",
	NULL);
	preload_resources(RES_SHADER, RESF_DEFAULT,
		"tunnel",
		"zbuf_fog",
		"glitch",
	NULL);
	preload_resources(RES_ANIM, RESF_DEFAULT,
		"scuttle",
		"wriggleex",
	NULL);
}

void stage3_end(void) {
	free_stage3d(&bgcontext);
}

void stage3_draw(void) {
	TIMER(&global.timer)

	set_perspective(&bgcontext, 300, 5000);
	stgstate.tunnel_angle += stgstate.tunnel_avel;
	bgcontext.crot[2] = -(creal(global.plr.pos)-VIEWPORT_W/2)/80.0;
#if 1
	FROM_TO(0, 160, 1) {
		bgcontext.cv[1] -= 0.5/2;
		stgstate.clr_r -= 0.2 / 160.0;
		stgstate.clr_b -= 0.1 / 160.0;
	}

	FROM_TO(0, 500, 1)
		stgstate.fog_exp += 3.0 / 500.0;

	FROM_TO(400, 500, 1) {
		stgstate.tunnel_avel += 0.005;
		bgcontext.cv[1] -= 0.3/2;
	}

	FROM_TO(1050, 1150, 1) {
		stgstate.tunnel_avel -= 0.010;
		bgcontext.cv[1] -= 0.2/2;
	}

	FROM_TO(1060, 1400, 1) {
		/*stgstate.clr_r -= 1.0 / 340.0;
		stgstate.clr_g += 1.0 / 340.0;
		stgstate.clr_b -= 0.5 / 340.0;*/
	}

	FROM_TO(1170, 1400, 1)
		stgstate.tunnel_side += 100.0 / 230.0;

	FROM_TO(1400, 1550, 1) {
		bgcontext.crot[0] -= 3 / 150.0;
		stgstate.tunnel_updn += 70.0 / 150.0;
		stgstate.tunnel_avel += 1 / 150.0;
		bgcontext.cv[1] -= 0.2/2;
	}

	FROM_TO(1570, 1700, 1) {
		stgstate.tunnel_updn -= 20 / 130.0;
	}

	FROM_TO(1800, 1850, 1)
		stgstate.tunnel_avel -= 0.02;

	FROM_TO(1900, 2000, 1) {
		stgstate.tunnel_avel += 0.013;
	}

	FROM_TO(2000, 2740, 1) {
		stgstate.tunnel_side -= 100.0 / 740.0;
		//stgstate.fog_exp -= 1.0 / 740.0;
		bgcontext.crot[0] += 11 / 740.0;
	}

	FROM_TO(2740, 2799, 1) {
		stgstate.fog_exp += 1.0 / 60.0;
		bgcontext.cv[1] += 1.0/2;
		stgstate.clr_r -= 0.3 / 60.0;
		stgstate.clr_g += 0.5 / 60.0;
		stgstate.tunnel_avel -= 0.7 / 60.0;
		bgcontext.crot[0] -= 11 / 60.0;
	}

	// 2800 - MIDBOSS

	int midboss_time = STAGE3_MIDBOSS_TIME;

	FROM_TO(2900 + midboss_time, 3100 + midboss_time, 1) {
		stgstate.clr_r += 0.3 / 200.0;
		stgstate.clr_g -= 0.5 / 200.0;
		bgcontext.cv[1] -= 90 / 200.0/2;
		stgstate.tunnel_avel -= 1 / 200.0;
		stgstate.fog_exp -= 1.0 / 200.0;
		stgstate.clr_b += 0.2 / 200.0;
	}

	FROM_TO(3300 + midboss_time, 3360 + midboss_time, 1) {
		stgstate.tunnel_avel += 2 / 60.0;
		stgstate.tunnel_side += 70 / 60.0;
	}

	FROM_TO(3600 + midboss_time, 3700 + midboss_time, 1) {
		stgstate.tunnel_side += 20 / 60.0;
		stgstate.tunnel_updn += 40 / 60.0;
	}

	FROM_TO(3830 + midboss_time, 3950 + midboss_time, 1) {
		stgstate.tunnel_avel -= 2 / 120.0;
	}

	FROM_TO(3960 + midboss_time, 4000 + midboss_time, 1) {
		stgstate.tunnel_avel += 2 / 40.0;
	}

	FROM_TO(4360 + midboss_time, 4390 + midboss_time, 1) {
		stgstate.clr_r -= .5 / 30.0;
	}

	FROM_TO(4390 + midboss_time, 4510 + midboss_time, 1) {
		stgstate.clr_r += .5 / 120.0;
	}

	FROM_TO(4299 + midboss_time, 5299 + midboss_time, 1) {
		stgstate.tunnel_side -= 90 / 1000.0;
		stgstate.tunnel_updn -= 40 / 1000.0;
		stgstate.clr_r -= 0.5 / 1000.0;
		bgcontext.crot[0] += 7 / 1000.0;
		stgstate.fog_exp -= 1.5 / 1000.0;
	}

	FROM_TO(5099 + midboss_time, 5299 + midboss_time, 1) {
		bgcontext.cv[1] += 90 / 200.0/2;
		stgstate.tunnel_avel -= 1.1 / 200.0;
		bgcontext.crot[0] -= 15 / 200.0;
		//stgstate.fog_exp += 3.0 / 200.0;
	}

	// 5300 - BOSS

	FROM_TO(5301 + midboss_time, 5500 + midboss_time, 1) {
		bgcontext.cv[1] -= 70 / 200.0/2;
		stgstate.clr_r += 1.1 / 200.0;
	}

	FROM_TO(5301 + midboss_time, 5700 + midboss_time, 1) {
		bgcontext.crot[0] -= 10 / 400.0;
		stgstate.fog_exp -= 2.0 / 400.0;
		//stgstate.tunnel_avel -= 0.5 / 200.0;
	}
#endif

	draw_stage3d(&bgcontext, 7000);
}

void scuttle_spellbg(Boss*, int t);

void stage3_spellpractice_events(void) {
	TIMER(&global.timer);

	AT(0) {
		if(global.stage->spell->draw_rule == scuttle_spellbg) {
			skip_background_anim(&bgcontext, stage3_draw, 2800, &global.timer, NULL);
			global.boss = stage3_spawn_scuttle(BOSS_DEFAULT_SPAWN_POS);
		} else {
			skip_background_anim(&bgcontext, stage3_draw, 5300 + STAGE3_MIDBOSS_TIME, &global.timer, NULL);
			global.boss = stage3_spawn_wriggle_ex(BOSS_DEFAULT_SPAWN_POS);
		}

		boss_add_attack_from_info(global.boss, global.stage->spell, true);
		boss_start_attack(global.boss, global.boss->attacks);

		start_bgm("stage3boss");
	}
}

ShaderRule stage3_shaders[] = { stage3_fog, stage3_tunnel, NULL };
ShaderRule stage3_postprocess[] = { stage3_glitch, NULL };

StageProcs stage3_procs = {
	.begin = stage3_start,
	.preload = stage3_preload,
	.end = stage3_end,
	.draw = stage3_draw,
	.event = stage3_events,
	.shader_rules = stage3_shaders,
	.postprocess_rules = stage3_postprocess,
	.spellpractice_procs = &stage3_spell_procs,
};

StageProcs stage3_spell_procs = {
	.preload = stage3_preload,
	.begin = stage3_start,
	.end = stage3_end,
	.draw = stage3_draw,
	.event = stage3_spellpractice_events,
	.shader_rules = stage3_shaders,
	.postprocess_rules = stage3_postprocess,
};
