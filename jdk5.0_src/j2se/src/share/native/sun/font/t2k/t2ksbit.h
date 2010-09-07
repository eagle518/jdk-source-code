/*
 * @(#)t2ksbit.h	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * T2KSBIT.H
 * Copyright (C) 1989-1998 all rights reserved by Type Solutions, Inc. Plaistow, NH, USA.
 * http://www.typesolutions.com/
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
#ifndef __T2K_SBIT__
#define __T2K_SBIT__
#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

#ifndef __T2K_TTCOMMON__
    #include "ttCommon.h"
#endif

#ifdef ENABLE_SBIT
/*
 * The fields are 2bytes wide, instead of 1 byte
 * since this allows us to store large rescaled values here. 
 * Otherwise bitmap scaling might overflow these fields.
 * In the font data these fields are stored in single bytes,
 * This is just our internal represenation of that, which
 * can on demand be rescaled to larger sizes.
 */

typedef struct {
	tt_uint16	height;
	tt_uint16	width;
	tt_int16	horiBearingX;
	tt_int16	horiBearingY;
	tt_uint16	horiAdvance;
	tt_int16	vertBearingX;
	tt_int16	vertBearingY;
	tt_uint16	vertAdvance;
} bigGlyphMetricsT2K;


#if 0

  /* This in ttCommon.h */
typedef struct {
	tt_uint16 indexFormat;		/* Format of this inexSubTable. */
	tt_uint16 imageFormat;		/* Format of EBDT/bdat image data. */
	tt_uint32 imageDataOffset;	/* Offset to image data in the EBDT/bdat table. */
} indexSubHeader;


typedef struct {
	tt_uint16 firstGlyphIndex;						/* First glyph index in this range. */
	tt_uint16 lastGlyphIndex;						/* Last glyph index in this range (inclusive). */
	tt_uint32 additionalOffsetToIndexSubTable;		/* Add to indexSubTableArryOffset to get offset from beginning of EBLC/bloc table. */ 
} indexSubTableArray;
#endif

/* for the hori and vert arrays in bitmapSizeTable */
#define NUM_SBIT_METRICS_BYTES 12
/* for the flags field in bitmapSizeTable */
#define SBIT_SMALL_METRIC_DIRECTION_IS_HORIZONTAL	0x01
#define SBIT_SMALL_METRIC_DIRECTION_IS_VERTICAL		0x02

/* One bitmapSizeTableT2K for each strike */
typedef struct {
	/* private */
	tsiMemObject *mem;
	/* public */
	tt_uint32 indexSubTableArrayOffset;		/* Offset to index subtable from beginning of EBLC/bloc table */
	tt_uint32 indexTableSize;					/* Total size of IndexSubTables and array */
	tt_uint32 numberOfIndexSubTables;			/* Number of IndexSubTables */
	tt_uint32 colorRef;						/* Not used, should be 0 */
	
	tt_uint8 hori[NUM_SBIT_METRICS_BYTES];		/* Line metrics for horizontally rendered text */
	tt_uint8 vert[NUM_SBIT_METRICS_BYTES];		/* Line metrics for vertically  rendered text */
	
	tt_uint16	startGlyphIndex;				/* First glyph Index for this strike. */
	tt_uint16	endGlyphIndex;					/* Last glyph Index for this strike. */
	tt_uint8	ppemX;							/* Horizontal pixels per Em */
	tt_uint8	ppemY;							/* Vertical pixels per Em */
	tt_uint8	bitDepth;						/* 1 for monochrome */
	tt_uint8	flags;							/* Horizontal or Vertical small metrics */
	
	indexSubTableArray *table;  			/* indexSubTableArray table[ numberOfIndexSubTables ] */
} bitmapSizeTableT2K;

