/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2018, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2018, Andrei Alexeyev <akari@alienslab.net>.
 */

#pragma once
#include "taisei.h"

#include "util.h"
#include "resource.h"

#include "renderer/api.h"

typedef struct ObjFileData {
	vec3_noalign *xs;
	int xcount;

	vec3_noalign *normals;
	int ncount;

	vec3_noalign *texcoords;
	int tcount;

	ivec3_noalign *indices;
	int icount;

	int fverts;
} ObjFileData;

struct Model {
	VertexArray *vertex_array;
	size_t num_vertices;
	size_t offset;
	Primitive primitive;
	bool indexed;
};

char* model_path(const char *name);
bool check_model_path(const char *path);
void* load_model_begin(const char *path, uint flags);
void* load_model_end(void *opaque, const char *path, uint flags);
void unload_model(void*); // Does not delete elements from the VBO, so doing this at runtime is leaking VBO space

Model* get_model(const char *name);

extern ResourceHandler model_res_handler;

#define MDL_PATH_PREFIX "res/models/"
#define MDL_EXTENSION ".obj"
