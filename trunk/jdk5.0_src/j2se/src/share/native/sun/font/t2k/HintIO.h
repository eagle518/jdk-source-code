/*
 * @(#)HintIO.h	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/* */

 
   
#ifdef ENABLE_TT_HINTING
 
/* ******************* fpgm class **************************************/
	/* Allocate an fpgm class, but do not read data yet.*/
	fpgmClass *New_fpgmEmptyClass
	( 
		tsiMemObject *mem, 
		tt_int32 numInstructions
	);

	/* Read in fpgm class instructions*/
	fpgmClass *Read_fpgmClass
	 (
	 	fpgmClass *t,
	 	InputStream *in
	 );

	/* Create and read fpgm class*/
	 fpgmClass * New_fpgmClass
	(
	 	tsiMemObject *mem, 
	 	InputStream *in, 
		tt_int32 numInstructions
	 );
	 
#ifdef ENABLE_WRITE
		/* NOT YET IMPLEMENTED- see Write_hmtxClass, for example*/
#endif


#define fpgmInstructionsPerLine 10
	void Print_fpgmClass( fpgmClass *t ) ;
	
 	void Delete_fpgmClass( fpgmClass *t );


 /*  ******************* prep class **************************************/
	/*  Allocate an prep class, but do not read data yet.*/
	prepClass *New_prepEmptyClass
	( 
		tsiMemObject *mem, 
		tt_int32 numInstructions
	);

	/*  Read in prep class instructions*/
	prepClass *Read_prepClass
	 (
	 	prepClass *t,
	 	InputStream *in
	 );

	/*  Create and read prep class*/
	prepClass *New_prepClass
	(
	 	tsiMemObject *mem, 
	 	InputStream *in, 
		tt_int32 numInstructions
	 );
	
	
	
	 
#ifdef ENABLE_WRITE
		/*  NOT YET IMPLEMENTED- see Write_hmtxClass, for example*/
#endif  

#define prepInstructionsPerLine 10
	void Print_prepClass( prepClass *t ) ;

	void Delete_prepClass( prepClass *t );

/* ******************* cvt class **************************************/
	/* Allocate an cvt class, but do not read data yet.*/
	cvtClass *New_cvtEmptyClass
	( 
		tsiMemObject *mem, 
		tt_int32 numFWords
	);

	/* Read in cvt class instructions*/
	cvtClass *Read_cvtClass
	 (
	 	cvtClass *t,
	 	InputStream *in
	 );

	/* Create and read cvt class*/
	cvtClass *New_cvtClass
	(
	 	tsiMemObject *mem, 
	 	InputStream *in, 
		tt_int32 numFWords
	 );
	 
#ifdef ENABLE_WRITE
		/* NOT YET IMPLEMENTED- see Write_hmtxClass, for example*/
#endif 
			/* ENABLE_WRITE */


#define cvtValuesPerLine 10
	void Print_cvtClass( cvtClass *t ) ;

	void Delete_cvtClass( cvtClass *t );
 
 /* ******************* Glyph Hinting Printing **************************************/
#define glyphInstructionsPerLine 10
	void Print_glyphClassInstructions(  GlyphClass  *t , tt_int32 glyphIndex) ;

 
  
#endif

