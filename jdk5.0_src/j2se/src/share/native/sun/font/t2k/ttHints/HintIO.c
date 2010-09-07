/*
 * @(#)HintIO.c	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/* */


 /*
 * TTHINTIO.c (uses same includes as TRUETYPE, plus its own include file.
 
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

/* This module is used to read/print hinting information stored 
	within the True Type font file.
*/
 
#include "syshead.h"
#include "config.h"
#include "dtypes.h"
#include "tsimem.h"
#include "t2kstrm.h"
#include "truetype.h"
#include "orion.h"
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


#include "HintIO.h"
#ifdef ENABLE_TT_HINTING
 
/* GetGlyphByIndex ReadFileIntoMemory
   Sometimes we print when no one else does.. */


/* ******************************************************************** */
/* ******************* fpgm class ************************************* */
/* ******************************************************************** */

	/* Allocate an fpgm class, but do not read data yet. */
 
	fpgmClass *New_fpgmEmptyClass
	( 
		tsiMemObject *mem, 
		tt_int32 numInstructions
	)
	{
		/* Allocate the class object. */
		fpgmClass *t = (fpgmClass *) tsi_AllocMem( mem, sizeof( fpgmClass ) );
		/* Save the private memory object */
		t->mem		 = mem;
		t->numInstructions = numInstructions;
		/* allocate a pointer, even when zero. */
	 	t->instructions = (tt_uint8 *) tsi_AllocMem( mem,numInstructions);
	 	assert( t->instructions != NULL );
		return t; /*****/
	}

	/* Read in fpgm class instructions */
	fpgmClass *Read_fpgmClass
	 (
	 	fpgmClass *t,
	 	InputStream *in
	 )
	{
	/*	tt_uint32 i; */
		tt_uint32 numInstructions=t->numInstructions;
	 	tt_uint8 *instructionPtr=t->instructions;
#if 0
		 	for ( i = 0; i < numInstructions; i++ ) 
			{
				 *instructionPtr++ = ReadUnsignedByteMacro( in );
			}	 	 
#else
			ReadSegment( in, instructionPtr, numInstructions );
#endif
 	 
		return t; /*****/
	}

	/* Create and read fpgm class*/
	fpgmClass *New_fpgmClass
	(
	 	tsiMemObject *mem, 
	 	InputStream *in, 
		tt_int32 numInstructions
	 )
	{
	 	fpgmClass *t= New_fpgmEmptyClass( mem,   numInstructions);
	 	return( Read_fpgmClass(t,in) ); 
	}


#ifdef ENABLE_WRITE
		/* NOT YET IMPLEMENTED- see Write_hmtxClass, for example*/
#endif /* ENABLE_WRITE */


#ifdef TT_ENABLE_PRINTF
	 void Print_fpgmClass( fpgmClass *t ) 
	{
		tt_int32 i;
		tt_int32 numInstructions=t->numInstructions;
	 	tt_uint8 *instructionPtr=t->instructions;
	 	tt_int32 numLines;
	 	tt_uint16 code; 	
	 	numLines=
	 	  (numInstructions+(fpgmInstructionsPerLine-1))/
	 	  		fpgmInstructionsPerLine; 	  		
		printf("\n---------- Begin fpgm (in octal) ----------\n");
	 	for ( i = 0; i < numInstructions; i++ ) 
		{
			/* Don't print fillers at the end. */
			if (i< numInstructions)
			{
				code= *instructionPtr++;
				printf("%4x, ", code );
			}
			/* Do a line break, occasionally */
			if ( ((i+1) %fpgmInstructionsPerLine) == 0 )
				 printf("\n");
		}	 	 
		printf("\n----------  End fpgm  ----------\n");
	}

#endif /* TT_ENABLE_PRINTF */


	void Delete_fpgmClass( fpgmClass *t )
	{
		if ( t != NULL ) {
			tsi_DeAllocMem( t->mem, t->instructions );
			tsi_DeAllocMem( t->mem, t );
		}
	}
	
	
	
	
	
	 
 /* ******************************************************************** */
/* ******************* prep class ************************************* */
/* ******************************************************************** */

	/* Allocate an prep class, but do not read data yet. */
	prepClass *New_prepEmptyClass
	( 
		tsiMemObject *mem, 
		tt_int32 numInstructions
	)
	{
		/* Allocate the class object. */
		prepClass *t = (prepClass *) tsi_AllocMem( mem, sizeof( prepClass ) );
		/* Save the private memory object */
		t->mem		 = mem;
		t->numInstructions = numInstructions;
		/* allocate a pointer, even when zero. */
	 	t->instructions = (tt_uint8 *) tsi_AllocMem( mem,numInstructions);
	 	assert( t->instructions != NULL );
		return t; /*****/
	}

	 /* Read in prep class instructions */
	prepClass *Read_prepClass
	 (
	 	prepClass *t,
	 	InputStream *in
	 )
	{
 		tt_uint32 numInstructions=t->numInstructions;
	 	tt_uint8 *instructionPtr=t->instructions;
#if 0
		 	for ( i = 0; i < numInstructions; i++ ) 
			{
				 *instructionPtr++ = ReadUnsignedByteMacro( in );
			}	 	 
#else
			ReadSegment( in, instructionPtr, numInstructions );
#endif
		
		return t; /*****/
	}

	/* Create and read prep class */
	prepClass *New_prepClass
	(
	 	tsiMemObject *mem, 
	 	InputStream *in, 
		tt_int32 numInstructions
	 )
	{
	 	prepClass *t= New_prepEmptyClass( mem,   numInstructions);
	 	return( Read_prepClass(t,in) ); 
	}


	#ifdef ENABLE_WRITE
		/* NOT YET IMPLEMENTED- see Write_hmtxClass, for example*/
	#endif /* ENABLE_WRITE */


