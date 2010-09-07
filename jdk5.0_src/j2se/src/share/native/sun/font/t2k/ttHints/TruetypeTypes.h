/*
 * @(#)TruetypeTypes.h	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 /* graphics:	
	 			
	Copyright ©1987-1993 Apple Computer, Inc.  All rights reserved.

 */

#ifndef trueTypeIncludes
#define trueTypeIncludes

#ifndef __T2K_DTYPES__
	#include "dtypes.h"
	
#ifndef __T2K_TTCOMMON__
#include "ttCommon.h"
#endif

#endif
typedef enum {
	leftSidePoint,
	rightSidePoint,
	topSidePoint,
	bottomSidePoint,
	metricPointCount
} metricPoints;

typedef short sfntControlValue;

/*
 * 		Head table - global gxFont information
 */

#define fontHeaderTableTag			0x68656164	/* 'head' */

#define buttHeadTableTag				0x62686564	/* 'bhed' */
/* this table is an alternate 'head' table for sbit only fonts that need to work with old quickDraw.
	Currently the 'head' has the unitsPerEm set to 4614.  Not all sbit-only fonts will have the 
	same scale.  The buttheadtable is the same as the 'head' but old scalers ignore it.  If the old QuickDraw 
	scaler gets a sfnt without a 'head' table it falls through to use the FDEF.  The New FDEF/marukan enabler
	written by Apple Japan uses the sbit data ('bdat').  This way we can associate the NFNTs with the sfnt
	and old QuickDraw will work with these sbit-only fonts on a Japanese system.
*/

typedef struct {
	tt_uint32	bc;
	tt_uint32	ad;
} BigDate;

typedef enum sfntHeaderFlagBits {
	Y_POS_SPECS_BASELINE = 1,
	X_POS_SPECS_LSB = 2,
	HINTS_USE_POINTSIZE = 4,
	USE_INTEGER_SCALING = 8,
	INSTRUCTIONS_CHANGE_ADVANCEWIDTHS = 16,
	X_POS_SPECS_BASELINE = 32,
	Y_POS_SPECS_TSB = 64,
	FONT_REQUIRES_LAYOUT = 128,
	FONT_HAS_NORMAL_LAYOUT = 256,
	FONT_CAN_REORDER = 512,
	FONT_CAN_REARRANGE = 1024
} sfntHeaderFlagBits;

#define SFNT_MAGIC					0x5F0F3CF5
#define SHORT_INDEX_TO_LOC_FORMAT		0
#define LONG_INDEX_TO_LOC_FORMAT		1
#define GLYPH_DATA_FORMAT				0
#define FONT_HEADER_VERSION			fixed1

typedef struct {
	fixed					version;			/* for this table, set to 1.0 */
	fixed					fontRevision;		/* For Font Manufacturer */
	tt_uint32		checkSumAdjustment;
	tt_uint32		magicNumber; 		/* signature, should always be 0x5F0F3CF5  == MAGIC */
	unsigned short		flags;
	unsigned short		unitsPerEm;		/* Specifies how many in Font Units we have per EM */

	BigDate			created;
	BigDate			modified;

	/** This is the gxFont wide bounding box in ideal space
	(baselines and metrics are NOT worked into these numbers) **/

	short			xMin;
	short			yMin;
	short			xMax;
	short			yMax;

	unsigned short		macStyle;			/* macintosh style word */
	unsigned short		lowestRecPPEM; 	/* lowest recommended pixels per Em */

	/*	 0: fully mixed directional glyphs, 1: only strongly L->R or T->B glyphs, 
	 *	-1: only strongly R->L or B->T glyphs, 2: like 1 but also contains neutrals,
	 *	-2: like -1 but also contains neutrals
	*/
	short			fontDirectionHint;
	short			indexToLocFormat;
	short			glyphDataFormat;
} sfntFontHeader;

/*
 * Hhea - horizontal metrics header
 * Vhea - vertical metrics header
 */

#define horizontalHeaderTableTag		0x68686561	/* 'hhea' */
#define verticalHeaderTableTag		0x76686561	/* 'vhea' */

#define METRIC_HEADER_FORMAT		fixed1

typedef struct {
	fixed		version;				/* for this table, set to 1.0 */
	short		ascender;
	short		descender;
	short		lineGap;			/* linespacing = ascender - descender + linegap */
	unsigned short	advanceMax;	
	short		sideBearingMin;		/* left or top */
	short		otherSideBearingMin;	/* right or bottom */
	short		extentMax; 			/* Max of ( SB[i] + bounds[i] ), i loops through all glyphs */
	short		caretSlopeNumerator;
	short		caretSlopeDenominator;
	short		caretOffset;

	tt_uint32	reserved1, reserved2;	/* set to 0 */

	short		metricDataFormat;		/* set to 0 for current format */
	unsigned short	numberLongMetrics;		/* if format == 0 */
} sfntMetricsHeader;

