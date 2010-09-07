/*
 * @(#)glyph.c	1.22 04/01/09
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * GLYPH.c
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
#include "t2kstrm.h"
#include "glyph.h"

#ifdef ENABLE_ORION
#include "orion.h"
#endif




int ReadDeltaXYValue( InputStream *in, short *dxPtr, short *dyPtr )
{
	int dx, dy;
	int d1, d2; /* Main and second axis	 */
	int quadrant;
	unsigned tt_int32 w1;
	
	w1 = ReadUnsignedByteMacro( in );
	w1 <<= 8;
	w1 |= ReadUnsignedByteMacro( in );
	/* ow1 = w1; */
	quadrant = w1 >> 14;
	w1 &= 0x3fff;
	
	if ( w1 < BASE1 ) {
		/* if ( ow1 == 0 || (ow1 == (1<<14)) ) */
		if ( w1 == 0 && quadrant <= 1 ) {
			short theDx, theDy;
			theDx = ReadUnsignedByteMacro( in );
			theDx <<= 8;
			theDx |= ReadUnsignedByteMacro( in );
			theDy = ReadUnsignedByteMacro( in );
			theDy <<= 8;
			theDy |= ReadUnsignedByteMacro( in );
			*dxPtr = theDx;
			*dyPtr = theDy;
			return quadrant == 0 ? 1 : 0; /*****/
		} else {
			d1 = w1;
			d2 = 0;
		}
	} else if ( w1 < BASE2 ) {
		w1 -= BASE1;
		/* assert( w1 >= 0 && w1 < 102*102 ); */
		d1 = w1 / 102 + 1;
		d2 = w1 % 102 + 1;
	} else if ( w1 < BASE3 ) {
		w1 -= BASE2;
		w1 <<= 8;
		w1 |= ReadUnsignedByteMacro( in );
		/* assert( w1 >= 0 && w1 < 724*724 ); */
		d1 = w1 / 724 + 1;
		d2 = w1 % 724 + 1;
	} else {
		w1 -= BASE3;
		w1 <<= 8;
		w1 |= ReadUnsignedByteMacro( in );
		w1 <<= 8;
		w1 |= ReadUnsignedByteMacro( in );
		/* assert( w1 >= 0 && w1 < 10650*10650 ); */
		d1 = w1 / 10650;
		d2 = w1 % 10650;
	}

	dx = dy = 0; /* to avoid warning with MS C */
	switch ( quadrant ) {
	case 0:
		dx = d1;
		dy = d2;
		break; /*****/
	case 1:
		dx = -d2;
		dy = d1;
		break; /*****/
	case 2:
		dx = -d1;
		dy = -d2;
		break; /*****/
	case 3:
		dx = d2;
		dy = -d1;
		break; /*****/
	}
	d1 = 1 - (dx & 1); /* oncurve */
	*dxPtr = (short)(dx >>1);
	*dyPtr = (short)dy;
	return d1;
}


#ifdef ENABLE_ORION
int ReadOrionDeltaXYValue( InputStream *in, void *model, short *dxPtr, short *dyPtr )
{
	int dx, dy;
	int d1, d2; /* Main and second axis	 */
	int quadrant;
	unsigned tt_int32 w1;
	unsigned char b0, b1;
	OrionModelClass *orion = (OrionModelClass *)model;
	
	assert( orion != 0 );
	
	b0 = SCODER_ReadSymbol( orion->literal[ orion->OrionState ], in );
	/* d2 = ((b0+b0)&6) + (orion->OrionState&1); */
	d2 = (b0+b0) + (orion->OrionState&1);
	d2 = d2 % orion->num_eb2;
	b1 = SCODER_ReadSymbol( orion->literal[ d2 + orion->num_eb1 ], in );
	
	w1 = b0;
	w1 <<= 8;
	w1 |= b1;
	/* ow1 = w1; */
	quadrant = w1 >> 14;
	w1 &= 0x3fff;
	
	if ( w1 < BASE1 ) {
		/* if ( ow1 == 0 || (ow1 == (1<<14)) ) */
		if ( w1 == 0 && quadrant <= 1 ) {
			short theDx, theDy;
			theDx = SCODER_ReadSymbol( orion->literal[orion->num_e-1], in );
			theDx <<= 8;
			theDx |= SCODER_ReadSymbol( orion->literal[orion->num_e-1], in );
			theDy = SCODER_ReadSymbol( orion->literal[orion->num_e-1], in );
			theDy <<= 8;
			theDy |= SCODER_ReadSymbol( orion->literal[orion->num_e-1], in );
			*dxPtr = theDx;
			*dyPtr = theDy;
			return quadrant == 0 ? 1 : 0; /*****/
		} else {
			d1 = w1;
			d2 = 0;
		}
	} else if ( w1 < BASE2 ) {
		w1 -= BASE1;
		/* assert( w1 >= 0 && w1 < 102*102 ); */
		d1 = w1 / 102 + 1;
		d2 = w1 % 102 + 1;
	} else if ( w1 < BASE3 ) {
		w1 -= BASE2;
		w1 <<= 8;
		w1 |= SCODER_ReadSymbol( orion->literal[orion->num_e-1], in );
		/* assert( w1 >= 0 && w1 < 724*724 ); */
		d1 = w1 / 724 + 1;
		d2 = w1 % 724 + 1;
	} else {
		w1 -= BASE3;
		w1 <<= 8;
		w1 |= SCODER_ReadSymbol( orion->literal[orion->num_e-1], in );
		w1 <<= 8;
		w1 |= SCODER_ReadSymbol( orion->literal[orion->num_e-1], in );
		/* assert( w1 >= 0 && w1 < 10650*10650 ); */
		d1 = w1 / 10650;
		d2 = w1 % 10650;
	}

	dx = dy = 0; /* to avoid warning with MS C */
	switch ( quadrant ) {
	case 0:
		dx = d1;
		dy = d2;
		break; /*****/
	case 1:
		dx = -d2;
		dy = d1;
		break; /*****/
	case 2:
		dx = -d1;
		dy = -d2;
		break; /*****/
	case 3:
		dx = d2;
		dy = -d1;
		break; /*****/
	}
	d1 = 1 - (dx & 1); /* oncurve */
	*dxPtr = (short)(dx >>1);
	*dyPtr = (short)dy;
	return d1;
}