#ifdef TT_ENABLE_PRINTF
 
	void Print_prepClass( prepClass *t ) 
	{
		tt_int32 i;
		tt_int32 numInstructions=t->numInstructions;
	 	tt_uint8 *instructionPtr=t->instructions;
	 	tt_int32 numLines;
	 	tt_uint16 code; 	
	 	numLines=
	 	  (numInstructions+(prepInstructionsPerLine-1))/
	 	  		prepInstructionsPerLine; 	  		
		printf("\n---------- Begin prep  (in octal) ----------\n");
	 	for ( i = 0; i < numInstructions; i++ ) 
		{
			/* Don't print fillers at the end. */
			if (i< numInstructions)
			{
				code= *instructionPtr++;
				printf("%4x, ", code );
			}
			/* Do a line break, occasionally */
			if ( ((i+1) %prepInstructionsPerLine) == 0 )
				 printf("\n");
		}	 	 
		printf("\n----------  End prep  ----------\n");
	}

#endif


	void Delete_prepClass( prepClass *t )
	{
		if ( t != NULL ) {
			tsi_DeAllocMem( t->mem, t->instructions );
			tsi_DeAllocMem( t->mem, t );
		}
	}
	
	
	
	
	
	

/* ******************************************************************** */
/* ******************* cvt class ************************************* */
/* ******************************************************************** */

	/* Allocate an cvt class, but do not read data yet. */
	cvtClass *New_cvtEmptyClass
	( 
		tsiMemObject *mem, 
		tt_int32 numFWords
	)
	{
		/* Allocate the class object.*/
		cvtClass *t = (cvtClass *) tsi_AllocMem( mem, sizeof( cvtClass ) );
		/* Save the private memory object*/
		t->mem		 = mem;
		t->numFWords = numFWords;
		/* allocate a pointer, even when zero.*/
	 	t->varFWords = (tt_uint16 *) tsi_AllocMem( mem,numFWords*2);
	 	assert( t->varFWords != NULL );
		return t; /*****/
	}

	/* Read in cvt class instructions */
	cvtClass *Read_cvtClass
	 (
	 	cvtClass *t,
	 	InputStream *in
	 )
	{
		tt_uint32 i;
		tt_uint32 numFWords=t->numFWords;
	 	tt_uint16 *varFWords=t->varFWords;
	 	for ( i = 0; i < numFWords; i++ ) 
		{
			 *varFWords++ =  ReadInt16( in );
		}	 	 
		return t; /*****/
	}

	/* Create and read cvt class*/
	cvtClass *New_cvtClass
	(
	 	tsiMemObject *mem, 
	 	InputStream *in, 
		tt_int32 numFWords
	 )
	{
	 	cvtClass *t= New_cvtEmptyClass( mem,   numFWords);
	 	return( Read_cvtClass(t,in) ); 
	}


#ifdef ENABLE_WRITE
		/* NOT YET IMPLEMENTED- see Write_hmtxClass, for example*/
#endif 


#ifdef TT_ENABLE_PRINTF
 
	void Print_cvtClass( cvtClass *t ) 
	{
		tt_int32 i;
		tt_int32 numFWords=t->numFWords;
	 	tt_int16 *varFWords= (tt_int16 *)t->varFWords;
	 	tt_int32 numLines;
	 	tt_int32 code; 	
	 	numLines=
	 	  (numFWords+(cvtValuesPerLine-1))/
	 	  		cvtValuesPerLine; 	  
	 	  				
		printf("\n---------- Begin cvt  ----------\n");
	 	for ( i = 0; i < numFWords; i++ ) 
		{
			/* Don't print fillers at the end.*/
			if (i< numFWords)
			{
				code= *varFWords++;
				printf("%6d, ", code );
			}
			/* Do a line break, occasionally*/
			if ( ((i+1) %cvtValuesPerLine) == 0 )
				 printf("\n");
		}	 	 
		
		printf("\n----------  End cvt  ----------\n");
	}

#endif /* TT_ENABLE_PRINTF */


	void Delete_cvtClass( cvtClass *t )
	{
		if ( t != NULL ) {
			tsi_DeAllocMem( t->mem, t->varFWords );
			tsi_DeAllocMem( t->mem, t );
		}
	}





#ifdef TT_ENABLE_PRINTF
 
	void Print_glyphClassInstructions(  GlyphClass  *t , tt_int32 glyphIndex) 
	{
	 
		tt_int32 i;
	 	tt_int32 numInstructions=t->hintLength;
	 	tt_uint8 *instructionPtr=t->hintFragment;
	 	tt_int32 numLines;
	 	tt_uint16 code; 	
	 	numLines=
	 	  (numInstructions+(glyphInstructionsPerLine-1))/
	 	  		glyphInstructionsPerLine; 	  		
		if(numInstructions>0)
		{
			printf("\n---------- Begin Glyph %d   Instructions  ----------\n", glyphIndex);
		 	for ( i = 0; i < numInstructions; i++ ) 
			{
				 /*Don't print fillers at the end.*/
				if (i< numInstructions)
				{
					code= *instructionPtr++;
					printf("%4x, ", code );
				}
				 /* Do a line break, occasionally*/
				if ( ((i+1) %glyphInstructionsPerLine) == 0 )
					 printf("\n");
			}	 	 
			printf("\n----------  End Glyph Instructions  ----------\n");
		}
	}

#endif 
 	/* TT_ENABLE_PRINTF */
#endif  