typedef sfntMetricsHeader sfntHorizontalHeader;
typedef sfntMetricsHeader sfntVerticalHeader;

/*
 * Maxp Table - maximum profile metrics
 */

#define maximumProfileTableTag			0x6d617870	/* 'maxp' */

#define MAX_PROFILE_VERSION		fixed1

typedef struct {
	fixed					version;				/* for this table, set to 1.0 */
	unsigned short		numGlyphs;
	unsigned short		maxPoints;		/* in an individual glyph */
	unsigned short		maxContours;			/* in an individual glyph */
	unsigned short		maxCompositePoints;		/* in an composite glyph */
	unsigned short		maxCompositeContours;	/* in an composite glyph */
	unsigned short		maxElements;			/* set to 2, or 1 if no twilightzone points */
	unsigned short		maxTwilightPoints;		/* max points in element zero */
	unsigned short		maxStorage;				/* max number of storage locations */
	unsigned short		maxFunctionDefs;		/* max number of FDEFs in any preprogram */
	unsigned short		maxInstructionDefs;		/* max number of IDEFs in any preprogram */
	unsigned short		maxStackElements;		/* max number of stack elements for any individual glyph */
	unsigned short		maxSizeOfInstructions;	/* max size in bytes for any individual glyph */
	unsigned short		maxComponentElements;	/* number of glyphs referenced at top level */
	unsigned short		maxComponentDepth;		/* levels of recursion, 1 for simple components */
} sfntMaxProfileTable;

/*
 * Hmtx - horizontal glyph metrics
 * Vmtx - vertical glyph metrics
 */
 
#define horizontalMetricsTableTag			0x686d7478	/* 'hmtx' */
#define verticalMetricsTableTag			0x766d7478	/* 'vmtx' */

typedef struct {
	short					advance;
	short 					sideBearing;
} sfntGlyphMetrics;

typedef sfntGlyphMetrics sfntHorizontalMetrics;
typedef sfntGlyphMetrics sfntVerticalMetrics;

/*
 *		Hdmx - horizontal device metrics
 */

#define horizontalDeviceMetricsTableTag		0x68646d78	/* 'hdmx' */

#define DEVWIDTHEXTRA	2	/* size + max */

/*
 *	Each record is n+2 bytes, padded to 4 byte alignment.
 *	First byte is ppem, second is maxWidth, rest are widths for each glyph
 */

typedef struct {
	short					version;
	short					numRecords;
	tt_int32					recordSize;
	/* Byte widths[numGlyphs+DEVWIDTHEXTRA] * numRecords */
} sfntDeviceMetrics;

/*
 *		Post Table - postscript table
 */
 
#define postscriptTableTag				0x706f7374	/* 'post' */

#define	stdPostTableFormat			0x10000
#define	wordPostTableFormat		0x20000
#define	bytePostTableFormat		0x28000
#define	richardsPostTableFormat		0x30000

typedef struct {
	fixed				version;
	fixed				italicAngle;
	short				underlinePosition;
	short				underlineThickness;
	short				isFixedPitch;
	short				pad;
	tt_uint32	minMemType42;
	tt_uint32	maxMemType42;
	tt_uint32	minMemType1;
	tt_uint32	maxMemType1;
/* if version == 2.0
	{
		numberGlyphs;
		unsigned short[numberGlyphs];
		pascalString[numberNewNames];
	}
	else if version == 2.5
	{
		numberGlyphs;
		tt_int8[numberGlyphs];
	}
*/		
} sfntPostScriptInfo;

/*
 *		Bdat Table
 *		BLoc Table
 */

#define bitmapLocationTableTag		0x626c6f63	/* 'bloc' */
#define bitmapDataTableTag			0x62646174	/* 'bdat' */

typedef enum outlinePacking {
	ONCURVE = 1,
	XSHORT = 2,
	YSHORT = 4,
	REPEAT_FLAGS = 8,
/* IF XSHORT */
	SHORT_X_IS_POS = 16,		/* the short vector is positive */
/* ELSE */
	NEXT_X_IS_ZERO = 16,		/* the relative x coordinate is zero */
/* ENDIF */
/* IF YSHORT */
	SHORT_Y_IS_POS = 32,		/* the short vector is positive */
/* ELSE */
	NEXT_Y_IS_ZERO = 32,		/* the relative y coordinate is zero */
/* ENDIF */
	OVERLAP_SIMPLE = 64		/* this simple glyph has overlapping contours under some circumstances (variations etc.) */
} outlinePacking;
/* Note: structure componentPacking=> ttCommon.h */

