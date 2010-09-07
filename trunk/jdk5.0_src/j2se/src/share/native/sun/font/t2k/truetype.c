/*
 * @(#)truetype.c	1.37 04/03/02
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * TRUETYPE.C
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
#include "truetype.h"
#include "HintIO.h"
#include "orion.h"
#include "util.h"
#include "autogrid.h"
#include "ghints.h"

/***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****/
/*
 *	for the flags field
 */
#define Y_POS_SPECS_BASELINE		0x0001
#define X_POS_SPECS_LSB				0x0002
#define HINTS_USE_POINTSIZE			0x0004
#define USE_INTEGER_SCALING			0x0008

#define SFNT_MAGIC 					0x5F0F3CF5

#define SHORT_INDEX_TO_LOC_FORMAT	0
#define LONG_INDEX_TO_LOC_FORMAT	1
#define GLYPH_DATA_FORMAT			0

/***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****/
/*
 * for maxp->maxStorage
 */
#define MINSTORAGE 64
/***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****/

static int IsFigure_cmapClass( cmapClass *t, tt_uint16 gIndex )
{
	register int i;
	
	assert( t != NULL );
	for ( i = 0; i < NUM_FIGURES; i++ ) {
		if ( t->figIndex[i] == gIndex ) {
			return true; /*****/
		}
	}
	return false; /*****/
}

/*
 * Format 0: Byte Encoding table
 */
static tt_uint16 Compute_cmapClass_Index0( cmapClass *t, tt_uint32 charCode )
{
	register tt_uint16 index = 0;
	register tt_uint8 *charP;
	if ( charCode < 256 ) {
		charP = t->cmapData + t->platform[t->preferedEncodingTable]->offset; /* point at subTable */
		charP += 6; 										/* skip format, length, and version */

		index = charP[charCode];
	}
	return index; /*****/
}

/*
 * Format 4: Segment mapping to delta values
 */
static tt_uint16 Compute_cmapClass_Index4( cmapClass *t, register tt_uint32 charCode )
{
	register tt_uint16 segCountX2, offset, idDelta, index, uTmp;
	register tt_uint8 *charP;

	if (charCode >= 0x10000) {
		return 0;
	}

	charP = t->cmapData + t->platform[t->preferedEncodingTable]->offset; /* point at subTable */
	charP += 6; 										/* skip format, length, and version */

	segCountX2 = *charP++; segCountX2 <<= 8; segCountX2 |= *charP++;
	charP += 6;											/* skip searchRange, entrySelector, and rangeShift */
	
	/* go one too far, so we skip the reserved word */
	/* Do a linear search, could be optimized by a binary search, later... */
	do {
		uTmp = *charP++; uTmp <<= 8; uTmp |= *charP++;
	} while ( charCode > uTmp );
	/* charP now points at startCount[] */
	
	charP += segCountX2; /* point at startCount[n] */
	uTmp = *charP; uTmp <<= 8; uTmp |= *(charP+1);
	index = 0;
	if ( charCode >= uTmp ) {
		offset  = (unsigned short)(charCode - uTmp);
		charP += segCountX2; 	/* point at idDelta[n] */
		idDelta = *charP; idDelta <<= 8; idDelta |= *(charP+1);
		charP += segCountX2; 	/* point at idRangeOffset[n] */
		uTmp = *charP; uTmp <<= 8; uTmp |= *(charP+1);
		if ( uTmp == 0 ) {
			index	= (unsigned short)(charCode + idDelta);
		} else {
			charP += uTmp + offset + offset; /* point at glyphIdArray[n] */
			uTmp = *charP; uTmp <<= 8; uTmp |= *(charP+1);
			index = (unsigned short)(uTmp + idDelta);
		}
	}
	return index; /*****/
}


/*
 * Format 6: Trimmed table mapping
 */
static tt_uint16 Compute_cmapClass_Index6( cmapClass *t, tt_uint32 charCode )
{
	register tt_uint16 entryCount, index;
	register tt_uint8 *charP;

	if (charCode >= 0x10000) {
		return 0;
	}

	charP = t->cmapData + t->platform[t->preferedEncodingTable]->offset; /* point at subTable */
	charP += 6; 										/* skip format, length, and version */

	index = *charP++;
	index <<= 8;
	index |= *charP++;
	charCode = (tt_uint32)(charCode - index); /* -firstCode */
	entryCount = *charP++;
	entryCount <<= 8;
	entryCount |= *charP++; /* entryCount */
	index = 0;
	if ( charCode < entryCount ) {
		charP += charCode; /* *2, since word Array */
		charP += charCode; /* *2, since word Array */
		index = *charP++;
		index <<= 8;
		index |= *charP++;
	} 
	return index; /*****/
}


/*
 * charCode -> glyphIndex
 */
static tt_uint16 Compute_cmapClass_GlyphIndex( cmapClass *t, tt_uint32 charCode )
{
	tt_uint16 gIndex = 0;
	
	if ( t->preferedFormat == 0 ) {
		gIndex = Compute_cmapClass_Index0( t, charCode );
	} else if ( t->preferedFormat == 6 ) {
		gIndex = Compute_cmapClass_Index6( t, charCode );
	} else if ( t->preferedFormat == 4 ) {
		gIndex = Compute_cmapClass_Index4( t, charCode );
	}

	return gIndex; /*****/
}

/*
 *
 */
static cmapClass *New_cmapClass( tsiMemObject *mem, tt_uint16 preferedPlatformID, tt_uint16 preferedPlatformSpecificID, InputStream *in )
{
	tt_int32 i, pass;
	tt_uint16 format;
	tt_uint8 *charP;
	cmapClass *t = (cmapClass *) tsi_AllocMem( mem, sizeof( cmapClass ) );
	t->mem						= mem;
	/* t->version					= ReadInt32( in ); */
	
	t->version				= ReadInt16(in);
	t->numEncodingTables	= ReadInt16(in);
	
	t->platform				= (sfnt_platformEntry **) tsi_AllocMem( mem, t->numEncodingTables * sizeof( sfnt_platformEntry * ) );

	for ( i = 0; i < t->numEncodingTables; i++ ) {
		t->platform[i] = (sfnt_platformEntry *)tsi_AllocMem( mem, sizeof( sfnt_platformEntry ) );
		t->platform[i]->platformID 	= ReadInt16(in);
		t->platform[i]->specificID	= ReadInt16(in);
		t->platform[i]->offset		= ReadInt32(in);
	}

	Rewind_InputStream( in );
	t->length = SizeInStream( in );
	t->cmapData	= (unsigned char*) tsi_AllocMem( mem, t->length );
	ReadSegment( in, t->cmapData, t->length );

	
/*	
preferedPlatformID = 3;
preferedPlatformSpecificID = 1;
*/
	
	t->preferedEncodingTable = 0;
	for ( pass = 0; pass <= 3; pass++ ) {
		for ( i = 0; i < t->numEncodingTables; i++ ) {
			charP = t->cmapData + t->platform[i]->offset;	/* point at subTable */
			
			format = *charP++;
			format <<= 8;
			format |= *charP;
			
			if ( pass == 0 && (format == 0 || format == 4 || format == 6) ) {
				int A, B;
				A = t->platform[i]->platformID == preferedPlatformID;
				B = t->platform[i]->specificID == preferedPlatformSpecificID;
				
				if ( (A && ( B || preferedPlatformSpecificID == 0xffff) ) ||
					 (B && ( A || preferedPlatformID         == 0xffff) ) ) {
					pass = 999; t->preferedEncodingTable = (short)i; break;/*****/
				}
			} else if ( pass == 1 && format == 0 ) {
				pass = 999; t->preferedEncodingTable = (short)i; break;/*****/
			} else if ( pass == 2 && format == 6 ) {
				pass = 999; t->preferedEncodingTable = (short)i; break;/*****/
			} else if ( pass == 3 && format == 4 ) {
				pass = 999; t->preferedEncodingTable = (short)i; break;/*****/
			}
		}
	}
	charP = t->cmapData + t->platform[t->preferedEncodingTable]->offset;	/* point at subTable */
	format = *charP++;
	format <<= 8;
	format |= *charP;
	t->preferedFormat = format;
	tsi_Assert( mem, t->preferedFormat == 0 || t->preferedFormat ==  6 || t->preferedFormat == 4, T2K_BAD_CMAP);
	
	{
		char c;
		i = 0;
		for ( c = '0'; c <= '9'; c++ ) {
			assert( i < NUM_FIGURES );
			t->figIndex[i] = Compute_cmapClass_GlyphIndex( t, c );
			if ( t->figIndex[i] == 0 ) t->figIndex[i] = 0xffff;
			i++;
		}
	}

	
	return t; /*****/
}


/*
 *
 */
static void Delete_cmapClass( cmapClass *t )
{
	if ( t != NULL ) {
		tt_int32 i;
		for ( i = 0; i < t->numEncodingTables; i++ ) {
			tsi_DeAllocMem( t->mem, t->platform[i] );
		}
		tsi_DeAllocMem( t->mem, t->platform );
		tsi_DeAllocMem( t->mem, t->cmapData );
		tsi_DeAllocMem( t->mem, t );
	}
}

typedef struct {
	tt_uint16	version;
	tt_uint16	numTables;
	sfnt_platformEntry platform[1];	/* platform[numTables] */
} sfnt_char2IndexDirectory;


#ifdef OLD_TMAN_EXAMPLE 

/*
 *	Char2Index structures, including platform IDs
 */
typedef struct {
	tt_uint16	format;
	tt_uint16	length;
	tt_uint16	version;
} sfnt_mappingTable;

/* JTS-working 6-1-92 adding support for Mapping Table 2 */
typedef struct {
	tt_uint16	firstCode;
	tt_uint16	entryCount;
	tt_int16	idDelta;
	tt_uint16	idRangeOffset;
}	sfnt_subHeader;

typedef struct {
	tt_uint16	subHeadersKeys[256];
	sfnt_subHeader	subHeaders[1];
}	sfnt_mappingTable2;
/* End work JTS-6-1-92 */



extern tt_uint16 sfnt_ComputeIndex2( sfnt_char2IndexDirectory *cmap, tt_uint16 charCode, tt_uint32 subTableOffset, fcg_WindowStuff *wStuff );
/*
 * Format 2: High-byte mapping through table
 */
tt_uint16 sfnt_ComputeIndex2( register sfnt_char2IndexDirectory *cmap, register tt_uint16 charCode, tt_uint32 subTableOffset, fcg_WindowStuff *wStuff )
{
	char				*charP;
	tt_uint16				index, mapMe;
	tt_uint16				highByte;
	tt_uint16				lowByte;
	tt_uint16				key;
	sfnt_mappingTable2	*Table2;
	sfnt_subHeader		*subHeader;

	charP = (char *)cmap + subTableOffset;				/* point at subTable */
	charP += 6; 										/* jump over format, length, and version */
	
	index = 0;		/* Assume the missing glyph */
	if (wStuff && wStuff->glyph->longCharCode != 0xffff)
		charCode = wStuff->glyph->longCharCode;
	
	highByte = charCode >> 8;
	
	lowByte = charCode & 0x00ff;
	
	Table2 = (sfnt_mappingTable2 *) charP;
	
	if ((key = Table2->subHeadersKeys[highByte]) != 0) {
		mapMe = lowByte;	/* We also need the low byte */
		if (wStuff)
			wStuff->glyph->numBytesUsed = 2;
	} else {
		mapMe = highByte;
		if (wStuff)
			wStuff->glyph->numBytesUsed = 1;
	}
	if ((wStuff == 0L) && (mapMe == 0)) {		/* JTS 7-2-93 for Inc-J AFM Files */
		mapMe = lowByte;
	}
	
	
	/* key >>= 3; */
	/* subHeader = &Table2->subHeaders[key]; */
	
  	subHeader = (sfnt_subHeader *) ((char  *) &Table2->subHeaders + Table2->subHeadersKeys [highByte]);
	
	if (mapMe < subHeader->firstCode)
		return 0;			/***** Missing *****/
	
	if ((Table2->subHeadersKeys[highByte] == 0) && (wStuff == 0L) && (charCode > 0xff))
		return 0;						/* single byte character (AFM fix) JTS 7-2-93 */
		
	mapMe -= subHeader->firstCode;					/* Subtract first code */


	if ( mapMe < subHeader->entryCount ) {
		tt_uint16 glyph;
		tt_uint16	*shortP;
		shortP = (tt_uint16 *)&(subHeader->idRangeOffset);
		shortP = (tt_uint16 *)((unsigned long)shortP + *shortP + mapMe + mapMe);
		
		if (*shortP)
			index = *shortP + subHeader->idDelta;
	}


#ifdef MS_CODE
  if (mapMe < subHeader->entryCount ) {
    tt_uint16 glyph;

    if (glyph = * ((tt_uint16 *) ((char *) &subHeader + subHeader->idRangeOffset) + mapMe))
      index = glyph + (tt_uint16) (subHeader->idDelta);
  }
#endif /* MS_CODE */

	return index;
}



#endif /* OLD_TMAN_EXAMPLE */

/***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****/
#ifdef ENABLE_KERNING
/***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****/
/*
 *
 */
static kernSubTable0Data *New_kernSubTable0Data( tsiMemObject *mem, InputStream *in )
{
	int i;
	kernSubTable0Data *t = (kernSubTable0Data *) tsi_AllocMem( mem, sizeof( kernSubTable0Data ) );
	t->mem				= mem;
	t->nPairs			= ReadInt16(in);
	t->searchRange		= ReadInt16(in);
	t->entrySelector	= ReadInt16(in);
	t->rangeShift		= ReadInt16(in);
	
	t->pairs			= (kernPair0Struct*) tsi_AllocMem( mem, t->nPairs * sizeof(kernPair0Struct) );
	for ( i = 0; i < t->nPairs; i++ ) {
		t->pairs[i].leftRightIndex	= (tt_uint32)ReadInt32(in);
		t->pairs[i].value 			= ReadInt16(in);
	}
#ifdef OLD
	/* Check Hoefler Text ! */
	for ( i = 0; i < t->nPairs-1; i++ ) {
		printf("%d:%d %d\n", i, t->pairs[i].leftRightIndex, t->pairs[i+1].leftRightIndex );
		if ( t->pairs[i].leftRightIndex >= t->pairs[i+1].leftRightIndex ) printf("******************************************************");
		/* assert( t->pairs[i].leftRightIndex < t->pairs[i+1].leftRightIndex ); */
	}
#endif
	
	return t; /*****/
}

static tt_int16 GetKernSubTable0Value( kernSubTable0Data *t, tt_uint16 left, tt_uint16 right )
{
	int low, mid, high;
	tt_uint32 leftRightIndex, mid_leftRightIndex;
	tt_int16 value = 0;
	
	leftRightIndex = left;
	leftRightIndex <<= 16;
	leftRightIndex |= right;
	
	low = 0;
	high = t->nPairs - 1;
	/* do a binary search */
	do {
		mid = (low + high) >> 1;
		mid_leftRightIndex = t->pairs[mid].leftRightIndex;
		if ( leftRightIndex > mid_leftRightIndex ) {
			low = mid+1;
		} else if ( leftRightIndex < mid_leftRightIndex ) {
			high = mid-1;
		} else { /* mid_leftRightIndex == leftRightIndex */
			value = t->pairs[mid].value;
			break; /*****/
		}
	} while ( high >= low );

	return value; /*****/
}

