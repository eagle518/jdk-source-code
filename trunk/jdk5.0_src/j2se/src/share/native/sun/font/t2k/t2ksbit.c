/*
 * @(#)t2ksbit.c	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * T2KSBIT.c
 * Copyright (C) 1989-1998 all rights reserved by Type Solutions, Inc. Plaistow, NH, USA.
 * http://www.typesolutions.com/
 * Author: Sampo Kaasila
 *.
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
#include "truetype.h"
#include "util.h"
#include "t2ksbit.h"


#ifdef ENABLE_SBIT


/*
 * The bitmapSizeTable class constructor.
 * Each strike is defined by one bitmapSizeTable.
 */
static bitmapSizeTableT2K *New_bitmapSizeTable( tsiMemObject *mem, InputStream *in, tt_uint32 blocOffset )
{
	register int i;
	tt_uint32 savepos;
	
	bitmapSizeTableT2K *t	= (bitmapSizeTableT2K *) tsi_AllocMem( mem, 
					      sizeof( bitmapSizeTableT2K ) );
	t->mem				= mem;
	
	t->indexSubTableArrayOffset		= (tt_uint32)ReadInt32(in);
	t->indexTableSize				= (tt_uint32)ReadInt32(in);
	t->numberOfIndexSubTables		= (tt_uint32)ReadInt32(in);
	t->colorRef						= (tt_uint32)ReadInt32(in);
	
	for ( i = 0; i < NUM_SBIT_METRICS_BYTES; i++ ) {
		t->hori[i] = ReadUnsignedByteMacro( in );
	}
	for ( i = 0; i < NUM_SBIT_METRICS_BYTES; i++ ) {
		t->vert[i] = ReadUnsignedByteMacro( in );
	}
	t->startGlyphIndex				= (tt_uint16)ReadInt16( in );
	t->endGlyphIndex				= (tt_uint16)ReadInt16( in );

	t->ppemX						= ReadUnsignedByteMacro( in );
	t->ppemY						= ReadUnsignedByteMacro( in );
	t->bitDepth						= ReadUnsignedByteMacro( in );
	t->flags						= ReadUnsignedByteMacro( in );

	t->table = (indexSubTableArray *)tsi_AllocMem( mem, t->numberOfIndexSubTables * sizeof( indexSubTableArray ) );

	savepos = Tell_InputStream( in );
	Seek_InputStream( in, blocOffset + t->indexSubTableArrayOffset );
	for ( i = 0; i < (int)t->numberOfIndexSubTables; i++ ) {
		t->table[i].firstGlyphIndex		= (tt_uint16)ReadInt16( in );
		t->table[i].lastGlyphIndex		= (tt_uint16)ReadInt16( in );
		t->table[i].additionalOffsetToIndexSubTable	= (tt_uint32)ReadInt32(in);
	}
	Seek_InputStream( in, savepos );
	return t; /*****/
}


/*
 * The bitmapSizeTable class destructor
 */
static void Delete_bitmapSizeTable( bitmapSizeTableT2K *t )
{
	
	tsi_DeAllocMem( t->mem, t->table );
	tsi_DeAllocMem( t->mem, t );
}


/*
 * The blocClass class contructor (EBLC/bloc table)
 */
blocClass *New_blocClass( tsiMemObject *mem, int fontIsSbitOnly, InputStream *in )
{
	F16Dot16 version;
	tt_uint32	startOffset;
	register int i;
	blocClass *t = NULL;
	
	startOffset			= Tell_InputStream( in );
	version				= ReadInt32(in);
	
	/* Assume we understand this version number range. */
	if ( version >= 0x00020000 && version < 0x00030000 ) {
		t 					= (blocClass *) tsi_AllocMem( mem, sizeof( blocClass ) );
		t->mem				= mem;
		t->startOffset  	= startOffset;
		t->fontIsSbitOnly 	= fontIsSbitOnly;
		
		/* Read the EBLC/bloc header. */
		t->version			= version;
		t->nTables			= (tt_uint32)ReadInt32(in);
		
		/* The EBLC/bloc header is followed immediately by the bitmapSizeTable array(s). */
		t->table		= (bitmapSizeTableT2K **) tsi_AllocMem( mem, t->nTables * sizeof( bitmapSizeTableT2K * ) );
		for ( i = 0; i < (int)t->nTables; i++ ) {
			t->table[i] = New_bitmapSizeTable( mem, in, t->startOffset ); /* One bitmapSizeTable for each strike */
		}
		/* Initialize data fields */
		t->gInfo.offsetA		= 0;
		t->gInfo.offsetB		= 0;
		t->gInfo.imageFormat	= 0;
		t->gInfo.glyphIndex		= 0;
		t->gInfo.ppemX			= 0;
		t->gInfo.ppemY			= 0;
		
		t->gInfo.baseAddr		= NULL;
		t->gInfo.rowBytes		= 0;
	}
	
	return t; /*****/
}

/*
 * The blocClass class destructor (EBLC/bloc table)
 */
void Delete_blocClass( blocClass *t )
{
	register int i;
	if ( t != NULL ) {
		for ( i = 0; i < (int)t->nTables; i++ ) {
			Delete_bitmapSizeTable( t->table[i] );
		}
		tsi_DeAllocMem( t->mem, t->table );
		tsi_DeAllocMem( t->mem, t->gInfo.baseAddr );
		tsi_DeAllocMem( t->mem, t );
	}
}


/*
 * The ebscClass class contructor (EBSC table)
 */