/*	gxBitmap	*/
#ifndef gxAnyNumber
#define gxAnyNumber	1
#endif

#define BITMAP_HEADER_VERSION		0x20000
#define BITMAP_DATA_VERSION		0x20000

typedef enum {
	bmComplete = 1,
	bmHorizontal = 2,
	bmVertical = 4,
	bmHoriVert = 6
} bitmapGlyphFormats;

typedef enum {
	ndxProportional = 1,
	ndxMono,
	ndxProportionalSmall
} bitmapIndexFormats;

typedef enum{
	proportionalFormat1 = 1,			/*small metrics and data, byte-aligned*/
	proportionalFormat2,			/*small metrics and data, bit-aligned*/
	proportionalCompressedFormat3,	/*metric info followed by compressed data*/
	monoCompressedFormat4,			/*just compressed data, metrics are in bloc*/
	monoFormat5 = 5,				/*just bit aligned data, metrics are in bloc*/
	proportionalByteFormat6,			/*metrics & byte-aligned image*/
	proportionalBitFormat7			/*metrics & bit-aligned image*/
} bitmapDataFormats;

typedef unsigned char uChar;
typedef char sChar;

typedef struct {
	sChar 		ascender;
	sChar		descender;
	uChar		widthMax;
	sChar		caretSlopeNumerator;
	sChar		caretSlopeDenominator;
	sChar		caretOffset;
	sChar		minOriginSB;
	sChar		minAdvanceSB;
	sChar		maxBeforeBL;
	sChar		minAfterBL;	
	sChar		pad1;
	sChar		pad2;
} sbitLineMetrics;	

typedef enum {
	flgHorizontal = 0x01,
	flgVertical = 0x02
} bitmapFlags;

typedef struct {
	tt_uint32	indexSubTableArrayOffset;	/*offset to corresponding index subtable array from beginning of bloc table*/
	tt_uint32	indexTablesSize;		/* number bytes of corresponding index subtables and array*/
	tt_uint32	numberOfIndexSubTables;	/* an index subtable for each range or format change*/
	tt_uint32	colorRef;				/*ignore for now*/
	sbitLineMetrics hori;				/* gxLine metrics*/
	sbitLineMetrics vert;			/*gxLine metrics*/
	unsigned short	startGlyphIndex;	/*lowest glyph index for this size*/
	unsigned short	endGlyphIndex;		/*highest glyph index for this size*/
	uChar		ppemX;			/* target horizontal ppem*/
	uChar		ppemY;			/* target vertical ppem*/
	uChar		bitDepth;			/* bit depth of the strike */
	sChar		flags;				/* see bitmapFlags, first two bits are for orientation, vertical or horizontal */
} bitmapSizeTable;

typedef struct {
	fixed		version;
/*	bitmapData in various formats		*/
} bdatHeader;


typedef struct {
	fixed		version;
	tt_int32			numSizes;
/*	bitmapSizeTable[numSizes]	 or 	bitmapSizeTable2[numSizes]  (the version will tell us which kind)*/
} blocHeader;

#if 0
 	// Moved to ttCommon.h
typedef struct {
	unsigned short	indexFormat;
	unsigned short	imageFormat;
	tt_uint32	imageDataOffset;	/*offset to corresponding image data from beginning of bdat table*/
} indexSubHeader;
#endif
	/* format 1 has variable length images of the same format*/
typedef struct {
	indexSubHeader	header;
	tt_uint32	offsetArray[gxAnyNumber];	/*offsetArrary[glyphIndex] + imageDataOffset = startOfBitDataForGlyph*/
							/*sizeOfArray = lastGlyph - firstGlyph + 1*/	
} indexSubTable1;

typedef struct {
	uChar	height;		
	uChar	width;		
	sChar	horiBearingX;	
	sChar	horiBearingY;	
	uChar	horiAdvance;		
	sChar	vertBearingX;	
	sChar	vertBearingY;	
	uChar	vertAdvance;		
} bigGlyphMetrics;

typedef struct {
	uChar	height;		
	uChar	width;		
	sChar	bearingX;	
	sChar	bearingY;	
	uChar	advance;		
} smallGlyphMetrics;

typedef struct {
	indexSubHeader	header;
	tt_uint32	imageSize;			/* all the glyphs are of the same size.  May be compressed, bit-aligned, or byte-aligned*/
	bigGlyphMetrics bigMetrics;			/*all glyphs have the same metrics*/
} indexSubTable2;

	/* format 3 has variable length images of the same format with word offsets*/