typedef struct {
	/* For caching results of GlyphExists() */
	tt_uint32 offsetA, offsetB;					/* Offset to the first byte, and the offset to one pst the last byte. */
	tt_uint16 glyphIndex;							/* The glyph index. */
	tt_uint16 ppemX, ppemY;						/* Requested size. */
	tt_uint16 substitutePpemX, substitutePpemY;	/* Use bitmap of this size instead with scaling, if different from ppemX and ppemY.  */
	tt_uint8	bitDepth;							/* Bit depth, 1 for monochrome data. */
	tt_uint8	flags;								/* Flags that with bitflags for hor. & ver. directional info. */
	tt_uint16	imageFormat;						/* Bitmap image format. */
	bigGlyphMetricsT2K bigM;						/* Combined metrics for both small and larlge metrics types. */
	int		smallMetricsUsed;					/* If true then it implies that we either have only horizontal or vertical metrics. */
	
	/* bitmap data */
	tt_int32 rowBytes;								/* Number of bytes per row. */
	tt_uint8 *baseAddr;							/* The address of the bitmap. */
} sbitGlypInfoData;

/* Microsoft calls this table EBLC, Apple calls this table bloc */
typedef struct {
	/* private */
	tsiMemObject *mem;
	tt_uint32	startOffset;
	int fontIsSbitOnly;
	
	/* glyph Info*/
	sbitGlypInfoData gInfo;
	
	/* public */
	F16Dot16	version; /* Initially set to 0x20000 */
	tt_uint32		nTables; /* number of strikes */
	
	bitmapSizeTableT2K **table; /* bitmapSizeTable *table[ nTables ] */
} blocClass;


typedef struct {
	tt_uint8 hori[NUM_SBIT_METRICS_BYTES];		/* Line metrics for horizontally rendered text */
	tt_uint8 vert[NUM_SBIT_METRICS_BYTES];		/* Line metrics for vertically  rendered text */
	tt_uint8 ppemX;							/* Target horizantal pixels per Em. */
	tt_uint8 ppemY;							/* Target vertical pixels per Em. */
	tt_uint8 substitutePpemX;					/* Use a bitmap of this size before scaling in the x direction. */
	tt_uint8 substitutePpemY;					/* Use a bitmap of this size before scaling in the y direction. */
} bitmapScaleEntry;

/* Embedded bitmap scaling table */
typedef struct {
	/* private */
	tsiMemObject *mem;
	tt_uint32	startOffset;
	
	/* public */
	F16Dot16	version; 	/* Initially set to 0x20000 */
	tt_uint32		numSizes;	/* number of sizes */
	
	bitmapScaleEntry *table; /* bitmapScaleEntry table[ nTables ] */
} ebscClass;

/*
 * The ebscClass class contructor (EBSC table)
 */
ebscClass *New_ebscClass( tsiMemObject *mem, InputStream *in );

/*
 * The ebscClass class destructor (EBSC table)
 */
void Delete_ebscClass( ebscClass *t );


/*
 * The blocClass class contructor (EBLC/bloc table)
 */
blocClass *New_blocClass( tsiMemObject *mem, int fontIsSbitOnly, InputStream *in );
/*
 * The blocClass class destructor (EBLC/bloc table)
 */
void Delete_blocClass( blocClass *t );


/*
 * Returns scaled font wide sbit metrics
 */
void GetFontWideSbitMetrics( blocClass *bloc, ebscClass *ebsc, tt_uint16 ppemX, tt_uint16 ppemY,
							T2K_FontWideMetrics *hori, T2K_FontWideMetrics *vert );

/*
 * Returns true if the glyph exists, and false otherwise
 * Caches some internal results so that we can get to the bits faster when we need them next.
 */
int FindGlyph_blocClass( blocClass *t, ebscClass *ebsc, InputStream *in, tt_uint16 glyphIndex, tt_uint16 ppemX, tt_uint16 ppemY, sbitGlypInfoData *result );

/* This is here so that bad data can not cause an infinite recursion crash */
#define MAX_SBIT_RECURSION_DEPTH 16
/*
 * Gets the bits.
 */
void ExtractBitMap_blocClass( blocClass *t, ebscClass *ebsc, sbitGlypInfoData *gInfo, InputStream *in, tt_uint32 bdatOffset, tt_uint8 greyScaleLevel, int recursionLevel );
 /* ENABLE_SBIT */
#endif

#ifdef __cplusplus
}
  /* __cplusplus */
#endif
 /* __T2K_SBIT__ */
#endif