ebscClass *New_ebscClass( tsiMemObject *mem, InputStream *in )
{
	F16Dot16 version;
	register int i, j;
	ebscClass *t = NULL;
	
	version			= ReadInt32(in);
	
	/* Assume we understand this version number range. */
	if ( version >= 0x00020000 && version < 0x00030000 ) {
		t 				= (ebscClass *) tsi_AllocMem( mem, sizeof( ebscClass ) );
		t->mem			= mem;

		t->version		= version;
		t->numSizes		= (tt_uint32)ReadInt32(in);
		
		t->table		= (bitmapScaleEntry *) tsi_AllocMem( mem, t->numSizes * sizeof( bitmapScaleEntry ) );
		for ( i = 0; i < (int)t->numSizes; i++ ) {
			for ( j = 0; j < NUM_SBIT_METRICS_BYTES; j++ ) {
				t->table[i].hori[j] = ReadUnsignedByteMacro( in );
			}
			for ( j = 0; j < NUM_SBIT_METRICS_BYTES; j++ ) {
				t->table[i].vert[j] = ReadUnsignedByteMacro( in );
			}
			t->table[i].ppemX			= ReadUnsignedByteMacro( in );
			t->table[i].ppemY			= ReadUnsignedByteMacro( in );
			t->table[i].substitutePpemX	= ReadUnsignedByteMacro( in );
			t->table[i].substitutePpemY	= ReadUnsignedByteMacro( in );
		}
	}
	return t; /*****/
}

/*
 * The ebscClass class destructor (EBSC table)
 */
void Delete_ebscClass( ebscClass *t )
{
	if ( t != NULL ) {
		tsi_DeAllocMem( t->mem, t->table );
		tsi_DeAllocMem( t->mem, t );
	}
}


/*
 *
 */
static void ReadBigMetrics( bigGlyphMetricsT2K *m, InputStream *in )
{
	m->height		= ReadUnsignedByteMacro( in );
	m->width		= ReadUnsignedByteMacro( in );
	m->horiBearingX	= (tt_int8)ReadUnsignedByteMacro( in );
	m->horiBearingY	= (tt_int8)ReadUnsignedByteMacro( in );
	m->horiAdvance	= ReadUnsignedByteMacro( in );
	m->vertBearingX	= (tt_int8)ReadUnsignedByteMacro( in );
	m->vertBearingY	= (tt_int8)ReadUnsignedByteMacro( in );
	m->vertAdvance	= ReadUnsignedByteMacro( in );
}

/*
 *
 */
static void ReadSmallMetrics( bigGlyphMetricsT2K *m, InputStream *in )
{
	/* We read smallGlyphMetrics but put the result into bigGlyphMetrics to simplify. */
	
	/* smallGlyphMetrics *m */
	m->height	= ReadUnsignedByteMacro( in );
	m->width	= ReadUnsignedByteMacro( in );
	m->horiBearingX	= (tt_int8)ReadUnsignedByteMacro( in );
	m->horiBearingY	= (tt_int8)ReadUnsignedByteMacro( in );
	m->horiAdvance	= ReadUnsignedByteMacro( in );
	/* Copy over to the other direction */
	m->vertBearingX = m->horiBearingX;
	m->vertBearingY = m->horiBearingY;
	m->vertAdvance	= m->horiAdvance;
#ifdef OLD
	/* smallGlyphMetrics *m */
	m->height	= ReadUnsignedByteMacro( in );
	m->width	= ReadUnsignedByteMacro( in );
	m->BearingX	= (tt_int8)ReadUnsignedByteMacro( in );
	m->BearingY	= (tt_int8)ReadUnsignedByteMacro( in );
	m->Advance	= ReadUnsignedByteMacro( in );
#endif
}

/*
 * Finds a substitute size for ppemX and ppemY
 * This should only be used if there are no outlines in the font.
 */
static int FintBestSubstitute( blocClass * t, tt_uint16 ppemX, tt_uint16 ppemY )
{
	register int i, limit = (int)t->nTables;
	int errorX, errorY, error, bestI, smallestError;

	/* A simple heuristic :-) */
	bestI = -1;
	smallestError = 0x7fffffff;
	for ( i = 0; i < limit; i++ ) {
		errorX = t->table[i]->ppemX; errorX -= ppemX;
		if ( errorX < 0 ) errorX *= -4;
		errorY = t->table[i]->ppemY; errorY -= ppemY;
		if ( errorY < 0 ) errorY *= -4;
		error = errorX + errorY;
		if ( error < smallestError ) {
			smallestError  = error;
			bestI = i;
		}
	}
	return bestI; /*****/
}



/*
 * Returns NULL or a pointer to the bitmapSizeTable for this size.
 */
