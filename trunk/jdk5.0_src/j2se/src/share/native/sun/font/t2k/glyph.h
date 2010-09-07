/*
 * @(#)glyph.h	1.18 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * GLYPH.H
 * Copyright (C) 1989-1998 all rights reserved by Type Solutions, Inc. Plaistow, NH, USA.
 * Author: Sampo Kaasila
 *
 * This software is the property of Type Solutions, Inc. and it is furnished
 * under a license and may be used and copied only in accordance with the
 * terms of such license and with the inclusion of the above copyright notice.
 * This software or any other copies thereof may not be provided or otherwise
 * made available to any other person or entity except as allowed under license.
 * No title to and ownership of the software or intellectual property
 * therewithin is hereby transferred.
 *
 * This information in this software is subject to change without notice
 */
#ifndef __T2K_GLYPH__
#define __T2K_GLYPH__
#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

#ifndef __T2K_TTCOMMON__
	#include "ttCommon.h"
#endif



#ifdef ENABLE_T2KE
/* We need the tsiColorDescriptor structure */
#include "t2ksc.h"
#include "t2kclrsc.h"
#endif

/* private constants */
#define BASE0 0
#define BASE1 2200
#define BASE2 12604
#define BASE3 14652
#define BASEMAX 16384


/*
 * Composite glyph constants
 */
#if 0
  /* This stuff is in TTCOMMON.h */

#define COMPONENTCTRCOUNT 			-1		/* ctrCount == -1 for composite */
#define ARG_1_AND_2_ARE_WORDS		0x0001	/* if set args are words otherwise they are bytes */
#define ARGS_ARE_XY_VALUES			0x0002	/* if set args are xy values, otherwise they are points */
#define ROUND_XY_TO_GRID			0x0004	/* for the xy values if above is true */
#define WE_HAVE_A_SCALE				0x0008	/* Sx = Sy, otherwise scale == 1.0 */
#define NON_OVERLAPPING				0x0010	/* set to same value for all components */
#define MORE_COMPONENTS				0x0020	/* indicates at least one more glyph after this one */
#define WE_HAVE_AN_X_AND_Y_SCALE	0x0040	/* Sx, Sy */
#define WE_HAVE_A_TWO_BY_TWO		0x0080	/* t00, t01, t10, t11 */
#define WE_HAVE_INSTRUCTIONS		0x0100	/* instructions follow */
#define USE_MY_METRICS				0x0200	/* */

#endif


typedef struct {
	/* private */
	tsiMemObject *mem;
	
	short curveType;			/* 2 or 3 */
	short contourCountMax;
	tt_int32 pointCountMax;
	
	tt_int32 colorPlaneCount;
	tt_int32 colorPlaneCountMax;
#ifdef ENABLE_T2KE
	tsiColorDescriptor *colors;
    /* tsiColorDescriptor colors[100]; */ /* hack for now */
#endif
	tt_uint16 gIndex; /* Glyph Index, just for editing purposes */
	
	/* public */
	short	contourCount;	/* number of contours in the character */
	short 	pointCount;		/* number of points in the characters + 0 for the sidebearing points */
	short	*sp;			/* sp[contourCount] Start points */
	short	*ep;  			/* ep[contourCount] End points */
	short	*oox;			/* oox[pointCount] Unscaled Unhinted Points, add two extra points for lsb, and rsb */
	short	*ooy;			/* ooy[pointCount] Unscaled Unhinted Points, set y to zero for the two extra points */
							/* Do NOT include the two extra points in sp[], ep[], contourCount */
							/* Do NOT include the two extra points in pointCount */
	tt_uint8 *onCurve;			/* onCurve[pointCount] indicates if a point is on or off the curve, it should be true or false */

	F26Dot6 *x, *y;
	
	short *componentData;
	tt_int32  componentSize;
	tt_int32  componentSizeMax;
	
	tt_uint8 *hintFragment;
	tt_int32 hintLength;
	
	short	xmin, ymin, xmax, ymax;

} GlyphClass;

GlyphClass *New_EmptyGlyph( tsiMemObject *mem, tt_int16 lsb, tt_uint16 aw );

tt_int32 Write_GlyphClassT2K( GlyphClass *glyph, OutputStream *out, void *model );
GlyphClass *New_GlyphClassT2K( tsiMemObject *mem, InputStream *in, char readHints, tt_int16 lsb, tt_uint16 aw, void *model );

GlyphClass *New_GlyphClassT2KE( void *t, register InputStream *in, tt_int32 byteCount, tt_int16 lsb, tt_uint16 aw );
tt_int32 Write_GlyphClassT2KE( GlyphClass *glyph, OutputStream *out );


void TEST_T2K_GLYPH( tsiMemObject *mem );

#ifdef T1_OR_T2_IS_ENABLED
void glyph_CloseContour( GlyphClass *t );
void glyph_AddPoint( GlyphClass *t, tt_int32 x, tt_int32 y, char onCurveBit );
void glyph_StartLine( GlyphClass *t, tt_int32 x, tt_int32 y );
#endif /* T1_OR_T2_IS_ENABLED */

#ifdef ENABLE_T2KE
void glyph_CloseColorContour( GlyphClass *glyph, tsiColorDescriptor *color );
#endif

#ifdef ENABLE_PRINTF
void glyph_PrintPoints( GlyphClass *t );
#endif

#ifdef ENABLE_WRITE
void WriteDeltaXYValue( OutputStream *out, int dx, int dy, char onCurve );
#endif
int ReadDeltaXYValue( InputStream *in, short *dxPtr, short *dyPtr );
#ifdef ENABLE_ORION
int ReadOrionDeltaXYValue( InputStream *in, void *model, short *dxPtr, short *dyPtr );
#endif


void Add_GlyphClass( GlyphClass **tPtr, GlyphClass *addMe, tt_uint16 flags, tt_int32 arg1, tt_int32 arg2, tt_int32 rarg1, tt_int32 rarg2, T2K_TRANS_MATRIX transform);

void Delete_GlyphClass( GlyphClass *t );
void ReverseContourDirectionDirect(	
	tt_int32	contourCount,	
	tt_int16	*sp,	/* sp[contourCount] Start points */
	tt_int16	*ep,	/* ep[contourCount] End points */
	F26Dot6 *x,
	F26Dot6 *y,
	tt_uint8 	*onCurve );
void ReverseContourDirection(GlyphClass *glyph);
void FlipContourDirectionShort(GlyphClass *glyph);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __T2K_GLYPH__ */

