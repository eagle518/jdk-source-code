/*
 * @(#)truetype.h	1.19 04/01/16
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * TRUETYPE.H
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
#ifndef __T2K_TRUETYPE__
#define __T2K_TRUETYPE__
#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

/*
 * T2K internal font wide metric data type. used here and in T2KSBIT.H
 */
typedef struct {
	int			isValid;			
	tt_int16		Ascender;			
	tt_int16		Descender;			
	tt_int16		LineGap;			
	tt_uint16		maxAW;	
	F16Dot16	caretDx, caretDy;
} T2K_FontWideMetrics;

#include "tt_prvt.h"
#include "t2ksbit.h"
#include "glyph.h"
#ifdef ALGORITHMIC_STYLES
#include "shapet.h"
#endif
#include "t1.h"
#ifdef ENABLE_T2KE
#include "T2KEXEC.H"
#endif

#ifdef ENABLE_TT_HINTING
/* The StyleFunc for hinted glyphs operates on the finished glyph; typically for italic and bold*/
/* or some combination- see tsi_SHAPET_BoldItalic_GLYPH_Hinted in shapet.c 			*/
/*  Note, because it is a finished glyph (the entire compound glyph must be finished), 	the x and y	*/
/* arrays (unhinted data) are no longer needed, and may be used for temporary results-	    */
/*		This could obviate the need to call the memory allocation routine.    	*/
	
	
	 	
	
typedef void (*StyleFuncPostPtr)( 
    short	contourCount,	/* number of contours in the character */
    short 	pointCount,	/* number of points in the characters + 0 for the sidebearing points */
    
    short	*sp,			/* sp[contourCount] Start points */
    short	*ep,  			/* ep[contourCount] End points */
    F26Dot6 *xx,			/* the x array, with phantom */
    F26Dot6 *yy,			/* the y array, with phantom */
    F26Dot6 *x,				/* the x array, doesn't need phantom (for temp use)*/
    F26Dot6 *y,				/* the y array, doesn't need  phantom (for temp use) */
    tsiMemObject *mem, 			 
    F16Dot16 xPixelsPerEmFixed, F16Dot16 yPixelsPerEm16Dot16,
    /*  ORIENTBOLD_STYLES */
    short curveType,			/* 2 (quadratic) or 3(cubic) */
    tt_uint8 *onCurve,			/* onCurve[pointCount] indicates if a point is on or off
	  				 the curve, it should be true or false */
    ContourData *cd,
    F16Dot16  params[ ]   );
#endif
/* The style Func for the original T2K styling operates on the raw glyph. */
typedef void (*StyleFuncPtr)( GlyphClass *glyph, tsiMemObject *mem, short UPEM, 
				  /*  ORIENTBOLD_STYLES */ ContourData *cd,
				  F16Dot16 params[] );


typedef tt_int32 (*StyleMetricsFuncPtr)( hmtxClass *hmtx, tsiMemObject *mem, short UPEM, F16Dot16 params[] );
#define MAX_STYLE_PARAMS 4

typedef struct {
	/* private */
	sfnt_OffsetTable *offsetTable0;

#ifdef ENABLE_T1
	T1Class *T1;
#endif
#ifdef ENABLE_CFF
	CFFClass *T2;
#endif
#ifdef ENABLE_T2KE
	T2KEClass *T2KE;
#endif

#ifdef ENABLE_SBIT
	blocClass *bloc;
	ebscClass *ebsc;
	tt_uint32 bdatOffset;
#endif
#ifdef ENABLE_TT_HINTING
	fpgmClass	*fpgm;
	cvtClass	*cvt;
	prepClass	*prep;
#endif

	ttcfClass *ttcf;
	
	headClass *head;
	maxpClass *maxp;
	locaClass *loca;
	hheaClass *hhea;
	hheaClass *vhea;
	hmtxClass *hmtx;
	hmtxClass *hmtxPlain;
	hmtxClass *hmtxBold;
	hmtxClass *hmtxItalic;
	hmtxClass *hmtxBoldItalic;
	cmapClass *cmap;
	kernClass *kern;
	tt_uint16 preferedPlatformID, preferedPlatformSpecificID;
	
	void *globalHintsCache;
	
	/* For the next 4 values, see T2K_AlgStyleDescriptor, below */
        /* StyleFuncPtr StyleFunc; - no longer used */
#ifdef ENABLE_TT_HINTING
	StyleFuncPostPtr StyleFuncPost;
#endif
	StyleMetricsFuncPtr StyleMetricsFunc;
	F16Dot16 params[MAX_STYLE_PARAMS];
	tt_int32 hmtxLinearAdjustment; /* Must be set if the metrics function changes linear metrics. */
	tt_int32 hmtxLinearAdjustmentBold;
	tt_int32 hmtxLinearAdjustmentItalic;
	tt_int32 hmtxLinearAdjustmentBoldItalic;
	
	
	InputStream *in;
	OutputStream *out;
	tsiMemObject *mem;
	
	/* OrionModelClass *model; */
	void *model;
	/* public */
	
} sfntClass;