static bitmapSizeTableT2K *FindBitmapSizeTable( blocClass *t, ebscClass *ebsc, tt_uint16 ppemX, tt_uint16 ppemY, sbitGlypInfoData *result )
{
	int i, limit = (int)t->nTables;
	bitmapSizeTableT2K *bst = NULL;		/* Initialize */

	result->ppemX			= ppemX;
	result->ppemY			= ppemY;
	result->substitutePpemX	= ppemX;
	result->substitutePpemY = ppemY;
	/* Determine the strike */
	for (;;) {
		for ( i = 0; i < limit; i++ ) {
			if ( (tt_uint16)t->table[i]->ppemX == ppemX && (tt_uint16)t->table[i]->ppemY == ppemY ) {
				bst = t->table[i];
				break;/*****/
			}
		}
		if ( bst == NULL ) {
			if ( ebsc != NULL ) {
				/* See if we can scale another embedded bitmap to this size instead. */
				for ( i = 0; i < (int)ebsc->numSizes; i++ ) {
					if ( ebsc->table[i].ppemX == ppemX && ebsc->table[i].ppemY == ppemY ) {
						ppemX	= result->substitutePpemX = ebsc->table[i].substitutePpemX;
						ppemY	= result->substitutePpemY = ebsc->table[i].substitutePpemY;
						ebsc	= NULL;	/* Set to NULL, since we are done with this table */
						break; /*****/
					}
				}
				/* If we found a hit in the ebsc table then try again. */
				if ( ebsc == NULL ) continue; /*****/
			}
			/* Only do this if there are no outlines in the font. */
			if ( t->fontIsSbitOnly ) {
				if ((i = FintBestSubstitute(t, ppemX, ppemY)) >= 0 ) {
					result->substitutePpemX = (tt_uint16)t->table[i]->ppemX;
					result->substitutePpemY = (tt_uint16)t->table[i]->ppemY;
					bst = t->table[i];
				}
			}
		}
		break; /*****/
	}
	
	return bst; /*****/
}


/*
 * Rescales the value passed in to the new size.
 */
static tt_int32 RescalePixelValue( tt_int16 value, tt_uint16 ppem, tt_uint16 substitutePpem )
{
	if ( substitutePpem != ppem ) {
		tt_int32 result = (value  * ppem + (substitutePpem>>1)) / substitutePpem;
		return result;	/*****/
	} else {
		return value;	/*****/
	}
}


/*
 * Returns scaled font wide sbit metrics
 */
void GetFontWideSbitMetrics( blocClass *bloc, ebscClass *ebsc, tt_uint16 ppemX, tt_uint16 ppemY,
							T2K_FontWideMetrics *hori, T2K_FontWideMetrics *vert )
{
	sbitGlypInfoData data;
	bitmapSizeTableT2K *bst;
	tt_uint16 substitutePpemX, substitutePpemY;
	int i;

	bst = FindBitmapSizeTable( bloc, ebsc, ppemX, ppemY, &data );
	if ( bst != NULL ) {
		tt_int8 num = bst->hori[3], denom = bst->hori[4];

		substitutePpemX = data.substitutePpemX;
		substitutePpemY = data.substitutePpemY;

		if (num == 0 && denom == 0) {
			num = 1;
		}
		
		hori->isValid	= true;
		hori->Ascender	= (tt_int16)RescalePixelValue( (tt_int8)bst->hori[0], ppemY, substitutePpemY );
		hori->Descender	= (tt_int16)RescalePixelValue( (tt_int8)bst->hori[1], ppemY, substitutePpemY );
		hori->LineGap	= 0;
		hori->maxAW		= (tt_uint16)RescalePixelValue( bst->hori[2], ppemX, substitutePpemX );
		hori->caretDy	= (tt_int16)RescalePixelValue( num, ppemX, substitutePpemX );
		hori->caretDx	= (tt_int16)RescalePixelValue( denom, ppemY, substitutePpemY );
		/* Scale up */
		for ( i = 0; i < 16; i++ ) {
			if ( hori->caretDx >= ONE16Dot16 || hori->caretDx <= -ONE16Dot16 ) break; /*****/
			if ( hori->caretDy >= ONE16Dot16 || hori->caretDy <= -ONE16Dot16 ) break; /*****/
			hori->caretDx <<= 1;
			hori->caretDy <<= 1;
		}
		
		vert->isValid	= true;
		vert->Ascender	= (tt_int16)RescalePixelValue( (tt_int8)bst->vert[0], ppemX, substitutePpemX );
		vert->Descender	= (tt_int16)RescalePixelValue( (tt_int8)bst->vert[1], ppemX, substitutePpemX );
		vert->LineGap	= 0;
		vert->maxAW		= (tt_uint16)RescalePixelValue( bst->vert[2], ppemY, substitutePpemY );
		vert->caretDx	= (tt_int16)RescalePixelValue( (tt_int8)bst->vert[4], ppemX, substitutePpemX );
		vert->caretDy	= (tt_int16)RescalePixelValue( (tt_int8)bst->vert[3], ppemY, substitutePpemY );
		/* Scale up */
		for ( i = 0; i < 16; i++ ) {
			if ( vert->caretDx >= ONE16Dot16 || vert->caretDx <= -ONE16Dot16 ) break; /*****/
			if ( vert->caretDy >= ONE16Dot16 || vert->caretDy <= -ONE16Dot16 ) break; /*****/
			vert->caretDx <<= 1;
			vert->caretDy <<= 1;
		}
	} else {
		hori->isValid 	= false;
		vert->isValid 	= false;
	}
}

/*
 * Returns true if the glyph exists, and false otherwise
 * Caches some internal results in sbitGlypInfoData so that we can get to the bits faster when we need them next.
 */
