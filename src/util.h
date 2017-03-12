
#ifndef TSUTIL_H
#define TSUTIL_H

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <zlib.h> // compiling under mingw may fail without this...
#include <png.h>
#include <SDL.h>
#include "hashtable.h"

//
// compatibility
//

#ifndef __GNUC__ // clang defines this too
#define __attribute__(...)
#endif

//
// string utils
//

#undef strlcat
#undef strlcpy
#define strlcat SDL_strlcat
#define strlcpy SDL_strlcpy

char* copy_segment(const char *text, const char *delim, int *size);
bool strendswith(const char *s, const char *e)  __attribute__((pure));
bool strstartswith(const char *s, const char *p)  __attribute__((pure));
bool strendswith_any(const char *s, const char **earray)  __attribute__((pure));
void stralloc(char **dest, const char *src);
char* strjoin(const char *first, ...) __attribute__((sentinel));
char* strfmt(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void strip_trailing_slashes(char *buf);
#undef strdup
#define strdup SDL_strdup

//
// math utils
//

#include <complex.h>

// These definitions are common but non-standard, so we provide our own
#undef M_PI
#undef M_PI_2
#define M_PI 3.14159265358979323846
#define M_PI_2 1.57079632679489661923

// This is a workaround to properly specify the type of our "complex" variables...
// Taisei code always uses just "complex" when it actually means "complex double", which is not really correct...
// gcc doesn't seem to care, other compilers do (e.g. clang)
#ifdef complex
    #undef complex
    #define complex _Complex double
#endif

// needed for mingw compatibility:
#undef min
#undef max

double min(double, double) __attribute__((const));
double max(double, double) __attribute__((const));
double clamp(double, double, double) __attribute__((const));
double approach(double v, double t, double d) __attribute__((const));
double psin(double) __attribute__((const));
int sign(double) __attribute__((const));

//
// gl/video utils
//

typedef struct {
    int fpstime;  // frame counter
    int fps;
    int show_fps;
    double stagebg_fps;
} FPSCounter;

void frame_rate(int *lasttime);
void calc_fps(FPSCounter *fps);
void set_ortho(void);
void colorfill(float r, float g, float b, float a);
void fade_out(float f);

//
// i/o utils
//

typedef void (*KVCallback)(const char *key, const char *value, void *data);

char* read_all(const char *filename, int *size);
bool parse_keyvalue_stream_cb(SDL_RWops *strm, KVCallback callback, void *data);
bool parse_keyvalue_file_cb(const char *filename, KVCallback callback, void *data);
Hashtable* parse_keyvalue_stream(SDL_RWops *strm, size_t tablesize);
Hashtable* parse_keyvalue_file(const char *filename, size_t tablesize);
void png_init_rwops(png_structp png, SDL_RWops *rwops);

char* SDL_RWgets(SDL_RWops *rwops, char *buf, size_t bufsize);

//
// misc utils
//

int getenvint(const char *v) __attribute__((pure));

//
// safeguards against some dangerous practices
//

#undef fopen
FILE* fopen() __attribute__((deprecated("Use SDL_RWFromFile instead")));

#undef strncat
char* strncat() __attribute__((deprecated("This function likely doesn't do what you expect, use strlcat")));

#undef strncpy
char* strncpy() __attribute__((deprecated("This function likely doesn't do what you expect, use strlcpy")));

#endif
