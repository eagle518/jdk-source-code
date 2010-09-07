/*
 * @(#)FSglue.h	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
 /*	FSglue.h
**
**		Based on FSGlue.h, this file declares the "key" data structures.  These structures
**	manage the state of the TrueType gxFont scaler's operations.  The main key structure
**	makes the connection between the per-stage permanent blocks of the scaler context and
**	the TT scaler's view of the world.
**
**	Copyright:	© 1992 by Apple Computer, Inc., all rights reserved.
**
  */

#define ACCENTS

#ifndef fsglueIncludes
#define fsglueIncludes
#ifndef cpuPrinter
#ifndef privateMacrosIncludes
#endif
#endif
#ifndef interpreterIncludes
#include "Fnt.h"
#endif

#ifndef scanConverterIncludes
#include "sc.h"
#endif


#ifndef fontScalerIncludes
	/* MTE: don't need it yet.#include "FontScaler.h"*/
#endif

 
#ifndef	nil
#define	nil	(void*)0
#endif

#define truetypeFont					0x74727565	/* 'true' TrueType formatted gxFont */
#define SIDE_BEARING_PERCENT   		 0x0CCD
#define BitTestToBoolean(bitTest)		((bitTest) != 0)
 	#define kBusErrorValue			(void*)0x50ff8001
 
#define LONG_WORD_ALIGN( n )	(n) += 3, (n) &= ~3
#define FIXEDSQRT2 				0x00016A0A
#define FixedToF26Dot6(x)		(((x) + (1 << 9)) >> 10)
#define F26Dot6ToFixed(x)		((x) << 10)
#define RoundF26Dot6(x)			(((x) + 0x20) & ~0x3F)

#define POINTSPERINCH				72		/* Default device resolution */
#define FIXEDPOINTSPERINCH			((tt_int32)72 << 16)
#define MAX_TWILIGHT_CONTOURS		1

enum engineDistance {
	kGreyEngineDistance,
	kBlackEngineDistance,
	kWhiteEngineDistance,
	kIllegalEngineDistance
};

enum phantomPoints {
	leftSidePhantom,
	rightSidePhantom,
	topSidePhantom,
	bottomSidePhantom,
	publicPhantomCount,
/* these are private phantom points */	
	leftEdgePhantom = publicPhantomCount,
	rightEdgePhantom,
	topEdgePhantom,
	bottomEdgePhantom,
	privatePhantomCount
};

typedef tt_int32 phantomPoint;

typedef struct {
	tt_int32       	offset;
	void*	ptr;
} offset_ptr;

typedef struct {
	tt_int32       	size;
	void*	ptr;
} size_ptr;

#define kInstructionJumpTableCount		256

typedef struct 
{
	FntFunc instructionJumpTable[kInstructionJumpTableCount];
} perScaler, *perScalerPtr;



typedef struct {
	struct	perFont * xPerFontContext;

	offset_ptr		userCoord;			/* shortFrac[axisCount] */
	offset_ptr		globalCoord;		/* shortFrac[globalCoordCount][axisCount] */
	offset_ptr		styledCvt;			/* FWord[perFont.cvtCount] iff hasVariantCVT */
	boolean		hasStyleCoord;			/* true if coord is non-default */
	boolean		pad[3];
} perVariation, *perVariationPtr;

 typedef struct { 	
 		memoryContext    aMemoryContext; 		/*  Original scaler.*/
 	fnt_funcDef		*funcDefPtr;			/*  Point to function definitions.*/
	tt_int32			numFuncDefs;			/*  total number of function definition entries.*/
	fnt_instrDef	*instrDefPtr;			/*  Point to instruction definitions*/
       	tt_int32			numInstrDefs;			/*  total number of instruction entries.*/
	tt_int32	       	functionDefsOffset;
	tt_int32	       	instructionDefsOffset;
	tt_int32	       	compoundCloneOffset;		/* place in scratch temp buffer to build acnt clone */

	tt_int32	       	storageSize;		/* size of buffer needed in perTrans */
	tt_int32	       	twilightZoneSize;	/* size of buffer needed in perGlyphScratch */
	tt_int32	       	stackSize;	/* size of buffer needed in perGlyphScratch */

	tt_int32	       	tempGlyphSize;		/* storage for temp part of outline */
	tt_int32	       	permGlyphSize;		/* storage for perm part of outline */

#ifdef SBIT_SUPPORT
	/* for new sbit information*/
	tt_uint32		blocHeaderVersion;
	tt_uint32		offsetToSizeTables;		/* not used when cacheing*/
	/* then we cache the variable length data*/
	/*all of the bitmapSizeTable2s*/
	/*followed by the index tables*/
#endif

	fastInt		maxContourCount;	/* worst-case contour count */
	fastInt		maxPointCount;		/* worst-case gxPoint count */

	/* MTE:We no longer need to the maxProfile table because t2k has it.*/
	/*   This should be changed to a ptr.*/
	sfntMaxProfileTable maxProfile;	/* copy of profile */

	tt_int32		gvarHeaderPlusOffsetSize;
	fastInt		axisCount;
	fastInt		globalCoordCount;

	fontMetricsHeader	fmtx;

	tt_uint16		fontFlags;			/* copy of header.flags */
	FWord		emResolution;		/* copy of header.unitsPerEm */
	FWord		xMin, yMin, xMax, yMax;		/* copy of header bbox */
	FWord		minimumPixPerEm;	/* copy of header.lowestRecPPEM */
	FWord		defaultSideBearing;	/* computed from emResolution */	
	tt_int16		indexToLocFormat;	/* caching gxFont table access functions */
	tt_uint16		horiLongMetrics;	/* number of *real* metrics, 0 means fake them */
	tt_uint16		vertLongMetrics;	/* number of *real* metrics, 0 means fake them */
	tt_int16		cvtCount;			/* computed from table size, or 0 if no table */

#ifdef SBIT_SUPPORT
	tt_uint16		numberOfSizes;
#endif

	boolean		usefulOutlines;		/* based on 'glyf' format.(in 'head')  in which 1 means this is bitmaps only */
	boolean		usefulBitmaps;		/* based on existence of 'bloc' table */
	boolean		fontProgramRan;	/* Set after the gxFont program is run */
	boolean		accentsExist;		/* looking for accentTable */
	boolean		expectFPGM;		/* the 'fpgm' table should exist */
	boolean		expectPREP;		/* the 'prep' table should exist */
	boolean		expectFVAR;		/* the 'fvar' table should exist */
	boolean		expectGVAR;		/* the 'gvar' table should exist */
	boolean		expectCVAR;		/* the 'cvar' table should exist */
	boolean		expectAVAR;		/* the 'avar' table should exist */
	boolean		useFMTX;			/* use the perFont->fmtx for font metrics at NewTransform */
	tt_uint8			IDefCount;			/* cache this across calls to NewTransform */
	perVariation	theVary;		/*  fake out a variation. */
} perFont, *perFontPtr;