int FindGlyph_blocClass( blocClass *t, ebscClass *ebsc, InputStream *in, tt_uint16 glyphIndex, tt_uint16 ppemX, tt_uint16 ppemY, sbitGlypInfoData *result )
{
	int i;
	tt_uint32 offsetA, offsetB;
	tt_uint16 imageFormat = 0, firstGlyphIndex;
	bitmapSizeTableT2K *bst;
	
	assert( t != NULL );
	assert( result != NULL );
	
	
	bst = FindBitmapSizeTable( t, ebsc, ppemX, ppemY, result );
	
	offsetA = offsetB = 0;
	imageFormat = 0;
	
	/* See if we found a strike and if the glyphIndex is within the strike. */
	if ( bst != NULL && glyphIndex >= bst->startGlyphIndex && glyphIndex <= bst->endGlyphIndex ) {
		/* Search for the range containing the glyph, since there can be multiple ranges within one strike. */
		for ( i = 0; i < (int)bst->numberOfIndexSubTables; i++ ) {
			firstGlyphIndex = bst->table[i].firstGlyphIndex;
			if ( glyphIndex >= firstGlyphIndex && glyphIndex <= bst->table[i].lastGlyphIndex ) {
				/* Ok we found a range containing the glyphIndex! */
				tt_uint16 indexFormat;
				tt_uint32 imageDataOffset = 0, dataOffset = 0;
				
				result->bitDepth	= bst->bitDepth;
				result->flags		= bst->flags;

				Seek_InputStream( in, t->startOffset + bst->indexSubTableArrayOffset + bst->table[i].additionalOffsetToIndexSubTable );
				/* Here we have the indexSubHeader data */
				/* All 5 formats start with the same header. */
				indexFormat 		= (tt_uint16)ReadInt16( in );	/* Format of this indexSubTable. */
				imageFormat			= (tt_uint16)ReadInt16( in );	/* Format of the EBDT/bdat data. */
				imageDataOffset		= (tt_uint32)ReadInt32( in );	/* Offset to the image data in the EBDT/bdat table. */
				/* End indexSubHeader data. */
				
				dataOffset = Tell_InputStream( in );
				switch ( indexFormat ) {
				case 1: 
					/* Variable metrics, 4 byte offsets to image. */
					Seek_InputStream( in, dataOffset + (glyphIndex-firstGlyphIndex) * sizeof(tt_uint32) );
					offsetA = imageDataOffset + (tt_uint32)ReadInt32( in );
					offsetB = imageDataOffset + (tt_uint32)ReadInt32( in );
					break; /*****/
				case 2: 
					/* Constant metrics, constant image size. */
					offsetB = (tt_uint32)ReadInt32( in ); /* == imageSize */
					offsetA = imageDataOffset + (glyphIndex-firstGlyphIndex) * offsetB;
					offsetB = offsetA + offsetB;
					ReadBigMetrics( &(result->bigM), in );
					break; /*****/
				case 3: 
					/* Variable metrics, 2 byte offsets to image. */
					Seek_InputStream( in, dataOffset + (glyphIndex-firstGlyphIndex) * sizeof(tt_uint16) );
					offsetA = imageDataOffset + (tt_uint16)ReadInt16( in );
					offsetB = imageDataOffset + (tt_uint16)ReadInt16( in );
					break; /*****/
				case 4:
					/* Variable metrics, sparse glyph codes, 2 byte offsets to image. */
					{
						tt_uint32 numGlyphs, index;
						tt_uint16 glyphCode = 0, offset = 0;
						
						numGlyphs = (tt_uint32)ReadInt32( in );
						for ( index = 0; index < numGlyphs; index++ ) {
							glyphCode	= (tt_uint16)ReadInt16( in );
							offset		= (tt_uint16)ReadInt16( in );
							if ( glyphCode == glyphIndex ) {
								offsetA = imageDataOffset + offset;
								(tt_uint16)ReadInt16( in ); 				/* Skip glyphCode. */
								offset		= (tt_uint16)ReadInt16( in );  /* Read the next offset. */
								offsetB = imageDataOffset + offset;
								break; /*****/
							}
						}
					
					}
					break; /*****/
				case 5: 
					/* Constant metrics, sparse glyph codes, constant image size. */
					{
						tt_uint32 imageSize, numGlyphs, index;
						tt_uint16 glyphCode;
						
						imageSize = (tt_uint32)ReadInt32( in ); /* == imageSize */
						ReadBigMetrics( &(result->bigM), in);
						numGlyphs = (tt_uint32)ReadInt32( in ); /* == numGlyphs */
						for ( index = 0; index < numGlyphs; index++ ) {
							glyphCode	= (tt_uint16)ReadInt16( in );
							if ( glyphCode == glyphIndex ) {
								offsetA = imageDataOffset + index * imageSize;
								offsetB = offsetA + imageSize;
								break; /*****/
							}
						}
					
					}
					break; /*****/
				} /* switch */
				break; /*****/  /* Break out of surrounding for loop since we are done! */
			} /* if */
		} /* for */
	} /* if */
	result->offsetA		= offsetA;
	result->offsetB		= offsetB;
	result->imageFormat	= imageFormat;
	result->glyphIndex	= glyphIndex;
	return offsetA != 0; /*****/
}

/*
 * Reads a bitmap from the input stream.
 */
