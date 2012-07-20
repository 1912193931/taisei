/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information. 
 * ---
 * Copyright (C) 2011, Lukas Weber <laochailan@web.de>
 */

#include "stage3.h"
#include "global.h"
#include "stage.h"
#include "stageutils.h"
#include "stage3_events.h"

static Stage3D bgcontext;

void stage3_fog(int fbonum) {
	Shader *shader = get_shader("zbuf_fog");
	
	float f = 0;
	if(global.timer > 5100) {
		float v = (global.timer-5100)*0.0005;
		f =  v < 0.1 ? v : 0.1;
	}
	
	glUseProgram(shader->prog);
	glUniform1i(uniloc(shader, "depth"),2);
	glUniform4f(uniloc(shader, "fog_color"),10*f,0,0.1-f,1.0);
	glUniform1f(uniloc(shader, "start"),0.4);
	glUniform1f(uniloc(shader, "end"),0.8);
	glUniform1f(uniloc(shader, "exponent"),4.0);
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, resources.fbg[fbonum].depth);
	glActiveTexture(GL_TEXTURE0);
	
	draw_fbo_viewport(&resources.fbg[fbonum]);
	glUseProgram(0);
}

Vector **stage3_fountain_pos(Vector pos, float maxrange) {
	Vector p = {0, 400, 1500};
	Vector r = {0, 0, 3000};
	
	Vector **list = linear3dpos(pos, maxrange, p, r);
	
	int i;
	
	for(i = 0; list[i] != NULL; i++) {
		if((*list[i])[2] > 0)
			(*list[i])[2] = -9000;
	}
	
	return list;
}

void stage3_fountain_draw(Vector pos) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, get_tex("stage1/border")->gltex);
	
	glPushMatrix();
	glTranslatef(pos[0], pos[1], pos[2]);
	glRotatef(-90, 1,0,0);
	glScalef(1000,3010,1);
		
	draw_quad();
	
	glPopMatrix();
	
	glDisable(GL_TEXTURE_2D);
}

Vector **stage3_lake_pos(Vector pos, float maxrange) {
	Vector p = {0, 600, 0};
	Vector d;
	
	int i;
	
	for(i = 0; i < 3; i++)
		d[i] = p[i] - pos[i];
	
	if(length(d) > maxrange) {
		return NULL;
	} else {
		Vector **list = calloc(2, sizeof(Vector*));
		
		list[0] = malloc(sizeof(Vector));
		for(i = 0; i < 3; i++)
			(*list[0])[i] = p[i];
		list[1] = NULL;
		
		return list;
	}	
}

void stage3_lake_draw(Vector pos) {	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, get_tex("stage3/lake")->gltex);
	
	glPushMatrix();
	glTranslatef(pos[0], pos[1]+140, pos[2]);
	glScalef(15,15,15);
		
// 	glRotatef(180,1,0,0);
	
	draw_model("lake");
	glPopMatrix();
	
	glPushMatrix();
	glTranslatef(pos[0], pos[1]+944, pos[2]+50);
	glScalef(30,30,30);
	
	glBindTexture(GL_TEXTURE_2D, get_tex("stage3/mansion")->gltex);
	
	draw_model("mansion");
	glPopMatrix();
	
	
	glDisable(GL_TEXTURE_2D);
}

Vector **stage3_corridor_pos(Vector pos, float maxrange) {
	Vector p = {0, 2400, 50};
	Vector r = {0, 2000, 0};
	
	Vector **list = linear3dpos(pos, maxrange, p, r);
	
	int i;
	
	for(i = 0; list[i] != NULL; i++) {
		if((*list[i])[1] < p[1])
			(*list[i])[1] = -9000;
	}
	
	return list;
}

void stage3_corridor_draw(Vector pos) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, get_tex("stage3/planks")->gltex);
	
	glMatrixMode(GL_TEXTURE);
	glScalef(1,2,1);
	glMatrixMode(GL_MODELVIEW);
	
	glPushMatrix();
	glTranslatef(pos[0], pos[1], pos[2]);
	
	glPushMatrix();
	glRotatef(180, 1,0,0);
	glScalef(300,2000,1);
	
	draw_quad();
	glPopMatrix();
		
	glBindTexture(GL_TEXTURE_2D, get_tex("stage3/wall")->gltex);
	
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glRotatef(90,0,0,1);
	glScalef(1,10,1);
	glMatrixMode(GL_MODELVIEW);
	
	glPushMatrix();
	glTranslatef(100,5,75);
	glRotatef(90, 0,1,0);
	glScalef(150,2000,1);
	draw_quad();
	glPopMatrix();
	
	glPushMatrix();
	glTranslatef(-100,5,75);
	glRotatef(180,1,0,0);
	glRotatef(-90, 0,1,0);
	glScalef(150,2000,1);
	draw_quad();
	glPopMatrix();
	
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	
	glDisable(GL_TEXTURE_2D);
	
	glColor3f(0.01,0.01,0.01);
	glPushMatrix();
	glTranslatef(0,0,150);
	glScalef(500,2000,1);
	draw_quad();
	glPopMatrix();
	
	glPopMatrix();
	glColor3f(1,1,1);
}

void stage3_start() {
	init_stage3d(&bgcontext);
	
// 	bgcontext.cx[2] = -10000;
// 	bgcontext.cv[2] = 19.7;
// 	bgcontext.crot[0] = 80;
	
	// for testing
	bgcontext.cx[1] = 2000;
	bgcontext.cx[2] = 130;
	bgcontext.cv[1] = 10;
	bgcontext.crot[0] = 80;
	
	add_model(&bgcontext, stage3_lake_draw, stage3_lake_pos);
	add_model(&bgcontext, stage3_fountain_draw, stage3_fountain_pos);
	add_model(&bgcontext, stage3_corridor_draw, stage3_corridor_pos);
}

void stage3_end() {
	free_stage3d(&bgcontext);
}

void stage3_draw() {
	set_perspective(&bgcontext, 130, 3000);
	
	draw_stage3d(&bgcontext, 4000);
	
	if(bgcontext.cx[2] >= -1000 && bgcontext.cv[2] > 0)
		bgcontext.cv[2] -= 0.17;
	
	if(bgcontext.cx[1] < 100 && bgcontext.cv[2] < 0)
		bgcontext.cv[2] = 0;
	
	if(bgcontext.cx[2] >= 0 && bgcontext.cx[2] <= 10)
		bgcontext.cv[1] += 0.2;
	
	if(bgcontext.cx[1] >= 1200 && bgcontext.cx[1] <= 2000)
		bgcontext.cv[1] += 0.02;
}

void stage3_loop() {
	ShaderRule shaderrules[] = { stage3_fog, NULL };
	stage_loop(stage_get(4), stage3_start, stage3_end, stage3_draw, stage3_events, shaderrules, 5500);
}