typedef struct {
	gxMapping		baseMap;				/* input   : current complete map */
	gxMapping		stretchBase;			/* output : stretch factors from the baseMap */
	gxMapping		remainingBase;			/* output : stretch € remaining = baseMap */
} transformState;

typedef	struct {
		/*  MTE: add a pointer to the font. */
        struct	perVariation * xPerVaryContext;

		/*  Keep track of the pointers, with correct types. */
     	F26Dot6		*scaledCvtTransPtr;
	F26Dot6		*storageTransPtr;
        tt_int32 storageBytes;
			F26Dot6		*twilightTransPtr;/*  long twilightBytes; */
			/*  Also, allocate the stack only once, 	 */
			/* 	because the profile specifies the worst case. */
			F26Dot6		*stackTransPtr;
 
	offset_ptr		scaledCvt;			/* F26Dot6[theFont.cvtCount] */
	offset_ptr		storage;				/* F26Dot6[max.storage] */
	offset_ptr		twilightZone;			/* the storage for the twilight element */
	offset_ptr		stackZone;				/* the storage for the stack element */
			tt_int32	stackBytes;				/*  size of stac (from max profile) */
	offset_ptr		sbit;					/* index subTables for an sbit strike*/
	offset_ptr		sbitHead;				/* sbitTransHeader */
	
	
	
	
#ifdef check_cvt_access_direction
	offset_ptr		cvtFlags;
#endif

	Fixed			pointSize;				/* copied from input pointSize */
	Fixed			pixelDiameter;			/* computed from input spotSize */

	transformState				transState;
	fnt_GlobalGraphicStateType	globalGS;	/* global interpreter state */
	tt_int32			imageState;			/* am I rotated or stretched */

	boolean		executeInstructions;		/* Controls all fnt_execute() calls */
	boolean		returnDeviceMetrics;	/* All metric values should be returned as device (vs. fractional) */
	boolean		preProgramRan;		/* Was the preprogram run yet? */
	boolean		cvtHasBeenScaled;		/* Have we scaled the cvt yet? */
	boolean		verticalOrientation;		/* Glyphs will appear vertically */

	tt_uint8		methodToBeUsedIfPossible;	/* 0 = outlines; 1 = scaled gxBitmap; 2 = exact gxBitmap;
	 							     3 = exact gxBitmap except for pointsize; 4 = error */	
	tt_uint8		methodToBeUsedOtherwise;	/* 0 = outlines; 1 = scaled gxBitmap; 2 = exact gxBitmap;
	 							     3 = exact gxBitmap except for pointsize; 4 = error */	
	 								 
	tt_uint8		pad;	/* make long aligned, boolean is of type unsigned char */

} perTransformation, *perTransformationPtr;
typedef struct {

 
	gxMapping	bitmapTMatrix;		/* A copy of the currentTMatrix before fsg_ReduceMatrix */
		/* assume this data starts on long boundary. */
		/* reorder to fix up long-alignment */
	fastInt	desiredHorPpem;
	fastInt	desiredVerPpem;
	fastInt	bestHorPpemFont;	/* strike subtable for best gxBitmap data in gxFont */
	fastInt	bestVerPpemFont;

	tt_uint32	additionalIndexOffsetGlyph;	/* add to indexArrayOffsetFont to get offset from beginning of bloc*/
	tt_uint32	numberOfIndexSubTables;	/* an index subtable for each range or format change*/
	tt_int32	lengthGlyphData;	/*size of the glyph data*/
	tt_uint32	glyphDataOffset;	/*offset to corresponding glyph data from beginning of bdat table*/

	/* <Tak> support bdat format=4 */
	tt_uint32	whiteTreeOffset;	/*from top of 'bdat'*/
	tt_uint32	whiteTreeSize;
	tt_uint32	blackTreeOffset;	/*from top of 'bdat'*/
	tt_uint32	blackTreeSize;

	tt_uint16	startGlyphIndex;	/*lowest glyph index for this size*/
	tt_uint16	endGlyphIndex;		/*highest glyph index for this size*/
	tt_uint16	startGlyphRange;
	tt_uint16	endGlyphRange;
	
	tt_int8		bitmapSizeFlags;	/*tells orientation of metrics in smallGlyphMetrics*/
	tt_uint8		indexFormat;		
	tt_uint8		dataFormat;			
	tt_int8		pad1;
} sbitTransHeader, *sbitTransHeaderPtr;		/*this struct will be put in the sbit storage area*/

