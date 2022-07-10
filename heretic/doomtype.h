//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	Simple basic typedefs, isolated here to make it easier
//	 separating modules.
//    


#ifndef __DOOMTYPE__
#define __DOOMTYPE__

#include <stdint.h>
#include <stdio.h>

#include "config.h"

#define O_BINARY	0x0200

uint32_t doom_sprintf(uint8_t*, const uint8_t*, ...);
uint32_t doom_sscanf(const uint8_t*, const uint8_t*, ...);
uint32_t doom_fscanf(FILE*, const uint8_t*, ...);
uint32_t doom_printf(const uint8_t*, ...);
int32_t doom_open(const char*,uint32_t, ...);
int32_t doom_close(int32_t) __attribute((regparm(2)));
int32_t doom_write(int32_t,void*,uint32_t) __attribute((regparm(2)));
int32_t doom_read(int32_t,void*,uint32_t) __attribute((regparm(2)));
int32_t doom_lseek(int32_t,int32_t,uint32_t) __attribute((regparm(2)));
int32_t doom_fstat(int32_t,void*) __attribute((regparm(2)));
int32_t doom_atoi(char*) __attribute((regparm(2)));

void *doom_I_ZoneBase(uint32_t *size) __attribute((regparm(2)));

void *doom_Z_Malloc(uint32_t size, uint32_t tag, void *ptr) __attribute((regparm(2)));
void doom_Z_Free(void*) __attribute((regparm(2)));

void *doom_malloc(uint32_t) __attribute((regparm(2)));
void *doom_calloc(uint32_t,uint32_t) __attribute((regparm(2)));
void *doom_realloc(void*,uint32_t) __attribute((regparm(2)));
void doom_free(void*) __attribute((regparm(2)));

void dos_exit(int32_t) __attribute((regparm(2),noreturn));

FILE *doom_fopen(const char *filename, const char *mode) __attribute((regparm(2)));
int32_t doom_fclose(FILE *stream) __attribute((regparm(2)));
size_t doom_fread(void *ptr, size_t size, size_t count, FILE *stream) __attribute((regparm(2)));
size_t doom_fwrite(const void *ptr, size_t size, size_t count, FILE *stream) __attribute((regparm(2)));
int32_t doom_fseek(FILE *stream, int32_t offset, uint32_t origin) __attribute((regparm(2)));
size_t doom_ftell(FILE *stream) __attribute((regparm(2)));
int doom_fgetc(FILE *stream) __attribute((regparm(2)));
int doom_fputc(int, FILE *stream) __attribute((regparm(2)));
int doom_ungetc(int character, FILE *stream) __attribute((regparm(2)));

static inline int doom_feof(FILE *stream)
{
	uint8_t *ptr = (uint8_t*)stream + 12;
	return !!(*ptr & 16);
}

static inline char x_toupper(char in)
{
	if(in >= 'a' && in <= 'z')
		in &= 0xDF;
	return in;
}

static inline char x_tolower(char in)
{
	if(in >= 'A' && in <= 'Z')
		in |= 0x20;
	return in;
}

static inline int x_isspace(char in)
{
	if(in == 0x20)
		return 1;
	if(in == 0x0c)
		return 1;
	if(in == 0x0a)
		return 1;
	if(in == 0x0d)
		return 1;
	if(in == 0x09)
		return 1;
	if(in == 0x0b)
		return 1;
	return 0;
}

static inline int x_isprint(char in)
{
	if(in < ' ')
		return 0;
	if(in >= 0x7F)
		return 0;
	return 1;
}

static inline int x_isdigit(char in)
{
	if(in < '0')
		return 0;
	if(in > '9')
		return 0;
	return 1;
}

static inline int x_isalpha(char in)
{
	if(in < 'A')
		return 0;
	if(in > 'Z' && in < 'a')
		return 0;
	if(in > 'z')
		return 0;
	return 1;
}

// MEH
#define M_snprintf(t,l,s,...)	doom_sprintf(t,s,__VA_ARGS__)
#define DEH_printf	doom_printf
#define DEH_snprintf(t,l,s,...)	doom_sprintf(t,s,__VA_ARGS__)
#define tprintf	doom_printf
#define hprintf(x)	doom_printf("%s\n",x)


#if defined(_MSC_VER) && !defined(__cplusplus)
#define inline __inline
#endif

// #define macros to provide functions missing in Windows.
// Outside Windows, we use strings.h for str[n]casecmp.

/*
#if !HAVE_DECL_STRCASECMP || !HAVE_DECL_STRNCASECMP

#include <string.h>
#if !HAVE_DECL_STRCASECMP
#define strcasecmp stricmp
#endif
#if !HAVE_DECL_STRNCASECMP
#define strncasecmp strnicmp
#endif

#else

#include <strings.h>

#endif
*/

//
// The packed attribute forces structures to be packed into the minimum 
// space necessary.  If this is not done, the compiler may align structure
// fields differently to optimize memory access, inflating the overall
// structure size.  It is important to use the packed attribute on certain
// structures where alignment is important, particularly data read/written
// to disk.
//

#ifdef __GNUC__

#if defined(_WIN32) && !defined(__clang__)
#define PACKEDATTR __attribute__((packed,gcc_struct))
#else
#define PACKEDATTR __attribute__((packed))
#endif

#define PRINTF_ATTR(fmt, first) __attribute__((format(printf, fmt, first)))
#define PRINTF_ARG_ATTR(x) __attribute__((format_arg(x)))
#define NORETURN __attribute__((noreturn))

#else
#if defined(_MSC_VER)
#define PACKEDATTR __pragma(pack(pop))
#else
#define PACKEDATTR
#endif
#define PRINTF_ATTR(fmt, first)
#define PRINTF_ARG_ATTR(x)
#define NORETURN
#endif

#ifdef __WATCOMC__
#define PACKEDPREFIX _Packed
#elif defined(_MSC_VER)
#define PACKEDPREFIX __pragma(pack(push,1))
#else
#define PACKEDPREFIX
#endif

#define PACKED_STRUCT(...) PACKEDPREFIX struct __VA_ARGS__ PACKEDATTR

// C99 integer types; with gcc we just use this.  Other compilers
// should add conditional statements that define the C99 types.

// What is really wanted here is stdint.h; however, some old versions
// of Solaris don't have stdint.h and only have inttypes.h (the 
// pre-standardisation version).  inttypes.h is also in the C99 
// standard and defined to include stdint.h, so include this. 

#include <inttypes.h>

#if defined(__cplusplus) || defined(__bool_true_false_are_defined)

// Use builtin bool type with C++.

typedef bool boolean;

#else

typedef enum 
{
    false, 
    true
} boolean;

#endif

typedef uint8_t byte;
typedef uint8_t pixel_t;
typedef int16_t dpixel_t;

#include <limits.h>

#ifdef _WIN32

#define DIR_SEPARATOR '\\'
#define DIR_SEPARATOR_S "\\"
#define PATH_SEPARATOR ';'

#else

#define DIR_SEPARATOR '/'
#define DIR_SEPARATOR_S "/"
#define PATH_SEPARATOR ':'

#endif

#define arrlen(array) (sizeof(array) / sizeof(*array))

#endif

