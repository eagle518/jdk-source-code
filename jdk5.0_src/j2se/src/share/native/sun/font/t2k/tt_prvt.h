/*
 * @(#)tt_prvt.h	1.18 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * TT_PRVT.H
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
 /* Private TrueType structures */
#ifndef __T2K_TT_PRVT__
#define __T2K_TT_PRVT__
#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

#define tag_FontHeader          0x68656164        /* 'head' */
#define tag_BFontHeader       	0x62686564        /* 'bhed', is used by Apple instead of 'head' for bitmap only fonts */
												  
#define tag_HoriHeader          0x68686561        /* 'hhea' */
#define tag_VertHeader          0x76686561        /* 'vhea' */
#define tag_IndexToLoc          0x6c6f6361        /* 'loca' */
#define tag_MaxProfile          0x6d617870        /* 'maxp' */
#define tag_ControlValue        0x63767420        /* 'cvt ' */
#define tag_MetricValue         0x6d767420        /* 'mvt ' */
#define tag_PreProgram          0x70726570        /* 'prep' */
#define tag_GlyphData           0x676c7966        /* 'glyf' */
#define tag_HorizontalMetrics   0x686d7478        /* 'hmtx' */
#define tag_CharToIndexMap      0x636d6170        /* 'cmap' */
#define tag_Kerning             0x6b65726e        /* 'kern' */
#define tag_HoriDeviceMetrics   0x68646d78        /* 'hdmx' */
#define tag_Encryption          0x63727970        /* 'cryp' */
#define tag_NamingTable         0x6e616d65        /* 'name' */
#define tag_FontProgram         0x6670676d        /* 'fpgm' */
#define tag_VDMX        		0x56444D58        /* 'VDMX' */
#define tag_LTSH        		0x4C545348        /* 'LTSH' */
#define tag_fvar        		0x66766172        /* 'fvar' */


#define tag_fvar        		0x66766172        /* 'fvar' */

#define tag_FontVariation		0x66766172			/* 'fvar' */
#define tag_GlyphVariation		0x67766172			/* 'gvar' */
#define tag_CVTVariation		0x63766172			/* 'cvar' */

#define tag_TTCollectionID		0x74746366			/* 'ttcf' */


#define tag_EBLC				0x45424C43			/* 'EBLC' */
#define tag_bloc				0x626c6f63			/* 'bloc' */

#define tag_EBDT				0x45424454			/* 'EBDT' */
#define tag_bdat				0x62646174			/* 'bdat' */

#define tag_EBSC				0x45425343			/* 'EBSC' */

typedef struct {
	tt_uint32	leftRightIndex; /* leftIndex << 16 || rightIndex */
	tt_int16	value; 
} kernPair0Struct;

typedef struct {
	/* private */
	tsiMemObject *mem;
	/* public */
	tt_uint16 nPairs;
	tt_uint16 searchRange;
	tt_uint16 entrySelector;
	tt_uint16 rangeShift;
	kernPair0Struct *pairs;
} kernSubTable0Data;

typedef struct {
	/* private */
	tsiMemObject *mem;
	/* public */
	tt_uint16	version;
	/* tt_uint16	length; OLD */
	tt_int32 	length;
	tt_uint16	coverage;
	/* kernSubTable0Data *data; */
	void *data;
} kernSubTable;

typedef struct {
	/* private */
	tsiMemObject *mem;
	
	/* public */
	tt_uint16	version;
	tt_int32	nTables;
	
	kernSubTable **table; /* kernSubTable *table[] */
} kernClass;

typedef struct {
	/* private */
	tsiMemObject *mem;
	
	/* public */
	tt_int32	tag;
	tt_int32	checkSum;
        tt_int32	offset;
	tt_int32	length;
} sfnt_DirectoryEntry;

typedef struct {
	/* private */
	tsiMemObject *mem;
	
	
	/* public */
	int		version;			/* tt_int32  : 0x10000 (1.0)					*/
	short	numOffsets;			/* tt_uint16 : number of tables				*/
	short	searchRange;		/* tt_uint16 : (max2 <= numOffsets)*16			*/
	short	entrySelector;		/* tt_uint16 : log2(max2 <= numOffsets)		*/
	short	rangeShift;			/* tt_uint16 : numOffsets*16-searchRange		*/
	sfnt_DirectoryEntry **table;	/* sfnt_DirectoryEntry : *table[numOffsets] 	*/
} sfnt_OffsetTable;


/* --- */
typedef struct {
	tt_uint16	platformID;
	tt_uint16	specificID;
	tt_uint32	offset;
} sfnt_platformEntry;

typedef struct {
	/* private */
	tsiMemObject *mem;
	
	tt_int16 version;
	tt_int16 numEncodingTables;
	
	sfnt_platformEntry **platform; /* *entries[numEncodingTables] */
	tt_uint8 *cmapData;
	tt_int32 length;
	
	tt_int16 preferedEncodingTable;
	tt_uint16 preferedFormat;

#define NUM_FIGURES 10	
	tt_uint16 figIndex[NUM_FIGURES];
	
	/* public */
} cmapClass;
/* --- */

typedef struct {
	/* private */
	tsiMemObject *mem;
	tt_uint32 version;
	/* public */
	tt_uint32 directoryCount;
	tt_uint32 *tableOffsets;
} ttcfClass;
/* --- */