hmtxClass *New_hmtxEmptyClass( tsiMemObject *mem, tt_int32 numGlyphs, tt_int32 numberOfHMetrics );
void Delete_hmtxClass( hmtxClass *t );

/* Some useful getter methods */
short GetUPEM( sfntClass *t);
short GetMaxPoints( sfntClass *t);

void GetFontWideOutlineMetrics( sfntClass *font, T2K_FontWideMetrics *hori, T2K_FontWideMetrics *vert );

/* The T2K_AlgStyleDescriptor provides the "plugin" routines for  manipulating the geometry 	*/
/*		of a glyph. 																			*/
/* The StyleMetricsFunc is used to adjust the Linear Advance, which is the theoritical advance 	*/
/*		in the absence of pixelization; it is used for "fractional" layout.						*/
/*																								*/
/*		The metrics functon,StyleMetricsFunc, is complicated because it changes the hmtx table, */
/* 			but original hmtx values are needed for hinting. Therefore, the routine returns		*/
/*			the amount of FUNIT adjustment (see hmtxLinearAdjustment, above) so that it can 	*/
/*			be used by the "hinter" to find  the original values.	The metric function is the	*/
/*			same for either T2K autogrid or TrueType Hinting.									*/	
/*																								*/
/* There is one function specification:	StyleFuncPost. 		   	*/
/* The original "StyleFunc" is no longer used.			    	*/
/* See shapet.h, shapet.c for examples of style functions. See t2k.h for general usage.	*/
  
typedef struct {
 /*	StyleFuncPtr StyleFunc; No Longer Used. */

#ifdef ENABLE_TT_HINTING
	StyleFuncPostPtr StyleFuncPost;
#endif
	StyleMetricsFuncPtr StyleMetricsFunc;
	F16Dot16 params[MAX_STYLE_PARAMS];
} T2K_AlgStyleDescriptor;

/* Caller does something like in = New_InputStream3( t->mem, data, length ); */
#define FONT_TYPE_1 1
#define FONT_TYPE_2 22
#define FONT_TYPE_TT_OR_T2K 2

#define New_sfntClass( mem, fontType, in, styling, errCode ) New_sfntClassLogical( mem, fontType, 0, in, styling, errCode )

sfntClass *New_sfntClassLogical( tsiMemObject *mem, short fontType, tt_int32 fontNum, InputStream *in, T2K_AlgStyleDescriptor *styling, int *errCode );
#define CMD_GRID 2
#define CMD_TT_TO_T2K 3
#define CMD_T2K_TO_TT 4
#define CMD_HINT_ROMAN 5
#define CMD_HINT_OTHER 6
#define CMD_TT_TO_T2KE 7

sfntClass *New_sfntClass2( sfntClass *sfnt0, int cmd, int param );

void WriteToFile_sfntClass( sfntClass *t, const char *fname );
void Purge_cmapMemory( sfntClass *t );

#define tag_T2KG        		0x54324B47        /* 'T2KG' */
#define tag_T2KC        		0x54324B43        /* 'T2KC' */
sfnt_DirectoryEntry *GetTableDirEntry_sfntClass( sfntClass *t, tt_int32 tag );
/* caller need to do Delete_InputStream on the stream */
InputStream *GetStreamForTable( sfntClass *t, tt_int32 tag  );

void Delete_sfntClass( sfntClass *t, int *errCode );

/* 	mem									= tsi_NewMemhandler(); */
/* 	tsi_DeleteMemhandler( mem ); */

GlyphClass *GetGlyphByIndex( sfntClass *t, tt_int32 index, char readHints, tt_uint16 *aw );
GlyphClass *GetGlyphByCharCode( sfntClass *t, tt_int32 charCode, char readHints, tt_uint16 *aw );

int IsFigure( sfntClass *t, tt_uint16 gIndex );

#ifdef ENABLE_KERNING
void GetSfntClassKernValue( sfntClass *t, tt_uint16 leftGIndex, tt_uint16 rightGIndex, tt_int16 *xKern, tt_int16 *yKern );
#endif /* ENABLE_KERNING */

tt_uint16 GetSfntClassGlyphIndex( sfntClass *t, tt_uint32 charCode );


tt_int32 GetNumGlyphs_sfntClass( sfntClass *t );

void t2k_SetHmtx(sfntClass *t);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __T2K_TRUETYPE__ */