#ifdef ENABLE_PRINTF
static void Print_kernSubTable0Data( kernSubTable0Data *t )
{
	int i;
	printf("nPairs:         %d\n", t->nPairs);
	printf("searchRange:    %d\n", t->searchRange);
	printf("entrySelector:  %d\n", t->entrySelector);
	printf("rangeShift:     %d\n", t->rangeShift);
	
	for ( i = 0; i < t->nPairs; i++ ) {
		printf("%d: <%d - %d> -> %d\n", i, (tt_uint16)(t->pairs[i].leftRightIndex >> 16), (tt_uint16)(t->pairs[i].leftRightIndex & 0xffff), t->pairs[i].value );
		/*
		assert( GetKernSubTable0Value( t, (tt_uint16)(t->pairs[i].leftRightIndex >> 16), (tt_uint16)(t->pairs[i].leftRightIndex & 0xffff) ) == t->pairs[i].value );
		*/
	  }
}
#endif

/*
 *
 */
static void Delete_kernSubTable0Data( kernSubTable0Data *t )
{
	if ( t != NULL ) {
		tsi_DeAllocMem( t->mem, t->pairs );
		tsi_DeAllocMem( t->mem, t );
	}
}

/*
 *
 */
static kernSubTable *New_kernSubTable( tsiMemObject *mem, int appleFormat, InputStream *in )
{
	kernSubTable *t = (kernSubTable *) tsi_AllocMem( mem, sizeof( kernSubTable ) );
	t->mem			= mem;
	
	if ( appleFormat ) {
		t->length  		= ReadInt32(in);
		t->coverage		= (tt_uint16)ReadInt16(in);
		ReadInt16(in); /* The tuple index */
		t->version 		= (tt_uint16)(t->coverage & 0x00ff);
	} else {
		t->version		= (tt_uint16)ReadInt16(in);
		t->length		= (tt_uint16)ReadInt16(in);
		t->coverage		= (tt_uint16)ReadInt16(in);
	}
	
	t->data			= NULL;
	if ( t->version == 0 && t->length > 0 ) {
		t->data			= New_kernSubTable0Data( mem, in );
	}
	
	return t; /*****/
}

#ifdef ENABLE_PRINTF
static void Print_kernSubTable(kernSubTable *t) {
	printf("---------- Begin kern subtable----------\n");
	printf("version:    	%d\n", t->version);
	printf("length:    		%d\n", t->length);
	printf("coverage:    	%d\n", t->coverage);
	if ( t->version == 0 ) {
		Print_kernSubTable0Data( (kernSubTable0Data *)t->data );
	}
	printf("----------  End kern  subtable----------\n");
}
#endif

/*
 *
 */
static void Delete_kernSubTable( kernSubTable *t )
{
	if ( t->version == 0 ) {
		Delete_kernSubTable0Data( (kernSubTable0Data *)t->data );
	}
	tsi_DeAllocMem( t->mem, t );
}

/*
 *
 */
static kernClass *New_kernClass( tsiMemObject *mem, InputStream *in )
{
	int i;
	int appleFormat = 0;
	
	kernClass *t = (kernClass *) tsi_AllocMem( mem, sizeof( kernClass ) );
	t->mem			= mem;
	t->version		= ReadInt16(in);
	t->nTables		= ReadInt16(in);
	
	if ( t->version == 1 && t->nTables == 0 ) { /* Apple 1.0 in 16.16 format */
		t->nTables 	= ReadInt32(in);
		appleFormat	= 1;
	}
	
	t->table		= (kernSubTable **) tsi_AllocMem( mem, t->nTables * sizeof( kernSubTable * ) );
	for ( i = 0; i < t->nTables; i++ ) {
		t->table[i] = New_kernSubTable( mem, appleFormat, in );
	}
	
	return t; /*****/
}

static tt_int16 GetKernValue( kernClass *t, tt_uint16 left, tt_uint16 right )
{
	tt_int16 value = 0;
	if ( t->nTables > 0 && t->table[0]->version == 0 ) {
		value = GetKernSubTable0Value( (kernSubTable0Data *)t->table[0]->data, left, right );
	}

	return value; /*****/
}


#ifdef ENABLE_PRINTF
static void Print_kernClass(kernClass *t) {
	int i;
	printf("---------- Begin kern ----------\n");
	printf("version:    	%d\n", t->version);
	printf("nTables:    	%d\n", t->nTables);
	for ( i = 0; i < t->nTables; i++ ) {
		Print_kernSubTable( t->table[i] );
	}
	printf("----------  End kern  ----------\n");
}
#endif


/*
 *
 */
static void Delete_kernClass( kernClass *t )
{
	if ( t != NULL ) {
		int i;
		for ( i = 0; i < t->nTables; i++ ) {
			Delete_kernSubTable( t->table[i] );
		}
		tsi_DeAllocMem( t->mem, t->table );
		tsi_DeAllocMem( t->mem, t );
	}
}

#ifdef ENABLE_KERNING
 void GetSfntClassKernValue( sfntClass *t, tt_uint16 leftGIndex, tt_uint16 rightGIndex, tt_int16 *xKern, tt_int16 *yKern )
{
	*xKern = 0;
	*yKern = 0;
	if ( t->kern != NULL ) {
		*xKern = GetKernValue( t->kern, leftGIndex, rightGIndex );
	}
}
#endif /* ENABLE_KERNING */




/***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****/
#endif /* ENABLE_KERNING */
/***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****/


/*
 *
 */
static sfnt_DirectoryEntry *New_sfnt_DirectoryEntry( tsiMemObject *mem, InputStream *in )
{
	sfnt_DirectoryEntry *t = (sfnt_DirectoryEntry *) tsi_AllocMem( mem, sizeof( sfnt_DirectoryEntry ) );
	t->mem			= mem;
	t->tag			= ReadInt32(in);
	t->checkSum		= ReadInt32(in);
	t->offset		= ReadInt32(in);
	t->length		= ReadInt32(in);
	
	return t; /*****/
}


#ifdef ENABLE_WRITE
/*
 *
 */
static sfnt_DirectoryEntry *New_sfnt_EmptyDirectoryEntry( tsiMemObject *mem )
{
	sfnt_DirectoryEntry *t = (sfnt_DirectoryEntry *) tsi_AllocMem( mem, sizeof( sfnt_DirectoryEntry ) );
	t->mem			= mem;
	t->tag			= 0;
	t->checkSum		= 0;
	t->offset		= 0;
	t->length		= 0;
	
	return t; /*****/
}
#endif /* ENABLE_WRITE */

#ifdef ENABLE_WRITE
static void Write_sfnt_DirectoryEntry( sfnt_DirectoryEntry *t, OutputStream *out )
{
	WriteInt32( out, t->tag );
	WriteInt32( out, t->checkSum );
	WriteInt32( out, t->offset );
	WriteInt32( out, t->length );
}
#endif

#ifdef ENABLE_PRINTF
static void Print_sfnt_DirectoryEntry( sfnt_DirectoryEntry *t, tt_int32 i)
{
	printf("%d : %c%c%c%c offset = %ld, length = %ld\n", i,
								(char)((t->tag >> 24) & 0xff),
								(char)((t->tag >> 16) & 0xff),
								(char)((t->tag >>  8) & 0xff),
								(char)((t->tag >>  0) & 0xff),
							    t->offset, t->length );

}
#endif	


/*
 *
 */
static void Delete_sfnt_DirectoryEntry( sfnt_DirectoryEntry *t )
{
	tsi_DeAllocMem( t->mem, t );
}

/***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****/
/***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****/



/*
 *
 */
static sfnt_OffsetTable *New_sfnt_OffsetTable( tsiMemObject *mem, InputStream *in )
{
	tt_int32 i;
	sfnt_OffsetTable *t = (sfnt_OffsetTable *) tsi_AllocMem( mem, sizeof( sfnt_OffsetTable ) );
	t->mem				= mem;
	t->version			= ReadInt32(in);
	t->numOffsets		= ReadInt16(in);
	t->searchRange		= ReadInt16(in);
	t->entrySelector	= ReadInt16(in);
	t->rangeShift		= ReadInt16(in);
	
	t->table 			= (sfnt_DirectoryEntry **) tsi_AllocMem( mem, t->numOffsets * sizeof(sfnt_DirectoryEntry *) );
	for ( i = 0; i < t->numOffsets; i++ ) {
		t->table[i] = New_sfnt_DirectoryEntry( mem, in );
		/* Print_sfnt_DirectoryEntry( t->table[i], i ); */
	}
	return t; /*****/
}

#ifdef OLD_OLD_ENABLE_WRITE
/*
 *
 */
static sfnt_OffsetTable *New_sfnt_EmptyOffsetTable( tsiMemObject *mem, short numberOfOffsets )
{
	tt_int32 i;
	sfnt_OffsetTable *t = (sfnt_OffsetTable *) tsi_AllocMem( mem, sizeof( sfnt_OffsetTable ) );
	t->mem				= mem;
	t->version			= 0x10000;
	t->numOffsets		= numberOfOffsets;
	t->searchRange		= 0;
	t->entrySelector	= 0;
	t->rangeShift		= 0;
	
	t->table 			= (sfnt_DirectoryEntry **) tsi_AllocMem( mem, numberOfOffsets * sizeof(sfnt_DirectoryEntry *) );
	for ( i = 0; i < t->numOffsets; i++ ) {
		t->table[i] = New_sfnt_EmptyDirectoryEntry( mem );
	}
	return t; /*****/
}
#endif /* OLD_OLD_ENABLE_WRITE */



#ifdef ENABLE_WRITE
static void SortTableDirectory( sfnt_OffsetTable *offsetTable )
{
	int						j, swap;
	sfnt_DirectoryEntry		temp;
	
	for ( swap = true; swap; ) {	/* Bubble-Sort it */
		swap = false;
		for ( j = offsetTable->numOffsets-2; j >= 0; j--) {
			if ( (unsigned long)(offsetTable->table[j]->tag) > (unsigned long)(offsetTable->table[j+1]->tag) ) {
				swap 					  = true;
				temp 					  = *offsetTable->table[j];
				*offsetTable->table[j] 	  = *offsetTable->table[j+1];
				*offsetTable->table[j+1]  = temp;
			}
		}
	}
}
#endif /* ENABLE_WRITE */



#ifdef ENABLE_WRITE
/*
 *
 */
static void Recompute_OffsetTableFields( sfnt_OffsetTable *t )
{
	register unsigned short i;

	t->searchRange = 0;
	for ( i = 1; i <= t->numOffsets*16; i = (unsigned short)(i+i) ) {
		t->searchRange = i;
	}
	for ( i = 1; (1<<i) <= t->numOffsets; i++ ) {
		t->entrySelector = i;
	}
	t->rangeShift = (short)(t->numOffsets*16 - t->searchRange);
}
#endif /* ENABLE_WRITE */

#ifdef ENABLE_WRITE

/*
 *
 */
static void CreateTableIfMissing( sfnt_OffsetTable *t, tt_int32 tag )
{
	tt_int32 i;
	int found = false;
	
	for ( i = 0; i < t->numOffsets; i++ ) {
		if ( t->table[i]->tag == tag ) {
			found = true;
			break; /*****/
		}
	}
	if ( !found ) {
#ifdef ENABLE_PRINTF
printf("Adding Table %c%c%c%c\n", (char) (tag>>24)&0xff,(char) (tag>>16)&0xff,(char) (tag>>8)&0xff,(char) (tag>>0)&0xff ); 
#endif
		i = t->numOffsets++;
		t->table = (sfnt_DirectoryEntry **)tsi_ReAllocMem( t->mem, t->table , t->numOffsets * sizeof(sfnt_DirectoryEntry *) );
		assert( t->table != NULL );
		t->table[i] = New_sfnt_EmptyDirectoryEntry( t->mem );
		t->table[i]->tag = tag;
		SortTableDirectory(t);
		Recompute_OffsetTableFields(t);
	}
}
#endif /* ENABLE_WRITE */

#ifdef ENABLE_WRITE

/*
 *
 */
static void DeleteTable( sfnt_OffsetTable *t, tt_int32 tag )
{
	tt_int32 i;
	int found = false;
	
	for ( i = 0; i < t->numOffsets; i++ ) {
		if ( t->table[i]->tag == tag ) {
			found = true;
			break; /*****/
		}
	}
	if ( found ) {
#ifdef ENABLE_PRINTF
printf("Deleting Table %c%c%c%c\n", (char) (tag>>24)&0xff,(char) (tag>>16)&0xff,(char) (tag>>8)&0xff,(char) (tag>>0)&0xff );
#endif
		Delete_sfnt_DirectoryEntry( t->table[i] );
		t->numOffsets--;
		t->table[i] = t->table[t->numOffsets];
		SortTableDirectory(t);
		Recompute_OffsetTableFields(t);
	}
}
#endif /* ENABLE_WRITE */


#ifdef ENABLE_WRITE
/*
 *
 */
static void Write_sfnt_OffsetTable( sfnt_OffsetTable *t, OutputStream *out )
{
	register tt_int32 i;
	
	Recompute_OffsetTableFields( t );
	WriteInt32( out, t->version );
	WriteInt16( out, t->numOffsets );
	WriteInt16( out, t->searchRange );
	WriteInt16( out, t->entrySelector );
	WriteInt16( out, t->rangeShift );

	for ( i = 0; i < t->numOffsets; i++ ) {
		Write_sfnt_DirectoryEntry( t->table[i], out );
	}
}
#endif /* ENABLE_WRITE */

/*
 *
 */
static void Delete_sfnt_OffsetTable( sfnt_OffsetTable *t )
{
	if ( t != NULL ) {
		tt_int32 i;
		
		for ( i = 0; i < t->numOffsets; i++ ) {
			Delete_sfnt_DirectoryEntry( t->table[i] );
		}
		tsi_DeAllocMem( t->mem, t->table );
		tsi_DeAllocMem( t->mem, t );
	}
}


/***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****/
/***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****/
/*
 *
 */
static hheaClass *New_hheaClass( tsiMemObject *mem, InputStream *in )
{
	hheaClass *t = (hheaClass *) tsi_AllocMem( mem, sizeof( hheaClass ) );
	t->mem						= mem;
	t->version					= ReadInt32( in );
	t->Ascender					= ReadInt16( in );
	t->Descender				= ReadInt16( in );
	t->LineGap					= ReadInt16( in );
	t->advanceWidthMax			= ReadInt16( in );
	t->minLeftSideBearing		= ReadInt16( in );
	t->minRightSideBearing		= ReadInt16( in );
	t->xMaxExtent				= ReadInt16( in );
	t->caretSlopeRise			= ReadInt16( in );
	t->caretSlopeRun			= ReadInt16( in );
	t->caretOffset				= ReadInt16( in );
	t->reserved2				= ReadInt16( in );
	t->reserved3				= ReadInt16( in );
	t->reserved4				= ReadInt16( in );
	t->reserved5				= ReadInt16( in );
	t->metricDataFormat			= ReadInt16( in );
	t->numberOfHMetrics			= ReadInt16( in );
	
	return t; /*****/
}

#ifdef OLD_OLD_ENABLE_WRITE
static void Write_hheaClass( hheaClass *t, OutputStream *out) {
	WriteInt32( out, t->version );
	
	WriteInt16( out, t->Ascender );
	WriteInt16( out, t->Descender );
	WriteInt16( out, t->LineGap );
	WriteInt16( out, t->advanceWidthMax );
	WriteInt16( out, t->minLeftSideBearing );
	WriteInt16( out, t->minRightSideBearing );
	WriteInt16( out, t->xMaxExtent );
	WriteInt16( out, t->caretSlopeRise );
	WriteInt16( out, t->caretSlopeRun );
	WriteInt16( out, t->reserved1 );
	WriteInt16( out, t->reserved2 );
	WriteInt16( out, t->reserved3 );
	WriteInt16( out, t->reserved4 );
	WriteInt16( out, t->reserved5 );
	WriteInt16( out, t->metricDataFormat );
	WriteInt16( out, t->numberOfHMetrics );

}
#endif /* OLD_OLD_ENABLE_WRITE */