#endif /* ENABLE_ORION */

#ifndef ENABLE_WRITE
void TEST_T2K_GLYPH( tsiMemObject *UNUSED(mem) )
{

}
#endif



#ifdef T1_OR_T2_IS_ENABLED
static void glyph_AllocContours( GlyphClass *t, short contourCountMax )
{
	short ctr;
	short	*sp;			/* sp[contourCount] Start points */
	short	*ep;  			/* ep[contourCount] End points */
	
	if ( t->contourCountMax < contourCountMax ) {
		t->contourCountMax = contourCountMax;
		sp = (short*) tsi_AllocMem( t->mem, sizeof(short) * contourCountMax * 2);
		ep   = &sp[ contourCountMax ];
		
		for ( ctr = 0; ctr < t->contourCount; ctr++ ) {
			sp[ctr] = t->sp[ctr];
			ep[ctr] = t->ep[ctr];
		}
		tsi_DeAllocMem( t->mem, t->sp );
		t->sp = sp;
		t->ep = ep;
	}
}
#endif /* T1_OR_T2_IS_ENABLED */

#ifdef T1_OR_T2_IS_ENABLED
void glyph_CloseContour( GlyphClass *t )
{
	short ctr, point;
	glyph_AllocContours( t, (short)(t->contourCount + 2) );
	
	if ( t->pointCount <= 0 ) {
		//to distinguish countours with zero points and one point assign -1 here
		t->ep[t->contourCount] = -1;
	} else {
		t->ep[t->contourCount] = (short)(t->pointCount - 1);
	}
	t->contourCount++;
	
	point = 0;
	for ( ctr = 0; ctr < t->contourCount; ctr++ ) {
		t->sp[ctr] = point;
		point = (short)(t->ep[ctr] + 1);
	}
	/* eliminate duplicate points */
	if ( t->pointCount > 0 ) {
		short ptA, ptB;
		
		ptA = t->sp[t->contourCount-1];
		ptB = t->ep[t->contourCount-1];
		
		if ( t->oox[ptA] == t->oox[ptB] && t->ooy[ptA] == t->ooy[ptB] && t->onCurve[ptA] == t->onCurve[ptB] ) {
			t->pointCount--;
			t->ep[t->contourCount-1] = (short)(t->pointCount - 1);
		}
	}
}
#endif /* T1_OR_T2_IS_ENABLED */

#ifdef ENABLE_T2KE
/* Dynamically allocates color plane information */
static void AllocNewColorPlane( GlyphClass *glyph )
{
	glyph->colorPlaneCount++;


	if ( glyph->colorPlaneCount > glyph->colorPlaneCountMax ) {
		glyph->colorPlaneCountMax = glyph->colorPlaneCount + (glyph->colorPlaneCount>>1) + 8;
		if ( glyph->colors == NULL ) {
			glyph->colors = (tsiColorDescriptor *)tsi_AllocMem( glyph->mem, glyph->colorPlaneCountMax * sizeof(tsiColorDescriptor) );
		} else {
			glyph->colors = (tsiColorDescriptor *)tsi_ReAllocMem( glyph->mem, glyph->colors, glyph->colorPlaneCountMax * sizeof(tsiColorDescriptor) );
		}
	}
	assert( glyph->colors != NULL );
}