static tt_uint8 *CreateBitMap( tsiMemObject *mem, InputStream *in, int width, int height, int bitDepth, char greyScaleLevel, int byteAligned, tt_int32 *rowBytesPtr )
{
	register tt_int32 N, rowBytes;
	register tt_uint8 *baseAddr, *row;
	tt_uint8 nonzero = 0, *addr;
	int n,x,y, bitCount, bitBuffer;
	
	
	rowBytes	= greyScaleLevel == 0 ? (width+7) / 8 : width;
	N 			= rowBytes * height;
	
	baseAddr = row = (unsigned char*) tsi_AllocMem( mem, N * sizeof( unsigned char ) );
	
	bitCount 	= 0;
	bitBuffer 	= 0;
	if ( greyScaleLevel > 0 ) {
		if ( bitDepth == 1 ) {
			for ( y = 0; y < height; y++ ) {
				for ( x = 0; x < width; x++ ) {
					if ( bitCount-- == 0 ) {
						bitBuffer	= ReadUnsignedByteMacro( in );
						bitCount	= 7;
					}
					bitBuffer += bitBuffer; /* same as bitBuffer <= 1 */
					row[x] 	= (tt_uint8)(bitBuffer & 0x100 ? 120 : 0);
				}
				row += rowBytes;					/* Go down one scanline/row */
				if ( byteAligned ) bitCount = 0; 	/* flush out remaining pad bits */
			}
		} else {
			int depth, value, maxValue, maxValueDiv2;
			
			maxValue 		= (1 << bitDepth) - 1;
			maxValueDiv2	= maxValue >> 1;
			for ( y = 0; y < height; y++ ) {
				for ( x = 0; x < width; x++ ) {
					value = 0;
					for ( depth = 0; depth < bitDepth; depth++ ) {
						if ( bitCount-- == 0 ) {
							bitBuffer	= ReadUnsignedByteMacro( in );
							bitCount	= 7;
						}
						bitBuffer	+= bitBuffer; /* same as bitBuffer <= 1 */
						value		+= value;
						if ( bitBuffer & 0x100 ) value++;
					}
					value		= (value * 120 + maxValueDiv2) / maxValue;		/* Normalize value to the range [0,120] */
					row[x]		= (tt_uint8)value;
				}
				row += rowBytes;					/* Go down one scanline/row */
				if ( byteAligned ) bitCount = 0;	/* flush out remaining pad bits */
			}
		}
	} else {
		register tt_uint8 outByte; /* Cache in a register to minimize memory read and writes. */
		if ( bitDepth == 1 ) {
			for ( y = 0; y < height; y++ ) {
				outByte = 0;						/* Initialize to zero. */
				for ( x = 0; x < width; x++ ) {
					if ( bitCount-- == 0 ) {
						bitBuffer = ReadUnsignedByteMacro( in );
						bitCount = 7;
					}
					bitBuffer += bitBuffer; 		/* Same as bitBuffer <= 1 */
					if ( bitBuffer & 0x100 ) {
						outByte |= (0x80 >> (x&7));
					}
					if ( (x & 0x07) == 0x07 ) {
						row[x>>3] = outByte; 		/* Write out the outByte, since we now have 8 valid bits. */
						outByte = 0;
					}
				}
				if ( (x & 0x07) != 0 ) {
					row[x>>3] = outByte;			/* Write out the outByte, if there are bits remaining. */
				}
				row += rowBytes;					/* Go down one scanline/row */
				if ( byteAligned ) bitCount = 0;	/* flush out remaining pad bits */
			}
		} else {
			int depth, value, maxValue, maxValueDiv2;
			
			maxValue 		= (1 << bitDepth) - 1;
			maxValueDiv2	= maxValue >> 1;
			for ( y = 0; y < height; y++ ) {
				outByte = 0;
				for ( x = 0; x < width; x++ ) {
					value = 0;						/* Initialize to zero. */
					for ( depth = 0; depth < bitDepth; depth++ ) {
						if ( bitCount-- == 0 ) {
							bitBuffer	= ReadUnsignedByteMacro( in );
							bitCount	= 7;
						}
						bitBuffer	+= bitBuffer; 	/* Same as bitBuffer <= 1 */
						value		+= value;
						if ( bitBuffer & 0x100 ) value++;
					}
					if ( value >= maxValueDiv2 ) {
						outByte |= (0x80 >> (x&7));
					}
					if ( (x & 0x07) == 0x07 ) {
						row[x>>3] = outByte; 		/* Write out the outByte, since we now have 8 valid bits. */
						outByte = 0;
					}
				}
				if ( (x & 0x07) != 0 ) {
					row[x>>3] = outByte;			/* Write out the outByte, if there are bits remaining. */
				}
				row += rowBytes;					/* Go down one scanline/row */
				if ( byteAligned ) bitCount = 0;	/* flush out remaining pad bits */
			}
		}
	}
	*rowBytesPtr = rowBytes;

	/* return NULL if the bitmap is empty (workaround for bad fonts) */
	addr = baseAddr;
	for (n=0;n<N;n++) {
	    if (addr[n] != 0 ) {
		nonzero = 1;
		break;
	    }
	}
	if (!nonzero) {
	    tsi_DeAllocMem( mem, (char*)baseAddr );
	    baseAddr = NULL;
	}
	return baseAddr; /*****/
}


extern void  tsi_ValidateMemory( register tsiMemObject *t );

/*
 * Or bitmap-2 into bitmap-1.
 * bitmap1 = bitmap1 OR bitmap2;
 */
