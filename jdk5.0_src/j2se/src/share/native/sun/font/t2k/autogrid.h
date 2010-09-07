/*
 * @(#)autogrid.h	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * File:		AUTOGRID.H
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
#ifndef __T2K_AUTOGRID__
#define __T2K_AUTOGRID__

#include "dtypes.h"

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */


/***********************************************/
/** BEGIN EXTERNAL API data types and defines **/
#define IN
#define OUT
#define INOUT

/***********   CVT    *****************/
/****  Double heights (round+flat) ****/

/*
 * These only have the natural conceptual meaning
 * for the (fontType == ag_ROMAN) case
 */
#define ag_ASCENDER_HEIGHT		0
#define ag_CAP_HEIGHT			1
#define ag_FIGURE_HEIGHT		2
#define ag_X_HEIGHT				3
#define ag_UC_BASE_HEIGHT		4
#define ag_LC_BASE_HEIGHT		5
#define ag_FIGURE_BASE_HEIGHT	6
#define ag_DESCENDER_HEIGHT		7
#define ag_PARENTHESES_TOP		8
#define ag_PARENTHESES_BOTTOM	9
#define ag_MAX_HEIGHTS_IN		10


typedef struct {
	short 	flat;		/* set to actual flat height */
	short	round;		/* set to actual round height */
	short	overLap;	/* set to round - flat. This means that this value is normally */
						/* positive for heights at the top of characters, but negative */
						/* at the bottom of characters. */
						/* If the absolute of the overlaps on top and bottom are almost identical it */
						/* is a VERY good idea making their absolute values exactly identical. */
} ag_HeightType;


/* CW8 does not take this ?!! Why?, works OK in CW 7 */
/* enum ag_FontCategory { ag_ROMAN, ag_KANJI }; */
/* so for now we do this */
typedef int ag_FontCategory;
#define ag_ROMAN 1
#define ag_KANJI 2
#define ag_ROMAN_MONOSPACED 3

#define ag_MAXWEIGHTS		12
   
typedef struct {
	/* PUBLIC, INPUT: set by the caller */
	/* These are common heights */
	ag_HeightType	heights[ag_MAX_HEIGHTS_IN]; /* set unused heights to zero */
	/* These are common weights, at least xWeight[0], and yWeight[0] have to be set */
	short xWeight[ag_MAXWEIGHTS]; /* xWeight[0] is the ancestral x-weight, set unused weights to zero */
	short yWeight[ag_MAXWEIGHTS]; /* yWeight[0] is the ancestral y-weight, set unused weights to zero */
} ag_GlobalDataType;

typedef struct {
	unsigned short maxZones;
	unsigned short maxTwilightPoints;
	unsigned short maxStorage;
	unsigned short maxFunctionDefs;
	unsigned short maxInstructionDefs;
	unsigned short maxStackElements;
	unsigned short maxSizeOfInstructions;
} ag_HintMaxInfoType;


typedef struct {
	/* PUBLIC, INPUT: set by the caller and allocated by the caller */
	short	contourCount; /* number of contours in the character */
	short 	pointCount;   /* number of points in the characters + two for the sidebearing points */
	short	*sp;  		/* sp[contourCount] Start points */
	short	*ep;  		/* ep[contourCount] End points */
	short	*oox;		/* oox[pointCount] Unscaled Unhinted Points, add two extra points for lsb, and rsb */
	short	*ooy;		/* ooy[pointCount] Unscaled Unhinted Points, set y to zero for the two extra points */
						/* Do NOT include the two extra points in sp[], ep[], contourCount */
						/* Do include the two extra points in pointCount */
	unsigned char *onCurve;	/* onCurve[pointCount] indicates if a point is on or off the curve, it should be true or false */
	/* PUBLIC, OUTPUT: not set by caller. However this is read and allocated by the caller */
	tt_int32	*x;			/* x[pointCount]Scaled auto-gridded Points, in 26.6 bit format */
	tt_int32	*y;			/* y[pointCount]Scaled auto-gridded Points, in 26.6 bit format  */
	tt_int32	advanceWidth26Dot6;
	tt_int32    	advanceWidthInt;
} ag_ElementType;

typedef void *ag_HintHandleType;

/*** END EXTERNAL API data types and defines ***/
/***********************************************/



/***********************************************/
/*** BEGIN EXTERNAL API function entry points **/

/*
 * Description:		Call this first
 * 					This returns the hintHandle which is passed to all the other external calls.
 * 					In a multi-threaded environment each thread needs to have it's own hintHandle.
 * How used:		Call with the maximum point count for the font, the units per Em,
 *					and a pointer (hintHandle) to ag_HintHandleType.
 * Side Effects: 	Sets *hintHandle.
 * Return value: 	Returns < 0 if an error was encountered and 0 otherwise.
 */
extern int ag_HintInit( IN tsiMemObject *mem, IN int maxPointCount, IN short unitsPerEm, OUT ag_HintHandleType *hintHandle );

/*
 * Description:		Call this to set global hint information.
 * 					Always, set at least one x and one y weight.
 * 					Set any unused heights and weights to zero.
 * 					Call this once, before calling ag_AutoGridOutline(), or ag_AutoHintOutline()
 * How used:		Call with the hintHandle, a pointer to ag_GlobalDataType, and the font type.
 * Side Effects: 	Stores the passed in global hint information and the fontType within the private ag_DataType data structure.
 * Return value: 	Returns < 0 if an error was encountered and 0 otherwise.
 */
extern int ag_SetHintInfo( IN ag_HintHandleType hintHandle, IN ag_GlobalDataType *gData, ag_FontCategory fontType );

