/*
 * @(#)Fnt.h	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
	Copyright ©1987-1993 Apple Computer, Inc.  All rights reserved.
*/

#ifndef interpreterIncludes
#define interpreterIncludes

#ifndef _H_setjmp
#include <setjmp.h>
#endif

#ifndef fontScalerDefinitionIncludes
#include "FSCdefs.h"
#endif

#ifndef trueTypeIncludes
#include "TruetypeTypes.h"
#endif


/* #include "scaler_memory.h" */
 
#if defined(debugging) && defined(mutatorLinked)
#define check_cvt_access_direction
#endif

/*	This bugs exist in the 1993 release of these fonts.

	This are detected in the non-debug version.
	€ Helvetica Black (FDEFs beyond the number specified in the MaxProfile table)
	€ Helvetica Compressed (FDEFs beyond the number specified in the MaxProfile table)
	€ Palatino Roman (SHC with contour number out of range)
	
	Other known bugs are not addressed, both because they are read-bugs and do not trash memory,
	and because they are in common routines that we don't want to slow down.
	€ Helvetica Narrow Bold (RCVT out of range)
	€ Oxford (SFVTL with point out of range)
*/
#define performRuntimeErrorChecking

enum interpreterError {
	interp_range_error = 1,
	interp_assertion_error,
	interp_internal_error,
	interp_normalize_error,
	interp_bounding_box_error,
	interp_font_program_error,
	interp_unimplemented_instruction_error
};

#define fnt_pixelShift				6
#define fnt_pixelSize				(1 << fnt_pixelShift)
#define maximumFWord				32767
#define minimumFWord				-32768

#define MAXBYTE_INSTRUCTIONS		256

typedef struct {
	shortFrac	x;
	shortFrac	y;
} shortVector;

typedef struct {
	FWord	xMin;
	FWord	yMin;
	FWord	xMax;
	FWord	yMax;
} unscaledBounds;

typedef struct {
	FWord	advance;
	FWord	sideBearing;
} unscaledMetrics;

struct fnt_ElementType	{
	/*
	 *	Permanent part, stored in this order in permanentBlock
	*/
	fastInt	contourCount;
	fastInt 	pointCount;
	F26Dot6		*x;			/* Scaled Hinted Points */
	F26Dot6		*y;			/* Scaled Hinted Points */
	tt_int16		*sp;  		/* Start points */
	tt_int16		*ep;  		/* End points */
	tt_uint8		*onCurve;	/* indicates if a gxPoint is on or off the gxCurve */
	tt_uint8		*f;  		/* Internal flags, one byte for every gxPoint */

	/*
	 *	Temporary part, stored in this order in temporaryBlock
	*/
	F26Dot6	*ox;			/* Scaled Unhinted Points */
	F26Dot6	*oy;			/* Scaled Unhinted Points */
	FWord	*oox;		/* Unscaled Unhinted Points */
	FWord	*ooy;		/* Unscaled Unhinted Points */

	tt_int32     	       	glyphIndex;	
	unscaledBounds		bounds;
	unscaledMetrics		hori;
	unscaledMetrics		vert;
};

typedef struct fnt_ElementType fnt_ElementType;

typedef struct {
	tt_int32 start;		/* offset to first instruction */
	tt_uint16 length;		/* number of bytes to execute <4> */
	tt_uint16 pgmIndex;	/* index to appropriate preprogram for this func (0..1) */
} fnt_funcDef;

typedef struct {
	tt_int32 start;
	tt_uint16 length;
	tt_uint8  pgmIndex;
	tt_uint8  opCode;
} fnt_instrDef;

 
/* MTE: No Compile: typedef struct fnt_LocalGraphicStateType; */
 
/* MTE: Not needed. typedef void (*TracingFunc)(struct scalerContext*, struct fnt_LocalGraphicStateType*);*/

#if 0
typedef void (*TracingFunc)( void*, struct fnt_LocalGraphicStateType*);
typedef void (*FntFunc)(struct fnt_LocalGraphicStateType*);
typedef void (*InterpreterFunc)(struct fnt_LocalGraphicStateType*, tt_uint8*, tt_uint8*);
typedef void (*FntMoveFunc)(struct fnt_LocalGraphicStateType*, fnt_ElementType*, ArrayIndex, F26Dot6);
typedef F26Dot6 (*FntProjFunc)(struct fnt_LocalGraphicStateType*, F26Dot6 x, F26Dot6 y);
#ifdef use_engine_characteristics_in_hints
	typedef F26Dot6 (*FntRoundFunc)(F26Dot6 xin, F26Dot6 engine, struct fnt_LocalGraphicStateType* gs);
#else
	typedef F26Dot6 (*FntRoundFunc)(F26Dot6 xin, struct fnt_LocalGraphicStateType* gs);
#endif
#else
typedef void (*TracingFunc)( void*, void *);
typedef void (*FntFunc)(void *);
typedef void (*InterpreterFunc)(void *, tt_uint8*, tt_uint8*);
typedef void (*FntMoveFunc)(void *, fnt_ElementType*, ArrayIndex, F26Dot6);
typedef F26Dot6 (*FntProjFunc)(void *, F26Dot6 x, F26Dot6 y);
#ifdef use_engine_characteristics_in_hints
	typedef F26Dot6 (*FntRoundFunc)(F26Dot6 xin, F26Dot6 engine, void * gs);