#ifdef ENABLE_PRINTF
static void Print_hheaClass(hheaClass *t) {
	printf("---------- Begin hhea ----------\n");
	printf("Ascender:         %ld\n", (long)t->Ascender);
	printf("Descender:        %ld\n", (long)t->Descender);
	printf("LineGap:          %ld\n", (long)t->LineGap);
	printf("metricDataFormat: %ld\n", (long)t->metricDataFormat);
	printf("numberOfHMetrics: %ld\n", (long)t->numberOfHMetrics);
	printf("caretSlopeRise:   %ld\n", (long)t->caretSlopeRise);
	printf("caretSlopeRun:    %ld\n", (long)t->caretSlopeRun);
	printf("caretOffset:      %ld\n", (long)t->caretOffset);
	printf("----------  End hhea  ----------\n");
}
#endif

/*
 *
 */
static void Delete_hheaClass( hheaClass *t )
{
	if ( t != NULL ) {
		tsi_DeAllocMem( t->mem, t );
	}
}

/***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****/
/***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****/

hmtxClass *New_hmtxEmptyClass( tsiMemObject *mem, tt_int32 numGlyphs, tt_int32 numberOfHMetrics )
{
	hmtxClass *t = (hmtxClass *) tsi_AllocMem( mem, sizeof( hmtxClass ) );
	/* Some fonts have more htmx entries than glyphs */
	int cnt = (numGlyphs>numberOfHMetrics) ? numGlyphs : numberOfHMetrics;
	t->mem		 = mem;
	t->numGlyphs = numGlyphs;
	t->numberOfHMetrics = numberOfHMetrics;

	t->lsb = (short *) tsi_AllocMem( mem, cnt * sizeof(tt_int16) );  assert( t->lsb != NULL );
	t->aw  = (unsigned short *) tsi_AllocMem( mem, cnt * sizeof(tt_uint16) ); assert( t->aw != NULL );
	
	return t; /*****/
}


static hmtxClass *New_hmtxClass( tsiMemObject *mem, InputStream *in, tt_int32 numGlyphs, tt_int32 numberOfHMetrics )
{
	register tt_int32 i;
	tt_uint16 last_aw;
	hmtxClass *t;

#ifdef OLD
	/* NB If this code is revived see New_hmtxEmptyClass for the
	 * correct array size allocation */
	t 			 = (hmtxClass *) tsi_AllocMem( mem, sizeof( hmtxClass ) );
	t->mem		 = mem;
	t->numGlyphs = numGlyphs;
	t->numberOfHMetrics = numberOfHMetrics;
	t->lsb = (short *) tsi_AllocMem( mem, numGlyphs * sizeof(tt_int16) );  assert( t->lsb != NULL );
	t->aw  = (unsigned short *) tsi_AllocMem( mem, numGlyphs * sizeof(tt_uint16) ); assert( t->aw != NULL );
#endif /*****/
	
	t = New_hmtxEmptyClass( mem, numGlyphs, numberOfHMetrics );
	
	for ( i = 0; i < numberOfHMetrics; i++ ) {
		t->aw[i]  = ReadInt16( in );
		t->lsb[i] = ReadInt16( in );
	}	
    /* We check for presence of the second subtable to workaround crash reported in the bug 4987174.
       NB: we want to start using more generic (and robust) solution later on (see bug XXXX). */
    if (SizeInStream(in) >= (numberOfHMetrics*4+(numGlyphs-numberOfHMetrics)*2)) { 
  	  for ( last_aw = t->aw[i-1]; i < numGlyphs; i++ ) {
		t->aw[i]  = last_aw;
		t->lsb[i] = ReadInt16( in );
	  }
    } else {
      for ( last_aw = t->aw[i-1]; i < numGlyphs; i++ ) {
		t->aw[i]  = last_aw;
		t->lsb[i] = 0;
	  }
    }
	return t; /*****/
}

static hmtxClass *Copy_hmtxClass(tsiMemObject *mem, hmtxClass* hmtx) {
	register tt_int32 i;
	tt_uint16 last_aw;
	hmtxClass *t;
	
	t = New_hmtxEmptyClass(mem, hmtx->numGlyphs, hmtx->numberOfHMetrics);
	
	for ( i = 0; i < hmtx->numberOfHMetrics; i++ ) {
	    t->aw[i]  = hmtx->aw[i];
	    t->lsb[i] = hmtx->lsb[i];
	}	
	for ( last_aw = t->aw[i-1]; i < hmtx->numGlyphs; i++ ) {
		t->aw[i]  = last_aw;
		t->lsb[i] = hmtx->lsb[i];
	}
	return t;
}

#ifdef ENABLE_WRITE
static void Write_hmtxClass( hmtxClass *t, OutputStream *out) {
	register tt_int32 i;
	for ( i = 0; i < t->numberOfHMetrics; i++ ) {
		WriteInt16( out, t->aw[i] );
		WriteInt16( out, t->lsb[i] );
	}	
	for ( ;i < t->numGlyphs; i++ ) {
		WriteInt16( out, t->lsb[i] );
	}	
}
#endif /* ENABLE_WRITE */
#ifdef ENABLE_PRINTF
static void Print_hmtxClass(hmtxClass *t) {
	register tt_int32 i;
	printf("---------- Begin hmtx ----------\n");
	for ( i = 0; i < 3; i++ ) {
		printf("aw[%d]  = %ld\n", i, (long)t->aw[i] );
		printf("lsb[%d] = %ld\n", i, (long)t->lsb[i] );
	}
	printf("...\n");
	for ( i = t->numGlyphs - 3; i < t->numGlyphs; i++ ) {
		printf("aw[%d]  = %ld\n", i, (long)t->aw[i] );
		printf("lsb[%d] = %ld\n", i, (long)t->lsb[i] );
	}
	printf("----------  End hmtx  ----------\n");
}
#endif

/*
 *
 */
void Delete_hmtxClass( hmtxClass *t )
{
	if ( t != NULL ) {
		tsi_DeAllocMem( t->mem, t->lsb );
		tsi_DeAllocMem( t->mem, t->aw );
		tsi_DeAllocMem( t->mem, t );
	}
}


/***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****/

static ttcfClass *New_ttcfClass( tsiMemObject *mem, InputStream *in )
{
	tt_uint32 info;
	ttcfClass *t = NULL;
	tt_uint32 i;
	
	info = (tt_uint32)ReadInt32( in );
	if ( info == tag_TTCollectionID ) {
		t 					= (ttcfClass *) tsi_AllocMem( mem, sizeof( ttcfClass ) );
		t->mem				= mem;
		t->version			= (tt_uint32)ReadInt32( in );
		t->directoryCount	= (tt_uint32)ReadInt32( in );
		t->tableOffsets		= (tt_uint32 *) tsi_AllocMem( mem, sizeof(tt_uint32 *) * t->directoryCount ); 
		for ( i = 0; i < t->directoryCount; i++ ) {
			t->tableOffsets[i] = (tt_uint32)ReadInt32( in ); 
		}
#ifdef OLD
		{
			
			printf("ttcf:version = 0x%x\n", t->version );
			printf("ttcf:directoryCount = %d\n", t->directoryCount );
			for ( i = 0; i < t->directoryCount; i++ ) {
				printf("ttcf:tableOffsets[%d] = %d\n", i, t->tableOffsets[i] );
			}
		}
#endif
	}
	Rewind_InputStream( in );
	return t; /*****/
}

/*
 *
 */
static void Delete_ttcfClass( ttcfClass *t )
{
	if ( t != NULL ) {
		tsi_DeAllocMem( t->mem, t->tableOffsets );
		tsi_DeAllocMem( t->mem, t );
	}
}
/***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****/


/*
 *
 */
static headClass *New_headClass( tsiMemObject *mem, InputStream *in )
{
	headClass *t = (headClass *) tsi_AllocMem( mem, sizeof( headClass ) );
	t->mem					= mem;
	t->version				= ReadInt32( in );
	t->fontRevision			= ReadInt32( in );
	t->checkSumAdjustment	= ReadInt32( in );
	t->magicNumber			= ReadInt32( in );
			
	t->flags				= ReadInt16( in );
	t->unitsPerEm			= ReadInt16( in );
			
	t->created_bc			= ReadInt32( in );
	t->created_ad			= ReadInt32( in );
	t->modified_bc			= ReadInt32( in );
	t->modified_ad			= ReadInt32( in );
			
	t->xMin					= ReadInt16( in );
	t->yMin					= ReadInt16( in );
	t->xMax					= ReadInt16( in );
	t->yMax					= ReadInt16( in );

	t->macStyle				= ReadInt16( in );
	t->lowestRecPPEM		= ReadInt16( in );
	t->fontDirectionHint	= ReadInt16( in );
	t->indexToLocFormat		= ReadInt16( in );
	t->glyphDataFormat		= ReadInt16( in );
	
	return t; /*****/
}

#ifdef ENABLE_WRITE
static void Write_headClass( headClass *t, OutputStream *out) {
	WriteInt32( out, t->version );
	WriteInt32( out, t->fontRevision );
	WriteInt32( out, t->checkSumAdjustment );
	WriteInt32( out, t->magicNumber );

	WriteInt16( out, t->flags );
	WriteInt16( out, t->unitsPerEm );

	WriteInt32( out, t->created_bc );
	WriteInt32( out, t->created_ad );
	WriteInt32( out, t->modified_bc );
	WriteInt32( out, t->modified_ad );

	WriteInt16( out, t->xMin );
	WriteInt16( out, t->yMin );
	WriteInt16( out, t->xMax );
	WriteInt16( out, t->yMax );

	WriteInt16( out, t->macStyle );
	WriteInt16( out, t->lowestRecPPEM );
	WriteInt16( out, t->fontDirectionHint );
	WriteInt16( out, t->indexToLocFormat );
	WriteInt16( out, t->glyphDataFormat );
}
#endif /* ENABLE_WRITE */

#ifdef ENABLE_PRINTF
static void Print_headClass(headClass *t) {
	printf("---------- Begin head ----------\n");
	printf("unitsPerEm:       %ld\n", t->unitsPerEm);
	printf("lowestRecPPEM:    %ld\n", t->lowestRecPPEM);
	printf("glyphDataFormat:  %ld\n", t->glyphDataFormat);
	printf("----------  End head  ----------\n");
}
#endif

/*
 *
 */
static void Delete_headClass( headClass *t )
{
	if ( t != NULL ) {
		tsi_DeAllocMem( t->mem, t );
	}
}

/***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****/
/***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****/

/*
 *
 */
static maxpClass *New_maxpClass( tsiMemObject *mem, InputStream *in )
{
	maxpClass *t = (maxpClass *) tsi_AllocMem( mem, sizeof( maxpClass ) );
	t->mem						= mem;
	t->version					= ReadInt32( in );
	t->numGlyphs				= (tt_uint16)ReadInt16( in );
	t->maxPoints				= ReadInt16( in );
	t->maxContours				= ReadInt16( in );
	t->maxCompositePoints		= ReadInt16( in );
	t->maxCompositeContours		= ReadInt16( in );
	t->maxElements				= ReadInt16( in );
	t->maxTwilightPoints		= ReadInt16( in );
	t->maxStorage				= ReadInt16( in );
	if (t->maxStorage < MINSTORAGE) {
	  t->maxStorage = MINSTORAGE;
	}
	t->maxFunctionDefs			= ReadInt16( in );
	t->maxInstructionDefs		= ReadInt16( in );
	t->maxStackElements			= ReadInt16( in );
	t->maxSizeOfInstructions	= ReadInt16( in );
	t->maxComponentElements		= ReadInt16( in );
	t->maxComponentDepth		= ReadInt16( in );
	
	return t; /*****/
}

#ifdef ENABLE_WRITE
static void Write_maxpClass( maxpClass *t, OutputStream *out ) {
	WriteInt32( out, t->version );
		
	WriteInt16( out, t->numGlyphs			);
	WriteInt16( out, t->maxPoints			);
	WriteInt16( out, t->maxContours			);
	WriteInt16( out, t->maxCompositePoints	);
	WriteInt16( out, t->maxCompositeContours);
	WriteInt16( out, t->maxElements			);
	WriteInt16( out, t->maxTwilightPoints	);
	WriteInt16( out, t->maxStorage			);
	WriteInt16( out, t->maxFunctionDefs		);
	WriteInt16( out, t->maxInstructionDefs	);
	WriteInt16( out, t->maxStackElements	);
	WriteInt16( out, t->maxSizeOfInstructions );
	WriteInt16( out, t->maxComponentElements );
	WriteInt16( out, t->maxComponentDepth	);
}
#endif /* ENABLE_WRITE */


#ifdef ENABLE_PRINTF
static void Print_maxpClass( maxpClass *t ) {
	printf("---------- Begin maxp ----------\n");
	printf("numGlyphs:             %ld\n", (long)t->numGlyphs);
	printf("maxPoints:             %ld\n", (long)t->maxPoints);
	printf("maxContours:           %ld\n", (long)t->maxContours);
	printf("maxCompositePoints:    %ld\n", (long)t->maxCompositePoints);
	printf("maxCompositeContours:  %ld\n", (long)t->maxCompositeContours);
	printf("maxElements:           %ld\n", (long)t->maxElements);
	printf("maxTwilightPoints:     %ld\n", (long)t->maxTwilightPoints);
	printf("maxStorage:            %ld\n", (long)t->maxStorage);
	printf("maxFunctionDefs:       %ld\n", (long)t->maxFunctionDefs);
	printf("maxInstructionDefs:    %ld\n", (long)t->maxInstructionDefs);
	printf("maxStackElements:      %ld\n", (long)t->maxStackElements);
	printf("maxSizeOfInstructions: %ld\n", (long)t->maxSizeOfInstructions);
	printf("maxComponentElements:  %ld\n", (long)t->maxComponentElements);
	printf("maxComponentDepth:     %ld\n", (long)t->maxComponentDepth);
	printf("----------  End maxp  ----------\n");
}
#endif

/*
 *
 */
static void Delete_maxpClass( maxpClass *t )
{
	if ( t != NULL ) {
		tsi_DeAllocMem( t->mem, t );
	}
}


/***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****/
/***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****/


/*
 *
 */
static locaClass *New_locaClass( tsiMemObject *mem, InputStream *in, short indexToLocFormat, tt_int32 length )
{
	register tt_int32 i, n;
	locaClass *t = (locaClass *) tsi_AllocMem( mem, sizeof( locaClass ) );
	t->mem		= mem;
	/* n 			= numGlyphs + 1; */
	
	n = length >> (1 + indexToLocFormat); /* Modified 6/29/98 --- Sampo */

	t->n 				= n;
	t->indexToLocFormat = indexToLocFormat;
	t->offsets  		= (tt_int32 *) tsi_AllocMem( mem, n * sizeof( tt_int32 ) );
	
	if ( indexToLocFormat ==  1 ) {
		for ( i = 0; i < n; i++ ) {
			t->offsets[i] = ReadInt32( in );
		}
	} else if ( indexToLocFormat == 0 ) {
		for ( i = 0; i < n; i++ ) {
			tt_uint16 offset = (tt_uint16)ReadInt16( in );
			t->offsets[i] = 2L * (tt_int32)offset;
		}
	} else {
		assert( false );
		/* printf( "locaClass:Bad indexToLocFormat\n"); */
	}
	return t; /*****/
}


