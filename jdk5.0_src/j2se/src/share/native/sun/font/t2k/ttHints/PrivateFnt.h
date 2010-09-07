/*
 * @(#)PrivateFnt.h	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 /*
	Copyright ©1987-1993 Apple Computer, Inc.  All rights reserved.
*/
#ifndef privateFntIncludes
#define privateFntIncludes

#include "Fnt.h"

#define POP( p )     ( *(--p) )
#define PUSH( p, x ) ( *(p)++ = (x) )

#define BADCOMPILER

#ifdef BADCOMPILER
#define BOOLEANPUSH( p, x ) PUSH( p, ((x) ? 1 : 0) ) /* MPW 3.0 */
#else
#define BOOLEANPUSH( p, x ) PUSH( p, x )
#endif

#define MAX(a,b)	((a) > (b) ? (a) : (b))

#define bitcount(a, count) \
{ \
	count = 0; \
	while (a) \
	{ \
		a >>= 1; \
		count++; \
	} \
}

#define GETBYTE(ptr)	( (tt_uint8)*ptr++ )
#define MABS(x)			( (x) < 0 ? (-(x)) : (x) )

#define BIT0( t ) ( (t) & 0x01 )
#define BIT1( t ) ( (t) & 0x02 )
#define BIT2( t ) ( (t) & 0x04 )
#define BIT3( t ) ( (t) & 0x08 )
#define BIT4( t ) ( (t) & 0x10 )
#define BIT5( t ) ( (t) & 0x20 )
#define BIT6( t ) ( (t) & 0x40 )
#define BIT7( t ) ( (t) & 0x80 )

/******** 12 BinaryOperators **********/
#define LT_CODE		0x50
#define LTEQ_CODE	0x51
#define GT_CODE		0x52
#define GTEQ_CODE	0x53
#define EQ_CODE		0x54
#define NEQ_CODE	0x55
#define AND_CODE	0x5A
#define OR_CODE		0x5B
#define ADD_CODE	0x60
#define SUB_CODE	0x61
#define DIV_CODE	0x62
#define MUL_CODE	0x63
#define MAX_CODE	0x8b
#define MIN_CODE	0x8c

/******** 9 UnaryOperators **********/
#define ODD_CODE		0x56
#define EVEN_CODE		0x57
#define NOT_CODE		0x5C
#define ABS_CODE		0x64
#define NEG_CODE		0x65
#define FLOOR_CODE		0x66
#define CEILING_CODE	0x67

/******** 6 RoundState Codes **********/
#define RTG_CODE		0x18
#define RTHG_CODE		0x19
#define RTDG_CODE		0x3D
#define ROFF_CODE		0x7A
#define RUTG_CODE		0x7C
#define RDTG_CODE		0x7D

/****** LocalGS Codes *********/
#define POP_CODE	0x21
#define SRP0_CODE	0x10
#define SRP1_CODE	0x11
#define SRP2_CODE	0x12
#define SLOOP_CODE	0x17
#define LMD_CODE	0x1A

/****** Element Codes *********/
#define SCE0_CODE	0x13
#define SCE1_CODE	0x14
#define SCE2_CODE	0x15
#define SCES_CODE	0x16

/****** Control Codes *********/
#define IF_CODE		0x58
#define ELSE_CODE	0x1B
#define EIF_CODE	0x59
#define ENDF_CODE	0x2d
#define MD_CODE		0x49

/******* Push Codes ********/
#define NPUSHB_CODE 0x40
#define NPUSHW_CODE 0x41
#define PUSHB_START 0xb0
#define PUSHB_END 	0xb7
#define PUSHW_START 0xb8
#define PUSHW_END 	0xbf

/* flags for UTP, IUP, MovePoint */
#define XMOVED 0x01
#define YMOVED 0x02

/* <13> defines for kanji adjust from here*/
#define ADJUSTBASE 0x8F							/* <15> base opcode for adjust instruction */
#define PIXEL 64
#define LG2PIXEL 6
#define HALFPIXEL (PIXEL >> 1)
#define HALFPIXELM (HALFPIXEL-1)
#define HALFPIXELP (HALFPIXEL+1)
	/* MTE: was named "floor" which conflicts with standard library "c" usage. */
#define fntfloor( x ) ((x) & ~0x3F )

typedef F26Dot6 pixel;
typedef F26Dot6 subPixel;

void PostInterpreterError(fnt_LocalGraphicStateType *gs, tt_int32 error);
void FatalInterpreterError(fnt_LocalGraphicStateType *gs, tt_int32 error);

void fnt_IllegalInstruction(fnt_LocalGraphicStateType *gs);
void fnt_Normalize(fnt_LocalGraphicStateType*, F26Dot6 x, F26Dot6 y, shortVector*);
void fnt_SkipPushCrap(register fnt_LocalGraphicStateType *gs);
void fnt_MovePoint(fnt_LocalGraphicStateType *gs, fnt_ElementType *element, ArrayIndex gxPoint, F26Dot6 delta);
void fnt_MoveAPoint( fnt_LocalGraphicStateType* gs, F26Dot6* x, F26Dot6* y, F26Dot6 delta);
void fnt_XMovePoint(fnt_LocalGraphicStateType *, fnt_ElementType *element, ArrayIndex gxPoint, F26Dot6 delta) ;
void fnt_YMovePoint(fnt_LocalGraphicStateType *, fnt_ElementType *element, ArrayIndex gxPoint, F26Dot6 delta) ;
F26Dot6 fnt_Project(fnt_LocalGraphicStateType *gs, F26Dot6 x, F26Dot6 y);
F26Dot6 fnt_OldProject(fnt_LocalGraphicStateType *gs, F26Dot6 x, F26Dot6 y);
F26Dot6 fnt_XProject(fnt_LocalGraphicStateType *gs, F26Dot6 x, F26Dot6 y);
F26Dot6 fnt_YProject(fnt_LocalGraphicStateType *gs, F26Dot6 x, F26Dot6 y);
fixed fnt_GetCVTScale(fnt_LocalGraphicStateType *gs);
F26Dot6 fnt_GetCVTEntryFast(fnt_LocalGraphicStateType *gs, ArrayIndex n);
F26Dot6 fnt_GetCVTEntrySlow(fnt_LocalGraphicStateType *gs, ArrayIndex n);
F26Dot6 fnt_GetSingleWidthFast(fnt_LocalGraphicStateType *gs);
F26Dot6 fnt_GetSingleWidthSlow(fnt_LocalGraphicStateType *gs);
void fnt_ChangeCvt(fnt_LocalGraphicStateType *gs, fnt_ElementType *element, ArrayIndex number, F26Dot6 delta);
void fnt_Check_PF_Proj(fnt_LocalGraphicStateType *gs);
void fnt_ComputeAndCheck_PF_Proj(register fnt_LocalGraphicStateType *gs);

void fnt_InnerTraceExecute(register fnt_LocalGraphicStateType *gs, tt_uint8 *ptr, register tt_uint8 *eptr);
void fnt_InnerExecute(register fnt_LocalGraphicStateType *gs, tt_uint8 *ptr, tt_uint8 *eptr);
F26Dot6* GrowStackForPush(fnt_LocalGraphicStateType* gs, tt_int32 count);

#endif