void glyph_CloseColorContour( GlyphClass *glyph, tsiColorDescriptor *color )
{
	tt_int32 index = glyph->colorPlaneCount-1;
	glyph_CloseContour( glyph );
	if ( index >= 0 && glyph->colors[index].curveType == color->curveType &&
				glyph->colors[index].paintType 	== color->paintType && color->paintType == 0 &&
				glyph->colors[index].ARGB 		== color->ARGB ) {
		glyph->colors[index].numContoursInThisPlane++;
	} else {
		assert( glyph->contourCount != 1 || glyph->colorPlaneCount == 0 );
		AllocNewColorPlane( glyph );
		index = glyph->colorPlaneCount-1;
		glyph->colors[index] = *color;
		glyph->colors[index].numContoursInThisPlane = 1;
	}
}
#endif


#ifdef ENABLE_PRINTF
void glyph_PrintPoints( GlyphClass *t )
{
	int i;
	
	printf("+++++\n");
	printf("t->contourCount = %d\n", t->contourCount );
	if ( t->contourCount > 0 ) {
		for ( i = 0; i < t->contourCount; i++ ) {
			printf("%d: sp = %d, ep = %d\n", i, t->sp[i], t->ep[i] );
		}
		for ( i = 0; i <= t->ep[ t->contourCount-1 ]; i++ ) {
			printf("%d: x = %d, y = %d, %s\n", i, t->oox[i], t->ooy[i], t->onCurve[i] ? "on" : "off" );
		}
	}
}
#endif


#ifdef T1_OR_T2_IS_ENABLED
void glyph_AddPoint( GlyphClass *t, tt_int32 x, tt_int32 y, char onCurveBit )
{
	register short *oox, *ooy;
	register tt_uint8 *onCurve;
	int i, limit;
	
	if ( t->pointCount >= t->pointCountMax ) { 
		/* ReAllocate */
		t->pointCountMax = t->pointCountMax + (t->pointCountMax >> 1 ) + 32;
	
		/* + 2 for the sidebearing points */
		oox				= (short*) tsi_AllocMem( t->mem, (t->pointCountMax+2) * ( 2 * sizeof(short) + sizeof(tt_uint8)) );
		ooy				= &oox[ t->pointCountMax+2 ];
		onCurve			= (tt_uint8 *)&ooy[ t->pointCountMax+2 ];
		
		limit = t->pointCount + 2;
		for ( i = 0; i < limit; i++ ) {
			oox[i]		= t->oox[i];
			ooy[i]		= t->ooy[i];
			onCurve[i]	= t->onCurve[i];
		}
		tsi_DeAllocMem( t->mem, t->oox );
		t->oox		= oox;
		t->ooy		= ooy;
		t->onCurve	= onCurve;
	}
	i = t->pointCount;
	t->oox[i]		= (short)x;
	t->ooy[i]		= (short)y;
	t->onCurve[i]	= onCurveBit;
	t->pointCount	= (short)(i + 1);
}

void glyph_StartLine( GlyphClass *t, tt_int32 x, tt_int32 y )
{
	int prevPoint = t->pointCount-1;
	
	if ( prevPoint < 0 || t->oox[prevPoint] != x || t->ooy[prevPoint] != y ) {
		glyph_AddPoint( t, x, y, 1 );
	}
}

#endif /* T1_OR_T2_IS_ENABLED */


/*
 *
 */
GlyphClass *New_EmptyGlyph( tsiMemObject *mem, tt_int16 UNUSED(lsb), tt_uint16 aw )
{
	register short *oox, *ooy;
	register tt_uint8 *onCurve;
	register int pointCount;
	GlyphClass *t = (GlyphClass *) tsi_AllocMem( mem, sizeof( GlyphClass ) );

	t->mem				= mem;
	oox	= ooy			= NULL;
	t->sp	= t->ep		= NULL;
	onCurve				= NULL;
	t->hintFragment		= NULL;
	t->componentData	= NULL;
	t->x = t->y 		= NULL;
	
	t->colorPlaneCount		= 0;
 	t->colorPlaneCountMax	= 0;
#ifdef ENABLE_T2KE
 	t->colors				= NULL;
#endif
  	t->curveType			= 2;
	t->contourCountMax		= 0;
 	t->pointCountMax		= 0;
 	t->componentSizeMax		= 0;
 	
	t->contourCount			= 0;
	t->xmin 				= 0;
	t->ymin 				= 0;
	t->xmax 				= 0;
	t->ymax 				= 0;
	pointCount 				= 0;
 	t->componentSize		= 0;
 	t->hintLength 			= 0;
 	t->hintFragment			= NULL;
 	
	oox				= (short*) tsi_AllocMem( t->mem, (pointCount+2) * ( 2 * sizeof(short) + sizeof(tt_uint8)) );
	ooy				= &oox[ pointCount+2 ];
	onCurve			= (tt_uint8 *)&ooy[ pointCount+2 ];

	ooy[pointCount + 0] = 0;
	oox[pointCount + 0] = 0;
	
	ooy[pointCount + 1] = 0;
	oox[pointCount + 1] = (short)(oox[pointCount + 0] + aw);

	t->oox		= oox;
	t->ooy		= ooy;
	t->onCurve	= onCurve;
	t->pointCount = (short)pointCount;
	/* printf("contourCount = %ld, pointCount = %ld\n", (tt_int32)t->contourCount, (tt_int32)t->pointCount ); */
	return t; /*****/
}


