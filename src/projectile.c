/*
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA  02110-1301, USA.
 
 ---
 Copyright (C) 2010, Lukas Weber <laochailan@web.de>
 */

#include "projectile.h"

#include <stdlib.h>
#include <math.h>
#include "global.h"

ProjCache _projs;

void load_projectiles() {
	load_texture(FILE_PREFIX "gfx/projectiles/ball.png", &_projs.ball);
	load_texture(FILE_PREFIX "gfx/projectiles/rice.png", &_projs.rice);
	load_texture(FILE_PREFIX "gfx/projectiles/bigball.png", &_projs.bigball);
	load_texture(FILE_PREFIX "gfx/proyoumu.png", &_projs.youmu);
}

Projectile *create_projectile(int x, int y, int v, float angle, ProjRule rule, Texture *tex, Color clr) {
	Projectile *p, *last = global.projs;
	
	p = malloc(sizeof(Projectile));
	
	if(last != NULL) {
		while(last->next != NULL)
			last = last->next;
		last->next = p;
	} else {
		global.projs = p;
	}
	
	*p = ((Projectile){ global.frames,x,y,x,y,v,angle,rule,tex,FairyProj,NULL,last,clr });
	
	return p;
}

void delete_projectile(Projectile *proj) {
	if(proj->prev != NULL)
		proj->prev->next = proj->next;
	if(proj->next != NULL)
		proj->next->prev = proj->prev;	
	if(global.projs == proj)
		global.projs = proj->next;
	
	free(proj);
}

void free_projectiles() {
	Projectile *proj = global.projs;
	Projectile *tmp;
	
	while(proj != 0) {
		tmp = proj;
		proj = proj->next;
		delete_projectile(tmp);
	} 
	
	global.projs = NULL;
}

int test_collision(Projectile *p) {	
	if(p->type == FairyProj) {
		float angle = atan((float)(global.plr.y - p->y)/(global.plr.x - p->x));
		
		int projr = sqrt(pow(p->tex->w/4*cos(angle),2)*8/10 + pow(p->tex->h/2*sin(angle)*8/10,2));
		if(sqrt(pow(p->x-global.plr.x,2) + pow(p->y-global.plr.y,2)) < projr+5)
			return 1;
	} else {
		Fairy *f = global.fairies;
		while(f != NULL) {
			float angle = atan((float)(f->y - p->y)/(f->x - p->x));
			
			int projr = sqrt(pow(p->tex->w/4*cos(angle),2)*8/10 + pow(p->tex->h/2*sin(angle)*8/10,2));			
			if(sqrt(pow(p->x-f->x,2) + pow(p->y-f->y,2)) < projr+10) {
				f->hp--;
				return 2;
			}
			f = f->next;
		}
	}
	return 0;
}

void draw_projectiles() {
	Projectile *proj = global.projs;
	int size = 3;
	int i = 0, i1;
	Texture **texs = calloc(size, sizeof(Texture *));
	texs[i++] = proj->tex;
	
	while(proj != NULL) {
		for(i1 = 0; i1 < i; i1++)
			if(proj->tex == texs[i1])
				goto next0;
		
		if(i >= size)
			texs = realloc(texs, (size++)*sizeof(Texture *));
		texs[i++] = proj->tex;
next0:
		proj = proj->next;
	}
		
	glEnable(GL_TEXTURE_2D);	
	for(i1 = 0; i1 < i; i1++) {
		Texture *tex = texs[i1];
				
		glBindTexture(GL_TEXTURE_2D, tex->gltex);	
		
		float wq = ((float)tex->w/2.0)/tex->truew;
		float hq = ((float)tex->h)/tex->trueh;
		
		proj = global.projs;
		while(proj != NULL) {
			if(proj->tex != tex)
				goto next1;
			glPushMatrix();
			
			glTranslatef(proj->x, proj->y, 0);
			glRotatef(proj->angle, 0, 0, 1);
			glScalef(tex->w/4, tex->h/2,0);
			
			glBegin(GL_QUADS);
				glTexCoord2f(0,0); glVertex2f(-1, -1);
				glTexCoord2f(0,hq); glVertex2f(-1, 1);
				glTexCoord2f(wq,hq); glVertex2f(1, 1);
				glTexCoord2f(wq,0);	glVertex2f(1, -1);
				
				glColor3fv((float *)&proj->clr);
				glTexCoord2f(wq,0); glVertex2f(-1, -1);
				glTexCoord2f(wq,hq); glVertex2f(-1, 1);
				glTexCoord2f(2*wq,hq); glVertex2f(1, 1);
				glTexCoord2f(2*wq,0); glVertex2f(1, -1);
			glEnd();
			
			glPopMatrix();
			glColor3f(1,1,1);			
next1:
			proj = proj->next;
		}
	}
	glDisable(GL_TEXTURE_2D);
}

void process_projectiles() {
	Projectile *proj = global.projs, *del = NULL;
	while(proj != NULL) {
		proj->rule(proj);
		
		int v = test_collision(proj);
		if(v == 1)
			game_over();
		
		if(v || proj->x + proj->tex->w/2 < 0 || proj->x - proj->tex->w/2 > VIEWPORT_W
			 || proj->y + proj->tex->h/2 < 0 || proj->y - proj->tex->h/2 > VIEWPORT_H) {
			del = proj;
			proj = proj->next;
			delete_projectile(del);
			if(proj == NULL) break;
		} else {
			proj = proj->next;
		}
	}		
}

void simple(Projectile *p) { // sure is physics in here
	p->y = p->sy + p->v*sin((p->angle-90)/180*M_PI)*(global.frames-p->birthtime);
	p->x = p->sx + p->v*cos((p->angle-90)/180*M_PI)*(global.frames-p->birthtime);
}
