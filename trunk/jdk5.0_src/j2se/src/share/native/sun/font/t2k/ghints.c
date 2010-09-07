/*
 * @(#)ghints.c	1.18 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * File:		GHINTS.C
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
#include "util.h"
#include "t2kstrm.h"
#include "truetype.h"
#include "autogrid.h"
#include "ghints.h"



static int GetYMax( GlyphClass *glyph )
{
	int ctr, point, startPoint, endPoint;
	short *ooy = glyph->ooy, ymax = -32768;
	
	for ( ctr = 0; ctr < glyph->contourCount; ctr++ ) {
		startPoint = glyph->sp[ctr];
		endPoint   = glyph->ep[ctr];
		if ( startPoint < endPoint ) {
			for ( point = startPoint; point <= endPoint; point++ ) {
				if ( ooy[point] > ymax ) {
					ymax = ooy[point];
				}
			}
		}
	}
	return ymax; /******/
}


static int GetYMin( GlyphClass *glyph )
{
	int ctr, point, startPoint, endPoint;
	short *ooy = glyph->ooy, ymin = 32767;
	
	for ( ctr = 0; ctr < glyph->contourCount; ctr++ ) {
		startPoint = glyph->sp[ctr];
		endPoint   = glyph->ep[ctr];
		if ( startPoint < endPoint ) {
			for ( point = startPoint; point <= endPoint; point++ ) {
				if ( ooy[point] < ymin ) {
					ymin = ooy[point];
				}
			}
		}
	}
	return ymin; /******/
}

#define CURRENT_GHINT_VERSION 0

void ReadGHints( ag_GlobalDataType *gHints, InputStream *in)
{
	int version, i;
	
	version = ReadInt16( in );
	assert( version == 0 );
	for ( i = 0; i < ag_MAX_HEIGHTS_IN; i++ ) {
		gHints->heights[i].flat		= ReadInt16( in );
		gHints->heights[i].overLap	= ReadInt16( in );
		gHints->heights[i].round 	= (short)(gHints->heights[i].flat + gHints->heights[i].overLap);
	}
	for ( i = 0; i < ag_MAXWEIGHTS; i++ ) {
		gHints->xWeight[i] = ReadInt16( in );
		gHints->yWeight[i] = ReadInt16( in );
	}
}


#ifdef ENABLE_WRITE
void WriteGHints( ag_GlobalDataType *gHints, OutputStream *out)
{
	int i;
	
	
	WriteInt16( out, CURRENT_GHINT_VERSION );
	for ( i = 0; i < ag_MAX_HEIGHTS_IN; i++ ) {
		WriteInt16( out, gHints->heights[i].flat );
		WriteInt16( out, gHints->heights[i].overLap );
	}
	for ( i = 0; i < ag_MAXWEIGHTS; i++ ) {
		WriteInt16( out, gHints->xWeight[i] );
		WriteInt16( out, gHints->yWeight[i] );
	}
}
#endif /* ENABLE_WRITE */

/*
 *
 */
static short MedianHeight( sfntClass *font, register const unsigned char
*s, register tt_int16 MaxOrMin )
{
	short n, k, arr[32];
	tt_uint16 charCode;
	GlyphClass *glyph;
	tt_uint16 aw;

	for ( k = n = 0; n < 32 && s[n] != 0 ; n++ ) {
		charCode = s[n];

		glyph = GetGlyphByCharCode( font, charCode, false, &aw );
		if ( glyph->contourCount < 0 ) {
			tt_uint16 gIndex;
			gIndex = (tt_uint16)glyph->componentData[1];
			Delete_GlyphClass( glyph );
			glyph = GetGlyphByIndex( font, gIndex, false, &aw );
		}
		if ( glyph->contourCount != 0 && glyph->pointCount > 0 ) {
		  /* Skip empty glyphs 7/10/97, and 7/15/98 ---Sampo */
		 arr[k++] = (short)(MaxOrMin ? GetYMax( glyph ) : GetYMin( glyph ));
		}
		Delete_GlyphClass( glyph );
	}
	if ( k == 0 ) {
	   return 0; /*****/
	} else {
           util_SortShortArray( arr, k );
	   return arr[k >> 1]; /*****/ /* changed n>>1 to k>>1 7/15/98 ---Sampo */
	}
}


