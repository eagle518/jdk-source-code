/*
 * @(#)OrientDB.c	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


/* OrientDB: this module is used to save orientation flags for			*/
/* each contour.														*/
 
#include "OrientDB.h"

/* This is a general purpose catchall when the program detects an		*/
/* error in the contour orientation state.								*/
/* Set a break point to catch these errors. 							*/
tt_int32 BadOrientationState(void)
{
	return(0);
}	

/* Return the total number of words (32 bits) required to store all		*/
/*  of the orientation information.										*/
static tt_int32 NeededContourWords( tt_int32 numContours )
{
	if (numContours==0)
		numContours=1; /* always at least one. */
	return(numContours+3)/4;
}
	
/* The number of words to be allocated is nominally 0, but for			*/
/* fonts with a lot of contours then some must be allocated.			*/
static tt_int32 NeededAllocationContourBytes( tt_int32 numContours )
{
 	int contourWords= NeededContourWords(numContours );
 	return(contourWords*4);
}

/* Before calling this routine, the "NeededAllocationContourWords"		*/
/*  routine is called. If the byte count is non-zero, then the 			*/
/*	appropriate memory allocation must be passed to this routine via	*/
/*	the "addr" variable. If the needed byte count is zero, then			*/
/* call this routine with "addr" set to zero.							*/
/* return 0 for success, 1 for error. This routine is always successful,*/
/* unless the memory address was incorrect.								*/

static tt_int32  SetupAllocatedContourData( 
	void *addr, tt_int32 numContours, ContourData *cd)
 {
  	tt_int32 i;
  	cd->initializedContour=0;
  	cd->active=0; /* When zero, no actions are taken. */
  	cd->numContours = numContours;
   	cd->ContourDataArray= (tt_uint8 *)addr;
  	cd->initializedContour=1; /* When zero, no actions are taken. */
  	return(0);
 }
 
 /* The usual way to initialize a ContourData structure is to 			*/
 /* allocate the structure, and then call this routine to complete		*/
 /* the process. This routine will perform the necessary memory			*/
 /* allocation: if it fails, then the normal exception handling is		*/
 /* utilizied.															*/

tt_int32 InitContourData( 
	tsiMemObject *mem, tt_int32 numContours,ContourData *cd)
 {
 	tt_int32 numBytesNeeded=NeededAllocationContourBytes(numContours );
 	void *addr=0;
 	tt_int32 result;
 	addr= (void *) tsi_AllocMem( mem, numBytesNeeded);
 	result= SetupAllocatedContourData(addr, numContours, cd);
  	return(0);
 }

/* If the font (ie. PostScript always uses standard orientation, then	*/
/* no extra work is required.											*/ 
tt_int32 InitContourDataEmpty(ContourData *cd)
 {
    cd->initializedContour=0;
  	cd->active=0; /* When zero, no actions are taken. */
  	cd->numContours = 0;
   	cd->ContourDataArray= 0;
  	cd->initializedContour=0; /* When zero, no actions are taken. */
  	return(0);
 }

/* After a cd is setup (see InitContourData), it can be initialized		*/
/*  to the default data, then the Set and Get routines are used.		*/
/* The process assumes that each contour is set once and "getted" once.	*/
/*	Upon completion, the verify routine is called to make sure that	*/
/*	every contour was read at  once.								  */
void InitializeDefaultContourData(ContourData *cd) 
{
	tt_int32 i;
	if(cd->initializedContour)
		for(i=0;i<cd->numContours;i++) 
 			cd->ContourDataArray[i]=0;
}
void VerifyContourUsage(ContourData *cd) 
{
	tt_int32 i;
#ifdef ORIENTATIONTEST
	if(cd->initializedContour)
		for(i=0;i<cd->numContours;i++) 
 			if (
 					(
 						 (cd->ContourDataArray[i]& (ORIENTATIONSET+ORIENTATIONREAD))
 						!= (ORIENTATIONSET+ORIENTATIONREAD)
 					)
 					&&
 					( cd->ContourDataArray[i]!=0)
 				)
 				BadOrientationState( );	
#endif
}

/* When the contour allocation is no longer needed, it can be released.*/
void ReleaseContourData(tsiMemObject *mem, ContourData *cd)
{
	if(cd->initializedContour)
 		if ( cd->ContourDataArray) {
 				tsi_DeAllocMem( mem, (void *) cd->ContourDataArray );
 				cd->ContourDataArray =0;
 			}
}

