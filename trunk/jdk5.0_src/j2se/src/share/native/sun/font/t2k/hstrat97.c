/*
 * @(#)hstrat97.c	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * File:		HSTRAT97.C
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
#include "syshead.h"

#include "config.h"
#include "dtypes.h"
#include "tsimem.h"
#include "autogrid.h"
#include "agridint.h"
 
#define STRATEGY_97
#ifdef STRATEGY_97


tt_int32 absPixelError = 0;
tt_int32 relPixelError = 0;
tt_int32 totEvals	    = 0;

 
#define Z_DELETE_FLAG		0x0001
#define Z_DONE_FLAG 		0x0002
#define Z_HEIGHT_FLAG	    0x0004

#ifdef OLD
	#define Z_BLACK_FLAG		0x0008
	#define Z_LGRAY_FLAG		0x0010 /* left of stem */
	#define Z_RGRAY_FLAG		0x0020 /* right of stem */
	#define Z_WHITE_FLAG		0x0040
	#define Z_INSIDE_FLAG		0x0080
#endif

typedef struct {
	tt_int16 ptA;
	tt_int16 ptB;
	tt_int16 cvtNumber;
	tt_int16 lineA; /* equal to this line */
	tt_int16 lineB;
	tt_int16 lineC; /* Is set to the index of the guy with lineB equal to this object */
	tt_uint16 flags;
} hintLine;



#define COMPARE_LT( ooz, A, B ) (ooz[A] < ooz[B])
#define COMPARE_GT( ooz, A, B ) (ooz[A] > ooz[B])
#define COMPARE_EQ( ooz, A, B ) (ooz[A] == ooz[B])
#define COMPARE_NEQ( ooz, A, B ) (ooz[A] != ooz[B])

static void downHeap( hintLine **Array, int first, int last, tt_int16 *ooz )
{
	int i, child, parent;
	hintLine *temp;

	/* Enforce heap condition betwen first and last */
	/* Swap the parent node with its largest child until the bottom of the tree is reached */
	for ( i = first; (child = i+i+1) <= last; i = child ) {
		if ( child + 1 <= last && COMPARE_GT( ooz, Array[child+1]->ptA, Array[child]->ptA ) ) {
			child++;
		}
		/* child is the largest child of i */
		temp 			= Array[i];
		Array[i] 		= Array[child];
		Array[child]	= temp;
	}
	/* Walk the item in question back up the tree until that item is in place */
	for(;;) {
		parent = (i-1)>>1;
		
		if ( parent < first || parent == i || COMPARE_GT( ooz, Array[parent]->ptA, Array[i]->ptA ) ) break; /*****/
		temp			= Array[i];
		Array[i]		= Array[parent];
		Array[parent]	= temp;
		i 				= parent;
	}
}

/*
 * Sorts from 0 to N-1
 */
static void HeapSort( hintLine **Array, int N, tt_int16 *ooz )
{
	int i;
	hintLine *temp;


	N--; /* Set N equal to largest Index */
	
	/* Ensure heap property for array */
	for ( i = (N-1)/2; i >= 0; i-- ) {
		downHeap( Array, i, N, ooz );
	}
	for ( i = N; i > 0; ) {
		temp 		= Array[i];
		Array[i]	= Array[0]; /* Copy the biggest one, into the last position */
		Array[0]	= temp;
		downHeap( Array, 0, --i, ooz );
	}
	/*
	for ( i = 0; i < N; i++ ) {
		assert( (ooz[Array[ i ]->ptA] <= ooz[Array[ i + 1 ]->ptA]) );
	}
	*/
#ifdef OLD
	printf("*****\n");
	for ( i = 0; i <= N; i++ ) {
		printf("%ld ; %ld\n", (tt_int32)Array[ i ]->ptA, (tt_int32)ooz[Array[ i ]->ptA] );
	}
	printf("*****\n");
#endif
}


/* static FILE *log_fp = NULL; */


#ifdef OLDOLD

#define pow1(x) (x)
#define pow2(x) ((x)*(x))
#define pow3(x) ((x)*(x)*(x))
#define round(a) (int)((a<0)?((a)-.5):((a)+.5))  /* nearest int */
#define LIMIT(v,mn,mx) ((v>(mx))?(mx):((v<(mn))?(mn):v))


/*
 * Divides the 3 dimensional space into a Complex == Kanji, and a
 * non-complex class.
 */