typedef struct {
	offset_ptr			permanentOutline;	/* permanent part of glyph element */

	fastInt			contourCount;
	fastInt			pointCount;
	unscaledBounds		bounds;
	unscaledMetrics	hori;
	unscaledMetrics	vert;
	tt_int32			scanControl;		/* flags for dropout control etc.  */	
	sc_BitMapData		bitMapInfo;		/* gxBitmap info structure */

#ifdef	SBIT_SUPPORT
		tt_uint32		glyphIndex;
		tt_uint32		whiteTreeOffset;	/*from top of 'bdat'*/
		tt_uint32		blackTreeOffset;	/*from top of 'bdat'*/
		tt_uint32		dataOffset;		/*from top of 'bdat'*/

		tt_int16		hAdvanceSource;
		
		tt_uint8			pixHeightSource;
		tt_uint8			pixWidthSource;
		tt_int8			hBearingXsource;
		tt_int8			hBearingYsource;
			
#endif

} perGlyph, *perGlyphPtr;

typedef struct
{
	size_ptr			actualBitmap;
	size_ptr			scanConvertScratch;	
	size_ptr			dropoutScratch;
	void*				glyphTemphBuffer;			/* storage for glyph's temp outlines */

	fnt_ElementType		elementStorage[ELEMENTCOUNT];	
	sc_GlobalData		globalScanData;		/* scan converter scratch */
} perGlyphScratch;


/* typedef MTE: make it a type. */
struct fsg_SplineKey 
{
	memoryContext*		memContext;		
 	perFontPtr				theFont;
	perVariationPtr			theVari;
	perTransformationPtr	theTrans;
	
	/*  The following parameters are setup by the glyph code. */
		perGlyphPtr				theGlyph;
		perGlyphScratch*		theScratch;
		
		struct {
			void		*functionDefs;
			void		*instructionDefs;
			void		*compoundClone;
		} fontBlockPtrs;
};

#ifdef	KEYDEBUGGING
void			CheckKeyField(void *theField, char *fieldName, char *sourceName, tt_int32 lineNumber);
newKeyPtr	CheckMemoryBase(newKeyPtr theKey,void *theField, char *fieldName, char *sourceName, tt_int32 lineNumber);
extern		newKeyPtr	debugKey;

/* Access macros used mainly for debugging, they catch out of order accesses */

#define	TheScaler(k)		(CheckKeyField((k)->theScaler,"theScaler",__FILE__,__LINE__),(k)->theScaler)
#define	TheFont(k)		(CheckKeyField((k)->theFont,"theFont",__FILE__,__LINE__),(k)->theFont)
#define	TheVari(k)		(CheckKeyField((k)->theVari,"theVari",__FILE__,__LINE__),(k)->theVari)
#define	TheTrans(k)		(CheckKeyField((k)->theTrans,"theTrans",__FILE__,__LINE__),(k)->theTrans)
#define	TheGlyph(k)		(CheckKeyField((k)->theGlyph,"theGlyph",__FILE__,__LINE__),(k)->theGlyph)
#define	TheScratch(k)		(k)->theScratch

#else

#define	TheScaler(k)		(k)->theScaler
#define	TheFont(k)		(k)->theFont
#define	TheVari(k)		(k)->theVari
#define	TheTrans(k)		(k)->theTrans
#define	TheGlyph(k)		(k)->theGlyph
#define	TheScratch(k)		(k)->theScratch

#endif

/* for the key->imageState field */
#define ROTATED		0x400
#define STRETCHED 		0x1000

#endif