/* The contour data  consists of two bits: local and global orientation	*/		 		
void SetContourDataSet(
	ContourData *cd,tt_int32 contourIndex, tt_int32 localFlag, tt_int32 globalFlag )
{
  	tt_uint8 setupBits;
  	if( cd->initializedContour==0)
  		return;
	if( (contourIndex<cd->numContours))
	{
  		setupBits= 
			(localFlag?LOCALORIENTATIONBIT:0) 
			+
			(globalFlag?GLOBALORIENTATIONBIT:0)
			+ ORIENTATIONSET;/* write once.*/
#ifdef ORIENTATIONTEST
		/* Perform a lot of tests... */
		if ( 
			(contourIndex>=cd->numContours)
				||
			(contourIndex<0)
				||
			(cd->ContourDataArray[contourIndex] &
					(ORIENTATIONSET | ORIENTATIONREAD))
			)
				BadOrientationState();
#endif
 	 	cd->ContourDataArray[contourIndex]=setupBits;
 	}
 	else
 		BadOrientationState( );
 }	
 /* The contour data  consists of two bits: local and global orientation	*/		 		
void SetContourDataSetQuick(
	ContourData *cd,tt_int32 contourIndex, tt_int32 localFlag, tt_int32 globalFlag )
{
  	tt_uint8 setupBits;
  	if( cd->initializedContour==0)
  		return;
	if( (contourIndex<cd->numContours))
	{
  		setupBits= 
			(localFlag?LOCALORIENTATIONBIT:0) 
			+
			(globalFlag?GLOBALORIENTATIONBIT:0);
			/*+ ORIENTATIONSET;  DONT SETIT IN QUICK MODE*/
  	 	cd->ContourDataArray[contourIndex]=setupBits;
 	}
 	else
 		BadOrientationState( );
 }	
	
/* the local and global flags can be returned. */	 		
void GetContourDataSet(
	ContourData *cd,tt_int32 contourIndex,tt_int32 *localFlag, tt_int32 *globalFlag)
{
 	tt_uint8 setupBits;
 	if( cd->initializedContour==0)
  		{
 			*localFlag=  0;
			*globalFlag= 0;
			return;
		}
	if( (contourIndex<cd->numContours))
	{
#ifdef ORIENTATIONTEST
		/* Perform a lot of tests... */
		if ( 
			(contourIndex>=cd->numContours)
				||
			(contourIndex<0)
				||
			(! (cd->ContourDataArray[contourIndex]&ORIENTATIONSET))
				||
			(cd->ContourDataArray[contourIndex]&ORIENTATIONREAD)	
 		  )
				BadOrientationState();
		 cd->ContourDataArray[contourIndex]|=ORIENTATIONREAD; /* read once.*/
#endif
 		setupBits= cd->ContourDataArray[contourIndex];
		*localFlag= (setupBits&LOCALORIENTATIONBIT)?1:0;
		*globalFlag= (setupBits&GLOBALORIENTATIONBIT)?1:0;
	}
	else
	{	/* this should not happen. */
		*localFlag=  0;
		*globalFlag= 0;
		 BadOrientationState( );
	}
}		

void GetContourDataSetQuick(
	ContourData *cd,tt_int32 contourIndex,tt_int32 *localFlag, tt_int32 *globalFlag)
{
 	tt_uint8 setupBits;
 	if( cd->initializedContour==0)
  		{
 			*localFlag=  0;
			*globalFlag= 0;
			return;
		}
	if((contourIndex<cd->numContours))
	{
#ifdef ORIENTATIONTEST
		/* Perform a lot of tests... */
		if ( 
			(contourIndex>=cd->numContours)
				||
			(contourIndex<0)
				||
			(! (cd->ContourDataArray[contourIndex]&ORIENTATIONSET))
				||
			(cd->ContourDataArray[contourIndex]&ORIENTATIONREAD)	
 		  )
				BadOrientationState();
 #endif
 		setupBits= cd->ContourDataArray[contourIndex];
		*localFlag= (setupBits&LOCALORIENTATIONBIT)?1:0;
		*globalFlag= (setupBits&GLOBALORIENTATIONBIT)?1:0;
	}
	else
	{	/* this should not happen. */
		*localFlag=  0;
		*globalFlag= 0;
		 BadOrientationState( );
	}
}		
 

/* It sometimes happens that the orientation is flipped, as in 		*/
/*		any affine flip which changes the orientation (like	a 		*/
/*		mirror image). The values of both types of orientation are	*/
/*		flipped.													*/ 		
void FlipContourDataList(
	ContourData *cd, tt_int32 startIndex, tt_int32 endIndex)
{
	tt_int32 localFlag,  globalFlag,  index;
	if( cd->initializedContour)
	{
		for (index= startIndex; index<=endIndex;index++)
		{
			 GetContourDataSetQuick( cd, index,&localFlag,&globalFlag);
			 SetContourDataSetQuick( cd, index, localFlag?0:1, globalFlag?0:1);
		}
	}
 }		
		
 




	 		
 