static int isComplexFunc(int contourCoun,int lastPoint,int numExtrema)
{
	double node1   ;  /* working variable */
	double node2   ;  /* working variable */
	double node3   ;  /* working variable */
	double node5   ;  /* working variable */
	int    isComplexType;      /* output variable */

	/* node1 -- contourCoun */
	node1    = -1.22729 + 0.346177  * LIMIT((double)contourCoun,1,16);

	/* node2 -- lastPoint */
	node2    = -1.34061 + 0.0175805 * LIMIT((double)lastPoint,3,303);

	/* node3 -- numExtrema */
	node3    = -1.2401 +  0.0794337 * LIMIT((double)numExtrema,2,62);

	/* node5 -- Triple */
	node5    = 0 + 0.546357 + 0.355424*(double)node1 
	            - 0.122533*(double)node2 + 0.91759*(double)node3 
	            - 0.299488*pow2((double)node1) - 0.136898*pow2((double)node2) 
	            - 0.668702*pow2((double)node3)
	             + 0.0610883*(double)node1*(double)node2 
	            + 0.0978108*(double)node1*(double)node3 
	            + 0.361687*(double)node2*(double)node3 
	            - 0.076096*(double)node1*(double)node2*(double)node3 
	            + 0.0556557*pow3((double)node1) + 0.00358536*pow3((double)node2) 
	            + 0.0845124*pow3((double)node3) ;

	/* node4 -- isComplex */
	isComplexType = round(  0 + 0.503937 + 0.500108*node5  );

	/* perform output limiting on isComplex */
	isComplexType = LIMIT( isComplexType, 0, 1 );

	return isComplexType; /*****/
}

#endif /* OLDOLD */

/*
 * Divides the 3 dimensional space into a Complex == Kanji, and a
 * non-complex class.
 */
static int isComplexChar(int contourCount,int lastPoint,int numExtrema)
{
	/* Working variables scaled by a factor of 1024 */
	register tt_int32 node1, node2, node3; 
	tt_int32 result, node1pow2, node2pow2, node3pow2, node12; 

	/* node1 -- contourCount */
	if ( contourCount > 16 ) contourCount = 16; /* valid 1 .. 16 */
	node1    = -1257 + 354 * contourCount; /* -0.881113  -- 4.311542 */

	/* node2 -- lastPoint */
	if ( lastPoint > 303 ) lastPoint = 303; /* valid 3 .. 303 */
	node2    = -1372 + 18  * lastPoint; /* -1.2878 -- 3.9876 */

	/* node3 -- numExtrema */
	numExtrema &= 63; /* valid 2 .. 62 (63) */
	node3    = -1270 + 81  * numExtrema; /*  -1.081 -- 3.6848 */
	
	node1pow2 = node1 * node1 >> 10;
	node2pow2 = node2 * node2 >> 10;
	node3pow2 = node3 * node3 >> 10;
	node12    = node1 * node2 >> 10;

	/* result -- Triple */
	result    =    (559 + 8)
	
				+ (364 * node1 >> 10)
	            - (      node2 >>  3) /* * 128 >> 10 == >> 3 */
	            + (940 * node3 >> 10)
	            
	            - (307 * node1pow2 >> 10)
	            - (140 * node2pow2 >> 10) 
	            - (685 * node3pow2 >> 10)
	            
	            + (        node12 				 	>>  4) /* * 64 >> 10 == >> 4 */
	            + (100  * (node1 * node3 >> 10) 	>> 10)
	            + (370  * (node2 * node3 >> 10)	 	>> 10)
	            
	            - ( 78  * (node12 * node3 >>10)	>> 10)
	            
	            + ( 57  * (node1pow2 * node1 >> 10) >> 10)
	            + (       (node2pow2 * node2      ) >> 18)
	            + ( 87  * (node3pow2 * node3 >> 10) >> 10);
	/* Added 6/17/98 to avoid categorizing the B, and g as complex in MSMINCHO.TTC */
	/* result, contourCount, lastPoint, numExtrema (311,3,86,14), (46,3,48,10) */
	if ( result > 0 && result < 333 && contourCount <= 3 && numExtrema <= 14 && lastPoint <= 86 && contourCount*5 > numExtrema  ) {
		result = 0;
	}

	return result > 0; /*****/
}


/*
 * Description:		This is the NEW function that does the glyph grid-fitting
 * 					or hinting.
 * Algorithm:		Under Construction!
 * Philosophy		Robust, predictable, small distorsion.
 * How used:		Call with pointers to ag_ElementType and ag_DataType.
 * Side Effects: 	ag_ElementType and ag_DataType.
 * Return value: 	0 if no errors encountered, and < 0 otherwise
 */