typedef struct {
	/* private */
	tsiMemObject *mem;
	
	
	/* public */
    tt_int32		version;			/* for this table, set to 1.0 */
    tt_int32		fontRevision;		/* For Font Manufacturer */
	tt_int32		checkSumAdjustment;
	tt_int32		magicNumber; 		/* signature, should always be 0x5F0F3CF5  == MAGIC */
	tt_int16		flags;
	tt_int16		unitsPerEm;			/* Specifies how many in Font Units we have per EM */

	tt_int32		created_bc;
	tt_int32		created_ad;
	tt_int32		modified_bc;
	tt_int32		modified_ad;

	/** This is the font wide bounding box in ideal space
	(baselines and metrics are NOT worked into these numbers) **/
	tt_int16		xMin;
	tt_int16		yMin;
	tt_int16		xMax;
	tt_int16		yMax;

	tt_int16		macStyle;				/* Macintosh style word */
	tt_int16		lowestRecPPEM; 			/* lowest recommended pixels per Em */

	/* 0: fully mixed directional glyphs, */
	/* 1: only strongly L->R or T->B glyphs, -1: only strongly R->L or B->T glyphs, */
	/* 2: like 1 but also contains neutrals, -2: like -1 but also contains neutrals */
	tt_int16		fontDirectionHint;

	tt_int16		indexToLocFormat;		/* 0 for short, 1 for long */
	tt_int16		glyphDataFormat;		/* 0 for current format */
} headClass;

typedef struct {
	/* private */
	tsiMemObject *mem;
	
	
	/* public */
    tt_int32		version;			/* for this table, set to 1.0 */
	tt_int16		Ascender;
	tt_int16		Descender;
	tt_int16		LineGap;
	
	tt_uint16		advanceWidthMax;
	tt_int16 		minLeftSideBearing;
	tt_int16 		minRightSideBearing;
	tt_int16		xMaxExtent;
	
	tt_int16		caretSlopeRise;
	tt_int16		caretSlopeRun;
	
	tt_int16 		caretOffset;
	tt_int16 		reserved2;
	tt_int16 		reserved3;
	tt_int16 		reserved4;
	tt_int16 		reserved5;
	
	tt_int16		metricDataFormat;
	tt_uint16		numberOfHMetrics;
} hheaClass;


typedef struct {
	/* private */
	tsiMemObject *mem;
	
	/* public */
	tt_int32 numGlyphs;
	tt_int32 numberOfHMetrics;
	tt_int16 *lsb;
	tt_uint16 *aw;
} hmtxClass;

typedef struct {
	/* private */
	tsiMemObject *mem;
	
	
	/* public */
	tt_int32	version;				/* for this table, set to 1.0 */
	tt_uint16	numGlyphs;
	tt_int16	maxPoints;				/* in an individual glyph */
	tt_int16	maxContours;			/* in an individual glyph */
	tt_int16	maxCompositePoints;		/* in an composite glyph */
	tt_int16	maxCompositeContours;	/* in an composite glyph */
	tt_int16	maxElements;			/* set to 2, or 1 if no twilightzone points */
	tt_int16	maxTwilightPoints;		/* max points in element zero */
	tt_int16	maxStorage;				/* max number of storage locations */
	tt_int16	maxFunctionDefs;		/* max number of FDEFs in any preprogram */
	tt_int16	maxInstructionDefs;		/* max number of IDEFs in any preprogram */
	tt_int16	maxStackElements;		/* max number of stack elements for any individual glyph */
	tt_int16	maxSizeOfInstructions;	/* max size in bytes for any individual glyph */
	tt_int16	maxComponentElements;	/* number of glyphs referenced at top level */
	tt_int16	maxComponentDepth;		/* levels of recursion, 1 for simple components */
} maxpClass;


typedef struct {
	/* private */
	tsiMemObject *mem;
	
	/* public */
	
	tt_int32 *offsets;
	int n;
	short indexToLocFormat;

} locaClass;


#ifdef ENABLE_TT_HINTING
/*   when using TT hinting. Declare sfnt objects for hinting. */
	typedef struct {
		/* private */
		tsiMemObject *mem;		/* memory manager object*/
		/* public */
		tt_uint8 *instructions;	/* point to array of instructions*/
		tt_uint32 numInstructions;	/* Number of 1 byte instructions,*/
  	} fpgmClass;
  	
	typedef struct {
		/* private */
		tsiMemObject *mem;		/* memory manager object*/
 		/* public */
		tt_uint8 *instructions;	/* point to array of instructions*/
		tt_int32 numInstructions;	/* Number of 1 byte instructions.	*/			
 	} prepClass;

	typedef struct {
		/* private */
		tsiMemObject *mem;		/* memory manager object*/
	 	/* public */
		tt_uint16 *varFWords;		/* point to array of instructions*/
		tt_int32 numFWords;		/* Number of U16 FWords (in em space) */			
 	} cvtClass;
 	
 	/* MTE Note: new structures are not required for glyp instructions: only
	   that a flag is set to ensure that the instructions are read.
	   See routine: "New_GlyphClass".
 	*/
#endif

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __T2K_TT_PRVT__ */
