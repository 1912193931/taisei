/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information.
 * ---
 * Copyright (C) 2011, Lukas Weber <laochailan@web.de>
 */

#ifndef TAISEIGL_H
#define TAISEIGL_H

#include <SDL_platform.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#endif

enum {
	TGLEXT_draw_instanced = 0,
	_TGLEXT_COUNT
};

extern int tgl_ext[_TGLEXT_COUNT];

void load_gl_functions(void);
void check_gl_extensions(void);

#ifdef __WINDOWS__
PFNGLACTIVETEXTUREPROC glActiveTexture;
PFNGLBLENDEQUATIONPROC glBlendEquation;
#endif

#ifndef __APPLE__
PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate;
PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced;

PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;

PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLBUFFERSUBDATAPROC glBufferSubData;
PFNGLDELETEBUFFERSPROC glDeleteBuffers;

PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLDELETEPROGRAMPROC glDeleteProgram;

PFNGLCREATESHADERPROC glCreateShader;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLDELETESHADERPROC glDeleteShader;

PFNGLGETACTIVEUNIFORMPROC glGetActiveUniform;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;

PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM2FPROC glUniform2f;
PFNGLUNIFORM3FPROC glUniform3f;
PFNGLUNIFORM4FPROC glUniform4f;

PFNGLUNIFORM1IPROC glUniform1i;
PFNGLUNIFORM2IPROC glUniform2i;
PFNGLUNIFORM3IPROC glUniform3i;
PFNGLUNIFORM4IPROC glUniform4i;

PFNGLUNIFORM2FVPROC glUniform2fv;
PFNGLUNIFORM3FVPROC glUniform3fv;
PFNGLUNIFORM4FVPROC glUniform4fv;
#endif

#endif