#ifdef ENABLE_ORION
static unsigned tt_int32 ReadScoderUnsignedNumber( InputStream *in, SCODER *sc )
{
	unsigned char value;
	unsigned tt_int32 n = 0;
	unsigned tt_int32 shift = 0;
	
	do {
		value = SCODER_ReadSymbol( sc, in );
		n |= ((value & 0x7f) << shift );
		shift += 7;
	} while (value & 0x80);
	return n; /*****/
}
#endif

/* #define BAD_CONTOUR_DIRECTION_TEST */
#ifdef BAD_CONTOUR_DIRECTION_TEST
/*
 * REMOVE
 */
static void FlipContourDirection(GlyphClass *glyph)
{
	short	ctr, j;
	short	*oox = 	glyph->oox;
	short	*ooy = 	glyph->ooy;
	tt_uint8 	*onCurve = glyph->onCurve;

	for ( ctr = 0; ctr < glyph->contourCount; ctr++ ) {
	 	short	flips, start, end;
	 	
	 	start	= glyph->sp[ctr];
	 	end		= glyph->ep[ctr];
	 	
	 	flips = (short)((end - start)/2);
	 	start++;
		for ( j = 0; j < flips; j++ ) {
			tt_int16	tempX, tempY;
			tt_uint8	pointType;
			tt_int16   indexA = (tt_int16)(start + j);
			tt_int16   indexB = (tt_int16)(end   - j);
	 		
	 		tempX				= oox[indexA];
	 		tempY				= ooy[indexA];
	 		pointType			= onCurve[indexA];
	 		
	 		oox[indexA]			= oox[indexB];
	 		ooy[indexA]			= ooy[indexB];
	 		onCurve[indexA]		= onCurve[indexB];

	 		oox[indexB]			= tempX;
	 		ooy[indexB]			= tempY;
	 		onCurve[indexB]		= pointType;
		}
	}
}
#endif /* BAD_CONTOUR_DIRECTION_TEST */

/*
 *
 */
