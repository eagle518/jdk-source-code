/*
 * @(#)Hint.h	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
  
 /* **	Copyright:	© 1990-1993 by Apple Computer, Inc., all rights reserved.  */

/* TTScaler (hinting/TrueType) scaler is a set of modules to apply hints
	to the T2K TrueType implementation. As such, it is not a complete scaler
	implementation.
	
	The implementation does not require the T2K code to include TTScaler headers.
	However, the full body of T2K headers are read by the TTScaler files.
*/

#ifndef	TTHintIncludes
#define	TTHintIncludes

/* Read in the important includes from T2K */	
#include "syshead.h"
#include "config.h"
#include "dtypes.h"
#include "tsimem.h"
#include "t2kstrm.h"
#include "truetype.h"
#include "glyph.h"
#include "t2k.h"
#include "t2ksc.h"
#include "autogrid.h"	
 
/* Added from MacTypes.h, with name change  */
typedef tt_int32 				       		Fract;
typedef tt_int32 		       				Fixed;
typedef char *							Ptr;

/* Added from FixMath.h with name change */
 
#define fixed1				(  0x00010000L)
#define fract1				(  0x40000000L)

typedef tt_int32  scalerError;
enum {
    scalerError_NoError=0, 			/* Normal Return*/
    scalerError_Memory=1,  			/*  Insufficient Memory..*/
    scalerError_ScanError=2,  			/*  Insufficient Memory..*/
    scalerError_POINT_MIGRATION_ERR=3
};

/**** enumerated types from privateFontScaler.h ****/
/* actually taken from embeddedBitmap.h, so its a 2-level copy!*/
typedef enum {
    stretchImageAndMetrics,
    onlyStretchMetrics
} bitmapStretchPreferences;

typedef enum {
    preferMatchToOutlineToBitmap,
    preferMatchToBitmapToOutline,
    preferOutlineToMatchToBitmap,
    onlyUseOutline,
    onlyUseBitmap,
    onlyUseExactBitmap
} bitmapMethodPreferences;

typedef enum {
    outlineWillBeUsed,
    scaledBitmapWillBeUsed,
    bitmapWillBeUsed,
    bitmapSansPointSizeWillBeUsed,
    nothingWillBeUsed
} bitmapMethod;


/* See font scaler for most of these missing values. */
typedef tt_int32 scalerBlockType;
typedef struct{ int x;}  scalerFontInfo;
typedef struct{ int x;}  scalerInfo;
typedef struct{ int x;} scalerVariationInfo,scalerTransformInfo;

typedef struct{ int x;}  scalerKerning,scalerKerningNote,scalerMetrics;

/* Reconstituted from observed usage... */
	#define noImageGlyphFlag 1
typedef struct {
  tt_uint32 bandingTop;
  tt_uint32 bandingBottom;
  tt_uint32 grflags;
	/* tt_uint32 glyphIndex */
} scalerGlyph;
	
/* Reconstituted from observed usage... */
typedef struct {
  Fixed xMin; 
  Fixed yMin; 
  Fixed xMax; 
  Fixed yMax; 
} Bounds;

/* Reconstituted from observed usage... */
typedef struct {
  tt_int32 x; /* must be fixed point */
  tt_int32 y; /* must be fixed point */
} Position;

/* Reconstituted from observed usage... */
typedef struct 
{	
  char 	*image;			 /* Image bit data  */
  tt_uint16 rowBytes;		 /* bytes per row (divisible by 8 ?) */
  Bounds bounds;			 /* limit of used area. */
  Position topLeft;		 
  
  tt_uint32	actualBitmapSize ;
  tt_uint32	scanConvertScratchSize;
  tt_uint32	dropoutScratchSize;

} scalerBitmap;


#ifndef IfDebug
#define IfDebug(a,b)
#endif

#ifndef DebugCode
#define DebugCode(a) 
#endif

#ifndef IfDebugMessage
#define IfDebugMessage(a,b,c)
#endif

#ifndef DebugMessage
#define DebugMessage(b,c)
#endif

typedef short fastInt;

typedef struct   
{
  fixed		map[3][3];
}gxMappingX ;
 


typedef enum {
  applyHintsTransform=1,		/*  Should always be on!*/
  deviceMetricsTransform=2,	/*  Don't know*/
  forceWordScalerTransformFlags= 0x5555555L
} ScalerTransformFlags; 
	
typedef struct{fixed x;fixed y;} SpotSize;
typedef struct{fixed x;fixed y;} Resolution;


typedef struct {
  ScalerTransformFlags flags;	/* See enumerations above*/
  fixed pointSize; 	/* TTHintTran.c: theTrans->pointSize= theTransform->pointSize;*/
  /* could also be a simple integer??*/
	 
  SpotSize spotSize;			/* TTHintTran.c: */
  /* theTrans->pixelDiameter	= */
  /* Magnitude(theTransform->spotSize.x, theTransform->spotSize.y);*/
  /* No way to tell where the fixed point is.*/
  Resolution resolution;		/* See above initialization code.*/
  gxMappingX fontMatrixData;	/* contains the mapping */
  gxMappingX  	      		/* Gx mapping is declared as fixed point, */
  /* But  last column is  fract. */
  *fontMatrixPtr; /* point to the font matrix*/
	 				
} scalerTransform;
	 
#define scaler_hinting_error -1 /* I don't know what it should be.*/

typedef unsigned char boolean;

struct gxPoint {
    Fixed x;
    Fixed y;
};
typedef struct gxPoint  gxPoint;

 
 
struct gxMapping 
{
    Fixed  map[3][3];
};

typedef struct gxMapping gxMapping;

#define IntToFixed(a)	   ((Fixed)(a) << 16)
#define FixedRound(a)		(  (short)   (((Fixed)(a) +  fixed1/2) >> 16) )
#define ff(a)			   IntToFixed(a)

#define local static
#include <string.h>
#include <setjmp.h>

typedef struct {
    tsiMemObject  *aTSIMemObject;  /* MTE Use T2K memory allocation.*/
    sfntClass 	  *aSfntClassFont;		   
    T2K       	  *aT2KScaler; 		   
}  memoryContext; 

/* Read in the includes for  Hinting. */
#include "FntDebug.h"
#include "FSCdefs.h"
#include "TruetypeTypes.h"
#include "FSError.h"  
#include "PinkGlue.h"
 
#include "Fnt.h"
#include "FontMath.h"
#include "PrivateFnt.h"
#include "FntDebug.h"

#include "FntInstructions.h"
#include "PrivateFnt.h"
#include "FixMulDiv.h"
#include "FontMath.h"
#include <string.h>
#include "FSglue.h"
#include "GlyphOutline.h"
#include "InterpreterGlue.h"

/* The scaler context is only fully visible to scaler: the t2k caller would
   normally uses a void * pointer to access the context as a cookie.
*/

#include "HintFont.h"
#include "HintTran.h"
#include "HintGlyph.h"

/* 
	The SWAPWINC function was originally designed to perform byte swapping of font data.
	However, the T2K system byte swaps the data as part of the readin process. 
	Therefore, no additional swapping is performed by SWAPWINC
*/
#define SWAPWINC(data) ((*data++))

#include "ScalerMatrix.h"

#endif
















