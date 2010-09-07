/*
 * @(#)scoder.c	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * File:		SCODER.c
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

#include "dtypes.h"
#include "config.h"
#include "tsimem.h"
#include "t2kstrm.h"
#include "scoder.h"

#ifdef ENABLE_ORION
/*
 * Internal function used for sequencing the look-up table.
 */
void SCODER_SequenceLookUp( SCODER *t )
{
	unsigned char symbol[ No_of_chars ], thisSymbol;
	register int i, j, k, numSymbols, thisNumSlots;
	register unsigned char *numBitsUsed = t->numBitsUsed;
	tt_uint32 numBits;


	/* We have to align the symbols with the smallest bit-lengths first  */
	/* A block has to start at a multiple of it's number of slots used ! */
	numSymbols = 0;
	for ( numBits = 1; numBits <= t->maxBits; numBits++ ) {
		for ( i = 0; i < No_of_chars; i++ ) {
			if ( numBitsUsed[i] == numBits ) {
				symbol[numSymbols++] = (unsigned char)i;
			}
		}
		/* Now all entries numBits == numBitsUsed are done, go to the next bitlength */
	}
	assert( numSymbols <= No_of_chars );
	
	assert( (tt_uint32)(1L << t->maxBits) == t->numEntries );
	for ( k = i = 0; i < numSymbols; i++ ) {
		thisSymbol = symbol[i];
		thisNumSlots = 1L << (t->maxBits - t->numBitsUsed[thisSymbol]);
		assert( k % thisNumSlots ==  0 );
		for ( j = 0; j < thisNumSlots; j++ ) {
			t->LookUpSymbol[k++] = thisSymbol;
		}
		assert( (tt_uint32)k <= t->numEntries );
	}
}


/*
 * This standard constructor recreates the SCODER object from a stream.
 */
SCODER *New_SCODER_FromStream( tsiMemObject *mem, InputStream *in )
{
	int i;
	tt_int32 maxBits = 0;
	unsigned char value, bitCategory1, bitCategory2;
	SCODER *t = (SCODER *) tsi_AllocMem( mem, sizeof( SCODER ) );

	t->mem				= mem;
	t->numBitsUsed 		= (unsigned char *)tsi_AllocMem( mem, No_of_chars * sizeof( unsigned char ) );
	
	maxBits = ReadUnsignedByteMacro( in );
	for ( i = 0; i < No_of_chars; ) {
		value 			= ReadUnsignedByteMacro( in );
		bitCategory1	= (unsigned char)(value >> 4);
		bitCategory2	= (unsigned char)(value & 0x0f);
		t->numBitsUsed[i++] = (unsigned char)(bitCategory1 == 15 ? 0 : maxBits - bitCategory1);
		t->numBitsUsed[i++] = (unsigned char)(bitCategory2 == 15 ? 0 : maxBits - bitCategory2);
	}
	t->maxBits		= maxBits;	
	t->numEntries	= 1L << maxBits;	
	t->LookUpSymbol	= (unsigned char *) tsi_AllocMem( mem,t->numEntries * sizeof( unsigned char ) );
	t->LookUpBits	= NULL;
	SCODER_SequenceLookUp( t );
	return t; /*****/
}




/*
 * Read a symbol from the input stream.
 * This table driven algorithm is key to our fast de-compression speed.
 * We never need to do bit-level reads and tests.
 */
unsigned char SCODER_ReadSymbol( register SCODER *t, register InputStream *in )
{
	register tt_uint32 	tmp;
	register unsigned char 	symbol;
	register tt_uint32	maxBits 	= t->maxBits;
	register tt_uint32 	bitCountIn	= in->bitCountIn;
	register tt_uint32 	bitBufferIn	= in->bitBufferIn;
	
	/* If not enough bits then read in more */
	while ( bitCountIn < maxBits ) {
		/* Note, this may read maxBits-1 bits too far... */
		tmp         = ReadUnsignedByteMacro( in );
		/* We always keep the bits slammed up againts the "left" edge" */
		tmp        <<= 24 - bitCountIn;
		bitBufferIn |= tmp;
		bitCountIn  += 8;
	}
	symbol			= t->LookUpSymbol[ bitBufferIn >> (32 - maxBits) ]; /* A simple look-up :-) */
	tmp				= t->numBitsUsed[ symbol ];							/* A simple look-up :-) */
	bitCountIn	   -= tmp; 			/* Discard the consumed bits */
	bitBufferIn	  <<= tmp;
	in->bitCountIn	= bitCountIn;	/* Store the state */
	in->bitBufferIn	= bitBufferIn;	/* Done! */
	return symbol; 					/*****/
}




/*
 * The destructor.
 */
void Delete_SCODER( SCODER *t )
{
	if ( t != NULL ) {
		tsi_DeAllocMem( t->mem, t->numBitsUsed );
		tsi_DeAllocMem( t->mem, t->LookUpSymbol );
		tsi_DeAllocMem( t->mem, t->LookUpBits );
		tsi_DeAllocMem( t->mem, t );
	}
}

#endif /*  ENABLE_ORION from top of the file */

