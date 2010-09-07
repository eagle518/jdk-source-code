/*
 * @(#)ttCommon.h	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 /* graphics:	
	TrueType structures
	by		Cary Clark, Georgiann Delaney, Michael Fairman, Dave Good,
			Barton House, Robert Johnson, Keith McGreggor, Mike Reed,
			Oliver Steele, David Van Brink, Chris Yerga, Ian Ritchie
			
	Copyright ©1987-1993 Apple Computer, Inc.  All rights reserved.

 */

#ifndef __T2K_TTCOMMON__
#define __T2K_TTCOMMON__   /* Note: from both TruetypeTypes.h and glyph.h (T2K) */


typedef enum componentPacking {	
  COMPONENTCTRCOUNT = -1,	
  ARG_1_AND_2_ARE_WORDS = 1,		/* if not, they are bytes */	
  ARGS_ARE_XY_VALUES = 2,		/* if not, they are points */	
  ROUND_XY_TO_GRID = 4,	
  WE_HAVE_A_SCALE = 8,			/* if not, Sx = Sy = 1.0 */	
  NON_OVERLAPPING = 16,	
  MORE_COMPONENTS = 32,			/* if not, this is the last one */	
  WE_HAVE_AN_X_AND_Y_SCALE = 64,	/* Sx != Sy */
  WE_HAVE_A_TWO_BY_TWO = 128,		/* t00, t01, t10, t11 */
  WE_HAVE_INSTRUCTIONS = 256,		/* short count followed by instructions follows */	
	USE_MY_METRICS = 512,				/* use my metrics for parent glyph */
	OVERLAP_COMPOUND = 1024			/* this compound glyphs has overlapping contours between or within components */
} componentPacking;


/* Note: from both TruetypeTypes.h and glyph.h (T2K) */
typedef struct {
		tt_uint16 firstGlyphIndex;						/* First glyph index in this range. */
		tt_uint16 lastGlyphIndex;						/* Last glyph index in this range (inclusive). */
		tt_uint32 additionalOffsetToIndexSubTable;		/* Add to indexSubTableArryOffset to get offset from beginning of EBLC/bloc table. */ 
	} indexSubTableArray;


/* Note: from both TruetypeTypes.h and glyph.h (T2K) */
typedef struct {       
  unsigned short	indexFormat;
  unsigned short	imageFormat;
  unsigned long	imageDataOffset;    /*offset to corresponding image data from beginning of bdat table*/
} indexSubHeader;

#endif 
	/* __T2K_TTCOMMON__ */