static void MergeBits( sbitGlypInfoData *bit1, sbitGlypInfoData *bit2, tt_uint8 xOffset2, tt_uint8 yOffset2, tt_uint8 greyScaleLevel  )
{
	register int x2, x1;
	register tt_uint8 *row1	= bit1->baseAddr;
	register tt_uint8 *end1;
	register tt_uint8 *row2	= bit2->baseAddr;
	register int width1		= bit1->bigM.width;
	register int width2		= bit2->bigM.width;
	tt_int32 rowBytes1 		= bit1->rowBytes;
	tt_int32 rowBytes2 		= bit2->rowBytes;
	int	y;
	
	if ( row1 == NULL || row2 == NULL )	return;			/****** Bail out ******/
	end1 = row1 + bit1->bigM.height * rowBytes1;
	row1 += rowBytes1 * yOffset2;						/* Apply yOffset2 */
	if ( greyScaleLevel > 0 ) {
		/* Gray scale bitmap */
		for ( y = bit2->bigM.height; y > 0; y-- ) {		/* Equivalent to for ( y = 0; y < height; y++ ), since y is not used inside the loop. */
			for ( x2 = 0, x1 = xOffset2; x2 < width2 && x1 < width1; x2++, x1++ ) {
				/* Not clear what is best, so do a fuzzy logic OR which is the same as a numeric max operator */
				register tt_uint8 v2 = row2[x2];			/* Read the value from bitmap2. */
				if ( v2 != 0 && v2 > row1[x1] ) {		/* See if the value greater than the value in bitmap 1. */
					row1[x1] = v2;						/* If so, then transfer to bitmap 1. */
				}
			}
			row1 += rowBytes1;							/* Go down one scanline/row */
			row2 += rowBytes2;
			if ( row1 >= end1 ) break; /***** out of bounds ******/
		}
	} else {
		register int bitBuffer = 0;						/* Cache input bits here to gain speed */
		/* Black and white bitmap */
		for ( y = bit2->bigM.height; y > 0; y-- ) {		/* Equivalent to for ( y = 0; y < height; y++ ), since y is not used inside the loop. */
			for ( x2 = 0, x1 = xOffset2; x2 < width2 && x1 < width1; x2++, x1++ ) {
				if ( (x2 & 7) == 0 ) {
					bitBuffer = row2[x2>>3];
				}
				bitBuffer += bitBuffer; 		/* Same as bitBuffer <= 1 */
				if ( bitBuffer & 0x100 ) {      /* Same as if ( row2[x2>>3] & (0x80 >> (x2&7)) ) */
					row1[x1>>3] |= (0x80 >> (x1&7));	/* Set the bit in bitmap 1 */
				}
			}
			row1 += rowBytes1;							/* Go down one scanline/row */
			row2 += rowBytes2;
			if ( row1 >= end1 ) break; /***** out of bounds ******/
		}
	}
}

/*
 * Scales the bitmap in the y direction.
 */
static void ScaleYBits( tt_uint8 *baseAddr1, tt_uint8* baseAddr2, tt_int32 height1, tt_int32 height2, tt_int32 rowBytes )
{
	register tt_uint8 *row1 = baseAddr1;
	register tt_uint8 *row2 = baseAddr2;
	register tt_int32 i, pos1, pos2, totalDistance;
	
	/* We note that height2 * height1 == height * height2 == total distance */
	totalDistance = height2 * height1;
	/* We can think of pos1 and pos2 as positions in bitmap 1 and 2 */
	pos2 = height1>>1;	/* Start at a pixel center */
	/* pos1 = height2>>1; since we ensure pos1 >= pos2 below, this would result in a positional error between [0,height2] */
	pos1 = height2; /* To center the positional error for pos1 add 1/2 pixel bias so error is beween [-height2/2,height2/2] */
	/* We step exactly on pixel centers in the destination bitmap, so it's positional error is zero */
	if ( height2 > height1 ) {
		/* We will increase the number scanlines. */
		while ( pos2 < totalDistance ) {
			if ( pos1 < pos2 ) {
				pos1 += height2;
				row1 += rowBytes;
			}
			/* Copy row1 to row2 */
			for ( i = 0; i < rowBytes; i++ ) row2[i] = row1[i];
			pos2 += height1;			/* Go down one scanline/row, in the output bitmap (2) */
			row2 += rowBytes;
		}
	} else {
		/* We will decrease the number of scanlines. */
		while ( pos2 < totalDistance ) {
			while ( pos1 < pos2 ) {
				pos1 += height2;
				row1 += rowBytes;
			}
			/* Copy row1 to row2 */
			for ( i = 0; i < rowBytes; i++ ) row2[i] = row1[i];
			pos2 += height1;			/* Go down one scanline/row, in the output bitmap (2) */
			row2 += rowBytes;
		}
	}
}


/*
 * Scales the bitmap in the x direction. (this is more tricky than the y direction)
 */
static void ScaleXBits( tt_uint8 *baseAddr1, tt_uint8* baseAddr2, tt_int32 height, tt_int32 width1, tt_int32 width2,
					tt_int32 rowBytes1, tt_int32 rowBytes2, tt_uint8 greyScaleLevel )
{
	register tt_uint8 *row1 = baseAddr1;
	register tt_uint8 *row2 = baseAddr2;
	register tt_int32 pos1, pos2;
	tt_int32 x1, x2, y;

	/* We note that width2 * width1 == width * width2 == total distance */
	for ( y = 0; y < height; y++ ) {
		/* We can think of pos1 and pos2 as positions in bitmap 1 and 2 */
		pos2 = width1>>1;	/* Start at a pixel center */
		/* pos1 = width2>>1; since we ensure pos1 >= pos2 below, this would result in a positional error between [0,width2[ */
		pos1 = width2; /* To center the positional error for pos1 add 1/2 pixel bias so error is beween [-width2/2,width2/2[ */
		/* We step exactly on pixel centers in the destination bitmap, so it's positional error is zero */
		/* We will increase or decrease the number of pixels. */
		if ( greyScaleLevel > 0 ) {
			/* Gray scale */
			for ( x1 = x2 = 0; x2 < width2; ) {
				while ( pos1 < pos2 ) {
					x1++;
					pos1 += width2;
				}
				/* Copy pixel from x1 to x2 */
				row2[x2] = row1[x1];
				x2++;
				pos2 += width1;
			}
		} else {
			/* Monochrome */
			register tt_uint8 outByte = 0;			/* Use a register to minimize memory reads and writes. */
			register tt_uint8 inByte = row1[0];	/* Use a register to minimize memory reads. */

			for ( x1 = x2 = 0; x2 < width2; ) {
				while ( pos1 < pos2 ) {
					x1++;
					pos1 += width2;
					inByte = (tt_uint8)(inByte + inByte); 			/* Shift left by one */
					if ( (x1 & 0x07) == 0 ) {
						inByte = row1[x1>>3];
					}
				}
				/* Copy pixel from x1 to x2 */
				if ( inByte & 0x80 ) {
					outByte |= (0x80 >> (x2&7));
				}
				if ( (x2 & 0x07) == 0x07 ) {
					row2[x2>>3] = outByte;		/* Write it out, since we now have 8 valid bits */
					outByte = 0;
				}
				x2++;
				pos2 += width1;
			}
			if ( (x2 & 0x07) != 0 ) {
				row2[(x2-1)>>3] = outByte;		/* Write it out if there are bits left. */
			}
		}
		row1 += rowBytes1;						/* Go to the next scanline. */
		row2 += rowBytes2;
	}
}