GlyphClass *New_GlyphClassT2K( tsiMemObject *mem, register InputStream *in, char UNUSED(readHints), tt_int16 lsb, tt_uint16 aw, void *model )
{
	register tt_int32 i;
	register short *oox, *ooy;
	register tt_uint8 *onCurve;
	register int pointCount;
	GlyphClass *t = (GlyphClass *) tsi_AllocMem( mem, sizeof( GlyphClass ) );
	
	
	t->mem				= mem;
	oox	= ooy			= NULL;
	t->sp	= t->ep		= NULL;
	onCurve				= NULL;
	t->hintFragment		= NULL;
	t->componentData	= NULL;
	t->x = t->y 		= NULL;

	t->colorPlaneCount		= 0;
 	t->colorPlaneCountMax	= 0;
#ifdef ENABLE_T2KE
 	t->colors				= NULL;
#endif
 	t->curveType			= 2;
 	t->contourCountMax		= 0;
 	t->pointCountMax		= 0;

	t->contourCount			= ReadInt16( in );
	t->xmin 				= 0;
	t->ymin 				= 0;
	t->xmax 				= 0;
	t->ymax 				= 0;
	pointCount 				= 0;
 	t->componentSize		= 0;
 	t->hintLength 			= 0;
 	t->hintFragment			= NULL;
	if ( t->contourCount < 0 ) {
		/* Composite Glyph */
		short flags;
 		int glyphIndex;
 		int weHaveInstructions;
 		register short *componentData;
 		register tt_int32 componentSize = 0;
 		
 		t->componentSizeMax		= 1024;
 		componentData			= (short*) tsi_AllocMem( t->mem, t->componentSizeMax * sizeof(short) );
 		do {
 			if ( componentSize >= t->componentSizeMax - 10 ) {
 				/* Reallocate */
 				t->componentSizeMax += t->componentSizeMax/2;
 				componentData = (short*) tsi_ReAllocMem( t->mem, componentData, t->componentSizeMax * sizeof(short) );
 				assert( componentData != NULL );
 			}
 			flags = ReadInt16( in );
 			weHaveInstructions = (flags & WE_HAVE_INSTRUCTIONS) != 0;
 			componentData[ componentSize++] = flags;
 			
 			glyphIndex = ReadInt16( in );
 			assert( glyphIndex >= 0 );
 			componentData[ componentSize++] = (short)glyphIndex;
 			if ( (flags & ARG_1_AND_2_ARE_WORDS) != 0 ) {
 				/* arg 1 and 2 */
	 			componentData[ componentSize++] = ReadInt16( in );
	 			componentData[ componentSize++] = ReadInt16( in );
 			} else {
 				/* arg 1 and 2 as bytes */
 	 			componentData[ componentSize++] = ReadInt16( in );
 			}
 			
 			if ( (flags & WE_HAVE_A_SCALE) != 0 ) {
 				/* scale */
  	 			componentData[ componentSize++] = ReadInt16( in );
 			} else if ( (flags & WE_HAVE_AN_X_AND_Y_SCALE) != 0 ) {
 				/* xscale, yscale */
   	 			componentData[ componentSize++] = ReadInt16( in );
  	 			componentData[ componentSize++] = ReadInt16( in );
 			} else if ( (flags & WE_HAVE_A_TWO_BY_TWO) != 0 ) {
 				/* xscale, scale01, scale10, yscale */
   	 			componentData[ componentSize++] = ReadInt16( in );
  	 			componentData[ componentSize++] = ReadInt16( in );
   	 			componentData[ componentSize++] = ReadInt16( in );
  	 			componentData[ componentSize++] = ReadInt16( in );
 			}
 		} while ( (flags & MORE_COMPONENTS) != 0 );
 		t->hintLength = 0;
		oox			= (short*) tsi_AllocMem( t->mem, (pointCount+2) * 2 * sizeof(short) );
		ooy			= &oox[ pointCount+2 ];
		t->componentData = componentData;
		t->componentSize = componentSize;
	} else if ( t->contourCount > 0 ) {
		short x, y;
		short stmp = 0;
#ifdef ENABLE_ORION
		OrionModelClass *orion = (OrionModelClass *)model;
		tt_int32			switchPos, numSwitches;
		unsigned char  *switchData = NULL;
#endif
		
		
		
		
		/* Regular Glyph */
		t->sp = (short*) tsi_AllocMem( t->mem, sizeof(short) * 2 * t->contourCount);
		t->ep   = &t->sp[t->contourCount];
		
		
		for ( i = 0; i < t->contourCount; i++ ) {
		    t->sp[i]	= stmp;
		    
#ifdef ENABLE_ORION
			if ( orion != NULL ) {
				t->ep[i]    = (short)(ReadScoderUnsignedNumber( in, orion->ep) + stmp);
			} else {
		    	t->ep[i]    = (short)(ReadUnsignedNumber( in ) + stmp);
			}
#else
		    t->ep[i]    = (short)(ReadUnsignedNumber( in ) + stmp);
#endif
		    
		 	stmp = (short)(t->ep[i] + 1);
		}
		pointCount = stmp;

		t->hintLength = 0;
		
#ifdef ENABLE_ORION
		if ( orion != NULL ) {
			numSwitches = (pointCount+7)/8;;
			switchData = (unsigned char *) tsi_AllocMem( mem, numSwitches * sizeof( unsigned char ) );
			for ( switchPos = 0; switchPos < numSwitches; switchPos++ ) {
				switchData[ switchPos ] = SCODER_ReadSymbol( orion->control, in ); /* Initialize */
			}
			orion->OrionState = ORION_STATE_0 % orion->num_eb1; /* Initialize */;
		}
		switchPos = 0;

#endif
		

		oox				= (short*) tsi_AllocMem( t->mem, (pointCount+2) * ( 2 * sizeof(short) + sizeof(tt_uint8)) );
		ooy				= &oox[ pointCount+2 ];
		onCurve			= (tt_uint8 *)&ooy[ pointCount+2 ];

	 	t->contourCountMax	= t->contourCount;
	 	t->pointCountMax	= (short)pointCount;
		x = y = 0;
		t->xmin = 0x7fff;
		for ( i = 0; i < pointCount; i++ ) {
			short dx, dy;
#ifdef ENABLE_ORION
			if ( orion != NULL ) {
				if ( switchData[ switchPos>>3 ] & (1 << (switchPos&7)) ) {
					unsigned char index;
					int i256 = orion->OrionState << 8;
					/* copy */
					index		= SCODER_ReadSymbol( orion->copy[ orion->OrionState ], in );
					/* printf("%d, %d\n",orion->OrionState, index ); */
					i256       += index;
					dx 			= orion->dx[i256];
					dy 			= orion->dy[i256];
					onCurve[i]	= orion->onCurve[i256];
					/* printf("C dx = %d, dy = %d, onCurve = %d\n", dx, dy, (int)onCurve[i] ); */
				} else {
					/* literal */
					onCurve[i] = (unsigned char)ReadOrionDeltaXYValue( in, orion, &dx, &dy );
					/* printf("L dx = %d, dy = %d, onCurve = %d\n", dx, dy, (int)onCurve[i] ); */
				}
				switchPos++;
				Set_OrionState( orion, dx, dy, onCurve[i] );
			} else {
				onCurve[i] = (unsigned char)ReadDeltaXYValue( in, &dx, &dy );
			}

#else
			onCurve[i] = (unsigned char)ReadDeltaXYValue( in, &dx, &dy );
#endif
			x = (short)(x + dx);
			y = (short)(y + dy);
			if ( x < t->xmin ) t->xmin = x;
			oox[i] = x;
			ooy[i] = y;
		}
#ifdef ENABLE_ORION
		tsi_DeAllocMem( mem, switchData );
#endif			
		if ( t->xmin != lsb ) {
			short error = (short)(t->xmin - lsb);
			for ( i = 0; i < pointCount; i++ ) {
				oox[i] = (short)(oox[i] - error);
			}
			t->xmin = lsb;
		}
	}

	ooy[pointCount + 0] = 0;
	oox[pointCount + 0] = (short)(t->xmin - lsb);
	
	ooy[pointCount + 1] = 0;
	oox[pointCount + 1] = (short)(oox[pointCount + 0] + aw);
	
	t->oox		= oox;
	t->ooy		= ooy;
	t->onCurve	= onCurve;
	t->pointCount = (short)pointCount;
	/* printf("contourCount = %ld, pointCount = %ld\n", (tt_int32)t->contourCount, (tt_int32)t->pointCount ); */
	
#ifdef BAD_CONTOUR_DIRECTION_TEST
	FlipContourDirection(t);
#endif
	return t; /*****/
}