#else
	typedef F26Dot6 (*FntRoundFunc)(F26Dot6 xin, void * gs);
#endif
#endif



/* PARAMETERS CHANGEABLE BY TT INSTRUCTIONS */
typedef struct {
	F26Dot6	wTCI;     				/* width table cut in */
	F26Dot6	sWCI;     				/* single width cut in */
	F26Dot6	scaledSW; 			/* scaled single width */
	tt_int32	scanControl;			/* controls kind and when of dropout control */
	tt_int32	instructControl;		/* controls gridfitting and default setting */
	
	F26Dot6	minimumDistance;		/* moved from local gs  7/1/90  */
	FntRoundFunc RoundValue;	
	F26Dot6 	periodMask; 			/* ~(gs->period-1) 				*/
	fract		period45;
	tt_int16	period;				/* for power of 2 periods 		*/
	tt_int16 	phase;
	tt_int16 	threshold;				/* moved from local gs  7/1/90  */
	
	tt_int16	deltaBase;
	tt_int16	deltaShift;
	FWord	sW;         				/* single width, expressed in the same units as the character */
	tt_int8		autoFlip;   			/* The auto flip Boolean */
	tt_int8		pad;	
} fnt_ParameterBlock;				/* this is exported to client */

enum interpreterVersions {
	system6InterpreterVersion = 1,
	system7InterpreterVersion,
	windows3point1InterpreterVersion,
	system6point2JInterpreterVersion,
	KirinPrinterInterpreterVersion,
	system7point1InterpreterVersion,
	quickdrawGXInterpreterVersion
};

enum interpreterGetInfos {
	rotateInterpreterGetInfo = 0x100,
	stretchInterpreterGetInfo = 0x200,
	variationInterpreterGetInfo = 0x400,
	verticalMetricsInterpreterGetInfo = 0x800
};

enum interpreterQueries {
	versionInterpreterQuery = 1,
	rotatedInterpreterQuery = 2,
	stretchedInterpreterQuery = 4,
	variationInterpreterQuery = 8,
	verticalMetricsInterpreterQuery = 0x10
};

enum fntGetDataSelectors {
	randomFntGetData = 0x0001
};

#define NOGRIDFITFLAG		1
#define DEFAULTFLAG		2

#define TWILIGHTZONE	0		/* index into perGlyphScratch.elements */
#define GLYPHELEMENT	1		/* index into perGlyphScratch.elements */
#define ELEMENTCOUNT	2		/* size of perGlyphScratch.elements */

enum fnt_ProgramIndex {
	fontProgramIndex,
	preProgramIndex,
	maxFDefPrograms,
	noProgramIndex
};

typedef gxPoint point26Dot6;
typedef gxPoint pointInteger;

typedef struct fnt_GlobalGraphicStateType {
	/* Pass the fixed size stack.*/
	F26Dot6		*	stackZone;				/* Pass these so that they (*/
	tt_int32			stackSize;				/*  can be passed to the local state.*/

	F26Dot6*		store; 					/* the storage area */
	F26Dot6*		controlValueTable; 		/* the control value table */
	shortFrac*		variationCoord;			/* array of coordinates for current variation */
	FntFunc*		function; 				/* pointer to instruction definition area */
	fnt_funcDef*	funcDef; 				/* function Definitions identifiers */
	fnt_instrDef*	instrDef;				/* instruction Definitions identifiers */
	tt_uint8*		pgmList[maxFDefPrograms];	/* each program ptr is in here */

	Fixed			cvtScale;				/* original scale factor used in preprogram */
	point26Dot6		fractionalPPEM;			/* computed in SetGlobalGSMapping */
	pointInteger	integerPPEM;			/* computed in SetGlobalGSMapping */
	gxPoint			upemScale;				/* computed in SetGlobalGSMapping */
	gxPoint			cvtStretch;				/* computed in SetGlobalGSMapping */	
	tt_int32       		variationCoordCount;
	tt_int32	       	pointSize; 				/* the requested gxPoint size as an integer */
	tt_int32	       	instrDefCount;			/* number of currently defined IDefs */
	tt_int32	       	pgmIndex;				/* which preprogram is current */

	fnt_ParameterBlock defaultParBlock;		/* variables settable by TT instructions */
	fnt_ParameterBlock localParBlock;

#ifdef use_engine_characteristics_in_hints
	F26Dot6		engine[4]; 			/* Engine Characteristics */
#endif
#ifdef check_cvt_access_direction
	char*		cvtFlags;
#endif
#if defined(debugging) || defined(performRuntimeErrorChecking)
	sfntMaxProfileTable*	maxp;
#endif
#ifdef performRuntimeErrorChecking
	tt_int32	       	cvtCount;
#endif
#ifdef debugging
	tt_int32	       	glyphIndex;
#endif

	boolean	hasVariationCoord;			/* has a coord and it is non-default */
	boolean	preProgramHasDefs;			/* true means the prep contains FDEF(s) and/or IDEF(s), so a glyph program
										might need to call into it, so load it during the glyphs instructions */
	tt_int8		identityTransformation; 		/* true/false  (does not mean identity from a global sense) */
	tt_int8		non90DegreeTransformation;	/* bit 0 is 1 if non-90 degree, bit 1 is 1 if x scale doesn't equal y scale */
} fnt_GlobalGraphicStateType;