typedef struct {
	indexSubHeader	header;
	unsigned short	offsetArray[gxAnyNumber];	/*offsetArrary[glyphIndex] + imageDataOffset = startOfBitDataForGlyph*/
							/*sizeOfArray = lastGlyph - firstGlyph + 1*/	
} indexSubTable3;

/* Moved to ttCommon.h */
#if 0
typedef struct {
	unsigned short	firstGlyphIndex;
	unsigned short	lastGlyphIndex;
	tt_uint32	additionalOffsetToIndexSubtable;	/* add to indexSubTableArrayOffset to get offset from beginning of bloc*/
} indexSubTableArray;
#endif
typedef struct {
	smallGlyphMetrics	smallMetrics;
	/*	gxBitmap image data */
} glyphBitmap_1, glyphBitmap_2;

#define SIZEOF_glyphBitmap_1		5
#define SIZEOF_glyphBitmap_2		5

typedef struct {
	bigGlyphMetrics bigMetrics;
/*	gxBitmap image data */
} glyphBitmap_6, glyphBitmap_7;

#define SIZEOF_glyphBitmap_6		8
#define SIZEOF_glyphBitmap_7		8

typedef struct {
	bigGlyphMetrics bigMetrics;
	tt_uint32	whiteTreeOffset;	/*offset from 'height'*/
	tt_uint32	blackTreeOffset;	/*offset from 'height'*/
	tt_uint32	glyphDataOffset;	/*offset from 'height'*/
} glyphBitmap_3;

/*The data for the type 3 compressed gxBitmap will follow all the glyphBitmap_3 data
	and will be of variable length*/
/*	
	short whiteTree[]
	short blackTree[]
	compacted glyph data
 */

/* for monospaced bitmaps the metric information is in the 'bloc' indexSubTable2*/
typedef struct {
	tt_uint32	whiteTreeOffset;	/*offset from 'height'*/
	tt_uint32	blackTreeOffset;	/*offset from 'height'*/
	tt_uint32	glyphDataOffset;	/*offset from 'height'*/
} glyphBitmap4;

/*
 * ????? Table - accented glyphs
 */

typedef struct {
	tt_int32			format;
	unsigned short	firstAccentedGlyphIndex;
	unsigned short	lastAccentedGlyphIndex;
	tt_int32			glyphOffset;
	tt_int32			extOffset;
	tt_int32			accentOffset;
} sfntAccentTable;

/*
 * Gvar Table - glyph variations
 */
 
#define glyphVariationTableTag			0x67766172	/* 'gvar' */

#if !(defined(__pink__) && defined(Taligent_TRUETYPEFILE))
typedef short shortFrac;		/* 2.14 number representing -2.0000ä+1.9999 */
#endif
#define shortFrac1			(1 << 14)

typedef struct {
	shortFrac				coord[gxAnyNumber];		/* [axisCount] */
	/* room to grow since the header carries a tupleSize field */
} fontVariationTuple;

typedef enum {
	packed_points_mask = 0x80,		/* high bit is flag, low 7 are 0-based count */
	points_are_bytes = 0,			/* gxPoint numbers are byte deltas */
	points_are_words = 0x80			/* gxPoint numbers are word deltas */
} packedPointsFlags;

typedef enum {
	packed_deltas_mask = 0xC0,		/* high 2 bytes are flag bits, low 6 are 0-based count */
	deltas_are_bytes = 0,			/* points are moved by byte deltas */
	deltas_are_words = 0x40,		/* points are moved by word deltas */
	deltas_are_zero = 0x80			/* points are touched */
} packedDeltaFlags;

#define embedded_tuple_coord  0x8000 	/* coord follows immediately after the index */
typedef enum {
	intermediate_tuple = 0x4000,		/* this tuple is part of a series of intermediate axis tuples */
	private_point_numbers = 0x2000,	/* gxPoint numbers preceed tuple deltas */
	reserved_tuple_flag = 0x1000,	/* should be clear */
	tuple_index_mask = 0x0FFF		/* the high nibble is reserved for flag bits */
} tupleIndexFlags;
	
typedef struct {
	short	tupleSize;					/* size, in bytes, of this tuple's data */
	short	tupleIndex;				/* see above comment */
	shortFrac	embedded[gxAnyNumber];		/* [axisCount] if tupleIndex & embedded_tuple_coord */
	shortFrac	intermediate[gxAnyNumber];		/* [2*axisCount] if tupleIndex & intermediate_tuple */
} tupleVariation;