void FlipContourDirectionShort(GlyphClass *glyph)
{
	short	ctr, j;
	short	*oox = 	glyph->oox;
	short	*ooy = 	glyph->ooy;
	tt_uint8 	*onCurve = glyph->onCurve;

	for ( ctr = 0; ctr < glyph->contourCount; ctr++ ) {
	 	short	flips, start, end;
	 	
	 	start	= glyph->sp[ctr];
	 	end		= glyph->ep[ctr];
	 	
	 	flips = (short)((end - start)/2);
	 	start++;
		for ( j = 0; j < flips; j++ ) {
		        tt_int16   tempX, tempY;
			tt_uint8   pointType;
			tt_int16   indexA = (tt_int16)(start + j);
			tt_int16   indexB = (tt_int16)(end   - j);
	 		
	 		tempX				= oox[indexA];
	 		tempY				= ooy[indexA];
	 		pointType			= onCurve[indexA];
	 		
	 		oox[indexA]			= oox[indexB];
	 		ooy[indexA]			= ooy[indexB];
	 		onCurve[indexA]		= onCurve[indexB];

	 		oox[indexB]			= tempX;
	 		ooy[indexB]			= tempY;
	 		onCurve[indexB]		= pointType;
		}
	}
}
 

/* For a contour array, reverse each contour with respect
	to the x,y arrays of F26Dot6 */

void ReverseContourDirectionDirect(	
	tt_int32	contourCount,	
	tt_int16	*sp,	/* sp[contourCount] Start points */
	tt_int16	*ep,   	/* ep[contourCount] End points */
	F26Dot6 *x,
	F26Dot6 *y,
	tt_uint8 	*onCurve 
	)
  {
	short	ctr, j;
	for ( ctr = 0; ctr < contourCount; ctr++ ) {
	 	short	flips, start, end;	 	
	 	start	=  sp[ctr];
	 	end		=  ep[ctr];	 	
	 	flips = (short)((end - start)/2);
	 	start++;
	 	
		for ( j = 0; j < flips; j++ ) {
			F26Dot6 x32, y32;
			tt_uint8	pointType;
			tt_int16   indexA = (tt_int16)(start + j);
			tt_int16   indexB = (tt_int16)(end   - j);

	 		pointType			= onCurve[indexA];
	 		x32					= x[indexA];
	 		y32					= y[indexA];
	 		
 	 		onCurve[indexA]		= onCurve[indexB];
	 		x[indexA]			= x[indexB];
	 		y[indexA]			= y[indexB];

	 		onCurve[indexB]		= pointType;
	 		x[indexB]			= x32;
	 		y[indexB]			= y32;
		}
	}
}