/*
 * Description:		Always call this when the size changes, but not otherwise
 * 					A call to AutoGridOutline() is invalid without any previous call to SetScale()
 * 					However if the scale stays constant then multiple calls to AutoGridOutline(), or AutoHintOutline()
 * 					is OK as long as SetScale() was called once with the correct scale first.
 * 					For speed reasons do not call SetScale() more often than you have to.
 * How used:		Call with the hintHandle, and the x and y pixels per Em.
 * Side Effects: 	Remembers the x and y pixels per Em and sets up the internal cvt information.
 * Return value: 	Returns < 0 if an error was encountered and 0 otherwise.
 *					*xWeightIsOne = xWeight == onePix
 */
extern int ag_SetScale( IN ag_HintHandleType hintHandle, IN tt_int32 xPixelsPerEm, IN tt_int32 yPixelsPerEm, char *xWeightIsOne );

#ifdef ENABLE_AUTO_HINTING
/*
 * Description:		The caller should copy the 3 tables directly into the 3 corresponding TrueType tables
 * 					This function is only called when hints are desired. Not when we are just auto-gridding.
 * 					The caller HAS to deallocate all three memory pointers.
 * How used:		Call with the hintHandle, and three pairs of ((char **) and (tt_int32*)) for the font program,
 *					pre program and control value table.
 * Side Effects: 	None.
 * Return value: 	Returns < 0 if an error was encountered and 0 otherwise.
 */
extern int ag_GetGlobalHints( IN ag_HintHandleType hintHandle,
					   		OUT unsigned char **fpgm, OUT tt_int32 *fpgmLength,
					   		OUT unsigned char **ppgm, OUT tt_int32 *ppgmLength,
					   		OUT short **cvt,          OUT tt_int32 *cvtCount );
					   		
#endif /* ENABLE_AUTO_HINTING */		   		

#ifdef ENABLE_AUTO_GRIDDING
/*
 * Description:		This function auto grids the glyph stored within ag_ElementType.
 * How used:		Call with the hintHandle, a pointer to ag_ElementType,
 *					the isFigure boolean (isFigure should be set to true for figures 0..9, and false otherwise),
 *					the curve type (set curveType to 2 for TT and 3 for T1 )
 * Side Effects: 	ag_DataType and ag_ElementType.
 * Return value: 	Returns < 0 if an error was encountered and 0 otherwise.
 */
extern int ag_AutoGridOutline( IN ag_HintHandleType hintHandle, INOUT ag_ElementType *elem, IN short isFigure, IN short curveType, IN short grayScale );
#endif /* ENABLE_AUTO_GRIDDING */

#ifdef ENABLE_AUTO_HINTING
/*
 * Description:		This function auto hints the glyph stored within ag_ElementType.
 * 					The caller HAS to deallocate *hintFragment
 *					This can be called instead of AutoGridOutline() when the client needs
 *					the TT hints instead of the auto-gridded outline.
 * How used:		Call with the hintHandle, a pointer to ag_ElementType,
 *					the isFigure boolean (isFigure should be set to true for figures 0..9, and false otherwise),
 *					the curve type (set curveType to 2 for TT and 3 for T1 ),
 *					and char **hintFragment, and tt_int32 *hintLength for the TrueType glyph hint data.
 * Side Effects: 	ag_DataType, hintFragment, hintLength.
 * Return value: 	Returns < 0 if an error was encountered and 0 otherwise.
 */
extern int ag_AutoHintOutline( IN ag_HintHandleType hintHandle, IN ag_ElementType *elem, IN short isFigure, IN short curveType, OUT unsigned char *hintFragment[], OUT tt_int32 *hintLength );
#endif /* ENABLE_AUTO_HINTING */				   		

/*
 * Description:		This function can be used, for analyzing strokes for the purpose of
 * 					finding global stem-weights. It sets the content of the xDist[], and yDist[] arrays
 * 					It also sets *xDistCount, and *yDistCount to the number of links found in the x and y directions.
 * 					The caller HAS to deallocate *xDist and *yDist
 * How used:		Call with the hintHandle, a pointer to ag_ElementType,
 *					the isFigure boolean (isFigure should be set to true for figures 0..9, and false otherwise),
 *					the curve type (set curveType to 2 for TT and 3 for T1 )
 *					and two pairs of (short **, and tt_int32 *) for the x and y stem-weight data.
 * Side Effects: 	xDist[], and yDist[], and *xDistCount, *yDistCount.
 * Return value: 	Returns < 0 if an error was encountered and 0 otherwise.
 */
extern int ag_AutoFindStems( IN ag_HintHandleType hintHandle, IN ag_ElementType *elem, IN short isFigure, IN short curveType, OUT short *xDist[], OUT tt_int32 *xDistCount, OUT short *yDist[], OUT tt_int32 *yDistCount ); 

/*
 * Description:		Call this to get the data for the ag_HintMaxInfoType structure
 * 					This should be called after ALL the glyphs have been hinted.
 * How used:		Call with the hintHandle and pointer to a ag_HintMaxInfoType
 *					data structure.
 * Side Effects: 	Sets the contents of the ag_HintMaxInfoType data structure.
 * Return value: 	Returns < 0 if an error was encountered and 0 otherwise.
 */
extern int ag_GetHintMaxInfo( IN ag_HintHandleType hintHandle, OUT ag_HintMaxInfoType *p );

/*
 * Description:		Call this to cause the auto-grider to de-allocate it's memory
 * 					HintInit() and HintEnd() form a matching pair
 * How used:		Call with the hintHandle.
 * Side Effects: 	De-allocates internal memory within the ag_DataType data-structure.
 * Return value: 	Returns < 0 if an error was encountered and 0 otherwise.
 */
extern int ag_HintEnd( IN ag_HintHandleType hintHandle );

/**** END EXTERNAL API function entry points ***/
/***********************************************/


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif	/* __T2K_AUTOGRID__ */