int ag_DoGlyphProgram97( ag_ElementType* elem, ag_DataType *hData )
{
	register tt_int32 i, j, k, tmp32;
	tt_uint16 doneBit, extrema, slope, importantBit;
	tt_int16 cvtNumber;
	int maxMaxW;
	tt_int16 doX, doY;
	int lastPoint, lastPointPlus2;
	int errorCode = 0;
	tt_uint8 direction;
	tt_int16 *ooz1; /* is equal to either hData->oox, or hData->ooy */
	tt_int16 *ooz2; /* is equal to either hData->oox, or hData->ooy */
	tt_int32 max_i, min_i;
	int isComplex = false;
	
	register tt_uint16 *flags = hData->flags;
#if 0	/* unused */
	int limit = ONE_PERCENT_OF_THE_EM+1; /* 1% + 1 fUnit */
#endif
int limit1000 = ONE_PERCENT_OF_THE_EM/10+1; /* 1%/10 + 1 fUnit */
	int limit3 = ONE_PERCENT_OF_THE_EM*3+1; /* 3% + 1 fUnit */
	
	tt_int32 maxHintLines, zCount;
	hintLine *zData;	/* zData[ maxHintLines ]	*/
	hintLine **Array;	/* *Array[ maxHintLines ]	*/
	tt_int16 *pointToLineMap;
	tt_int16 ptA, ptB;
	int verbose;
	

	pointToLineMap = hData->pointToLineMap;
	lastPoint = hData->endPoint[ hData->numberOfContours-1 ];
	lastPointPlus2 = lastPoint + 2;
	/* Initialize to default TrueType interpreter state */
	hData->RP0 = hData->RP1 = hData->RP2 = 0;
	hData->inX = true;
	hData->inY = false;
	
	maxHintLines = 2*(lastPoint + 1) + 8 + 8;
	
	zData = (hintLine *)tsi_AllocMem( hData->mem, maxHintLines * sizeof(hintLine) );
	Array = (hintLine **)tsi_AllocMem( hData->mem, maxHintLines * sizeof(hintLine *) );
	
/* #define EXTRA_VERBOSE */

	/* First do x and then do y */
	for ( j = 0; j <  2; j++ ) {
		doX = doY = false;
		if ( j == 0 ) {
			/* X-axis */
			verbose = false;
#ifdef ENABLE_AUTO_HINTING
			assert( &hData->ttDataBase[hData->ttDataBaseMaxElements-1] == hData->ttdata );
#endif
			ag_SVTCA_X( hData );
			if ( hData->isFigure || hData->fontType != ag_ROMAN  ) {
				ptA = (short)(lastPoint+1);
				ptB = (short)(lastPoint+2);
				ag_MDAPX( hData, elem, true, ptA );
				ag_MDRPX( hData, elem, -1, false, 0, true, 'G', 'r', ptA, ptB );
			}
						
			doX 		 = true;
			maxMaxW 	 = (short)(ag_GetXMaxCvtVal( hData ) * 2); /* Just a heuristic maximum weight */
			cvtNumber	 = GLOBALNORMSTROKEINDEX;
			doneBit 	 = X_TOUCHED;
			importantBit = X_IMPORTANT;
			extrema		 = XEX;
			slope		 = IN_YF | IN_YB;
			direction	 = INC_XDIR;
			ooz1		 = hData->oox;
			ooz2		 = hData->ooy;
#ifdef EXTRA_VERBOSE
	if ( hData->numberOfContours == 2 && hData->endPoint[0] == 11 && lastPoint == 35 && 
	     hData->ooy[8] == hData->ooy[10] && hData->ooy[0] > hData->ooy[5]) {
		verbose = true;
		printf("***** Hello *****\n");
	}
#endif
		} else {
			/* Y-axis */
			verbose = false;
			ag_SVTCA_Y( hData );
			doY 		 = true;
			maxMaxW		 = (short)(ag_GetXMaxCvtVal( hData ) + ag_GetYMaxCvtVal( hData )); /* Just a heuristic maximum weight */
			cvtNumber	 = GLOBALNORMSTROKEINDEX + ag_MAXWEIGHTS;
			doneBit		 = Y_TOUCHED;
			importantBit = Y_IMPORTANT;
			extrema		 = YEX;
			slope		 = IN_XF | IN_XB;
			direction	 = INC_YDIR;
			ooz1		 = hData->ooy;
			ooz2		 = hData->oox;
		}
		zCount = 0;
		for ( i = 0; i < hData->linkCount; i++ ) {
			if ( hData->links[i].type == INC_LINK && hData->links[i].direction == direction ) {
				tt_int32 dx1, dy1;

				Array[ zCount ] = &zData[ zCount ];
				zData[ zCount ].ptA = ptA = hData->links[i].from;
				ptB = hData->links[i].to;
				
				dx1 = ooz1[ptA] - ooz1[ptB]; if ( dx1 < 0 ) dx1 = -dx1;
				dy1 = ooz2[ptA] - ooz2[ptB]; if ( dy1 < 0 ) dy1 = -dy1;
				zData[ zCount ].ptB = (short)(( dx1 > (dy1 >> 3) ) ? ptB : -1); /* Added 6/5/97 ---Sampo (atimes.ttf Y) */
				zData[ zCount ].flags  = 0;
				zCount++;
			}
		}
		{
/* #define ONLY_SOME_EXTREMA */
#define ONLY_SOME_EXTREMA

#ifdef ONLY_SOME_EXTREMA
			tt_int32 max, min;
			
			max = min = ooz1[0];
			max_i = min_i = 0;
#endif
			k = 0; /* numExtrema */
			for ( i = 0; i <= lastPoint; i++ ) {
				if ( doY && (flags[i] & HEIGHT) ) {
					Array[ zCount ] = &zData[ zCount ];
					zData[ zCount ].ptA = (short)i;
					zData[ zCount ].ptB = -1;
					zData[ zCount ].flags  = Z_HEIGHT_FLAG;
					zCount++;
				}
				if ( flags[i] & extrema ) {
					k++;
#ifdef ONLY_SOME_EXTREMA
					tmp32 = ooz1[i];
					if ( tmp32 > max ) {
						max = tmp32; max_i = i;
					} else if ( tmp32 < min ) {
						min = tmp32; min_i = i;
					}
					if ( (flags[i] & slope) ) {
#else
					if ( true ) {
#endif
						Array[ zCount ] = &zData[ zCount ];
						zData[ zCount ].ptA = (short)i;
						zData[ zCount ].ptB = -1;
						zData[ zCount ].flags  = 0;
						zCount++;
					}
				}
			}
	
#ifdef LOG_ON		
if ( log_fp == NULL ) {
	log_fp = fopen( "log.txt", "a" ); assert( log_fp != NULL );
	fprintf(log_fp, "contourCount lastPoint numExtrema isComplex\n");
}
fprintf( log_fp, "%d, %d, %d, %d\n", (int)elem->contourCount, (int)lastPoint, (int)k, (int)1);

fflush( log_fp );
#endif
if ( doX && hData->fontType == ag_KANJI ) {
	isComplex = isComplexChar( elem->contourCount,lastPoint,k );
    isComplex = true; /* force true as a quick temporary experiment, this will damage roman fonts.. */
	#ifdef OLDOLD
	if ( isComplex != isComplexFunc( elem->contourCount, lastPoint, k) ) {
		SysBeep(0);
	}
	#endif
}
#ifdef EXTRA_VERBOSE
			if ( verbose ) {
				printf("min_i = %d, max_i = %d\n", min_i, max_i  );
			}
#endif
#ifdef ONLY_SOME_EXTREMA
			Array[ zCount ] = &zData[ zCount ];
			zData[ zCount ].ptA = (short)min_i;
			zData[ zCount ].ptB = -1;
			zData[ zCount ].flags  = (tt_uint16)(isComplex ? Z_HEIGHT_FLAG : 0);
			zCount++;

			Array[ zCount ] = &zData[ zCount ];
			zData[ zCount ].ptA = (short)max_i;
			zData[ zCount ].ptB = -1;
			zData[ zCount ].flags  = (tt_uint16)(isComplex ? Z_HEIGHT_FLAG : 0);
			zCount++;
#endif

			if ( doX && !hData->isFigure && hData->fontType == ag_ROMAN ) {
				ptA = (short)(lastPoint+1);
				ptB = (short)(lastPoint+2);
				Array[ zCount ] = &zData[ zCount ];
				zData[ zCount ].ptA = (short)ptA;
				zData[ zCount ].ptB = -1;
				zData[ zCount ].flags  = 0;
				zCount++;
				Array[ zCount ] = &zData[ zCount ];
				zData[ zCount ].ptA = (short)ptB;
				zData[ zCount ].ptB = -1;
				zData[ zCount ].flags  = 0;
				zCount++;
			}
		}
		assert( zCount < maxHintLines );
		
		HeapSort( Array, zCount, ooz1 );
		for ( i = 0; i <= lastPointPlus2; i++ ) {
			pointToLineMap[i] = -1;
		}
		/* Set the lineA field */

		k = 0;
		Array[0]->lineA  = 0;
		pointToLineMap[ Array[0]->ptA ] = (short)k;
		for ( i = 1; i < zCount; i++ ) {
			tt_int32 dist = ooz1[Array[i]->ptA] - ooz1[Array[i-1]->ptA];
			
			if ( dist < 0 ) dist = -dist;
			if ( dist > limit1000 /* COMPARE_NEQ( ooz1, Array[i]->ptA, Array[i-1]->ptA ) */ ) {
				k++;
			}
			Array[i]->lineA  = (short)k;
			pointToLineMap[ Array[i]->ptA ] = (short)k;
		}
		
		/* Set the lineB field && cvtNumber */
		for ( i = 0; i < zCount; i++ ) {
			ptB = Array[i]->ptB;
			Array[i]->lineB = (short)(ptB >= 0 ? pointToLineMap[ptB] :  -1);
			if ( Array[i]->flags & Z_HEIGHT_FLAG ) {
#ifdef ENABLE_AUTO_HINTING
				Array[i]->cvtNumber = (short)(isComplex ? -1   				: ag_Height( hData, Array[i]->ptA ));
#else
				Array[i]->cvtNumber = (short)(isComplex ? CVT_EXIT_MARKER 	: ag_Height( hData, Array[i]->ptA ));
#endif
			} else {
				Array[i]->cvtNumber = -1;
			}
		}
			
		
		/* Eliminate hintLines on the same coordinate */
		k = 0;
		for ( i = 1; i <  zCount; i++ ) {
			if ( Array[k]->lineA == Array[i]->lineA ) {
				 int delete_I;
				 
				 /* eliminate the bigger one in the ooz2 direction */
				 delete_I = COMPARE_GT( ooz2, Array[i]->ptA, Array[k]->ptB );
				 if ( ooz1[Array[i]->ptA] != ooz1[Array[k]->ptB] ) {
				 	/* Never delete the smallest or largest point, or it will not be IP-ed !!! 6/3/97 ---Sampo */
				 	if ( delete_I ) {
				 		if ( i == 0 || i == zCount-1 ) {
				 			delete_I = false;
				 		}
				 	} else {
				 		if ( k == 0 || k == zCount-1 ) {
				 			delete_I = true;
				 		}
				 	}
				 }
				 
				 if ( delete_I ) {
				 	Array[k]->flags |= Array[i]->flags; /* propagate the Z_HEIGHT_FLAG */
				 	if ( Array[k]->cvtNumber < 0 ) Array[k]->cvtNumber = Array[i]->cvtNumber; /* propagate the cvt_number */
				 	if ( Array[i]->lineB >= 0 &&  Array[k]->lineB < 0 ) {
				 		Array[k]->lineB = Array[i]->lineB;
				 	}
				 	Array[i]->flags |= Z_DELETE_FLAG;
				 } else {
				 	Array[i]->flags |= Array[k]->flags; /* propagate the Z_HEIGHT_FLAG */
				 	if ( Array[i]->cvtNumber < 0 ) Array[i]->cvtNumber = Array[k]->cvtNumber; /* propagate the cvt_number */
				 	if ( Array[k]->lineB >= 0 && Array[i]->lineB < 0 ) {
				 		Array[i]->lineB = Array[k]->lineB;
				 	}
				 	Array[k]->flags	|= Z_DELETE_FLAG;
				 	k = i;
				 }
			} else {
				k = i;
			}
		}
		{
			
			/* Compact  */
			for ( k = i = 0; i <  zCount; i++ ) {
				if ( !(Array[i]->flags & Z_DELETE_FLAG) ) {
					Array[i]->lineC = -1;
					Array[k++] = Array[i];
				} else {
					/* ENSURE THAT WE WILL IP THIS LATER 6/3/97 ---SAMPO */
		#ifdef EXTRA_VERBOSE
					if ( verbose ) {
		printf("Deleting %d\n", Array[i]->ptA );
					}
		#endif
					flags[Array[i]->ptA] |= importantBit;
				}
			}
			zCount = k;
			
#ifdef OLD
			/* Zero out flat heights if the corresponding round height has been found, WHY ??? */
			if ( firstH >= 0 ) {
				unsigned tt_int32 status = 0;
				int index;
				assert( ag_MAX_HEIGHTS_IN <= 16 );
				for ( i = firstH; i <= lastH; i++ ) {
					index = Array[i]->cvtNumber;
					if ( index >= 0 && (index & 1) ) {
						index >>= 1;
						status |= (1L << index );
					}
				}
				for ( i = firstH; i <= lastH; i++ ) {
					index = Array[i]->cvtNumber;
					if ( index >= 0 && (index & 1) == 0 ) {
						index >>= 1;
						if ( status & ((1L << index )) ) {
							Array[i]->cvtNumber = -1;
						}
					}

				}
			}
#endif /* OLD */
		}
		
		
		/* set lineC */
		for ( i = 0; i <  zCount; i++ ) {
			tt_int16 lineB;
			assert( Array[i]->lineA == i );
			lineB = Array[i]->lineB;
			if ( lineB >= 0 && Array[lineB]->lineB != i ) {
				Array[lineB]->lineC = (short)i;
			}
		}
		
		for ( i = 1; i < zCount; i++ ) {
			assert( (ooz1[Array[ i-1 ]->ptA] <= ooz1[Array[ i ]->ptA]) );
		}


		
/*****************************/
		if ( zCount > 0 ) {
			tt_int32 maxLine, lineTo, prevAnchorLine = 0;
			tt_int16 from, to;
#ifdef ENABLE_AUTO_HINTING
			tt_int32 ttcodePosition = -1;
			tt_int32 ttdataPosition = -1;		
			tt_int32 ttcodePosition2 = -1;
			tt_int32 ttdataPosition2 = -1;		
#endif			
			/* Do heights */
			maxLine = 0;
			i = 0;
			
			#ifdef ENABLE_AUTO_HINTING
				/* Do not check every time, so that we do not loose speed */
				if ( (i & 0x0f) == 0 && (errorCode = ag_CheckArrayBounds( hData )) < 0 ) {
					return errorCode; /***** ERROR *****/
				}
			#endif
			ptA 		= Array[ i ]->ptA;
			cvtNumber   = Array[ i ]->cvtNumber;
			if ( cvtNumber >= 0 ) {
				( doX ? ag_MIAPX : ag_MIAPY )( hData, elem, true, ptA, cvtNumber );
#ifdef EXTRA_VERBOSE
				if ( verbose ) {
					if ( verbose ) {
						if ( doY ) printf("A y[%d] = %ld\n", ptA, elem->y[ptA] );
					}
					printf("MIAP%c cvt = %d, ptA = %d\n", doX ? 'X' : 'Y', cvtNumber, ptA );
					if ( verbose ) {
						if ( doY ) printf("B y[%d] = %ld\n", ptA, elem->y[ptA] );
					}
				}
#endif
			} else {
				( doX ? ag_MDAPX : ag_MDAPY )( hData, elem, true, ptA );
#ifdef EXTRA_VERBOSE
				if ( verbose ) {
					printf("MDAP%c cvt = %d, ptA = %d\n", doX ? 'X' : 'Y', cvtNumber, ptA );
				}
#endif
			}
			flags[ptA]		|= doneBit;
			Array[i]->flags	|= Z_DONE_FLAG;
			
			ag_INIT_STORE( hData );
			/* Do the other key-points */
			for ( i = 0; i <  zCount; i++ ) {
				tt_int32 lineB, lineC, lineFrom, recordLocation;
				
#ifdef EXTRA_VERBOSE
				if ( verbose && Array[i]->cvtNumber >= 0 ) {
					printf("%d cvt# = %d\n", Array[i]->ptA, Array[i]->cvtNumber );
				}
#endif
				
#ifdef ENABLE_AUTO_HINTING
				if ( i == prevAnchorLine ) {
					ttcodePosition = hData->ttcode - hData->ttCodeBase;
					ttdataPosition = &hData->ttDataBase[hData->ttDataBaseMaxElements-1] - hData->ttdata;
					assert( ttcodePosition >= 0 && ttdataPosition >= 0 );
				}
#endif
				goto skipTryAgain; /*********/

#ifdef ENABLE_AUTO_GRIDDING

tryAgain:

#endif
				/* undo the Z_DONE_FLAG flag */
				i = zCount - 1;
				while ( i > prevAnchorLine ) {
					Array[i--]->flags &=~ Z_DONE_FLAG;
				}
				
skipTryAgain:
/*** #BEGIN ***/
#ifdef ENABLE_AUTO_HINTING
				/* Reset them since we can get here by a jump or sequentially block! */
				hData->RP0 = hData->RP1 = hData->RP2 = -1;
#endif
				
				ptA = Array[ i ]->ptA;
				#ifdef ENABLE_AUTO_HINTING
					/* Do not check every time, so that we do not loose speed */
					if ( (i & 0x0f) == 0 && (errorCode = ag_CheckArrayBounds( hData )) < 0 ) {
						return errorCode; /***** ERROR *****/
					}
				#endif
				
				lineTo = -1;
				
				lineB = Array[i]->lineB;
				lineC = Array[i]->lineC;
				lineTo = lineFrom = -1;
				/* lineFrom -> i - > lineTo */
				if ( lineB >= 0 ) {
					if ( Array[lineB]->flags & Z_DONE_FLAG ) {
						lineFrom = lineB; /* lineB -> i */
					} else {
						lineTo = lineB; /* X -> i -> lineB */
					}
				}
				if ( lineC >= 0 ) {
					if ( Array[lineC]->flags & Z_DONE_FLAG ) {
						if ( lineFrom < 0 ) lineFrom = lineC; /* lineC -> i */
						lineFrom = lineFrom > lineC ? lineFrom : lineC;
					} else {
						assert( lineC > i );
						if ( lineTo < 0 ) lineTo = lineC; /* X -> i -> lineC */
						lineTo = lineTo < lineC ? lineTo : lineC;
					}
				}
				/* assert( lineFrom == -1 || lineFrom < i ); */
				/* assert( lineTo == -1 || lineTo > i ); */
			
				to = Array[i]->ptA;
				if ( !(Array[i]->flags & Z_DONE_FLAG) ) {
					if ( lineFrom >= 0 ) {
						from = Array[lineFrom]->ptA;
						assert( from >= 0 );
						ag_LINK( hData, elem, ooz1, doX, doY, ag_pixelSize, true, 'B', 'l', from, to );
#ifdef EXTRA_VERBOSE
						if ( verbose ) printf("LINK-A %d->%d\n", from, to );
#endif
					} else {
						from = Array[i-1]->ptA;
						assert( hData->flags[from] & doneBit );
						if ( i > 1 ) {
							tt_int32 dx1, dx2;
							tt_int32 dy1, dy2;
							tt_int16 from2 = Array[i-2]->ptA;
							
							dx1 = ooz1[to] - ooz1[from]; if ( dx1 < 0 ) dx1 = -dx1;
							dy1 = ooz2[to] - ooz2[from]; if ( dy1 < 0 ) dy1 = -dy1;
							dx2 = ooz1[to] - ooz1[from2]; if ( dx2 < 0 ) dx2 = -dx2;
							dy2 = ooz2[to] - ooz2[from2]; if ( dy2 < 0 ) dy2 = -dy2;
							
							/* dx1 > limit3 introduced 6/5/97 due to TNR j ---Sampo */
							/* maybe from = from2 should be totally removed ???? */
							if ( dx1 > limit3 && dy1 > (dx1+dx1) && dy2 < dy1/8 ) {
								from = from2;
							}
						}
						ag_ADJUST( hData, elem, doX, doY, Array[prevAnchorLine]->ptA, from, to );
#ifdef EXTRA_VERBOSE
						if ( verbose ) printf("ADJUST %d : %d->%d\n", Array[prevAnchorLine]->ptA, from, to );
						if ( verbose ) {
							if ( doY ) printf("x[%d] = %ld\n", from, elem->x[from] );
							if ( doY ) printf("x[%d] = %ld\n", to, elem->x[to] );
						}
#endif
					}
					ag_ASSURE_AT_LEAST_EQUAL( hData, elem, doX, Array[i-1]->ptA, to );
#ifdef EXTRA_VERBOSE
					if ( verbose ) printf("AT_LEAST %d->%d\n", Array[i-1]->ptA, to );
#endif
					if ( maxLine > i ) {
						ag_ASSURE_AT_MOST_EQUAL( hData, elem, doX, Array[maxLine]->ptA, to );
#ifdef EXTRA_VERBOSE
						if ( verbose ) printf("AT_MOST %d->%d\n", Array[maxLine]->ptA, to );
#endif
					}
					flags[to]		|= doneBit;
					Array[i]->flags	|= Z_DONE_FLAG;
				}
			
				recordLocation = false;
				if ( Array[i]->cvtNumber >= 0 && i != prevAnchorLine ) {
					tt_int32 A, B, C, k;
				
					AG_CHECK_AND_TWEAK( hData, elem, doX, Array[i]->cvtNumber, ptA );
#ifdef EXTRA_VERBOSE
						if ( verbose ) {
							printf("AG_CHECK_AND_TWEAK cvt# = %d, ptA = %d\n",Array[i]->cvtNumber, ptA  );
						}
#endif
#ifdef ENABLE_AUTO_HINTING
					if ( zCount > 0 ) {
						assert( ttcodePosition >= 0 && ttdataPosition >= 0 );
						ag_MovePushDataIntoInstructionStream( hData, ttcodePosition, ttdataPosition );
					}
#endif

#ifdef ENABLE_AUTO_GRIDDING
					if ( hData->storage[ STORE_return ] ) {
#else
					ag_IF( hData, elem, STORE_return );	/* STORE_return */

#endif
#ifdef EXTRA_VERBOSE
						if ( verbose ) {
							if ( doY ) printf("*y[12] = %ld\n", elem->y[12] );
							printf("loop mulRepeatCount = %ld, error = %ld, multiplier = %ld\n", hData->storage[ STORE_mulRepeatCount ], hData->storage[ STORE_error ], hData->storage[ STORE_multiplier ] );
						}
#endif
						maxLine = prevAnchorLine;
#ifdef ENABLE_AUTO_GRIDDING
						goto tryAgain; /***** ***** *****/
#else
						ag_JMPR( hData, elem, ttcodePosition );
#endif

#ifdef ENABLE_AUTO_GRIDDING
					} else {
#else
					ag_ELSE( hData, elem );							/* STORE_return */

#endif
#ifdef EXTRA_VERBOSE
							if ( verbose ) {
								if ( doY ) printf("*y[12] = %ld\n", elem->y[12] );
								printf("*****mulRepeatCount = %ld, error = %ld\n", 	hData->storage[ STORE_mulRepeatCount ], hData->storage[ STORE_error ] );
							}	
#endif
#ifdef ENABLE_AUTO_GRIDDING
						if ( hData->storage[ STORE_error ] != 0	) {
#else
						ag_IF( hData, elem, STORE_error ); /* STORE_error */
#endif
							
#ifdef ENABLE_AUTO_HINTING
							ttcodePosition2 = hData->ttcode - hData->ttCodeBase;
							ttdataPosition2 = &hData->ttDataBase[hData->ttDataBaseMaxElements-1] - hData->ttdata;
							assert( ttcodePosition2 >= 0 && ttdataPosition2 >= 0 );
#endif

#ifdef ENABLE_AUTO_GRIDDING
#ifdef EXTRA_VERBOSE
								if ( verbose ) {
									printf("***** ***** ***** FAIL doing IP/MDAP ***** ***** ***** \n" );
								}
#endif
#endif
							A = Array[prevAnchorLine]->ptA;
							C = Array[i]->ptA;
							for ( k = prevAnchorLine+1; k < i; k++ ) {
								B = Array[k]->ptA;
								( doX ? ag_IPPointX : ag_IPPointY )( hData, elem, A, B, C );
								if ( !hData->strat98 ) {
									( doX ? ag_MDAPX : ag_MDAPY )( hData, elem, true, B );
								}
#ifdef EXTRA_VERBOSE
								if ( verbose ) {
									printf("IP MDAP %d-%d-%d, y=%ld\n", A, B, C, elem->y[B] );
								}
#endif
							}
#ifdef ENABLE_AUTO_HINTING
							/* Reset them since we are in an IF block! */
							hData->RP0 = hData->RP1 = hData->RP2 = -1;
							ag_MovePushDataIntoInstructionStream( hData, ttcodePosition2, ttdataPosition2 );
#endif
							
							
#ifdef ENABLE_AUTO_GRIDDING
						}
#else
						ag_EIF( hData, elem );						/* STORE_error */
#endif
						prevAnchorLine = i;
						recordLocation = true; /***** ***** DONE ***** *****/

#ifdef EXTRA_VERBOSE
							if ( verbose ) {
								printf("SET prevAnchorLine = %d, point = %d\n", i, Array[ i ]->ptA );
							}
#endif
#ifdef ENABLE_AUTO_GRIDDING
					}
#else
					ag_EIF( hData, elem );							/* STORE_return */
#endif
				}
				
				
				from = to;
				if ( lineTo >= 0 && !(Array[lineTo]->flags & Z_DONE_FLAG) ) {
					assert( lineTo > i );
					to = Array[lineTo]->ptA;
					assert( from >= 0 );

					ag_LINK( hData, elem, ooz1, doX, doY, ag_pixelSize, true, 'B', 'l', from, to );
#ifdef EXTRA_VERBOSE
					if ( verbose ) {
						printf("LINK-B %d->%d\n", from, to );
						/*
						printf("x[%d] = %ld\n", from, elem->x[from] );
						printf("x[%d] = %ld\n", to, elem->x[to] );
						*/
					}
#endif
					Array[lineTo]->flags	|= Z_DONE_FLAG;
					flags[to]   |= doneBit;

					if ( lineTo > maxLine ) {
						tt_int32 indx;
						maxLine = lineTo;
						
						/* only tolerate one height in an interval */
						for ( indx = maxLine; indx > prevAnchorLine; indx--) {
							if ( Array[indx]->cvtNumber >= 0 ) break; /****/
						}
						for ( indx--; indx > prevAnchorLine; indx-- ) {
#ifdef EXTRA_VERBOSE
								if ( verbose && Array[indx]->cvtNumber >= 0 ) {
									printf("Clearing cvt for pt %d\n", Array[indx]->ptA );
								}
#endif
							Array[indx]->cvtNumber = -1;
						}
						
					} else {
						/* this can in rare cases wreck a stroke-weight ... */
						/* ag_ASSURE_AT_MOST_EQUAL( hData, elem, doX, Array[maxLine]->ptA, to ); */
						ag_ASSURE_AT_MOST_EQUAL2( hData, elem, doX, Array[maxLine]->ptA, to, from ); /* Changed 8/27/97 ---Sampo */
#ifdef EXTRA_VERBOSE
						if ( verbose ) printf("AT_MOST2 %d->%d +(%d)\n", Array[maxLine]->ptA, to, from );
#endif
					}
					{
						tt_int32 A, B, C, k;
						
						A = Array[i]->ptA;
						C = Array[lineTo]->ptA;
						for ( k = i+1; k < lineTo; k++ ) {
							B = Array[k]->ptA;
							if ( !(Array[k]->flags & Z_DONE_FLAG) && Array[k]->lineB < 0 && Array[k]->lineC < 0 ) {
								( doX ? ag_IPPointX : ag_IPPointY )( hData, elem, A, B, C );
								( doX ? ag_MDAPX : ag_MDAPY )( hData, elem, true, B );
								Array[k]->flags	|= Z_DONE_FLAG;
								flags[B]   |= doneBit;
#ifdef EXTRA_VERBOSE
								if ( verbose ) {
									printf("IP MDAP %d-%d-%d, y=%ld\n", A, B, C, elem->y[B] );
								}
#endif
							}
						}
					}
				}
	#ifdef ENABLE_AUTO_HINTING
				if ( recordLocation ) {
					recordLocation = false;
					ttcodePosition = hData->ttcode - hData->ttCodeBase;
					ttdataPosition = &hData->ttDataBase[hData->ttDataBaseMaxElements-1] - hData->ttdata;
					assert( ttcodePosition >= 0 && ttdataPosition >= 0 );
				}
	#endif
			} /* for i < ZCount */
			/* Added Nov 18, 1997 */
			if ( doX && !hData->isFigure && hData->fontType == ag_ROMAN ) {
				ag_ADJUSTSPACING( hData, elem, lastPoint+1, min_i, max_i, lastPoint+2 );
			}
			
			
			/* Interpolate Intermediate points */
			ptB = Array[ 0 ]->ptA;
			for ( i = 1; i <  zCount; i++ ) {
				ptA = ptB;
				ptB = Array[ i ]->ptA;
	#ifdef ENABLE_AUTO_HINTING
				/* Do not check every time, so that we do not loose speed */
				if ( (i & 0x0f) == 0 && (errorCode = ag_CheckArrayBounds( hData )) < 0 ) {
					return errorCode; /***** ERROR *****/
				}
	#endif
				assert( (flags[ptA] & doneBit) );
				assert( (flags[ptB] & doneBit) );
				for ( k = 0; k <= lastPoint; k++ ) {
					short coord_k = ooz1[k];
					if ( (flags[k] & importantBit) && !(flags[k] & doneBit) && 
					      (coord_k >= ooz1[ptA]) && (coord_k <= ooz1[ptB]) ) {
					    short prev = hData->prevPt[k];
					    short next = hData->nextPt[k];
					    
					    assert( prev >= 0 && next >= 0 );
					    /* The two if statements added 8/27/97, to eliminate unnecessary IPs */
					    if ( (flags[prev] & doneBit) && coord_k == ooz1[prev] ) {
					    	; /* The smoothing will fix this */
					    } else if ( (flags[next] & doneBit) && coord_k == ooz1[next] ) {
					    	; /* The smoothing will fix this */
					    } else {
						    assert( k != ptA );
						    assert( k != ptB );
							( doX ? ag_IPPointX : ag_IPPointY )( hData, elem, ptA, (short)k, ptB );
							flags[k]   |= doneBit;
							#ifdef EXTRA_VERBOSE
									if ( verbose ) {
										printf("IP Intermediate %d-%d-%d\n", ptA, k, ptB  );
									}
							#endif
						}
					}
				}	
			}
		}
	}
	ag_XSmooth( hData, elem ); /* Position untouched points in the x direction */
	ag_YSmooth( hData, elem ); /* Position untouched points in the y direction */

	tsi_DeAllocMem( hData->mem, (char *)zData );
	tsi_DeAllocMem( hData->mem, (char *)Array );
	return errorCode; /*****/
}

#endif /* STRATEGY_97 */