/*
 * Scales the bitmap and metrics in the X and Y direction.
 */
static void ScaleBits( tsiMemObject *mem, sbitGlypInfoData *bits, tt_uint8 greyScaleLevel )
{
	tt_int32 width2, height2;
	tt_uint8 *baseAddr2;
	tt_uint16 substitutePpemX = bits->substitutePpemX;
	tt_uint16 substitutePpemY = bits->substitutePpemY;
	tt_uint16 ppemX	= bits->ppemX;
	tt_uint16 ppemY	= bits->ppemY;
	tt_int32 width1	= bits->bigM.width;
	tt_int32 height1	= bits->bigM.height;
	int yOrder, xOrder, n;
	
	width2 	= (width1  * ppemX + (substitutePpemX>>1)) / substitutePpemX;
	height2	= (height1 * ppemY + (substitutePpemY>>1)) / substitutePpemY;
	
	/* The following logic ensures that we do x/y scaling in fastest order,
	 * since x scaling is slower than y scaling.
	 */
	yOrder = -1;	/* Initialize */
	xOrder = 0;
	if ( height2 > height1 ) {
		yOrder = 1; /* If we increase the number of scanlines do y second. */
		/* xOrder == 0, Less work if x is done first. */
	} else if ( height2 < height1 ) {
		yOrder = 0; /* Do y first since we decrease the number of scanlines. */
		xOrder = 1;	/* Less work if x is done second. */
	}
	if ( width1 == width2 ) xOrder = -1; /* Only scale x if needed. */
	
	for ( n = 0; n < 2; n++ ) {
		if ( yOrder == n ) {
			/* Y scale */
			baseAddr2 = (tt_uint8 *)tsi_AllocMem( mem, height2 * bits->rowBytes );
			ScaleYBits( bits->baseAddr, baseAddr2, height1, height2, bits->rowBytes );
			tsi_DeAllocMem( mem, bits->baseAddr );
			bits->baseAddr 			= baseAddr2; baseAddr2 = NULL;
			bits->bigM.height 		= (tt_uint16)height2;
			bits->bigM.horiBearingY = ( tt_int16)(( bits->bigM.horiBearingY * ppemY + (substitutePpemY>>1)) / substitutePpemY);
			bits->bigM.vertBearingY = ( tt_int16)(( bits->bigM.vertBearingY * ppemY + (substitutePpemY>>1)) / substitutePpemY);
			bits->bigM.vertAdvance 	= (tt_uint16)(( bits->bigM.vertAdvance  * ppemY + (substitutePpemY>>1)) / substitutePpemY);
		} else if ( xOrder == n ) {
			/* X scale */
			tt_int32 rowBytes2	= greyScaleLevel == 0 ? (width2+7) / 8 : width2;

			baseAddr2 = (tt_uint8 *)tsi_AllocMem( mem, rowBytes2 * height2 );
			ScaleXBits( bits->baseAddr, baseAddr2, height2, width1, width2, bits->rowBytes, rowBytes2, greyScaleLevel );
			tsi_DeAllocMem( mem, bits->baseAddr );
			bits->baseAddr 			= baseAddr2; baseAddr2 = NULL;
			bits->rowBytes 			= rowBytes2;
			bits->bigM.width 		= (tt_uint16)width2;
			bits->bigM.horiBearingX = ( tt_int16)(( bits->bigM.horiBearingX * ppemX + (substitutePpemX>>1)) / substitutePpemX);
			bits->bigM.vertBearingX = ( tt_int16)(( bits->bigM.vertBearingX * ppemX + (substitutePpemX>>1)) / substitutePpemX);
			bits->bigM.horiAdvance 	= (tt_uint16)(( bits->bigM.horiAdvance  * ppemX + (substitutePpemX>>1)) / substitutePpemX);
		}
	}
}


/* T2K internal local data type used for composite bitmaps. */
typedef struct {
	tt_uint16 	glyphIndex;
	tt_uint8 	xOffset, yOffset;
	sbitGlypInfoData gInfo;
} compositeGlyphDescriptor;

/*
 * Gets the bits.
 * The function is recursive.
 * sets: baseAddr, rowBytes, bigM
 * needs: GIndex -> offsetA, imageFormat, bitDepth, bigM, (mem)
 */
