/*
 * @(#)OrientDB.h	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef __T2K_OrientDB__
#define __T2K_OrientDB__

#include "syshead.h"
#include "config.h"
#include "dtypes.h"
#include "tsimem.h"

/* Usage within StyleFuncPost routines:											*/
/* The ContourData parameter provides information on the topology of			*/
/* each TrueType glyph contour (curveType==2); no information is provided		*/
/*			provided for PostScript, but PostScript uses standards, and can		*/
/*			be computed on the fly.												*/

/* For each contour, there are two types of information: local orientation		*/
/*		and global orientation. */						     

/*  In general, fonts are designed so that the black bits are to the right		*/
/*		of the direction of the contour. Global Orientation is 1 when this		*/
/* 		is false, or 0, when standard. Global orientation applies to a specific	*/
/*		contour, but to evaluate the black bits, all contours are considered,	*/
/*		and that is why it is called "global".									*/
/*																				*/
/*	In general a curve (all curves are closed) travels either clockwise or		*/
/*		or counter-clockwise when traveled in the direction of increasing point	*/
/*		indices. Local Orientation is 1 if counter-clockwise, or 0 if clockwise.*/
/*		There is a default rule for local orientation. If clockwise (local==0), */
/* 		then the outline is an exterior boundary.								*/
/*		Otherwise, the outline is an interior boundary (such as the inner		*/
/*		outline of the letter "o").												*/
/*																				*/
/*	By default, the global orientation is zero.									*/
/*		Then the local orientation, specifies interior or exterior.				*/
/*		The bolding algorithm uses the local information to control				*/
/*		the amount of bolding. Interior curves are given less bolding			*/
/*		in order to avoid closing or eliminating an interior hole.				*/
/*		For example, the letter "e" has a little hole which could completely	*/
/*		disappear if full bolding closes it.									*/
/*																				*/
/*	In the special case where the global orientation is 1, then the black bits	*/
/*		are on the "incorrect" side. This will typically change a bolding		*/
/*		algorithm. See routine tsi_SHAPET_BOLD_GLYPH_Hinted,					*/
/*		in "shapet.c", for an example.											*/
/*																				*/
/* For TrueType fonts, the local and global information is passed in structure, */
/*		the ContourData structure.												*/
/* For example, consider "cd" a pointer to the contour data.					*/
/* The call: 																	*/
/*																				*/
/*		GetContourDataSet(cd,ctr+cdIndexOffset, &localOrient, &globalOrient);	*/
/*  																			*/
/*	returns the local and global flags.											*/
/*																				*/
/*	For PostScript (curveType==3), the global orientation is assumed to be 0.	*/
/*		And to find the local orientation, call the routine 					*/
/* 			"FindNonZeroWindingCounts", see the same routine,					*/
/*		"tsi_SHAPET_BOLD_GLYPH_Hinted", in "shapet.c".							*/
  


/* ORIENTBOLD_STYLES */
/* This module provides some testing tools. In particular, if the 		*/
/* macro is turned on, then the following two routines are called		*/
/* when unusual situations occur. For production, leave the macro		*/
/* undefined.															*/
/* Although the code is never turned on except in the test environment,	*/
/* 		before releasing the code should be compiled without 			*/
/*		the ORIENTATIONTEST, to make sure it compiles correctly.		*/

#ifdef UIDebug 
#if 0
#define ORIENTATIONTEST
#endif
#endif

#define LOCALORIENTATIONBIT 	1
#define GLOBALORIENTATIONBIT 	2
/* The rollowing states are used analysis. */
#define ORIENTATIONSET			4
#define ORIENTATIONREAD			8
 
typedef struct
{		
	tt_int32 initializedContour;	/* if true, then maxprofile is setup. */
	tt_int32 active;				/* non-zero if processing.			  */
	tt_int32 current;				/* a position indicator. used only by */
								/* code which uses this data.		  */
	tt_int32 numContours;
 	tt_uint8 *ContourDataArray;
} ContourData;
 

tt_int32 BadOrientationState(void);	
tt_int32 InitContourData( 
	tsiMemObject *mem, tt_int32 numContours,ContourData *cd);
tt_int32 InitContourDataEmpty(ContourData *cd);
void InitializeDefaultContourData(ContourData *cd);
void VerifyContourUsage(ContourData *cd);
void ReleaseContourData(tsiMemObject *mem, ContourData *cd);

void SetContourDataSet(
	ContourData *cd,tt_int32 contourIndex, tt_int32 localFlag, tt_int32 globalFlag );
void SetContourDataSetQuick(
	ContourData *cd,tt_int32 contourIndex, tt_int32 localFlag, tt_int32 globalFlag );
void GetContourDataSet(
	ContourData *cd,tt_int32 contourIndex,tt_int32 *localFlag, tt_int32 *globalFlag);
	/* Same as GetContourDataSet, but no testing. */
void GetContourDataSetQuick(
	ContourData *cd,tt_int32 contourIndex,tt_int32 *localFlag, tt_int32 *globalFlag);
void FlipContourDataList(
	ContourData *cd, tt_int32 startIndex, tt_int32 endIndex);

#endif