#define sizeof_tupleVariation	4

#define tuples_share_point_numbers  0x8000
typedef enum {
	tuple_count_mask = 0x0FFF
} glyphVariationFlags;

typedef struct {
	short		tupleCount;				/* contains glyphVariationFlags in high-nibble */
	short		offsetToData;				/* to beginning of tuple data */
	tupleVariation	tuple[gxAnyNumber];			/* [tupleCount] */
	char			pointNumbers[gxAnyNumber];	/* packed [pointCount] */
	char			tupleData[gxAnyNumber];		/* packed ptCount, ptNumbers[ptCount], deltaX[], deltaY[] */
} glyphVariation;

#define sizeof_glyphVariation	4

enum {
	long_glyph_variation_offsets = 1		/* else unsigned short word offsets */
};

typedef struct {
	fixed				version;				/* 1.0 Fixed */
	unsigned short	axisCount;			/* number of variation axes (same as 'fvar') */
	unsigned short	globalCoordCount;		/* number of shared tuple coordinates */
	tt_int32					offsetToCoord;			/* from beginning of table to first shared tuple coordinate */
	unsigned short	glyphCount;
	unsigned short	flags;				/* long_glyph_variation_offsets */
	tt_int32					offsetToData;			/* from beginning of table to first glyphVariation */
	unsigned short	offset[gxAnyNumber];		/* [glyphCount + 1] relative to offsetToData */
	shortFrac			coord[gxAnyNumber];		/* [coordCount][axisCount] */
	glyphVariation	variation[gxAnyNumber];	/* variable size element [glyphCount] */
} glyphVariationHeader;

#define sizeof_glyphVariationHeader		20

/*
 * 		Cvar Table
 */
 
#define controlVariationTableTag		0x63766172	/* 'cvar' */

typedef struct {
	fixed			version;				/* 1.0 Fixed */
	/*
	 *	The next three shorts should look just like a glyphVariation
	*/
	short		tupleCount;
	short		offsetToData;
	tupleVariation	tuple[gxAnyNumber];		/* [variationCount] */
	sChar			tupleData[gxAnyNumber];
} cvtVariationHeader;

#define sizeof_cvtVariationHeader	8

/*
 *	Avar Table - transformations to the axis coordinates
*/

#define axisMappingFontTableTag		0x61766172

typedef struct {
	shortFrac		fromCoord;
	shortFrac		toCoord;
} shortFracCorrespondence;

#define sizeof_shortFracCorrespondence		4

typedef struct {
	short				pairCount;
	shortFracCorrespondence	pair[gxAnyNumber];
} shortFracSegment;

#define ComputeShortFracSegmentSize(segment)	(sizeof(short) + SWAPW((segment)->pairCount) * sizeof_shortFracCorrespondence)
#define NextShortFracSegment(segment)			(shortFracSegment*)((char*)(segment) + ComputeShortFracSegmentSize(segment))

typedef struct {
	fixed			version;		/* 1.0 */
	tt_int32       		axisCount;
	shortFracSegment	segment[gxAnyNumber];
} axisMappingHeader;

#define sizeof_axisMappingHeader		8

/*
 *		fmtx - font-wide metrics for ascent, descent, caret angle
 */
 
#define fontMetricsTableTag			0x666d7478	/* 'fmtx' */

enum fontMetricPointTypes {
	fontMetricsHorizontalBefore,
	fontMetricsHorizontalAfter,
	fontMetricsHorizontalCaretHead,
	fontMetricsHorizontalCaretBase,
	fontMetricsVerticalBefore,
	fontMetricsVerticalAfter,
	fontMetricsVerticalCaretHead,
	fontMetricsVerticalCaretBase,
	fontMetricPointCount
};

#define fontMetricsVersion		(0x20000)

typedef struct {
	fixed		version;
	tt_int32     	glyphIndex;
	unsigned char	pointIndex[fontMetricPointCount];
} fontMetricsHeader;

#define sizeof_fontMetricsHeader		sizeof(fontMetricsHeader)

/*
 *		Other tables
 */
#define indexToLocationTableTag	0x6c6f6361	/* 'loca' */
#define controlValueTableTag		0x63767420	/* 'cvt ' */
#define preProgramTableTag		0x70726570	/* 'prep' */
#define glyphDataTableTag		0x676c7966	/* 'glyf' */
#define kerningTableTag			0x6b65726e	/* 'kern' */
#define fontProgramTableTag		0x6670676d	/* 'fpgm' */
#define accentAttachmentTableTag	0x61636e74	/* 'acnt' */

#endif