#ifdef ENABLE_WRITE
static void Write_locaClass( locaClass *t, OutputStream *out ) {
	register tt_int32 i, n = t->n;
	t->indexToLocFormat = 0;
	
	for ( i = 0; i < n; i++ ) {
		if ( t->offsets[i] > 0x0000ffff * 2 ) {
			t->indexToLocFormat = 1;
			break; /*****/
		}
	}
	
	if ( t->indexToLocFormat ==  1 ) {
		for ( i = 0; i < n; i++ ) {
			WriteInt32( out, t->offsets[i] );
		}
	} else {
		for ( i = 0; i < n; i++ ) {
			WriteInt16( out, (tt_int16)(t->offsets[i]/2) );
		}
	}
}
#endif /* ENABLE_WRITE */



#ifdef ENABLE_PRINTF
static void Print_locaClass( locaClass *t ) {
	register tt_int32 i;
	printf("---------- Begin loca ----------\n");
	for ( i = 0; i < 3; i++ ) {
		printf("offset[%d] = %ld\n", i, t->offsets[i] );
	}
	printf("...\n");
	for ( i = t->n - 3; i < t->n; i++ ) {
		printf("offset[%d] = %ld\n", i, t->offsets[i] );
	}
	printf("----------  End loca  ----------\n");
}
#endif

/*
 *
 */
static void Delete_locaClass( locaClass *t )
{
	if ( t != NULL ) {
		tsi_DeAllocMem( t->mem, t->offsets );
		tsi_DeAllocMem( t->mem, t );
	}
}


/***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****/
/***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****/


/*
 * Outline Unpacking Constants
*/
#define ONCURVE			0x01
#define XSHORT			0x02
#define YSHORT			0x04
#define REPEAT_FLAGS	0x08 /* repeat flag n times */
/* IF XSHORT */
#define SHORT_X_IS_POS	0x10 /* the short vector is positive */
/* ELSE */
#define NEXT_X_IS_ZERO	0x10 /* the relative x coordinate is zero */
/* ENDIF */
/* IF YSHORT */
#define SHORT_Y_IS_POS	0x20 /* the short vector is positive */
/* ELSE */
#define NEXT_Y_IS_ZERO	0x20 /* the relative y coordinate is zero */
/* ENDIF */
/* 0x40, 0x80 are reserved */


/* #define FLIPTEST */
#ifdef FLIPTEST
/*
 *
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
#endif


/*
 *
 */
