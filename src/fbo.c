/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information.
 * ---
 * Copyright (C) 2011, Lukas Weber <laochailan@web.de>
 */

#include "fbo.h"
#include "global.h"

static float sanitize_scale(float scale) {
	// return ftopow2(clamp(scale, 0.25, 4.0));
	scale = max(0.1, scale);
	return ((int)(scale*VIEWPORT_W*VIEWPORT_H+0.9999))/(float)VIEWPORT_W/VIEWPORT_H;
}

void init_fbo(FBO *fbo, float scale) {
	glGenTextures(1, &fbo->tex);
	glBindTexture(GL_TEXTURE_2D, fbo->tex);

	fbo->scale = scale = sanitize_scale(scale);
	fbo->nw = topow2(scale * VIEWPORT_W);
	fbo->nh = topow2(scale * VIEWPORT_H);

	log_debug("FBO %p: q=%f, w=%i, h=%i", (void*)fbo, fbo->scale, fbo->nw, fbo->nh);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fbo->nw, fbo->nh, 0, GL_BGR, GL_UNSIGNED_BYTE, NULL);

	glGenFramebuffers(1,&fbo->fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo->fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo->tex, 0);

	glGenTextures(1, &fbo->depth);
	glBindTexture(GL_TEXTURE_2D, fbo->depth);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, fbo->nw, fbo->nh, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,fbo->depth, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void reinit_fbo(FBO *fbo, float scale) {
	if(!fbo->scale) {
		// fbo was never initialized
		init_fbo(fbo, scale);
		return;
	}

	if(fbo->scale != sanitize_scale(scale)) {
		delete_fbo(fbo);
		init_fbo(fbo, scale);
	}
}

void delete_fbo(FBO *fbo) {
	glDeleteFramebuffers(1, &fbo->fbo);
	glDeleteTextures(1, &fbo->depth);
	glDeleteTextures(1, &fbo->tex);
}

void draw_fbo(FBO *fbo) {
	float wq = (fbo->scale*VIEWPORT_W)/(float)fbo->nw;
	float hq = (fbo->scale*VIEWPORT_H)/(float)fbo->nh;
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glScalef(wq, hq, 1);
	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
		glTranslatef(VIEWPORT_W/2, VIEWPORT_H/2, 0);
		glScalef(VIEWPORT_W, VIEWPORT_H, 1);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, fbo->tex);
		glDrawArrays(GL_QUADS, 4, 4);
		glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
}

void draw_fbo_viewport(FBO *fbo) {
	// assumption: rendering into another, identical FBO

	glViewport(0, 0, fbo->scale*VIEWPORT_W, fbo->scale*VIEWPORT_H);
	set_ortho_ex(VIEWPORT_W,VIEWPORT_H);
	draw_fbo(fbo);
}
