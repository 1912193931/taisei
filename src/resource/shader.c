/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information.
 * ---
 * Copyright (C) 2011, Lukas Weber <laochailan@web.de>
 */

#include <stdio.h>

#include "taiseigl.h"
#include "shader.h"
#include "resource.h"
#include "config.h"
#include "list.h"
#include "taisei_err.h"

static char snippet_header_EXT_draw_instanced[] =
	"#version 120\n"
	"#extension GL_EXT_draw_instanced : enable\n"
;

static char snippet_header_ARB_draw_instanced[] =
	"#version 120\n"
	"#extension GL_ARB_draw_instanced : enable\n"
	"#define gl_InstanceID gl_InstanceIDARB"
;

static char snippet_header_gl31[] =
	"#version 140\n"
;

char* shader_path(const char *name) {
	return strjoin(get_prefix(), SHA_PATH_PREFIX, name, SHA_EXTENSION, NULL);
}

bool check_shader_path(const char *path) {
	return strendswith(path, SHA_EXTENSION);
}

static Shader* load_shader(const char *vheader, const char *fheader, const char *vtext, const char *ftext);

void* load_shader_file(const char *path) {
	char *text, *vtext, *ftext, *delim;

	text = read_all(path, NULL);

	vtext = text;
	delim = strstr(text, SHA_DELIM);

	if(delim == NULL) {
		warnx("load_shader_file(): expected '%s' delimiter.", SHA_DELIM);
		free(text);
		return NULL;
	}

	*delim = 0;
	ftext = delim + SHA_DELIM_SIZE;

	Shader *sha = load_shader(NULL, NULL, vtext, ftext);

	free(text);

	return sha;
}

void unload_shader(void *vsha) {
	Shader *sha = vsha;
	glDeleteProgram((sha)->prog);
	hashtable_free(sha->uniforms);
	free(sha);
}

static char* get_snippet_header(void) {
	if(glext.EXT_draw_instanced) {
		return snippet_header_EXT_draw_instanced;
	} else if(glext.ARB_draw_instanced) {
		return snippet_header_ARB_draw_instanced;
	} else {
		// probably won't work
		return snippet_header_gl31;
	}
}

void load_shader_snippets(const char *filename, const char *prefix) {
	int size, vhsize = 0, vfsize = 0, fhsize = 0, ffsize = 0, ssize, prefixlen;
	char *text, *vhead, *vfoot, *fhead, *ffoot;
	char *sec, *name, *nend, *send;
	char *vtext = NULL, *ftext = NULL;
	char *nbuf;

	printf("-- loading snippet file '%s'\n", filename);

	prefixlen = strlen(prefix);

	text = read_all(filename, &size);

	sec = text;

	vhead = copy_segment(text, "%%VSHADER-HEAD%%", &vhsize);
	vfoot = copy_segment(text, "%%VSHADER-FOOT%%", &vfsize);

	fhead = copy_segment(text, "%%FSHADER-HEAD%%", &fhsize);
	ffoot = copy_segment(text, "%%FSHADER-FOOT%%", &ffsize);

	if(!vhead || !fhead)
		errx(-1, "Syntax Error: missing HEAD section(s).");
	if((vfoot == NULL) + (ffoot == NULL) != 1)
		errx(-1, "Syntax Error: must contain exactly 1 FOOT section");

	while((sec = strstr(sec, "%%"))) {
		sec += 2;

		name = sec;
		nend = strstr(name, "%%");
		if(!nend)
			errx(-1, "Syntax Error: expected '%%'");

		sec = nend + 2;

		if(strncmp(name+1, "SHADER", 6) == 0)
			continue;

		send = strstr(sec, "%%");
		if(!send)
			send = text + size + 1;
		send--;

		ssize = send-sec;

		if(vfoot) {
			vtext = malloc(vhsize + ssize + vfsize + 1);
			ftext = malloc(fhsize + 1);

			memset(vtext, 0, vhsize + ssize + vfsize + 1);
			memset(ftext, 0, fhsize + 1);

			strlcpy(vtext, vhead, vhsize+1);
			strlcpy(ftext, fhead, fhsize+1);

			strlcpy(vtext+vhsize, sec, ssize+1);
			strlcpy(vtext+vhsize+ssize, vfoot, vfsize+1);

		} else if(ffoot) {
			ftext = malloc(fhsize + ssize + ffsize + 1);
			vtext = malloc(vhsize + 1);

			memset(ftext, 0, fhsize + ssize + ffsize + 1);
			memset(vtext, 0, vhsize + 1);

			strlcpy(ftext, fhead, fhsize+1);
			strlcpy(vtext, vhead, vhsize+1);

			strlcpy(ftext+fhsize, sec, ssize+1);
			strlcpy(ftext+fhsize+ssize, ffoot, ffsize+1);
		}

		nbuf = malloc(nend-name+prefixlen+1);
		strcpy(nbuf, prefix);
		strlcat(nbuf+prefixlen, name, nend-name+1);
		nbuf[nend-name+prefixlen] = 0;

		Shader *sha = load_shader(get_snippet_header(), NULL, vtext, ftext);
		insert_resource(RES_SHADER, nbuf, sha, 0, filename);

		free(nbuf);
		free(vtext);
		free(ftext);
	}

	free(vhead);
	free(fhead);
	free(vfoot);
	free(ffoot);
	free(text);
}