/* 
 * This is the local graphics state  
 */
typedef struct fnt_LocalGraphicStateType {
	fnt_ElementType *CE0, *CE1, *CE2; 	/* The character element pointers */
	shortVector proj; 					/* Projection Vector */
	shortVector free;					/* Freedom Vector */
	shortVector oldProj; 				/* Old Projection Vector */
	
	memoryContext* context;
	tt_int32       	stackSize;				/* the size of the stack in bytes */
	F26Dot6* stackBase; 				/* the stack area */
	F26Dot6* stackEnd; 					/* the end of the stack area */
	F26Dot6 *stackPointer;				/* the pointer to the top of the stack */

	tt_uint8 *insPtr; 						/* Pointer to the instruction we are about to execute */
	fnt_ElementType **elements;			/* points to an array of fnt_ElementType pointers */
	fnt_GlobalGraphicStateType *globalGS;
	TracingFunc TraceFunc;

	tt_int32       	Pt0, Pt1, Pt2; 			/* The internal reference points */
	tt_int32       	roundToGrid;			
	tt_int32       	loop; 					/* The loop variable */	

	/* Above is exported to client in FontScaler.h */

	FntMoveFunc MovePoint;
	FntProjFunc Project;
	FntProjFunc OldProject;
	InterpreterFunc Interpreter;
	F26Dot6 (*GetCVTEntry) (struct fnt_LocalGraphicStateType *gs, ArrayIndex n);
	F26Dot6 (*GetSingleWidth) (struct fnt_LocalGraphicStateType *gs);

	jmp_buf	env;		/* always be at the end, since it is unknown size */

	shortFrac	pfProj; /* = pvx * fvx + pvy * fvy */

	boolean	unscaledOutlineIsWrong;		/* the oox and ooy should not be used for IP, MDRP, MD */
	tt_uint8		opCode; 					/* The instruction we are executing */
	tt_uint8		projectionVectorIsNormal;	/* true if projection vector was set normal to a gxLine */
#ifdef support_fake_variation_cvt
	tt_uint8		fakeVariantCVT;			/* triggers hack to handle CVT for variations in MIRP and MIAP */
#endif
#ifdef debugging
	tt_int8		valid_pfProj;
#endif

} fnt_LocalGraphicStateType;

void CorrectUnscaledOutline(fnt_LocalGraphicStateType* gs);
void fnt_Execute(fnt_ElementType* elements[], fnt_GlobalGraphicStateType *globalGS, tt_uint8 *ptr, tt_uint8 *eptr,
			voidFunc TraceFunc, 	memoryContext* context, boolean hasStyleCoord, boolean hasVariantCVT,
			boolean hintingACompositGlyph);
/*
 * Init routine, to be called at boot time.
 */
extern void fnt_DefaultJumpTable(FntFunc jumpTable[]);

/*
 *	Export internal rounding routines so globalGraphicsState->defaultParBlock.RoundValue
 *	can be set in fsglue.c
*/

#ifdef use_engine_characteristics_in_hints
#define ENGINE_PARAMETER(engine)		, F26Dot6 engine
#define EngineCharacteristicCode(code)	code
#else
#define ENGINE_PARAMETER(engine)
#define EngineCharacteristicCode(code)
#endif
extern F26Dot6 fnt_RoundToDoubleGrid(F26Dot6 xin ENGINE_PARAMETER(engine), fnt_LocalGraphicStateType *gs);  
extern F26Dot6 fnt_RoundDownToGrid(F26Dot6 xin ENGINE_PARAMETER(engine), fnt_LocalGraphicStateType *gs);  
extern F26Dot6 fnt_RoundUpToGrid(F26Dot6 xin ENGINE_PARAMETER(engine), fnt_LocalGraphicStateType *gs);  
extern F26Dot6 fnt_RoundToGrid(F26Dot6 xin ENGINE_PARAMETER(engine), fnt_LocalGraphicStateType *gs);  
extern F26Dot6 fnt_RoundToHalfGrid(F26Dot6 xin ENGINE_PARAMETER(engine), fnt_LocalGraphicStateType *gs); 
extern F26Dot6 fnt_RoundOff(F26Dot6 xin ENGINE_PARAMETER(engine), fnt_LocalGraphicStateType *gs);  
extern F26Dot6 fnt_SuperRound(F26Dot6 xin ENGINE_PARAMETER(engine), fnt_LocalGraphicStateType *gs);  
extern F26Dot6 fnt_Super45Round(F26Dot6 xin ENGINE_PARAMETER(engine), fnt_LocalGraphicStateType *gs); 

#endif