/* This routine is similar to the above routine,
	but applies the reversal to all arrays within a glyphclass */

void ReverseContourDirection(GlyphClass *glyph)
{
#if 1
	short	ctr, j;
	short	*oox = 	glyph->oox;
	short	*ooy = 	glyph->ooy;
	tt_uint8 	*onCurve = glyph->onCurve;
	F26Dot6 *x = 	glyph-> x ;
	F26Dot6 *y = 	glyph-> y ;

	for ( ctr = 0; ctr < glyph->contourCount; ctr++ ) {
	 	short	flips, start, end;	 	
	 	start	= glyph->sp[ctr];
	 	end		= glyph->ep[ctr];	 	
	 	flips = (short)((end - start)/2);
	 	start++;
	 	
		for ( j = 0; j < flips; j++ ) {
			short	tempX, tempY;
			F26Dot6 x32, y32;
			tt_uint8	pointType;
			tt_int16   indexA = (tt_int16)(start + j);
			tt_int16   indexB = (tt_int16)(end   - j);
	 		
	 		tempX				= oox[indexA];
	 		tempY				= ooy[indexA];
	 		pointType			= onCurve[indexA];
	 		x32					= x[indexA];
	 		y32					= y[indexA];
	 		
	 		oox[indexA]			= oox[indexB];
	 		ooy[indexA]			= ooy[indexB];
	 		onCurve[indexA]		= onCurve[indexB];
	 		x[indexA]			= x[indexB];
	 		y[indexA]			= y[indexB];

	 		oox[indexB]			= tempX;
	 		ooy[indexB]			= tempY;
	 		onCurve[indexB]		= pointType;
	 		x[indexB]			= x32;
	 		y[indexB]			= y32;
		}
	}
	#endif
}