static void print_info_log(GLuint shader, tsglGetShaderiv_ptr lenfunc, tsglGetShaderInfoLog_ptr logfunc, const char *type) {
	int len = 0, alen = 0;
	lenfunc(shader, GL_INFO_LOG_LENGTH, &len);

	if(len > 1) {
		printf(" == GLSL %s info log (%u) ==\n", type, shader);
		char log[len];
		memset(log, 0, len);
		logfunc(shader, len, &alen, log);
		printf("%s\n", log);
		printf("\n == End of GLSL %s info log (%u) ==\n", type, shader);
	}
}

static void cache_uniforms(Shader *sha) {
	int i, maxlen = 0;
	GLint tmpi;
	GLenum tmpt;
	GLint unicount;

	sha->uniforms = hashtable_new_stringkeys(17);

	glGetProgramiv(sha->prog, GL_ACTIVE_UNIFORMS, &unicount);
	glGetProgramiv(sha->prog, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxlen);

	char name[maxlen];

	for(i = 0; i < unicount; i++) {
		glGetActiveUniform(sha->prog, i, maxlen, NULL, &tmpi, &tmpt, name);
		GLint loc = glGetUniformLocation(sha->prog, name);

		// don't cache builtin uniforms
		if(loc < 0) {
			continue;
		}

		// a 0 value indicates a non-existing element in the hashtable
		// however, 0 is also a perfectly valid uniform location
		// we distinguish 0 locations from invalid ones by storing them as -1
		if(loc == 0) {
			loc = -1;
		}

		hashtable_set_string(sha->uniforms, name, (void*)(intptr_t)loc);
	}

#ifdef DEBUG_GL
	hashtable_print_stringkeys(sha->uniforms);
#endif
}

static Shader* load_shader(const char *vheader, const char *fheader, const char *vtext, const char *ftext) {
	Shader *sha = malloc(sizeof(Shader));
	GLuint vshaderobj;
	GLuint fshaderobj;

	sha->prog = glCreateProgram();
	vshaderobj = glCreateShader(GL_VERTEX_SHADER);
	fshaderobj = glCreateShader(GL_FRAGMENT_SHADER);

	if(!vheader) {
		vheader = "";
	}

	if(!fheader) {
		fheader = "";
	}

	const GLchar *v_sources[] = { vheader, vtext };
	const GLchar *f_sources[] = { fheader, ftext };
	GLint lengths[] = { -1, -1 };

	glShaderSource(vshaderobj, 2, (const GLchar**)v_sources, lengths);
	glShaderSource(fshaderobj, 2, (const GLchar**)f_sources, lengths);

	glCompileShader(vshaderobj);
	glCompileShader(fshaderobj);

	print_info_log(vshaderobj, glGetShaderiv, glGetShaderInfoLog, "Vertex Shader");
	print_info_log(fshaderobj, glGetShaderiv, glGetShaderInfoLog, "Fragment Shader");

	glAttachShader(sha->prog, vshaderobj);
	glAttachShader(sha->prog, fshaderobj);

	glDeleteShader(vshaderobj);
	glDeleteShader(fshaderobj);

	glLinkProgram(sha->prog);

	print_info_log(sha->prog, glGetProgramiv, glGetProgramInfoLog, "Program");

	cache_uniforms(sha);

	return sha;
}

int uniloc(Shader *sha, const char *name) {
	int loc = (intptr_t)hashtable_get_string(sha->uniforms, name);

	// 0 and -1 are swapped in the hashtable to distinguish 0-location uniforms from non-existent ones
	// see the comment in cache_uniforms() above
	if(loc == -1) {
		loc = 0;
	} else if(loc == 0) {
		loc = -1;
	}

	return loc;
}

Shader* get_shader(const char *name) {
	return get_resource(RES_SHADER, name, RESF_REQUIRED)->shader;
}
