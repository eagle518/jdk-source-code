/*
 * @(#)MOD_mmio.c	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**
**	History	-
**	12/1/96		Created
**	7/17/97		Added compile time switch
**	11/10/97	Changed some preprocessor tests and flags to explicity test for flags rather
**				than assume
*/
/*****************************************************************************/
#include "X_API.h"

#if USE_MOD_API == TRUE
#include "MOD_mikmod.h"
/*

Name:
MMIO.C

Description:
Miscellaneous I/O routines.. used to solve some portability issues
(like big/little endian machines and word alignment in structures )
Also includes mikmod's ingenious error handling variable + some much
used error strings.

Portability:
All systems - all compilers

*/

/*
const char *ERROR_ALLOC_STRUCT="Error allocating structure";
const char *ERROR_LOADING_PATTERN="Error loading pattern";
const char *ERROR_LOADING_TRACK="Error loading track";
const char *ERROR_LOADING_HEADER="Error loading header";
const char *ERROR_NOT_A_MODULE="Unknown module format";
const char *ERROR_LOADING_SAMPLEINFO="Error loading sampleinfo";
const char *ERROR_OUT_OF_HANDLES="Out of sample-handles";
const char *ERROR_SAMPLE_TOO_BIG="Sample too big, out of memory";
*/
const char *ERROR_ALLOC_STRUCT="1";
const char *ERROR_LOADING_PATTERN="2";
const char *ERROR_LOADING_TRACK="3";
const char *ERROR_LOADING_HEADER="4";
const char *ERROR_NOT_A_MODULE="5";
const char *ERROR_LOADING_SAMPLEINFO="6";
const char *ERROR_OUT_OF_HANDLES="7";
const char *ERROR_SAMPLE_TOO_BIG="8";



const char *gModPlayerErrorMessage;

int _mm_fseek(long offset, int whence)
{
	if (whence != SEEK_SET)
		offset += modpos;
	modpos = offset;
	return (modpos <= modsize);
}

long _mm_ftell()
{
	return modpos;
}

SBYTE _mm_read_SBYTE(void)
{
	modpos++;
	return (SBYTE) modptr[modpos-1];
}

UBYTE _mm_read_UBYTE(void)
{
	modpos++;
	return (UBYTE) modptr[modpos-1];
}

UWORD _mm_read_M_UWORD(void)
{
	UWORD result=((UWORD)_mm_read_UBYTE())<<8;
	result|=_mm_read_UBYTE();
	return result;
}

UWORD _mm_read_I_UWORD(void)
{
	UWORD result=_mm_read_UBYTE();
	result|=((UWORD)_mm_read_UBYTE())<<8;
	return result;
}

SWORD _mm_read_M_SWORD()
{
	return((SWORD)_mm_read_M_UWORD());
}

SWORD _mm_read_I_SWORD()
{
	return((SWORD)_mm_read_I_UWORD());
}

ULONG _mm_read_M_ULONG()
{
	ULONG result=((ULONG)_mm_read_M_UWORD())<<16L;
	result|=_mm_read_M_UWORD();
	return result;
}

ULONG _mm_read_I_ULONG()
{
	ULONG result=_mm_read_I_UWORD();
	result|=((ULONG)_mm_read_I_UWORD())<<16L;
	return result;
}

SLONG _mm_read_M_SLONG()
{
	return((SLONG)_mm_read_M_ULONG());
}

SLONG _mm_read_I_SLONG()
{
	return((SLONG)_mm_read_I_ULONG());
}


int _mm_read_str(char *buffer,int number)
{
	long i;
	for (i = 0; i < number; i++)
		buffer[i] = (char) modptr[modpos+i];
	modpos += number;
	return (modpos <= modsize);
}


#define DEFINE_MULTIPLE_READ_FUNCTION(type_name, type)         \
int                                                            \
_mm_read_##type_name##S (type *buffer, int number)			   \
{                                                              \
	while(number>0){                                           \
		*(buffer++)=_mm_read_##type_name();					   \
		number--;											   \
	}														   \
	return (modpos <= modsize);								   \
}

DEFINE_MULTIPLE_READ_FUNCTION ( SBYTE,  SBYTE)
DEFINE_MULTIPLE_READ_FUNCTION (UBYTE, UBYTE)

DEFINE_MULTIPLE_READ_FUNCTION (M_SWORD,   SWORD)
DEFINE_MULTIPLE_READ_FUNCTION (M_UWORD, UWORD)
DEFINE_MULTIPLE_READ_FUNCTION (I_SWORD,   SWORD)
DEFINE_MULTIPLE_READ_FUNCTION (I_UWORD, UWORD)

DEFINE_MULTIPLE_READ_FUNCTION (M_SLONG,   SLONG)
DEFINE_MULTIPLE_READ_FUNCTION (M_ULONG, ULONG)
DEFINE_MULTIPLE_READ_FUNCTION (I_SLONG,   SLONG)
DEFINE_MULTIPLE_READ_FUNCTION (I_ULONG, ULONG)

#endif	// USE_MOD_API