static GlyphClass *New_GlyphClass( tsiMemObject *mem, register InputStream *in, char readHints, tt_int16 lsb, tt_uint16 aw )
{
	register tt_int32 i, j;
	register short *oox, *ooy;
	register tt_uint8 *onCurve;
	register int pointCount;
	GlyphClass *t = (GlyphClass *) tsi_AllocMem( mem, sizeof( GlyphClass ) );
	
        t->oox  = NULL;
        t->ooy  = NULL;
        t->onCurve = NULL;
        t->componentSizeMax = 0;	
	t->mem				= mem;
	oox	= ooy			= NULL;
	t->sp	= t->ep		= NULL;
	onCurve				= NULL;
	t->hintFragment		= NULL;
	t->componentData	= NULL;
	t->x = t->y 		= NULL;
	

	t->contourCount			= ReadInt16( in );
	t->xmin 				= ReadInt16( in );
	t->ymin 				= ReadInt16( in );
	t->xmax 				= ReadInt16( in );
	t->ymax 				= ReadInt16( in );
	pointCount 				= 0;
 	t->componentSize		= 0;
 	t->hintLength 			= 0;
 	t->hintFragment			= NULL;
 	
 	t->colorPlaneCount		= 0;
 	t->colorPlaneCountMax	= 0;
#ifdef ENABLE_T2KE
 	t->colors				= NULL;
#endif
	t->curveType			= 2;
 	t->contourCountMax		= 0;
 	t->pointCountMax		= 0;
        if (t->contourCount == 0) {
            tsi_DeAllocMem(mem, (char*)t);
            return NULL;
        }
	if ( t->contourCount < 0 ) {
		/* Composite Glyph */
		short flags;
 		int weHaveInstructions;
 		register short *componentData;
 		register tt_int32 componentSize = 0;
 		
 		t->componentSizeMax		= 1024;
 		componentData			= (short *) tsi_AllocMem( t->mem, t->componentSizeMax * sizeof(short) );
 		do {
 			if ( componentSize >= t->componentSizeMax - 10 ) {
 				/* Reallocate */
 				t->componentSizeMax += t->componentSizeMax/2;
 				componentData = (short *) tsi_ReAllocMem( t->mem, componentData, t->componentSizeMax * sizeof(short) );
 				assert( componentData != NULL );
 			}
 			flags = ReadInt16( in );
 			weHaveInstructions = (flags & WE_HAVE_INSTRUCTIONS) != 0;
 			componentData[ componentSize++] = flags;
 			
 			componentData[ componentSize++] = ReadInt16 (in); /* the glyph index */
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
 		if ( weHaveInstructions ) {
			t->hintLength = ReadInt16( in );
			if ( readHints ) {
				if ( t->hintLength > 0 ) {
					t->hintFragment = (tt_uint8 *) tsi_AllocMem( t->mem, t->hintLength * sizeof( tt_uint8 ) );
					ReadSegment( in, t->hintFragment, t->hintLength );
				 	/* Print_glyphClassInstructions(  t , glyphIndex); */
			    }
		    } else {
	    		in->pos += t->hintLength; t->hintLength = 0;
		    }
 		}
		oox			= (short *) tsi_AllocMem( t->mem, (pointCount+2) * 2 * sizeof(short) );
		ooy			= &oox[ pointCount+2 ];
		t->componentData = componentData;
		t->componentSize = componentSize;
	} else if ( t->contourCount > 0 ) {
		tt_uint8 flag;
		short stmp = 0;
		/* Regular Glyph */
		t->sp = (short *) tsi_AllocMem( t->mem, sizeof(short) * 2 * t->contourCount);
		t->ep   = &t->sp[t->contourCount];
		for ( i = 0; i < t->contourCount; i++ ) {
		    t->sp[i]	= stmp;
		    t->ep[i]    = ReadInt16( in );
		 	stmp = (short)(t->ep[i] + 1);
		}
                if (stmp == 1) {
                    tsi_DeAllocMem(mem, (char*)t->componentData);
                    tsi_DeAllocMem(mem, (char*)oox);
                    tsi_DeAllocMem(mem, (char*)t->sp);
                    tsi_DeAllocMem(mem, (char*)t);
                    return NULL;
                }
		pointCount = stmp;
		assert( pointCount > 0 );

		t->hintLength = ReadInt16( in );
		if ( readHints ) {
			if ( t->hintLength > 0 ) {
				t->hintFragment = (tt_uint8 *) tsi_AllocMem( t->mem, t->hintLength * sizeof( tt_uint8 ) );
				ReadSegment( in, t->hintFragment, t->hintLength );
				/* 	Print_glyphClassInstructions(  t , -1);*/
		    }
		} else {
	    	in->pos += t->hintLength; t->hintLength = 0;
		}
	    
	    /* flags	 		= tsi_AllocMem( t->mem, pointCount * sizeof(tt_uint8) ); */
		/* onCurve			= tsi_AllocMem( t->mem, pointCount * sizeof(tt_uint8) ); */
		oox				= (short *) tsi_AllocMem( t->mem, (pointCount+2) * ( 2 * sizeof(short) + sizeof(tt_uint8)) );
		ooy				= &oox[ pointCount+2 ];
		onCurve			= (tt_uint8 *)&ooy[ pointCount+2 ];
			
	 	t->contourCountMax		= t->contourCount;
	 	t->pointCountMax		= (short)pointCount;
	    for ( i = 0; i < pointCount; ) {
	        onCurve[i++] = flag = ReadUnsignedByteMacro(in);
	        if ( (flag & REPEAT_FLAGS) != 0 ) {
	            for ( j = ReadUnsignedByteMacro(in); j-- > 0 && i < pointCount; ) {
	            	onCurve[i++] = flag;
	            }
	        }
	    }
	
		/* Do x */
		stmp = 0;
		for ( i = 0; i < pointCount; i++ ) {
			flag = onCurve[i];
		 	if ( (flag & XSHORT) != 0 ) {
		 		if ( (flag & SHORT_X_IS_POS) != 0 ) {
		 			stmp = (short)(stmp + ReadUnsignedByteMacro(in));
		 		} else {
		 			stmp = (short)(stmp - ReadUnsignedByteMacro(in));
		 		}
		 	} else if ( (flag & NEXT_X_IS_ZERO) == 0 ) {
		 		stmp = (short)(stmp + ReadInt16( in ));
		 	}
			oox[i] = stmp;
		}
		/* Do y and onCurve */
		stmp = 0;
		for ( i = 0; i < pointCount; i++ ) {
			flag = onCurve[i];
		 	if ( (flag & YSHORT) != 0 ) {
		 		if ( (flag & SHORT_Y_IS_POS) != 0 ) {
		 			stmp = (short)(stmp + ReadUnsignedByteMacro(in));
		 		} else {
		 			stmp = (short)(stmp - ReadUnsignedByteMacro(in));
		 		}
		 	} else if ( (flag & NEXT_Y_IS_ZERO) == 0 ) {
		 		stmp = (short)(stmp + ReadInt16( in ));
		 	}
			ooy[i] = stmp;
			onCurve[i] = (unsigned char)((flag & ONCURVE) != 0);
		}
		assert( onCurve != NULL );
		/* tsi_DeAllocMem( t->mem, flags ); */
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
	/* printf("contourCount = %ld, pointCount = %ld\n", (long)t->contourCount, (long)t->pointCount ); */
	
#ifdef FLIPTEST
	FlipContourDirection( t );
#endif
	
	return t; /*****/
}

#ifdef ENABLE_WRITE
/*
 *
 */
static tt_int32 Write_GlyphClass( GlyphClass *t, OutputStream *out )
{
	tt_int32 i, j;
	int lastPoint = t->pointCount - 1;
	short x, y, delta;
	tt_uint8 *f;

	if ( t == NULL )  return SizeOutStream(out); /*****/
	if ( t->contourCount == 0 && t->componentSize == 0 ) return SizeOutStream(out); /*****/ /* Empty glyph */
	WriteInt16( out, (tt_int16)(t->componentSize > 0 ? COMPONENTCTRCOUNT : t->contourCount) );

	/* bbox */
	if ( t->componentSize == 0 ) {
		t->xmin = t->xmax = t->oox[0];
		t->ymin = t->ymax = t->ooy[0];
		for ( i = 1; i <= lastPoint; i++ ) {
			x = t->oox[i];
			y = t->ooy[i];
			if ( x > t->xmax ) {
				t->xmax = x;
			} else if ( x < t->xmin  ) {
				t->xmin = x;
			}
			if ( y > t->ymax ) {
				t->ymax = y;
			} else if ( y < t->ymin  ) {
				t->ymin = y;
			}
		}
	}
	
	WriteInt16( out, t->xmin ); WriteInt16( out, t->ymin );
	WriteInt16( out, t->xmax ); WriteInt16( out, t->ymax );

	if ( t->componentSize > 0 ) {
#ifdef OLD
		for ( i = 0; i < t->componentSize; i++ ) {
			WriteInt16( out, t->componentData[i] );
		}

		if ( t->hintLength > 0 ) {
			WriteInt16( out, t->hintLength );
			Write( out, t->hintFragment, t->hintLength );
		}
#endif
		/* Composite Glyph */
		tt_int16 flags;
 		i = 0;
 		do {
 			flags = t->componentData[i++];
 			if ( t->hintLength > 0 ) {
 				flags |= WE_HAVE_INSTRUCTIONS;
 			} else {
 				flags &= ~WE_HAVE_INSTRUCTIONS;
 			}
 			WriteInt16( out, flags );
			
 			WriteInt16( out, t->componentData[ i++ ] ); /* glyphIndex */
 			if ( (flags & ARG_1_AND_2_ARE_WORDS) != 0 ) {
 				/* arg 1 and 2 */
 				WriteInt16( out, t->componentData[ i++ ] );
 				WriteInt16( out, t->componentData[ i++ ] );
 			} else {
 				/* arg 1 and 2 as bytes */
 				WriteInt16( out, t->componentData[ i++ ] );
 			}
 			
 			if ( (flags & WE_HAVE_A_SCALE) != 0 ) {
 				/* scale */
 				WriteInt16( out, t->componentData[ i++ ] );
 			} else if ( (flags & WE_HAVE_AN_X_AND_Y_SCALE) != 0 ) {
 				/* xscale, yscale */
 				WriteInt16( out, t->componentData[ i++ ] );
 				WriteInt16( out, t->componentData[ i++ ] );
 			} else if ( (flags & WE_HAVE_A_TWO_BY_TWO) != 0 ) {
 				/* xscale, scale01, scale10, yscale */
 				WriteInt16( out, t->componentData[ i++ ] );
 				WriteInt16( out, t->componentData[ i++ ] );
 				WriteInt16( out, t->componentData[ i++ ] );
 				WriteInt16( out, t->componentData[ i++ ] );
 			}
 		} while ( (flags & MORE_COMPONENTS) != 0 );
 		assert( i == t->componentSize );
 		if ( t->hintLength > 0 ) {
 			WriteInt16( out, (tt_int16)t->hintLength );
			Write( out, t->hintFragment, t->hintLength );
 		}


		/* out.flush(); */
		return SizeOutStream(out); /*****/
	}


	for ( i = 0; i < t->contourCount; i++ ) {
		WriteInt16( out, t->ep[i] );
	}

	WriteInt16( out, (tt_int16)t->hintLength ); 
	Write( out, t->hintFragment, t->hintLength );


	f = (tt_uint8 *)tsi_AllocMem( t->mem, t->pointCount * sizeof(tt_uint8) );
	/* Calculate flags */
	x = y = 0;
	for ( i = 0; i <= lastPoint; i++ ) {
		tt_uint8 bitFlags = (tt_uint8)(t->onCurve[i] ? ONCURVE : 0);
		
		delta = (short)(t->oox[i] - x); x = (short)t->oox[i];
		if ( delta == 0 ) {
			bitFlags |= NEXT_X_IS_ZERO;
		} else if ( delta >= -255 && delta <= 255 ) {
			bitFlags |= XSHORT;
			if ( delta > 0 ) {
				bitFlags |= SHORT_X_IS_POS;
			}
		}

		delta = (short)(t->ooy[i] - y); y = (short)t->ooy[i];
		if ( delta == 0 ) {
			bitFlags |= NEXT_Y_IS_ZERO;
		} else if ( delta >= -255 && delta <= 255 ) {
			bitFlags |= YSHORT;
			if ( delta > 0 ) {
				bitFlags |= SHORT_Y_IS_POS;
			}
		}
		f[i] = bitFlags;
	}

	/* Write out bitFlags */
	for ( i = 0; i <= lastPoint;) {
		short repeat = 0;
		for ( j = i+1; j <= lastPoint && f[i] == f[j] && repeat < 255; j++ ) {
			repeat++;
		}
		if ( repeat > 1 ) {
			WriteUnsignedByte( out, (tt_uint8)(f[i] | REPEAT_FLAGS) );
			WriteUnsignedByte( out, (tt_uint8)repeat );
			i = j;
		} else {
			WriteUnsignedByte( out, f[i++] );
		}
	}

	/* Write out X */
	x = 0;
	for ( i = 0; i <= lastPoint; i++ ) {
		delta = (short)(t->oox[i] - x); x = (short)t->oox[i];
		if ( (f[i] & XSHORT) != 0 ) {
			if ( (f[i] & SHORT_X_IS_POS) == 0 ) delta = (short)-delta;
			WriteUnsignedByte( out, (tt_uint8)delta );
		} else if ( (f[i] & NEXT_X_IS_ZERO ) == 0 ) {
			WriteInt16( out, delta );
		}
	}

	/* Write out Y */
	y = 0;
	for ( i = 0; i <= lastPoint; i++ ) {
		delta = (short)(t->ooy[i] - y); y = (short)t->ooy[i];
		if ( (f[i] & YSHORT) != 0 ) {
			if ( (f[i] & SHORT_Y_IS_POS) == 0 ) delta = (short)-delta;
			WriteUnsignedByte( out, (tt_uint8)delta );
		} else if ( (f[i] & NEXT_Y_IS_ZERO ) == 0 ) {
			WriteInt16( out, delta );
		}
	}
	assert( f != NULL );
	tsi_DeAllocMem( t->mem, f );
	/* out.flush(); */
	return SizeOutStream(out); /*****/
}

#endif /* ENABLE_WRITE */

#ifdef NOT_NEEDED
static GlyphClass *Clone_GlyphClass( GlyphClass *base )
{
	GlyphClass *t = tsi_AllocMem( mem, sizeof( GlyphClass ) );
	int pointCount = base->pointCount;
	
	
	t->mem				= base->mem;
	t->sp	= t->ep		= NULL;
	onCurve				= NULL;
	t->hintFragment		= NULL;
	t->componentData	= NULL;
	t->x = t->y 		= NULL;
	

	t->contourCount			= ReadInt16( in );
	t->xmin 				= ReadInt16( in );
	t->ymin 				= ReadInt16( in );
	t->xmax 				= ReadInt16( in );
	t->ymax 				= ReadInt16( in );
	pointCount 				= 0;
 	t->componentSize		= 0;
 	t->hintLength 			= 0;
 	t->hintFragment			= NULL;
 	
	t->sp = tsi_AllocMem( t->mem, sizeof(short) * 2 * t->contourCount);
	t->ep   = &t->sp[t->contourCount];
	
	t->oox				= tsi_AllocMem( t->mem, (pointCount+2) * ( 2 * sizeof(short) + sizeof(tt_uint8)) );
	t->ooy				= &oox[ pointCount+2 ];
	t->onCurve			= (tt_uint8 *)&ooy[ pointCount+2 ];
	
 	return t;
}
#endif


/***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****/
/***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****/

sfnt_DirectoryEntry *GetTableDirEntry_sfntClass( sfntClass *t, tt_int32 tag ) {
	tt_int32 i;
	for ( i = 0; i < t->offsetTable0->numOffsets; i++ ) {
		if ( t->offsetTable0->table[i]->tag == tag ) {
			return t->offsetTable0->table[i]; /*****/
		}
	}
	return NULL; /******/
}

/* caller need to do Delete_InputStream on the stream */
InputStream *GetStreamForTable( sfntClass *t, tt_int32 tag  )
{
	InputStream *stream = NULL;
	sfnt_DirectoryEntry *dirEntry;
	
	dirEntry = GetTableDirEntry_sfntClass( t, tag );
	if ( dirEntry != NULL ) {
		stream = New_InputStream2( t->mem, t->in, dirEntry->offset, dirEntry->length, NULL );
	}
	return stream; /*****/
}


static void CacheKeyTables_sfntClass( sfntClass *t, InputStream *in, tt_int32 logicalFontNumber )
{
	InputStream *stream;
	sfnt_DirectoryEntry *dirEntry;
	
	Delete_ttcfClass( t->ttcf );
	Delete_sfnt_OffsetTable( t->offsetTable0);
	Delete_headClass( t->head );	t->head = NULL;
	Delete_hheaClass( t->hhea );	t->hhea = NULL;
	Delete_hheaClass( t->vhea );	t->vhea = NULL;
	Delete_hmtxClass( t->hmtxPlain);      t->hmtxPlain      = NULL;
	Delete_hmtxClass( t->hmtxBold);       t->hmtxBold       = NULL;
	Delete_hmtxClass( t->hmtxItalic);     t->hmtxItalic     = NULL;
	Delete_hmtxClass( t->hmtxBoldItalic); t->hmtxBoldItalic = NULL;
	Delete_maxpClass( t->maxp );
	Delete_locaClass( t->loca );	t->loca = NULL;
#ifdef ENABLE_TT_HINTING
       	Delete_fpgmClass( t->fpgm );t->fpgm = NULL;
       	Delete_prepClass( t->prep );t->prep = NULL;
       	Delete_cvtClass( t->cvt );	t->cvt = NULL;
#endif


#ifdef ENABLE_KERNING
	Delete_kernClass( t->kern );
#endif
#ifdef ENABLE_SBIT
	Delete_blocClass( t->bloc );
	Delete_ebscClass( t->ebsc );
#endif


	/* The initial optional collection data */
	t->ttcf = New_ttcfClass( t->mem, in);

	if ( t->ttcf != NULL ) {
		assert( logicalFontNumber >=0 && logicalFontNumber < (tt_int32)t->ttcf->directoryCount );
		Seek_InputStream( in, t->ttcf->tableOffsets[ logicalFontNumber ] );
	}
	/* The initial offset table */
	t->offsetTable0 = New_sfnt_OffsetTable( t->mem, in );

	/* The head table */
	dirEntry = GetTableDirEntry_sfntClass( t, tag_FontHeader );
#ifdef ENABLE_SBIT
	if ( dirEntry == NULL ) {
		dirEntry = GetTableDirEntry_sfntClass( t, tag_BFontHeader ); /* Try alternative form */
	}
#endif /* ENABLE_SBIT */
	if ( dirEntry != NULL ) {
		stream = New_InputStream2( t->mem, in, dirEntry->offset, dirEntry->length, NULL );
		t->head = New_headClass( t->mem, stream );
		Delete_InputStream( stream, NULL );
		#ifdef VERBOSE
			Print_headClass( t->head );
		#endif
	}

	/* The hhea table */
	dirEntry = GetTableDirEntry_sfntClass( t, tag_HoriHeader );
	if ( dirEntry != NULL ) {
		stream = New_InputStream2( t->mem, in, dirEntry->offset, dirEntry->length , NULL );
		t->hhea = New_hheaClass( t->mem, stream );
		Delete_InputStream( stream, NULL );
		#ifdef VERBOSE
			Print_hheaClass( t->hhea );
		#endif
	}
	/* The vhea table */
	dirEntry = GetTableDirEntry_sfntClass( t, tag_VertHeader );
	if ( dirEntry != NULL ) {
		stream = New_InputStream2( t->mem, in, dirEntry->offset, dirEntry->length , NULL );
		t->vhea = New_hheaClass( t->mem, stream );
		Delete_InputStream( stream, NULL );
		#ifdef VERBOSE
			Print_hheaClass( t->vhea );
		#endif
	}
	
	/* The maxp table */
	dirEntry = GetTableDirEntry_sfntClass( t, tag_MaxProfile );
	stream = New_InputStream2( t->mem, in, dirEntry->offset, dirEntry->length, NULL );
	t->maxp = New_maxpClass( t->mem, stream );
	Delete_InputStream( stream, NULL );
	#ifdef VERBOSE
		Print_maxpClass( t->maxp );
	#endif


	/* The loca table */
	dirEntry = GetTableDirEntry_sfntClass( t, tag_IndexToLoc );
	if ( dirEntry != NULL ) {
		stream = New_InputStream2( t->mem, in, dirEntry->offset, dirEntry->length, NULL );
		t->loca = New_locaClass( t->mem, stream, t->head->indexToLocFormat, dirEntry->length );
		Delete_InputStream( stream, NULL );
		#ifdef VERBOSE
			Print_locaClass( t->loca );
		#endif
	}

	/* The hmtx table */
	dirEntry = GetTableDirEntry_sfntClass( t, tag_HorizontalMetrics );
	if ( dirEntry != NULL ) {
		stream = New_InputStream2( t->mem, in, dirEntry->offset, dirEntry->length, NULL );
		t->hmtxPlain = New_hmtxClass( t->mem, stream, GetNumGlyphs_sfntClass(t), t->hhea->numberOfHMetrics );
		Delete_InputStream( stream, NULL );
		t->hmtxLinearAdjustment= 0;
		t2k_SetHmtx(t);
		#ifdef VERBOSE
			Print_hmtxClass( t->hmtx );
		#endif
	}
#ifdef ENABLE_TT_HINTING

	/* The cvt table */

       	dirEntry = GetTableDirEntry_sfntClass( t, tag_ControlValue );
	if ( dirEntry != NULL ) {
	      tt_int32 numFWords=dirEntry->length>>1;
		stream = New_InputStream2( t->mem, in, dirEntry->offset, dirEntry->length, NULL );
	       	t->cvt = New_cvtClass( t->mem, stream,numFWords);
	       	Delete_InputStream( stream, NULL );
	}

	/* The fpgm table */
	dirEntry = GetTableDirEntry_sfntClass( t, tag_FontProgram );

	if ( dirEntry != NULL && dirEntry->length > 0) {
	   tt_int32 numInstructions=dirEntry->length;
	   stream = New_InputStream2( t->mem, in, dirEntry->offset, dirEntry->length, NULL );
	   t->fpgm = New_fpgmClass( t->mem, stream,numInstructions);
	   Delete_InputStream( stream, NULL );
	}	
		
	/* The prep table */
	dirEntry =  GetTableDirEntry_sfntClass( t, tag_PreProgram );
	if ( dirEntry != NULL ) {
	   tt_int32 numInstructions=dirEntry->length;
	   stream = New_InputStream2( t->mem, in, dirEntry->offset, dirEntry->length, NULL );
	   t->prep = New_prepClass( t->mem, stream,numInstructions);
	   Delete_InputStream( stream, NULL );
	}	

#ifdef TT_ENABLE_PRINTF
      	Print_cvtClass( t->cvt );
	Print_fpgmClass( t->fpgm );
	Print_prepClass( t->prep );
#endif		
#endif	



#ifdef ENABLE_KERNING
	/* The kern table */
	dirEntry = GetTableDirEntry_sfntClass( t, tag_Kerning );
	
	/* HACK to avoid unknown skia kerning format, added 1/14/98 */
	if ( dirEntry != NULL && GetTableDirEntry_sfntClass( t, tag_fvar ) != NULL ) {
		dirEntry = NULL; /* pretend we have no kern table */
	}
	
	t->kern = NULL;
	if ( dirEntry != NULL ) {
		stream = New_InputStream2( t->mem, in, dirEntry->offset, dirEntry->length, NULL );
		t->kern = New_kernClass( t->mem, stream );
		Delete_InputStream( stream, NULL );
		#ifdef ENABLE_PRINTF
			/* Print_kernClass( t->kern ); */
		#endif
	}

#endif

#ifdef ENABLE_ORION
	/* Delete_OrionModelClass( (OrionModelClass *)t->model ); */
#endif	
#ifdef ENABLE_ORION
	if ( t->model == NULL ) {
		dirEntry = GetTableDirEntry_sfntClass( t, tag_T2KC );
		if ( dirEntry != NULL ) {
			stream = New_InputStream2( t->mem, in, dirEntry->offset, dirEntry->length, NULL );
			t->model = (void *)New_OrionModelClassFromStream( t->mem, stream );
			Delete_InputStream( stream, NULL );
		}
	}
#endif

#ifdef ENABLE_SBIT
	t->bdatOffset = 0;
	t->bloc = NULL;
	t->ebsc = NULL;
	dirEntry = GetTableDirEntry_sfntClass( t, tag_EBLC );
	if ( dirEntry == NULL ) {
		dirEntry = GetTableDirEntry_sfntClass( t, tag_bloc );
	}
	if ( dirEntry != NULL ) {
		/* do not use New_InputStream2 here */
		Seek_InputStream( in, dirEntry->offset );
		t->bloc = New_blocClass( t->mem, t->loca == NULL, in );
	
		dirEntry = GetTableDirEntry_sfntClass( t, tag_EBDT );
		if ( dirEntry == NULL ) {
			dirEntry = GetTableDirEntry_sfntClass( t, tag_bdat );
		}
		if ( dirEntry != NULL ) {
			F16Dot16 version;

			Seek_InputStream( in, dirEntry->offset );
			version = (F16Dot16)ReadInt32( in );
			if ( version >= 0x00020000 && version < 0x00030000 ) { /* Ok we know this format */
				t->bdatOffset = dirEntry->offset;
			}
		}
	}
	dirEntry = GetTableDirEntry_sfntClass( t, tag_EBSC );
	if ( (dirEntry != NULL) && (dirEntry->length > 0) ) {
		stream = New_InputStream2( t->mem, in, dirEntry->offset, dirEntry->length, NULL );
		t->ebsc = New_ebscClass( t->mem, stream );
		Delete_InputStream( stream, NULL );
	}
#endif

}



tt_int32 GetNumGlyphs_sfntClass( sfntClass *t )
{
	tt_int32 n;
#ifdef ENABLE_T1
	if ( t->T1 != NULL ) {
		return t->T1->NumCharStrings; /******/
	}
#endif
#ifdef ENABLE_CFF
	if ( t->T2 != NULL ) {
		return t->T2->NumCharStrings; /******/
	}
#endif
	assert( t->maxp != NULL );
	n = t->maxp->numGlyphs;
	if ( t->loca != NULL ) {
		if ( t->loca->n <= n ) { /* Added 6/29/98 --- Sampo */
			n = t->loca->n - 1;
		}
	}
	return n; /******/
}

#ifdef ALGORITHMIC_STYLES
#include "shapet.h"
#endif

/*
 *
 */
GlyphClass *GetGlyphByIndex( sfntClass *t, tt_int32 index, char readHints, unsigned short *aw )
{
	GlyphClass *glyph = NULL;
	sfnt_DirectoryEntry *dirEntry;

#ifdef ENABLE_T1
	if ( t->T1 != NULL ) {
		glyph = tsi_T1GetGlyphByIndex( t->T1, (tt_uint16)index, aw );
		#ifdef ALGORITHMIC_STYLES
		#ifndef ENABLE_TT_HINTING
			if ( t->StyleFunc != NULL ) {
				t->StyleFunc( glyph, t->mem, GetUPEM( t ), t->params );
			}
		#endif
		#endif
		return glyph; /*****/
	}
#endif	
#ifdef ENABLE_CFF
	if ( t->T2 != NULL ) {
		glyph = tsi_T2GetGlyphByIndex( t->T2, (tt_uint16)index, aw );
		#ifdef ALGORITHMIC_STYLES
		#ifndef ENABLE_TT_HINTING
			if ( t->StyleFunc != NULL ) {
				t->StyleFunc( glyph, t->mem, GetUPEM( t ), t->params );
			}
		#endif
		#endif
		return glyph; /*****/
	}
#endif	
#ifdef ENABLE_T2KE
	if ( t->T2KE != NULL && t->head->glyphDataFormat == 2001 ) {
		glyph = tsi_T2KEGetGlyphByIndex( t->T2KE, (tt_uint16)index, aw );
		#ifdef ALGORITHMIC_STYLES
		#ifndef ENABLE_TT_HINTING
			if ( t->StyleFunc != NULL ) {
				t->StyleFunc( glyph, t->mem, GetUPEM( t ), t->params );
			}
		#endif
		#endif
		return glyph; /*****/
	}
#endif
	dirEntry  = GetTableDirEntry_sfntClass( t, tag_GlyphData );
	
	if ( dirEntry != NULL && t->loca != NULL && t->hmtx != NULL &&
		 index >= 0 && index < GetNumGlyphs_sfntClass(t) ) {
		InputStream *stream = NULL;
		tt_uint32 offset1 = t->loca->offsets[index];
		tt_uint32 offset2 = t->loca->offsets[index+1];
		tt_uint32 length = offset2 - offset1;

		if ( offset2 > offset1  ) {
			stream = New_InputStream2( t->mem, t->in, dirEntry->offset + offset1, length, NULL );
			/* printf("index = %ld, offset = %ld length = %ld\n", index, offset1, length ); */
			if ( t->head->glyphDataFormat == 2000 ) {
				glyph = New_GlyphClassT2K( t->mem, stream, readHints, t->hmtx->lsb[index], t->hmtx->aw[index], t->model );
			} else {
				assert( t->head->glyphDataFormat == 0 );
				glyph = New_GlyphClass( t->mem, stream, readHints, t->hmtx->lsb[index], t->hmtx->aw[index] );
			}

			Delete_InputStream( stream, NULL );
			if (glyph == NULL) {
                          glyph = New_EmptyGlyph( t->mem, t->hmtx->lsb[index], t->hmtx->aw[index] );
                        }
#ifdef ALGORITHMIC_STYLES
#ifndef ENABLE_TT_HINTING

			else if ( t->StyleFunc != NULL ) {
				t->StyleFunc( glyph, t->mem, GetUPEM( t ), t->params );
			}
#endif
#endif
		} else {
			/* No outlines... We land here for the space character */
			glyph = New_EmptyGlyph( t->mem, t->hmtx->lsb[index], t->hmtx->aw[index] );
		}
		*aw = t->hmtx->aw[index];
	} else {
		/* printf("index = %ld\n", index ); */
		glyph = New_EmptyGlyph( t->mem, 0, 0);
		/* assert( false ); */
	}

	if (glyph)
	  glyph->gIndex=index;

	return glyph; /*****/
}

static void LoadCMAP( sfntClass *t )
{
	if ( t->cmap == NULL ) {
		InputStream *stream;
		sfnt_DirectoryEntry *dirEntry;

		dirEntry = GetTableDirEntry_sfntClass( t, tag_CharToIndexMap );
		stream = New_InputStream2( t->mem, t->in, dirEntry->offset, dirEntry->length, NULL );
		t->cmap = New_cmapClass( t->mem, t->preferedPlatformID, t->preferedPlatformSpecificID, stream );
		Delete_InputStream( stream, NULL );
	}
}

tt_uint16 GetSfntClassGlyphIndex( sfntClass *t, tt_uint32 charCode )
{
	tt_uint16 gIndex;
	
#ifdef ENABLE_T1
	if ( t->T1 != NULL ) {
		gIndex = tsi_T1GetGlyphIndex( t->T1, charCode );
		return gIndex; /*****/
	}
#endif
#ifdef ENABLE_CFF
	if ( t->T2 != NULL ) {
		gIndex = tsi_T2GetGlyphIndex( t->T2, charCode );
		return gIndex; /*****/
	}
#endif

	/* assert( t->cmap != NULL ); */
	LoadCMAP( t );
	
	gIndex = Compute_cmapClass_GlyphIndex( t->cmap, (tt_uint32)charCode );
	return gIndex; /*****/
}



int IsFigure( sfntClass *t, tt_uint16 gIndex )
{

#ifdef ENABLE_T1
	if ( t->T1 != NULL ) {
		/*tt_uint16 charCode = t->T1->charCode[gIndex];
		return ( charCode >= '0' && charCode <= '9' );*/
		tt_uint16 glyphIndex0 = tsi_T1GetGlyphIndex( t->T1, '0' );
		tt_uint16 glyphIndex9 = tsi_T1GetGlyphIndex( t->T1, '9' );
		return ( gIndex >= glyphIndex0 && gIndex <= glyphIndex9 );
	}
#endif
#ifdef ENABLE_CFF
	if ( t->T2 != NULL ) {
		/* SOON */
		return false; /*****/
	}
#endif

	LoadCMAP( t );
	return IsFigure_cmapClass( t->cmap, gIndex ); /*****/
}


/*
 *
 */
GlyphClass *GetGlyphByCharCode( sfntClass *t, tt_int32 charCode, char readHints, unsigned short *aw )
{
	tt_int32 index;
	
#ifdef ENABLE_T1
	if ( t->T1 != NULL ) {
		GlyphClass *glyph;
		glyph =  tsi_T1GetGlyphByCharCode( t->T1, (tt_uint16)charCode, aw ); /*****/
		#ifdef ALGORITHMIC_STYLES
		#ifndef ENABLE_TT_HINTING
			if ( t->StyleFunc != NULL ) {
				t->StyleFunc( glyph, t->mem, GetUPEM( t ), t->params );
			}
		#endif
		#endif
		return glyph; /*****/
	}
#endif	
#ifdef ENABLE_CFF
	if ( t->T2 != NULL ) {
		GlyphClass *glyph;
		glyph =  tsi_T2GetGlyphByCharCode( t->T2, (tt_uint16)charCode, aw ); /*****/
		#ifdef ALGORITHMIC_STYLES
		#ifndef ENABLE_TT_HINTING
			if ( t->StyleFunc != NULL ) {
				t->StyleFunc( glyph, t->mem, GetUPEM( t ), t->params );
			}
		#endif
		#endif
		return glyph; /*****/
	}
#endif	
	
	LoadCMAP( t );
	index = Compute_cmapClass_GlyphIndex( t->cmap, (tt_uint16)charCode );
	return GetGlyphByIndex( t, index, readHints, aw ); /*****/
}

/*
 * t->StyleMetricsFunc, if present indicates algorithmic bolding and is the
 * function that adjusts horizontal advances based on the bolding factor.
 * t->params[0] is the bolding factor to use with algorithmic styling
 * t->params[1] is the italicisation factor  to use with algorithmic styling
 * Therefore this method installs the appropriate advances for the currently
 * specified algorithmic bolding requirements. Note that it assumes that the
 * factors are fixed for this font.
 */
void t2k_SetHmtx(sfntClass *t) {
    if (t == NULL || t->hmtxPlain == NULL) {
	return;
    }
    if (t->StyleMetricsFunc == NULL ||
	(t->params[0] == ONE16Dot16 && t->params[1] == 0)) {
	t->hmtx = t->hmtxPlain;
	t->hmtxLinearAdjustment = 0;
    } else if (t->params[0] != ONE16Dot16 && t->params[1] == 0) {
	if (t->hmtxBold == NULL) {
	    t->hmtxBold = Copy_hmtxClass(t->mem, t->hmtxPlain);
	    t->hmtxLinearAdjustmentBold =
		t->StyleMetricsFunc(t->hmtxBold, t->mem,
				    GetUPEM(t), t->params);
	}
	t->hmtx = t->hmtxBold;
	t->hmtxLinearAdjustment = t->hmtxLinearAdjustmentBold;
    } else if (t->params[0] == ONE16Dot16 && t->params[1] != 0) {
	if (t->hmtxItalic == NULL) {
	    t->hmtxItalic = Copy_hmtxClass(t->mem, t->hmtxPlain);
	    t->hmtxLinearAdjustmentItalic =
		t->StyleMetricsFunc(t->hmtxItalic, t->mem,
				    GetUPEM(t), t->params);
	}
	t->hmtx = t->hmtxItalic;
	t->hmtxLinearAdjustment = t->hmtxLinearAdjustmentItalic;
    } else /* if (t->params[0] != ONE16Dot16 && t->params[1] != 0) */ {
	if (t->hmtxBoldItalic == NULL) {
	    t->hmtxBoldItalic = Copy_hmtxClass(t->mem, t->hmtxPlain);
	    t->hmtxLinearAdjustmentBoldItalic =
		t->StyleMetricsFunc(t->hmtxBoldItalic, t->mem,
				    GetUPEM(t), t->params);
	}
	t->hmtx = t->hmtxBoldItalic;
	t->hmtxLinearAdjustment = t->hmtxLinearAdjustmentBoldItalic;
    }
}

void t2k_SetStyling( sfntClass *t, T2K_AlgStyleDescriptor *styling )
{
	int i;

	if ( styling != NULL ) {
#ifdef ENABLE_TT_HINTING
	    assert( styling->StyleFuncPost != NULL );
	    t->StyleFuncPost = styling->StyleFuncPost;
#endif
	    t->StyleMetricsFunc = styling->StyleMetricsFunc;

	    for ( i = 0; i < MAX_STYLE_PARAMS; i++ ) {
		t->params[i] = styling->params[i];
	    }
	} else {
#ifdef ENABLE_TT_HINTING
		t->StyleFuncPost = 0L;
#endif
		t->StyleMetricsFunc = NULL;
	}
	t2k_SetHmtx(t);
}


#ifdef ENABLE_WRITE
static void CopyStyling( sfntClass *dst, sfntClass *src )
{
	int i;

	dst->StyleFunc = src->StyleFunc;
#ifdef ENABLE_TT_HINTING
	dst->StyleFuncPost = src->StyleFuncPost;
#endif
	dst->StyleMetricsFunc = src->StyleMetricsFunc;
	for ( i = 0; i < MAX_STYLE_PARAMS; i++ ) {
		dst->params[i] = src->params[i];
	}
}
#endif /* ENABLE_WRITE */


/*
 *
 */
sfntClass *New_sfntClassLogical( tsiMemObject *mem, short fontType, tt_int32 fontNum, InputStream *in, T2K_AlgStyleDescriptor *styling, int *errCode )
{
	sfntClass *t;
	
	assert( mem != NULL );
	assert( in != NULL );
	
	if ( errCode == NULL || (*errCode = setjmp(mem->env)) == 0 ) {
		/* try */
		t = (sfntClass *) tsi_AllocMem( mem, sizeof( sfntClass ) );
		t->mem		= mem;
		t->in 		= in;
		t->out		= NULL;
		
		t->offsetTable0 = NULL;
		t->head = NULL;
		t->hhea = NULL;
		t->vhea = NULL;
		t->hmtx = NULL;
		t->hmtxPlain      = NULL;
		t->hmtxBold       = NULL;
		t->hmtxItalic     = NULL;
		t->hmtxBoldItalic = NULL;
		t->maxp = NULL;
		t->loca = NULL;
		t->cmap = NULL;
		t->kern = NULL;
		
		t->model = NULL;
#ifdef ENABLE_SBIT
		t->bloc = NULL;
		t->ebsc = NULL;
#endif
#ifdef ENABLE_TT_HINTING
	/* MTE: these are the additional tables required for
	 		hinting. Note: glyph instructions must also
	 		be saved from the glyf  definition, for each glyph.
	 */
		t->fpgm = NULL;
		t->cvt = NULL;
		t->prep = NULL;
#endif
		
		t2k_SetStyling( t, styling );
		t->globalHintsCache = NULL; /* Used/Allocated/Deallocated by the T2K scaler, sort of weird but there are reasons */
#ifdef ENABLE_T1
		t->T1 = NULL;
#endif
#ifdef ENABLE_CFF
		t->T2 = NULL;
#endif
#ifdef ENABLE_T2KE
		t->T2KE = NULL;
#endif
		t->ttcf = NULL;
		if ( fontType == FONT_TYPE_TT_OR_T2K ) {
			CacheKeyTables_sfntClass( t, t->in, fontNum );
#ifdef ENABLE_T2KE
	if ( t->head->glyphDataFormat == 2001 ) {
		t->T2KE = tsi_NewT2KEClass( t, t->in, fontNum );
	}
#endif
#ifdef ENABLE_T1
		} else if ( fontType == FONT_TYPE_1 ) {
			/* Fix tsi_NewT1Class to accept InputStream... */
			/* t->T1 = tsi_NewT1Class( mem, in->base, in->maxPos ); */
			t->T1 = tsi_NewT1Class( mem, GetEntireStreamIntoMemory(in), SizeInStream(in) );
			if (t->T1 == NULL) {
			  if (errCode != NULL) {
			    *errCode = 1;
			  }
			  return t;
			}
			
			
			t->hmtx = t->T1->hmtx;
			t->T1->hmtx = NULL;
			t->hmtxLinearAdjustment =0;
			if ( t->StyleMetricsFunc != NULL ) {
				 t->hmtxLinearAdjustment=t->StyleMetricsFunc( t->hmtx, t->mem, GetUPEM( t ), t->params );
			}
#endif
#ifdef ENABLE_CFF
		} else if ( fontType == FONT_TYPE_2 ) {
			t->T2 = tsi_NewCFFClass( mem, t->in, fontNum ); /* last argument is the fontNumber */
			
			
			t->hmtx = t->T2->hmtx;
			t->T2->hmtx = NULL;
			t->hmtxLinearAdjustment =0;
			if ( t->StyleMetricsFunc != NULL ) {
				t->hmtxLinearAdjustment=t->StyleMetricsFunc( t->hmtx, t->mem, GetUPEM( t ), t->params );
			}
#endif
		} else {
			assert( false );
		}
		
		
		/* longjmp( t->mem->env, 9999 ); */
	} else {
		/* catch */
		tsi_EmergencyShutDown( mem );
		t = (sfntClass *)NULL;
	}
	return t; /*****/
}

#ifdef ENABLE_WRITE
static tt_uint8 *GetTTFPointer( sfntClass *sfnt )
{
	/* return sfnt->out ? sfnt->out->base : sfnt->in->base; */
	return sfnt->out ? sfnt->out->base : GetEntireStreamIntoMemory( sfnt->in );
}


static tt_uint32 GetTTFSize( sfntClass *sfnt )
{
	return sfnt->out ? SizeOutStream( sfnt->out ) : SizeInStream( sfnt->in );
}
#endif /* ENABLE_WRITE */

#ifdef ENABLE_WRITE
static void CalculateNewCheckSums( sfntClass *sfnt )
{
	tt_int32 i, j;
	InputStream *table;
	tt_uint8 *base = GetTTFPointer( sfnt );
	assert( base != NULL );
	
	/* assumes head.checkSumAdjustment == 0L; */
	for ( i = 0; i < sfnt->offsetTable0->numOffsets; i++) {
		tt_uint32 offset	= sfnt->offsetTable0->table[i]->offset;
		tt_uint32 length	= sfnt->offsetTable0->table[i]->length;
		tt_uint32 ck	= 0;
		length 		+= 3; length &= ~3;

		table = New_InputStream3( sfnt->mem, &base[offset], length, NULL );
		for ( j = 0; j < length; j += 4 ) {
			ck += (tt_uint32)ReadInt32( table );
		}
		sfnt->offsetTable0->table[i]->checkSum = ck;
		Delete_InputStream( table, NULL );
	}
}
#endif /* ENABLE_WRITE */

#ifdef ENABLE_WRITE

/* Result to be put into head.checkSumAdjustment */
static tt_int32 CalculateCheckSumAdjustment( sfntClass *sfnt )
{
	
	InputStream *ttf;
	tt_uint32 checkSumAdjustment, sum	= 0;
	tt_uint8 *base = GetTTFPointer( sfnt );
	tt_uint32 j, length = GetTTFSize( sfnt );

	ttf = New_InputStream3( sfnt->mem, base, length, NULL);
	for ( j = 0; j < length; j += 4 ) {
		sum += (tt_uint32)ReadInt32( ttf );
	}
	Delete_InputStream( ttf, NULL );
	checkSumAdjustment = 0x0b1b0afba - sum;
	return checkSumAdjustment; /*****/
}
#endif /* ENABLE_WRITE */

#ifdef ENABLE_WRITE
static void HandleSbits( sfntClass *sfnt0, sfntClass *sfnt1 )
{
	char buffer[256];
	
	if ( GetTableDirEntry_sfntClass( sfnt0, tag_EBLC ) != NULL ||
		 GetTableDirEntry_sfntClass( sfnt0, tag_bloc ) != NULL ||
		 GetTableDirEntry_sfntClass( sfnt0, tag_EBDT ) != NULL ||
		 GetTableDirEntry_sfntClass( sfnt0, tag_bdat ) != NULL ||
		 GetTableDirEntry_sfntClass( sfnt0, tag_EBSC ) != NULL ) {
	
	
		printf("**************\n");
		printf("The font contains embedded bitmaps!\n");
		printf("Please answer Y or N followed by <CR>.\n");
		do {
			printf("Should I go ahead and delete them ?\n");
			buffer[0] = 0;
			scanf("%s", buffer );
			assert( strlen(buffer) < 100 );
		} while ( buffer[0] != 'Y' && buffer[0] != 'y' && buffer[0] != 'N' && buffer[0] != 'n' );
		
		if ( buffer[0] == 'Y' || buffer[0] == 'y' ) {
			DeleteTable( sfnt0->offsetTable0, tag_EBLC );
			DeleteTable( sfnt1->offsetTable0, tag_EBLC );
			DeleteTable( sfnt0->offsetTable0, tag_bloc );
			DeleteTable( sfnt1->offsetTable0, tag_bloc );
			DeleteTable( sfnt0->offsetTable0, tag_EBDT );
			DeleteTable( sfnt1->offsetTable0, tag_EBDT );
			DeleteTable( sfnt0->offsetTable0, tag_bdat );
			DeleteTable( sfnt1->offsetTable0, tag_bdat );
			
			DeleteTable( sfnt0->offsetTable0, tag_EBSC );
			DeleteTable( sfnt1->offsetTable0, tag_EBSC );
		}
		
		printf("**************\n");
	}
}
#endif /* ENABLE_WRITE */


#ifdef ENABLE_WRITE
/*
 *
 */
sfntClass *New_sfntClass2( sfntClass *sfnt0, int cmd, int param )
{
	int err;
	ag_HintHandleType hintHandle;
	ag_FontCategory fontType;
	ag_GlobalDataType gData;
	unsigned char *fpgm, *ppgm;
	short *cvt;
	tt_int32 fpgmLength, ppgmLength, cvtCount;
	int hint = 1;
	char xWeightIsOne;
	tt_int32 offset, length, pos = 0;
	tt_int32 headIndex, locaIndex, hmtxIndex, maxpIndex, i;
	tt_uint32 checkSumAdjustmentOffset = 0;
	sfntClass *sfnt1 = (sfntClass *)tsi_AllocMem( sfnt0->mem, sizeof( sfntClass ) );
	int ppem = 0;

	assert( sfnt1 != NULL );
	sfnt1->mem = sfnt0->mem;
#ifdef ENABLE_T1
	sfnt1->T1 = NULL;
#endif
#ifdef ENABLE_CFF
	sfnt1->T2 = NULL;
#endif
#ifdef ENABLE_T2KE
	sfnt1->T2KE = NULL;
#endif
	sfnt1->ttcf = NULL;
	assert( sfnt1->mem != NULL );
	if ( cmd == CMD_HINT_ROMAN || cmd == CMD_HINT_OTHER || cmd == CMD_TT_TO_T2K || cmd == CMD_T2K_TO_TT || cmd == CMD_TT_TO_T2KE ) {
#ifndef ENABLE_AUTO_HINTING
		assert( false );
#endif
		;
	} else if ( cmd == CMD_GRID ) {
#ifndef ENABLE_AUTO_GRIDDING
		assert( false );
#endif
		ppem = param;
	}
	headIndex = locaIndex = hmtxIndex = maxpIndex = -1;
	
	sfnt1->in		= NULL;
	sfnt1->out		= New_OutputStream( sfnt1->mem, 100L * 1000L ); /* Set to intelligent size later... */
	sfnt1->offsetTable0 = NULL;
	sfnt1->head = NULL;
	sfnt1->hhea = NULL;
	sfnt1->vhea = NULL;
	sfnt1->hmtx = NULL;
	sfnt1->hmtxPlain      = NULL;
	sfnt1->hmtxBold       = NULL;
	sfnt1->hmtxItalic     = NULL;
	sfnt1->hmtxBoldItalic = NULL;
	sfnt1->maxp = NULL;
	sfnt1->loca = NULL;
	sfnt1->cmap = NULL;
	sfnt1->kern = NULL;
	
#ifdef ENABLE_SBIT
	sfnt1->bloc = NULL;
	sfnt1->ebsc = NULL;
	sfnt1->bdatOffset = 0;
#endif
	
	sfnt1->model = sfnt0->model; sfnt0->model = NULL;

	assert( sfnt0->in != NULL );
	
	CopyStyling( sfnt1, sfnt0 );
	
	
	Rewind_InputStream( sfnt0->in );
	CacheKeyTables_sfntClass( sfnt1, sfnt0->in, 0 );
	
err = ag_HintInit( sfnt1->mem, sfnt1->maxp->maxPoints + 2, sfnt0->head->unitsPerEm, &hintHandle );
assert( err == 0 );
fontType = cmd == CMD_HINT_ROMAN ? ag_ROMAN : ag_KANJI;
if ( cmd == CMD_HINT_ROMAN ) {
	printf("Hint Roman\n");
} else if( cmd == CMD_HINT_OTHER ) {
	printf("Hint Other/Kanji\n");
}

ComputeGlobalHints( sfnt0, hintHandle, &gData, false);
/* SetGlobalDataForTms( &gData ); */
err = ag_SetHintInfo( hintHandle, &gData, fontType );
assert( err == 0 );
err = ag_SetScale( hintHandle, ppem, ppem, &xWeightIsOne );
assert( err == 0 );
#ifdef ENABLE_AUTO_HINTING
err = ag_GetGlobalHints( hintHandle, &fpgm, &fpgmLength, &ppgm, &ppgmLength, &cvt, &cvtCount );
if ( err != 0 ) {
	printf("TrueType autohinting is disabled\n");
	fpgm = ppgm = 0;
	fpgmLength = ppgmLength = 0;
}
#else
fpgm = ppgm = 0;
cvt = 0;
fpgmLength = ppgmLength = cvtCount = 0;
#endif

	
	CreateTableIfMissing( sfnt0->offsetTable0, tag_PreProgram );
	CreateTableIfMissing( sfnt0->offsetTable0, tag_FontProgram );
	CreateTableIfMissing( sfnt0->offsetTable0, tag_ControlValue );
	
	CreateTableIfMissing( sfnt1->offsetTable0, tag_PreProgram );
	CreateTableIfMissing( sfnt1->offsetTable0, tag_FontProgram );
	CreateTableIfMissing( sfnt1->offsetTable0, tag_ControlValue );
	
	if ( cmd == CMD_T2K_TO_TT ) {
		assert( sfnt1->head->glyphDataFormat == 2000 );
	}

	if ( cmd == CMD_TT_TO_T2K || cmd == CMD_TT_TO_T2KE ) {
		assert( sfnt1->head->glyphDataFormat == 0 );
		
		HandleSbits( sfnt0, sfnt1 );
		
		DeleteTable( sfnt0->offsetTable0, tag_PreProgram );
		DeleteTable( sfnt1->offsetTable0, tag_PreProgram );
		DeleteTable( sfnt0->offsetTable0, tag_FontProgram );
		DeleteTable( sfnt1->offsetTable0, tag_FontProgram );
		DeleteTable( sfnt0->offsetTable0, tag_ControlValue );
		DeleteTable( sfnt1->offsetTable0, tag_ControlValue );
		
		CreateTableIfMissing( sfnt0->offsetTable0, tag_T2KG );
		CreateTableIfMissing( sfnt1->offsetTable0, tag_T2KG );
		
		if ( sfnt1->model != NULL ) {
			CreateTableIfMissing( sfnt0->offsetTable0, tag_T2KC );
			CreateTableIfMissing( sfnt1->offsetTable0, tag_T2KC );
		}
	} else {
		DeleteTable( sfnt0->offsetTable0, tag_T2KG );
		DeleteTable( sfnt1->offsetTable0, tag_T2KG );
	}
	DeleteTable( sfnt0->offsetTable0, tag_HoriDeviceMetrics );
	DeleteTable( sfnt1->offsetTable0, tag_HoriDeviceMetrics );
	DeleteTable( sfnt0->offsetTable0, tag_VDMX );
	DeleteTable( sfnt1->offsetTable0, tag_VDMX );
	DeleteTable( sfnt0->offsetTable0, tag_LTSH );
	DeleteTable( sfnt1->offsetTable0, tag_LTSH );


	Write_sfnt_OffsetTable( sfnt1->offsetTable0, sfnt1->out );
	for ( i = 0; i < sfnt0->offsetTable0->numOffsets; i++ ) {
		offset = sfnt0->offsetTable0->table[i]->offset;
		length = sfnt0->offsetTable0->table[i]->length;

		sfnt1->offsetTable0->table[i]->tag		= sfnt0->offsetTable0->table[i]->tag;
		sfnt1->offsetTable0->table[i]->checkSum	= sfnt0->offsetTable0->table[i]->checkSum;
		pos										= SizeOutStream(sfnt1->out);
		sfnt1->offsetTable0->table[i]->offset	= pos;
		/* sfnt1->offsetTable0->table[i]->length is set below */
#ifdef VERBOSE
printf("%ld *** %c%c%c%c offset = %ld, length = %ld\n", i,
							(char)((sfnt1->offsetTable0->table[i]->tag >> 24) & 0xff),
							(char)((sfnt1->offsetTable0->table[i]->tag >> 16) & 0xff),
							(char)((sfnt1->offsetTable0->table[i]->tag >>  8) & 0xff),
							(char)((sfnt1->offsetTable0->table[i]->tag >>  0) & 0xff),
							offset, length );
#endif
		switch ( sfnt0->offsetTable0->table[i]->tag ) {
		case tag_FontHeader:
			sfnt1->head->checkSumAdjustment	 = 0; /* Zero this to calc checksums right */
			sfnt1->head->magicNumber		 = SFNT_MAGIC;
			headIndex = i; /* Write out the loca after the glyph table is done */
			/* Write_headClass( sfnt1->head, sfnt1->out ); */
			break; /*****/
		case tag_IndexToLoc:
			locaIndex = i; /* Write out the loca after the glyph table is done */
			break;/******/
		case tag_HorizontalMetrics:
			hmtxIndex = i; /* Write out the hmtx after the glyph table is done */
			break;/******/
		case tag_MaxProfile:
			maxpIndex = i;
			/* Write_maxpClass( sfnt1->maxp, sfnt1->out ); */
			break; /*****/
		case tag_GlyphData:
			{
				tt_int32 i, n, startpos;
#ifdef TIME_GLYPH_DATA
				clock_t t1, t2;
				double duration;
#endif
				tt_uint16 theAW;
				
				n = GetNumGlyphs_sfntClass( sfnt0 );
				startpos = SizeOutStream(sfnt1->out);
#ifdef TIME_GLYPH_DATA
				t2 = clock(); 
#endif
				for ( i = 0; i < n; i++ ) {
					GlyphClass *glyph;
					short isFigure = false;
					glyph = GetGlyphByIndex( sfnt0, i, true, &theAW );
#ifdef ENABLE_T2KE
					glyph->gIndex = i;
#endif
					sfnt1->loca->offsets[i] = SizeOutStream(sfnt1->out) - startpos;
if ( i == 1000 ) {
	printf("1000\n");
}
					if ( glyph != NULL ) {
						ag_ElementType elem;
						tt_uint8 *hintFragment = NULL;
						tt_int32 hintLength = 0;

						elem.contourCount	= glyph->contourCount; 
						elem.pointCount		= glyph->pointCount;   
						elem.sp				= glyph->sp;  		
						elem.ep				= glyph->ep;  		
						elem.oox			= glyph->oox;		
						elem.ooy			= glyph->ooy;		
						elem.onCurve		= glyph->onCurve;

						if ( glyph->hintLength > 0 ) {
							assert( glyph->hintFragment != NULL );
							tsi_DeAllocMem( sfnt1->mem, (char *)glyph->hintFragment );
							glyph->hintFragment = NULL;
							glyph->hintLength = 0;
						}
						elem.ooy[glyph->pointCount + 0] = 0;
						elem.oox[glyph->pointCount + 0] = (short)(glyph->xmin - sfnt1->hmtx->lsb[i]);
						
						elem.ooy[glyph->pointCount + 1] = 0;
						elem.oox[glyph->pointCount + 1] = (short)(elem.oox[glyph->pointCount + 0] + sfnt1->hmtx->aw[i]);
						if ( cmd == CMD_TT_TO_T2K || cmd == CMD_T2K_TO_TT || cmd == CMD_TT_TO_T2KE ) {
							glyph->hintLength = 0;
						} else if ( cmd == CMD_HINT_ROMAN  || cmd == CMD_HINT_OTHER) {
#ifdef ENABLE_AUTO_HINTING
							if ( glyph->contourCount > 0 ) {	
								isFigure = (short)IsFigure( sfnt0, (unsigned short)i );
								err = ag_AutoHintOutline( hintHandle, &elem, isFigure, 2, &hintFragment, &hintLength );
								assert( err == 0 );
								glyph->hintLength = hintLength;
								if ( glyph->hintLength > 0 ) {
									glyph->hintFragment = hintFragment;
								}
							}
#endif
						} else if ( cmd == CMD_GRID ) {
#ifdef ENABLE_AUTO_GRIDDING
							if ( glyph->contourCount > 0 ) {	
								tt_int32 point;
								elem.x = (tt_int32 *)tsi_AllocMem( sfnt0->mem, 2 * (glyph->pointCount+2) * sizeof(tt_int32) );
								elem.y = &elem.x[(glyph->pointCount+2)];
								assert( elem.x != NULL );
								
								isFigure = (short)IsFigure( sfnt0, (tt_uint16)i );
								err = ag_AutoGridOutline( hintHandle, &elem, isFigure, (short)2, false /* Assume B&W for now */ );
								glyph->hintLength = 0;
								assert( err == 0 );
								/* scale =  ppem / upem * 64 */
								for ( point = 0; point < glyph->pointCount+2; point++ ) {
									glyph->oox[point] = (short)(elem.x[point] * sfnt0->head->unitsPerEm / 64 / ppem);
									glyph->ooy[point] = (short)(elem.y[point] * sfnt0->head->unitsPerEm / 64 / ppem);
								}

								tsi_DeAllocMem( sfnt0->mem, elem.x );
							}
#endif
						}
						if ( cmd == CMD_TT_TO_T2K ) {
							Write_GlyphClassT2K( glyph, sfnt1->out, sfnt1->model );
							sfnt1->head->glyphDataFormat = 2000;
						} else if ( cmd == CMD_TT_TO_T2KE ) {
#ifdef ENABLE_T2KE
							if ( i == 0 ) {
								Write_GlobalFuncsT2KE( sfnt1->out );
							}
							Write_GlyphClassT2KE( glyph, sfnt1->out );
							sfnt1->head->glyphDataFormat = 2001;
#else
							assert( false );
#endif
						} else {
							Write_GlyphClass( glyph, sfnt1->out  );
							sfnt1->head->glyphDataFormat = 0;
						}
						if ( glyph->hintLength > 0 ) {
							assert( glyph->hintFragment != NULL );
							tsi_DeAllocMem( sfnt1->mem, (char *)glyph->hintFragment );
							glyph->hintFragment = NULL;
							glyph->hintLength = 0;
						}
						if ( cmd == CMD_GRID /* && glyph->contourCount > 0 */ ) {
						/*
							assert( (elem.x[glyph->pointCount+0] & 63) == 0 );
							assert( (elem.x[glyph->pointCount+1] & 63) == 0 );
						*/
							sfnt1->hmtx->lsb[i] = (tt_int16)(glyph->xmin - elem.oox[glyph->pointCount + 0]);
							sfnt1->hmtx->aw[i]  = (tt_uint16)(elem.oox[glyph->pointCount + 1] - elem.oox[glyph->pointCount + 0]);
						}
					}
					if ( SizeOutStream(sfnt1->out) & 1 ) {
						/* Word align */
						WriteUnsignedByte( sfnt1->out, 0 );
					}
					Delete_GlyphClass( glyph );
				}
				sfnt1->loca->offsets[i] = SizeOutStream(sfnt1->out) - startpos;
#ifdef TIME_GLYPH_DATA
				t1 = clock(); 
				duration = ((difftime(t1,t2))/ (CLOCKS_PER_SEC));
				/* printf("Time = %f, %f c/s\n", (double)duration, (double)n/(double)duration); */
#endif
			}
			break; /*****/
		case tag_PreProgram:
			if ( hint ) {
				Write( sfnt1->out, ppgm, ppgmLength );
				/* if ( cmd == CMD_GRID ) WriteDataToFile( "NEW_PPGM.BIN", &sfnt0->in->base[offset], length );; */
				if ( cmd == CMD_GRID ) WriteDataToFile( "NEW_PPGM.BIN", GetEntireStreamIntoMemory(sfnt0->in) + offset, length );;
			} 
#ifdef OLD			
			else {
				Write( sfnt1->out, &sfnt0->in->base[offset], length );
			}
#endif
			break;/******/
		case tag_FontProgram:
			if ( hint ) {
				Write( sfnt1->out, fpgm, fpgmLength );
				if ( cmd == CMD_GRID ) WriteDataToFile( "NEW_FPGM.BIN", GetEntireStreamIntoMemory(sfnt0->in) + offset, length );;
			}
#ifdef OLD						
			else {
				Write( sfnt1->out, &sfnt0->in->base[offset], length );
			}
#endif
			break;/******/
		case tag_ControlValue:
			if ( hint ) {
				Write( sfnt1->out, (unsigned char *)cvt, cvtCount*2 );
			}
#ifdef OLD			
			else {
				Write( sfnt1->out, &sfnt0->in->base[offset], length );
			}
#endif
			break;/******/
		
		case tag_T2KG:
			WriteGHints( &gData, sfnt1->out );
			break;/******/
#ifdef ENABLE_ORION
		case tag_T2KC:
			Save_OrionModelClass( (OrionModelClass *)sfnt1->model, sfnt1->out );
			break;/******/
#endif
		default:
			Write( sfnt1->out, GetEntireStreamIntoMemory(sfnt0->in) + offset, length );
			break;/******/
		}
		/* set the length */
		sfnt1->offsetTable0->table[i]->length   = SizeOutStream( sfnt1->out ) - sfnt1->offsetTable0->table[i]->offset;
		while ( (pos = SizeOutStream( sfnt1->out )) & 3 ) {
			/* long word align */
			WriteUnsignedByte( sfnt1->out, 0 );
		}
	}
	assert( headIndex >= 0 );
	assert( locaIndex >= 0 );
	assert( hmtxIndex >= 0 );
	assert( maxpIndex >= 0 );
	/* Write out the hmtx table */
	sfnt1->offsetTable0->table[hmtxIndex]->offset = pos;
	Write_hmtxClass( sfnt1->hmtx, sfnt1->out );
	pos = SizeOutStream( sfnt1->out );
	sfnt1->offsetTable0->table[hmtxIndex]->length = pos - sfnt1->offsetTable0->table[hmtxIndex]->offset;
	while ( (pos = SizeOutStream( sfnt1->out )) & 3 ) {
		/* long word align */
		WriteUnsignedByte( sfnt1->out, 0 );
	}
	
	/* Write out the maxp table */
	{
		ag_HintMaxInfoType maxInfo;
		
		ag_GetHintMaxInfo( hintHandle, &maxInfo );
		
		sfnt1->maxp->maxElements			= maxInfo.maxZones;
		sfnt1->maxp->maxTwilightPoints		= maxInfo.maxTwilightPoints;
		sfnt1->maxp->maxStorage				= maxInfo.maxStorage;
		sfnt1->maxp->maxFunctionDefs		= maxInfo.maxFunctionDefs;
		sfnt1->maxp->maxInstructionDefs		= maxInfo.maxInstructionDefs;
		sfnt1->maxp->maxStackElements		= maxInfo.maxStackElements;
		sfnt1->maxp->maxSizeOfInstructions  = maxInfo.maxSizeOfInstructions;
		
		sfnt1->offsetTable0->table[maxpIndex]->offset = pos;
		Write_maxpClass( sfnt1->maxp, sfnt1->out );
		pos = SizeOutStream( sfnt1->out );
		sfnt1->offsetTable0->table[maxpIndex]->length = pos - sfnt1->offsetTable0->table[maxpIndex]->offset;
		while ( (pos = SizeOutStream( sfnt1->out )) & 3 ) {
			/* long word align */
			WriteUnsignedByte( sfnt1->out, 0 );
		}
	}
	
	/* Write out the loca table */
	sfnt1->offsetTable0->table[locaIndex]->offset = pos;
	Write_locaClass( sfnt1->loca, sfnt1->out );
	pos = SizeOutStream( sfnt1->out );
	sfnt1->offsetTable0->table[locaIndex]->length = pos - sfnt1->offsetTable0->table[locaIndex]->offset;
	while ( (pos = SizeOutStream( sfnt1->out )) & 3 ) {
		/* long word align */
		WriteUnsignedByte( sfnt1->out, 0 );
	}

	/* Write out the head table */
	checkSumAdjustmentOffset 		 = pos + 8;
	sfnt1->head->indexToLocFormat = sfnt1->loca->indexToLocFormat; /* propagate to the head table */
	sfnt1->offsetTable0->table[headIndex]->offset = pos;
	Write_headClass( sfnt1->head, sfnt1->out );
	pos = SizeOutStream( sfnt1->out );
	sfnt1->offsetTable0->table[headIndex]->length = pos - sfnt1->offsetTable0->table[headIndex]->offset;
	while ( (pos = SizeOutStream( sfnt1->out )) & 3 ) {
		/* long word align */
		WriteUnsignedByte( sfnt1->out, 0 );
	}
	
	
	

	/* Sort the table directory */
	SortTableDirectory( sfnt1->offsetTable0 );
	/* Set all the table check sums */
	CalculateNewCheckSums( sfnt1 );
	
	/* Write out the offset table */
	Rewind_OutputStream( sfnt1->out );
	Write_sfnt_OffsetTable( sfnt1->offsetTable0, sfnt1->out );
	

	/* Set the global checksum adjustment */
	{
		tt_uint32 checkSumAdjustment = CalculateCheckSumAdjustment( sfnt1 );
		tt_uint8 *ttf =  GetTTFPointer( sfnt1 );

		ttf[checkSumAdjustmentOffset + 0] = (tt_uint8)((checkSumAdjustment >> 24) & 0xff);
		ttf[checkSumAdjustmentOffset + 1] = (tt_uint8)((checkSumAdjustment >> 16) & 0xff);
		ttf[checkSumAdjustmentOffset + 2] = (tt_uint8)((checkSumAdjustment >>  8) & 0xff);
		ttf[checkSumAdjustmentOffset + 3] = (tt_uint8)((checkSumAdjustment >>  0) & 0xff);
	}
	sfnt1->StyleFunc = NULL; /* No styling on this font anymore. It has already been applied */

	/* Rebuild the cached tables */
	{
		InputStream *ttf;
		tt_uint8 *base = GetTTFPointer( sfnt1 );
		tt_uint32 length = GetTTFSize( sfnt1 );
		ttf = New_InputStream3( sfnt1->mem, base, length, NULL);
		CacheKeyTables_sfntClass( sfnt1, ttf, 0);
		Delete_InputStream( ttf, NULL );
	}
tsi_DeAllocMem( sfnt1->mem, (char *)fpgm );
tsi_DeAllocMem( sfnt1->mem, (char *)ppgm );
tsi_DeAllocMem( sfnt1->mem, (char *)cvt );
err = ag_HintEnd( hintHandle );
assert( err == 0 );

	assert( sfnt1 != NULL );
	return sfnt1; /*****/
}
#endif /* ENABLE_WRITE */

#ifdef ENABLE_WRITE
void WriteToFile_sfntClass( sfntClass *t, const char *fname )
{
	tt_uint8 *base  = GetTTFPointer( t );
	tt_uint32 length = GetTTFSize( t );

	WriteDataToFile( fname, base, length );
}
#endif /* ENABLE_WRITE */


short GetUPEM( sfntClass *t)
{
	tt_uint16 upem = 2048;
	assert( t != NULL );
	
#ifdef ENABLE_T1
	if ( t->T1 != NULL ) {
		upem = (short)t->T1->upem;
	} else 
#endif
#ifdef ENABLE_CFF
	if ( t->T2 != NULL ) {
		upem = (short)t->T2->upem;
	} else 
#endif
	if ( t->head != NULL ) {
		upem = t->head->unitsPerEm;
	}
	
	return upem; /*****/
}

short GetMaxPoints( sfntClass *t)
{
	assert( t != NULL );
#ifdef ENABLE_T1
	if ( t->T1 != NULL ) {
		return (short)t->T1->maxPointCount; /*****/
	}
#endif
#ifdef ENABLE_CFF
	if ( t->T2 != NULL ) {
		return (short)t->T2->maxPointCount; /*****/
	}
#endif
	assert( t->maxp != NULL );
	return (short)(t->maxp->maxPoints > t->maxp->maxCompositePoints ? t->maxp->maxPoints : t->maxp->maxCompositePoints); /*****/
}

void GetFontWideOutlineMetrics( sfntClass *font, T2K_FontWideMetrics *hori, T2K_FontWideMetrics *vert )
{
	int i;

	vert->isValid   = false; /* Initialize */
	hori->isValid   = false;
#ifdef ENABLE_T1
	if ( font->T1 != NULL ) {
		F16Dot16 	angle;
		hori->isValid   = true;
		hori->Ascender	= (short)font->T1->ascent; 
		hori->Descender	= (short)font->T1->descent; 
		hori->LineGap	= (short)font->T1->lineGap;
		hori->maxAW		= (short)font->T1->advanceWidthMax;
		angle 		= font->T1->italicAngle;
		hori->caretDx	= 0;
		hori->caretDy	= ONE16Dot16;
		if ( angle != 0 ) {
			if ( angle < 0 ) angle = -angle;
			/* The angle is measured from the y axis */
			hori->caretDx = util_FixSin( angle );
			hori->caretDy = util_FixCos( angle );
		}
		return; /*****/
	}
#endif
#ifdef ENABLE_CFF
	if ( font->T2 != NULL ) {
		F16Dot16 	angle;
		hori->isValid   = true;
		hori->Ascender	= (short)font->T2->ascent; 
		hori->Descender	= (short)font->T2->descent; 
		hori->LineGap	= (short)font->T2->lineGap;
		hori->maxAW		= (short)font->T2->advanceWidthMax;
		angle 		= font->T2->italicAngle;
		hori->caretDx	= 0;
		hori->caretDy	= ONE16Dot16;
		if ( angle != 0 ) {
			if ( angle < 0 ) angle = -angle;
			/* The angle is measured from the y axis */
			hori->caretDx = util_FixSin( angle );
			hori->caretDy = util_FixCos( angle );
		}
		return; /*****/
	}
#endif
	if ( font->hhea != NULL ) {
		hori->isValid   = true;
		hori->Ascender	= font->hhea->Ascender;
		hori->Descender	= font->hhea->Descender;
		hori->LineGap	= font->hhea->LineGap;
		hori->maxAW		= font->hhea->advanceWidthMax;
		hori->caretDx	= font->hhea->caretSlopeRun;
		hori->caretDy	= font->hhea->caretSlopeRise;
		/* Scale up */
		for ( i = 0; i < 16; i++ ) {
			if ( hori->caretDx >= ONE16Dot16 || hori->caretDx <= -ONE16Dot16 ) break; /*****/
			if ( hori->caretDy >= ONE16Dot16 || hori->caretDy <= -ONE16Dot16 ) break; /*****/
			hori->caretDx <<= 1;
			hori->caretDy <<= 1;
		}
	}
	if ( font->vhea != NULL ) {
		vert->isValid   = true;
		vert->Ascender	= font->vhea->Ascender;
		vert->Descender	= font->vhea->Descender;
		vert->LineGap	= font->vhea->LineGap;
		vert->maxAW		= font->vhea->advanceWidthMax;
		vert->caretDx	= font->vhea->caretSlopeRun;
		vert->caretDy	= font->vhea->caretSlopeRise;
		/* Scale up */
		for ( i = 0; i < 16; i++ ) {
			if ( vert->caretDx >= ONE16Dot16 || vert->caretDx <= -ONE16Dot16 ) break; /*****/
			if ( vert->caretDy >= ONE16Dot16 || vert->caretDy <= -ONE16Dot16 ) break; /*****/
			vert->caretDx <<= 1;
			vert->caretDy <<= 1;
		}
	}
}

/*
 *
 */
void Purge_cmapMemory( sfntClass *t )
{
	Delete_cmapClass( t->cmap );
	t->cmap = NULL;
}

/*
 *
 */
void Delete_sfntClass( sfntClass *t, int *errCode )
{
	if ( errCode == NULL || (*errCode = setjmp(t->mem->env)) == 0 ) {
		/* try */
		/* Delete_InputStream( t->in ); */
	#ifdef ENABLE_WRITE
		Delete_OutputStream( t->out );
	#endif
		Delete_ttcfClass( t->ttcf );
		Delete_sfnt_OffsetTable( t->offsetTable0);
		Delete_headClass( t->head );
		Delete_hheaClass( t->hhea );
		Delete_hheaClass( t->vhea );
		Delete_hmtxClass( t->hmtxPlain );
		Delete_hmtxClass( t->hmtxBold );
		Delete_hmtxClass( t->hmtxItalic );
		Delete_hmtxClass( t->hmtxBoldItalic );
		Delete_maxpClass( t->maxp );
		Delete_locaClass( t->loca );
		Delete_cmapClass( t->cmap );
	#ifdef ENABLE_KERNING
		Delete_kernClass( t->kern );
	#endif
	#ifdef ENABLE_T1
		tsi_DeleteT1Class( t->T1 );
	#endif
	#ifdef ENABLE_CFF
		tsi_DeleteCFFClass( t->T2 );
	#endif
	#ifdef ENABLE_T2KE
		tsi_DeleteT2KEClass( t->T2KE );
	#endif
	#ifdef ENABLE_ORION
		Delete_OrionModelClass( (OrionModelClass *)(t->model) );
	#endif
	#ifdef ENABLE_SBIT
		Delete_blocClass( t->bloc );
		Delete_ebscClass( t->ebsc );
	#endif
#ifdef ENABLE_TT_HINTING
		Delete_fpgmClass( t->fpgm );
		Delete_prepClass( t->prep );
		Delete_cvtClass( t->cvt );
#endif

		tsi_DeAllocMem( t->mem, t );
	} else {
		/* catch */
		tsi_EmergencyShutDown( t->mem );
	}
}



#ifdef OLD
	

#endif /* OLD */

/***** ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** *****/