void ComputeGlobalHints( sfntClass *font, ag_HintHandleType hintHandle, ag_GlobalDataType *gHints, int kanji  )
{
	tt_int32 i, j, k;

	short arr[7], median, average, tmp, diffA, diffB;
	
#ifdef OLD
	if ( kanji ) {
		for ( i = 0; i < ag_MAX_HEIGHTS_IN; i++ ) {
			gHints->heights[i].flat		= 0;
			gHints->heights[i].overLap	= 0;
		}
	} else
#endif
	{
		gHints->heights[ag_ASCENDER_HEIGHT].flat  	= MedianHeight( font, (const unsigned char *)"bdhkl", 1 );
		gHints->heights[ag_ASCENDER_HEIGHT].round 	= MedianHeight( font, (const unsigned char *)"f", 1 );

		gHints->heights[ag_CAP_HEIGHT].flat  		= MedianHeight( font, (const unsigned char *)"EFHIL", 1 );
		gHints->heights[ag_CAP_HEIGHT].round 		= MedianHeight( font, (const unsigned char *)"CGOQ", 1 );

		gHints->heights[ag_FIGURE_HEIGHT].flat  	= MedianHeight( font, (const unsigned char *)"7", 1 );
		gHints->heights[ag_FIGURE_HEIGHT].round 	= MedianHeight( font, (const unsigned char *)"089", 1 );

		gHints->heights[ag_X_HEIGHT].flat  			= MedianHeight( font, (const unsigned char *)"z", 1 );
		gHints->heights[ag_X_HEIGHT].round 			= MedianHeight( font, (const unsigned char *)"eco", 1 );

		gHints->heights[ag_UC_BASE_HEIGHT].flat  	= MedianHeight( font, (const unsigned char *)"AFHILTZ", 0 );
		gHints->heights[ag_UC_BASE_HEIGHT].round 	= MedianHeight( font, (const unsigned char *)"CJOSU", 0 );

		gHints->heights[ag_LC_BASE_HEIGHT].flat  	= MedianHeight( font, (const unsigned char *)"r", 0 );
		gHints->heights[ag_LC_BASE_HEIGHT].round 	= MedianHeight( font, (const unsigned char *)"ceos", 0 );

		gHints->heights[ag_FIGURE_BASE_HEIGHT].flat  = MedianHeight( font, (const unsigned char *)"1247", 0 );
		gHints->heights[ag_FIGURE_BASE_HEIGHT].round = MedianHeight( font, (const unsigned char *)"035689", 0 );

		gHints->heights[ag_DESCENDER_HEIGHT].flat	= MedianHeight( font, (const unsigned char *)"pq", 0 );
		gHints->heights[ag_DESCENDER_HEIGHT].round 	= MedianHeight( font, (const unsigned char *)"g", 0 );

		gHints->heights[ag_PARENTHESES_TOP].flat  	= MedianHeight( font, (const unsigned char *)"[]", 1 );
		gHints->heights[ag_PARENTHESES_TOP].round 	= MedianHeight( font, (const unsigned char *)"{}()", 1 );

		gHints->heights[ag_PARENTHESES_BOTTOM].flat  = MedianHeight( font, (const unsigned char *)"[]", 0 );
		gHints->heights[ag_PARENTHESES_BOTTOM].round = MedianHeight( font, (const unsigned char *)"{}()", 0 );

		for ( i = 0; i < ag_MAX_HEIGHTS_IN; i++ ) {
			gHints->heights[i].overLap = (short)(gHints->heights[i].round - gHints->heights[i].flat);
		}
		
		/* top */
		if ( gHints->heights[ ag_ASCENDER_HEIGHT ].overLap < 0 )	gHints->heights[ ag_ASCENDER_HEIGHT ].overLap = 0;
		if ( gHints->heights[ ag_CAP_HEIGHT ].overLap < 0 )			gHints->heights[ ag_CAP_HEIGHT ].overLap = 0;
		if ( gHints->heights[ ag_FIGURE_HEIGHT ].overLap < 0 )		gHints->heights[ ag_FIGURE_HEIGHT ].overLap = 0;
		if ( gHints->heights[ ag_X_HEIGHT ].overLap < 0 )			gHints->heights[ ag_X_HEIGHT ].overLap = 0;
		if ( gHints->heights[ ag_PARENTHESES_TOP ].overLap < 0 )	gHints->heights[ ag_PARENTHESES_TOP ].overLap = 0;
		/* bottom */
		if ( gHints->heights[ ag_UC_BASE_HEIGHT ].overLap > 0 )		gHints->heights[ ag_UC_BASE_HEIGHT ].overLap = 0;
		if ( gHints->heights[ ag_LC_BASE_HEIGHT ].overLap > 0 )		gHints->heights[ ag_LC_BASE_HEIGHT ].overLap = 0;
		if ( gHints->heights[ ag_FIGURE_BASE_HEIGHT ].overLap > 0 )	gHints->heights[ ag_FIGURE_BASE_HEIGHT ].overLap = 0;
		if ( gHints->heights[ ag_DESCENDER_HEIGHT ].overLap > 0 )	gHints->heights[ ag_DESCENDER_HEIGHT ].overLap = 0;
		if ( gHints->heights[ ag_PARENTHESES_BOTTOM ].overLap > 0 )	gHints->heights[ ag_PARENTHESES_BOTTOM ].overLap = 0;
		
		/* Now regularize the overlaps */
		arr[0] = gHints->heights[ ag_CAP_HEIGHT ].overLap;
		arr[1] = gHints->heights[ ag_X_HEIGHT ].overLap;
		arr[2] = gHints->heights[ ag_FIGURE_HEIGHT ].overLap;
		arr[3] = (short)-gHints->heights[ ag_UC_BASE_HEIGHT ].overLap;
		arr[4] = (short)-gHints->heights[ ag_LC_BASE_HEIGHT ].overLap;
		arr[5] = (short)-gHints->heights[ ag_FIGURE_BASE_HEIGHT ].overLap;
		arr[6] = (short)-gHints->heights[ ag_DESCENDER_HEIGHT ].overLap;
		util_SortShortArray( arr, 7 );
		median = arr[3];
		
		for ( j = 0; j < 4; j++ ) {
			short top, bottom;
			
			if ( j == 0 ) {
				top = ag_CAP_HEIGHT; 	bottom = ag_UC_BASE_HEIGHT;
			} else if ( j == 1 ) {
				top = ag_X_HEIGHT; 		bottom = ag_LC_BASE_HEIGHT;
			} else if ( j == 2 ) {
				top = ag_FIGURE_HEIGHT;	bottom = ag_FIGURE_BASE_HEIGHT;
			} else {
				top = ag_PARENTHESES_TOP;	bottom = ag_PARENTHESES_BOTTOM;
			}
			average = (short)((gHints->heights[ top ].overLap + -gHints->heights[ bottom ].overLap + 1 ) >> 1);
			for ( i = 0; i < 2; i++ ) {
				tmp = (short)(i == 0 ? median : average);
				diffA = (short)(tmp - gHints->heights[ top ].overLap);		if ( diffA < 0 ) diffA = (short)-diffA;
				diffB = (short)(tmp - -gHints->heights[ bottom ].overLap);	if ( diffB < 0 ) diffB = (short)-diffB;
				if ( diffA < 3 && diffB < 3 ) {
					gHints->heights[ top ].overLap = tmp;
					gHints->heights[ bottom ].overLap = (short)-tmp;
					break; /*****/
				} 
			}
		}
		diffB = (short)(median - gHints->heights[ ag_ASCENDER_HEIGHT ].overLap);	if ( diffB < 0 ) diffB = (short)-diffB;
		if ( diffB < 3 ) {
			gHints->heights[ ag_ASCENDER_HEIGHT ].overLap = median;
		}
		diffB = (short)(median - -gHints->heights[ ag_DESCENDER_HEIGHT ].overLap);	if ( diffB < 0 ) diffB = (short)-diffB;
		if ( diffB < 3 ) {
			gHints->heights[ ag_DESCENDER_HEIGHT ].overLap = (short)-median;
		}
	}

	for ( i = 0; i < ag_MAX_HEIGHTS_IN; i++ ) {
		gHints->heights[i].round = (short)(gHints->heights[i].flat + gHints->heights[i].overLap);
	}


	for ( i = 0; i < ag_MAXWEIGHTS; i++ ) {
		gHints->xWeight[i] = 0;
		gHints->yWeight[i] = 0;
	}
	{
		int err, fail;
		GlyphClass *glyph;
		ag_ElementType elem;
		short *xDist, *yDist;
		tt_int32 xDistCount, yDistCount;
		short curveType = 2;
		short isFigure = false;
		tt_uint16 aw;
		
		fail = true;
		if ( !kanji ) {
			glyph = GetGlyphByCharCode( font, (tt_uint16)'o', false, &aw  );
			if ( glyph->contourCount > 0 ) {
				elem.contourCount	= glyph->contourCount;
				elem.pointCount		= glyph->pointCount;
				elem.sp				= glyph->sp;
				elem.ep				= glyph->ep;
				elem.oox			= glyph->oox;
				elem.ooy			= glyph->ooy;
				elem.onCurve 		= glyph->onCurve;
				elem.x = NULL;
				elem.y = NULL;
				
				err = ag_AutoFindStems( hintHandle, &elem, isFigure, curveType, &xDist, &xDistCount, &yDist, &yDistCount ); 
				assert( err == 0 );
				assert( xDist != NULL && yDist != NULL );
				util_SortShortArray( xDist, xDistCount );
				util_SortShortArray( yDist, yDistCount );
				gHints->xWeight[0] = xDist[ xDistCount>>1 ];
				gHints->yWeight[0] = yDist[ yDistCount>>1 ];
			
				tsi_DeAllocMem( font->mem, xDist ); xDist = NULL;
				tsi_DeAllocMem( font->mem, yDist ); yDist = NULL;
				fail = false;
			}
			Delete_GlyphClass( glyph );
		}
		if ( kanji || fail ) {
			short x[32], y[32];
			k = GetNumGlyphs_sfntClass( font );
			j = 0;
			for ( i = k/64; i < k && j < 32; i += k/32 ) {
				int attempt = 0;
				do {
					glyph = GetGlyphByIndex( font, i+attempt, false, &aw );
					if ( glyph->contourCount > 0 ) {
						elem.contourCount	= glyph->contourCount;
						elem.pointCount		= glyph->pointCount;
						elem.sp				= glyph->sp;
						elem.ep				= glyph->ep;
						elem.oox			= glyph->oox;
						elem.ooy			= glyph->ooy;
						elem.onCurve 		= glyph->onCurve;
						elem.x = NULL;
						elem.y = NULL;
						
						err = ag_AutoFindStems( hintHandle, &elem, isFigure, curveType, &xDist, &xDistCount, &yDist, &yDistCount ); 
						assert( err == 0 );
						assert( xDist != NULL && yDist != NULL );
						util_SortShortArray( xDist, xDistCount );
						util_SortShortArray( yDist, yDistCount );
						x[j] = xDist[ xDistCount>>1 ];
						y[j] = yDist[ yDistCount>>1 ];
						assert( j < 32 );
						j++;
						tsi_DeAllocMem( font->mem, xDist ); xDist = NULL;
						tsi_DeAllocMem( font->mem, yDist ); yDist = NULL;
						attempt = 999; /* done */
					}
					Delete_GlyphClass( glyph );
 				} while ( (attempt++ < 4) && (i+attempt < k));
			}
			util_SortShortArray( x, j );
			util_SortShortArray( y, j );
			j >>= 1;
			gHints->xWeight[0] = x[ j ];
			gHints->yWeight[0] = y[ j ];
			{
				short min, max;
				if ( x[ j ] < y[j] ) {
					min = x[j]; max = y[j];
				} else {
					min = y[j]; max = x[j];
				}
				if ( (max - min) <= 1 ) {
					gHints->xWeight[0] = min;
					gHints->yWeight[0] = min;
				}
			}
		}
	}
	
/* #define HARDWIRE_WEIGHTS */
#ifdef HARDWIRE_WEIGHTS
/* Mincho 110/55 */
/* Gothic 158/127 */
gHints->xWeight[0] = 110;
gHints->yWeight[0] = 55;
#endif
	
#ifdef ENABLE_AUTO_HINTING
printf("gHints->xWeight[0] = %d, gHints->yWeight[0]	 = %d\n", gHints->xWeight[0], gHints->yWeight[0] );
#endif

	Purge_cmapMemory( font ); /* free up memory. We do not need the cmap after this point */
}