void ExtractBitMap_blocClass( blocClass *t, ebscClass *ebsc, sbitGlypInfoData *gInfo, InputStream *in, tt_uint32 bdatOffset, tt_uint8 greyScaleLevel, int recursionLevel )
{
	int i, numComponents;
	tt_int32 rowBytes, N;
	compositeGlyphDescriptor *comp;
	tt_uint8 *baseAddr	= NULL;
	
	rowBytes	= 0;
	Seek_InputStream( in, bdatOffset +  gInfo->offsetA );
	gInfo->smallMetricsUsed = false;

	switch( gInfo->imageFormat ) {
	case 1:
		/* Small metrics, byte-aligned data (a row is an integer number of bytes) */
		ReadSmallMetrics( &(gInfo->bigM), in );
		gInfo->smallMetricsUsed = true;
		/* Here we have the image data */
		baseAddr = CreateBitMap( t->mem, in, gInfo->bigM.width, gInfo->bigM.height, gInfo->bitDepth, greyScaleLevel, true, &rowBytes );
		break; /*****/
	case 2:
		/* Small metrics, bit-aligned data (rows are bit-aligned) */
		ReadSmallMetrics( &(gInfo->bigM), in );
		gInfo->smallMetricsUsed = true;
		/* Here we have the image data */
		baseAddr = CreateBitMap( t->mem, in, gInfo->bigM.width, gInfo->bigM.height, gInfo->bitDepth, greyScaleLevel, false, &rowBytes );
		break; /*****/
	/* Format 3 is obsolete */
	/* Format 4 is not supported, a private Apple format used in a few Apple fonts */
	case 5:
		/* metrics in EBLC/bloc, bit-aligned image data */
		/* Here we have the image data */
		baseAddr = CreateBitMap( t->mem, in, gInfo->bigM.width, gInfo->bigM.height, gInfo->bitDepth, greyScaleLevel, false, &rowBytes );
		break; /*****/
	case 6:
		/* Big metrics, byte-aligned data (a row is an integer number of bytes) */
		ReadBigMetrics( &(gInfo->bigM), in );
		/* Here we have the image data */
		baseAddr = CreateBitMap( t->mem, in, gInfo->bigM.width, gInfo->bigM.height, gInfo->bitDepth, greyScaleLevel, true, &rowBytes );
		break; /*****/
	case 7:
		/* Big metrics, bit-aligned data (rows are bit-aligned) */
		ReadBigMetrics( &(gInfo->bigM), in );
		/* Here we have the image data */
		baseAddr = CreateBitMap( t->mem, in, gInfo->bigM.width, gInfo->bigM.height, gInfo->bitDepth, greyScaleLevel, false, &rowBytes );
		break; /*****/
		
	case 8:
		/* Small metrics, component data */
		ReadSmallMetrics( &(gInfo->bigM), in );
		gInfo->smallMetricsUsed = true;
		ReadUnsignedByteMacro( in );			/* skip pad byte */
		goto readNumComponents; /*****/
	case 9:
		/* Big metrics, component data */
		ReadBigMetrics( &(gInfo->bigM), in );
		/* fall through */
		
readNumComponents:
		/* Joint code path for case 8 and 9. */
		/* Here we have the component data. */
		numComponents = (tt_uint16)ReadInt16( in );
		comp = (compositeGlyphDescriptor *) tsi_AllocMem( t->mem, numComponents * sizeof( compositeGlyphDescriptor ) );
		for ( i = 0; i < numComponents; i++ ) {
			comp[i].glyphIndex 	= (tt_uint16)ReadInt16( in );
			comp[i].xOffset		= ReadUnsignedByteMacro( in ); /* The documentation claims these are signed but that seems bogus... ---Sampo */
			comp[i].yOffset		= ReadUnsignedByteMacro( in );
		}
		/* Do FindGlyph_blocClass() after the above loop so that we do not have to skip back and forth in the input stream */
		for ( i = 0; i < numComponents; i++ ) {
			FindGlyph_blocClass( t, ebsc, in, comp[i].glyphIndex, gInfo->substitutePpemX, gInfo->substitutePpemY, &(comp[i].gInfo) );
		}
		rowBytes	= greyScaleLevel == 0 ? (gInfo->bigM.width+7) / 8 : gInfo->bigM.width;
		N			= rowBytes * gInfo->bigM.height;
	
		baseAddr = (unsigned char*) tsi_AllocMem( t->mem, N * sizeof( unsigned char ) );
		for ( i = 0; i < N; i++ ) baseAddr[i] = 0;	/* Clear it */
		gInfo->baseAddr = baseAddr;
		gInfo->rowBytes = rowBytes;
		if ( ++recursionLevel <= MAX_SBIT_RECURSION_DEPTH ) {
			for ( i = 0; i < numComponents; i++ ) {
				ExtractBitMap_blocClass( t, ebsc, &(comp[i].gInfo), in, bdatOffset, greyScaleLevel, recursionLevel );
				/* Combine this data with the gInfo bitmap */
				MergeBits( gInfo, &(comp[i].gInfo), comp[i].xOffset, comp[i].yOffset, greyScaleLevel );
				/* Free up this bitmap */
				tsi_DeAllocMem( t->mem, comp[i].gInfo.baseAddr ); comp[i].gInfo.baseAddr = NULL;
			}
		}
		tsi_DeAllocMem( t->mem, comp );
		break; /*****/
	}
	gInfo->baseAddr = baseAddr;
	gInfo->rowBytes = rowBytes;

	if ( gInfo->substitutePpemX != gInfo->ppemX || gInfo->substitutePpemY != gInfo->ppemY ) {
		ScaleBits( t->mem, gInfo, greyScaleLevel );
	}
}


#endif /* ENABLE_SBIT */