void Add_GlyphClass( GlyphClass **tPtr, GlyphClass *addMe, tt_uint16 flags, tt_int32 arg1, tt_int32 arg2, tt_int32 rarg1, tt_int32 rarg2, T2K_TRANS_MATRIX transform)
{
	register int i, j;
	GlyphClass *t;
	int pointCount, n, contourCount;
	short *sp, *ep, *oox, *ooy;
	tt_uint8 *onCurve;
	F26Dot6 *x, *y, xDelta, yDelta;
	volatile F26Dot6  tmpYdelta;
    	
	assert( addMe != NULL );
	
    /* apply transform */
    if (transform.t01 != 0 || transform.t10 != 0 ||
		transform.t00 != ONE16Dot16 || transform.t11 != ONE16Dot16) { /* check for identity transform */
       for ( i = 0; i < addMe->pointCount+2; i++ ) {
		  register F26Dot6 tmpX, tmpY;

		  tmpX = addMe->x[i]; tmpY = addMe->y[i];
		  addMe->x[i] = util_FixMul( transform.t00, tmpX ) + util_FixMul( transform.t01, tmpY );
		  addMe->y[i] = util_FixMul( transform.t10, tmpX ) + util_FixMul( transform.t11, tmpY );

		  tmpX = addMe->oox[i]; tmpY = addMe->ooy[i];
		  addMe->oox[i] = util_FixMul( transform.t00, tmpX ) + util_FixMul( transform.t01, tmpY );
		  addMe->ooy[i] = util_FixMul( transform.t10, tmpX ) + util_FixMul( transform.t11, tmpY );
	   }
	}

    /* NB: we may also need to apply transform to arg1/arg2/rawArg1/rawArg2 too sometime.
	   (if font is designed for Mac and follow Apple (non Microsoft) approach -
        see http://pfaedit.sourceforge.net/Composites/ for details) */

	t = *tPtr;
	//use addMe as base if current base is empty or has no points
	if ( t == NULL || t->pointCount == 0) {
			//if we are replacing previous base then we should free memory explicitly
			if ( t != NULL ) {
				Delete_GlyphClass(t);
			}
			if ( flags & ARGS_ARE_XY_VALUES ) { /* Added Feb 9, 98 ---Sampo */
				xDelta = arg1;
				yDelta = arg2; /* Assume they have already been scaled */
				if ( flags & ROUND_XY_TO_GRID ) {
					xDelta = (xDelta+32) & ~63;
					yDelta = (yDelta+32) & ~63;
				}
				if ( xDelta != 0 || yDelta != 0 ) {
					j = addMe->pointCount;
					for ( i = 0; i <j; i++ ) {
						addMe->x[i] += xDelta;
						addMe->y[i] += yDelta;
						addMe->oox[i] += rarg1;
						addMe->ooy[i] += rarg2;
					}
				}
			}
		*tPtr = addMe;
		return; /*****/
	}

	//skip subglyphs that have zero points
	if (addMe->pointCount == 0) 
		return;

	pointCount = t->pointCount + addMe->pointCount;
	n = pointCount + 2;
	contourCount = t->contourCount + addMe->contourCount;
	
	x 				= (F26Dot6 *) tsi_AllocMem( t->mem, (n) * 2 * sizeof( F26Dot6 ) );
	y 				= &x[ n ];
	oox				= (short *) tsi_AllocMem( t->mem, (n) * ( 2 * sizeof(short) + sizeof(tt_uint8)) );
	ooy				= &oox[ n ];
	onCurve			= (tt_uint8 *)&ooy[ n ];
	sp 				= (short *) tsi_AllocMem( t->mem, sizeof(short) * 2 * contourCount);
	ep   			= &sp[contourCount];
	
	t->pointCountMax   = (short)pointCount;
	t->contourCountMax = (short)contourCount;

	
	for ( i = 0; i < t->pointCount; i++ ) {
		x[i]		= t->x[i];
		y[i]		= t->y[i];
		
		oox[i]		= t->oox[i];
		ooy[i]		= t->ooy[i];

		onCurve[i]	= t->onCurve[i] ;
	}
	if ( !(flags & USE_MY_METRICS) ) {
		/* preserve the metrics from the previous glyph */
		x[pointCount + 0] = t->x[t->pointCount+0];
		y[pointCount + 0] = t->y[t->pointCount+0];
		x[pointCount + 1] = t->x[t->pointCount+1];
		y[pointCount + 1] = t->y[t->pointCount+1];
	}
	if ( flags & ARGS_ARE_XY_VALUES ) {
		xDelta = arg1;
		yDelta = arg2; /* Assume they have already been scaled */
		if ( flags & ROUND_XY_TO_GRID ) {
			xDelta = (xDelta+32) & ~63;
			yDelta = (yDelta+32) & ~63;
		}
	} else {
		xDelta = t->x[arg1] - addMe->x[arg2];
                /*
                 * Needed to get around a bug in the x86 compiler when
                 * optimized (running -xO2)
                 */
		tmpYdelta = t->y[arg1] - addMe->y[arg2];
		yDelta = tmpYdelta;
	}
	
	for ( j = 0, i = t->pointCount; i < pointCount; i++, j++ ) {
		x[i]		= addMe->x[j] + xDelta;
		y[i]		= addMe->y[j] + yDelta;
		oox[i]		= addMe->oox[j] + rarg1;
		ooy[i]		= addMe->ooy[j] + rarg2;
		onCurve[i]	= addMe->onCurve[j];
	}
	
	assert( t->ep[t->contourCount-1] == t->pointCount-1 );
	for ( i = 0; i < t->contourCount; i++ ) {
		sp[i]	= t->sp[i];
		ep[i]	= t->ep[i];
	}	
	for ( j = 0, i = t->contourCount; i < contourCount; i++, j++ ) {
		sp[i]	= (short)(addMe->sp[j] + t->pointCount);
		ep[i]	= (short)(addMe->ep[j] + t->pointCount);
	}	
	
	t->pointCount 	= (short)pointCount;
	t->contourCount = (short)contourCount;
	
	tsi_DeAllocMem( t->mem, t->oox ); t->oox = NULL; t->ooy = NULL;
	/* t->ooy points into t->oox */
	tsi_DeAllocMem( t->mem, t->x);
	/* t->y points into t->x */
	tsi_DeAllocMem( t->mem, t->sp );
	/* t->ep points into t->sp */
	/* tsi_DeAllocMem( t->mem, t->onCurve ); point into oox */

	t->x = x;
	t->y = y;
	t->oox = oox;
	t->ooy = ooy;
	t->onCurve = onCurve;
	t->sp = sp;
	t->ep = ep;
	
	t->curveType = addMe->curveType;
}

/*
 *
 */
void Delete_GlyphClass( GlyphClass *t )
{
	if ( t != NULL ) {
		tsi_DeAllocMem( t->mem, t->oox );
		/* t->ooy points into t->oox */
		tsi_DeAllocMem( t->mem, t->x);
		/* t->y points into t->x */
		tsi_DeAllocMem( t->mem, t->sp );
		/* t->ep points into t->sp */
		/* tsi_DeAllocMem( t->mem, t->onCurve ); point into oox */
		tsi_DeAllocMem( t->mem, t->hintFragment );
		tsi_DeAllocMem( t->mem, t->componentData );
		
#ifdef ENABLE_T2KE
		tsi_DeAllocMem( t->mem, t->colors );
#endif
		tsi_DeAllocMem( t->mem, t );
	}
}

