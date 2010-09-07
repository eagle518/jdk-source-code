/*
 * @(#)t2k.h	1.25 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * T2K.H
 * Copyright (C) 1989-1998 all rights reserved by Type Solutions, Inc. Plaistow, NH, USA.
 * http://www.typesolutions.com/
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
#ifndef __T2K_T2K__
#define __T2K_T2K__
#include "config.h"
#include "dtypes.h"
#include "tsimem.h"
#include "t2kstrm.h"
#include "truetype.h"
#include "glyph.h"
	/*  ORIENTBOLD_STYLES */
#include "OrientDB.h"
#include "Orient.h"

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */





#ifdef T2K_DOCUMENTATION
-------------------------
Q1) What files do I need to look at ?
A1) First you need to familiarize yourself with this file "T2K.H"."T2K.H" contains
documentation, a coding example and the actual T2K API.
Second you need to look at "CONFIG.H". "CONFIG.H" is the only file you normally
need to edit. The file configures T2K for your platform, and it enables
or disables optional features, and it allows you to build debug or non-debug versions.
The file itself contains more information. Turn off features you do not need in order
to minimize the size of the T2K font engine.
-------------------------
Q2) What is the basic principle for the usage of T2K API?
A2) The basic idea is that T2K was designed to be object oriented, even though the
actual implementation is only using ANSI C. This means that you will be creating
a number of objects when you use T2K. All classes have a constructor and destructor.
It is important that you call the proper destructor when you are done with a particular
object.
-------------------------
Q3) What is the best way of getting T2K going on a new platform ?
A3) It is best to first configure "CONFIG.H" and then look at the
coding example below at T2K_DOCUMENTATION_CODING_EXAMPLE.
We recommend that you start "outside in".

First create and destroy a "Memhandler" object:
(This is the "outermost" object.)

	tsiMemObject *mem = NULL;

	mem	= tsi_NewMemhandler( &errCode );
	assert( errCode == 0 );

	/* Destroy the Memhandler object. */
	tsi_DeleteMemhandler( mem );
	
Next create an InputStream object.
	tsiMemObject *mem = NULL;
	InputStream *in = NULL;

	mem	= tsi_NewMemhandler( &errCode );
	assert( errCode == 0 );

		in 	= New_InputStream3( mem, data1, size1, &errCode ); /* otherwise do this if you allocated the data  */
		assert( errCode == 0 );
		
		/* Destroy the InputStream object. */
		Delete_InputStream( in, &errCode  );
		
	/* Destroy the Memhandler object. */
	tsi_DeleteMemhandler( mem );
	
Next you would create the "sfntClass" object and then finally the "T2K" scaler object.
-------------------------
Q4) Ok, then what do I do with the T2K scaler object ?
A4) First you need to set the "transformation" with the
T2K_NewTransformation() call/method. Basically you specify the
pointsize, x and y resolution, and a 2*2 transformation matrix,
and true/false for if you want embedded bitmaps to be enabled.

Then you call T2K_RenderGlyph() to actually get bitmap and/or outline data.
After you are done with the output data you need to call T2K_PurgeMemory to
free up memory.
-------------------------
Q5)	So I call T2K_RenderGlyph(), but where do I get access to the bitmap data ?
A5) You get access to the bitmap data through public fields in the T2K class/structure.
Find the following fields:
	/* Begin bitmap data */
	tt_int32 width, height;
	F26Dot6 fTop26Dot6, fLeft26Dot6;
	tt_int32 rowBytes;
	unsigned char *baseAddr; /* unsigned char baseAddr[N], 	N = t->rowBytes * t->height */
#ifdef ENABLE_T2KE
	tt_uint32 *baseARGB;
#endif /* ENABLE_T2KE */
	/* End bitmap data */
	
baseAddr is either a bit-array, or a byte array.
baseARGB is a 32 bit array (ARGB). baseARGB is still experimental and not released.
-------------------------
Q6)	Can you give me a simple/naive example of how to actually draw a character ?
A6) Ok, something like this should work:
/*
 *	Slow naive example on how to get bitmap data from the T2K scaler object.
 *  The example assumes a screen-coordinate system where the top leftmost position on the screen is 0,0.
 */
static void MyDrawCharExample( T2K *scaler, int x, int y  )
{
	tt_uint16 left, right, top, bottom;
	unsigned short R, G, B, alpha;
	tt_uint32 *baseARGB = NULL;
	int xi, yi, xd;
	char *p;
	
	p = (char *)scaler->baseAddr;
#ifdef ENABLE_T2KE
	baseARGB = scaler->baseARGB;
#endif


	left 	= 0 + x;
	top 	= 0 + y;
	right	= scaler->width  + x;
	bottom	= scaler->height + y;


	if ( baseARGB == NULL && p == NULL ) return; /*****/
	assert( T2K_BLACK_VALUE == 120 );
	
	MoveTo( x, y );
	
	for ( yi = top; yi < bottom; yi++ ) {
		for ( xi = left; xi < right; xi++ ) {
			xd = xi - left;

#ifdef USE_COLOR
			if ( baseARGB != NULL ) {
				alpha = baseARGB[xd] >> 24;			/* Extract alpha */
				R = (baseARGB[xd] >> 16) & 0xff;	/* Extract Red */
				G = (baseARGB[xd] >>  8) & 0xff;	/* Extract Green */
				B = (baseARGB[xd] >>  0) & 0xff;	/* Extract Blue */
			} else {
				alpha = p[xd];
				alpha = alpha + alpha + (alpha>>3); /* map [0-120] to [0,255] */
				R = G = B = 0;						/* Set to Black */
			}

			if ( alpha ) {
				RGBColor colorA, colorB; 		/* RGBColor contains 16 bit color info for R,G,B each */
				
				GetCPixel( xi, yi, &colorB );	/* Get the background color */
				alpha++; /* map to 0-256 */
				/* newAlpha = old_alpha + (1.0-old_alpha) * alpha */
				R = (((tt_int32)(256-alpha) * (colorB.red	>> 8) 	+ alpha * R	)>>8);	/* Blend foreground and background colors */
				G = (((tt_int32)(256-alpha) * (colorB.green	>> 8) 	+ alpha * G )>>8);
				B = (((tt_int32)(256-alpha) * (colorB.blue	>> 8) 	+ alpha * B )>>8);
				
				assert( R >= 0 && R <= 255 );

				colorA.red		= R << 8;	/* Map 8 bit data to 16 bit data */
				colorA.green	= G << 8;
				colorA.blue		= B << 8;
				RGBForeColor( &colorA );	/* Set the forground color/paiont to colorA */
				MoveTo( xi, yi );			/* Paint pixel xi, yi with colorA */
				LineTo( xi, yi );
			}
#else
			/* Paint pixel xi, yi */
			if ( p[ xd>>3] & (0x80 >> (xd&7)) ) {
				MoveTo( xi, yi );
				LineTo( xi, yi );
			}
#endif
		}
		/* Advance to the next row */
		p += scaler->rowBytes;
		if ( baseARGB != NULL ) {
			baseARGB += scaler->rowBytes;
		}
	}
}
-------------------------
Q7)	How do I draw a string with the above MyDrawCharExample() ?
A7) 
	F16Dot16 x, y;
	
	x = y = 12 << 16;
	while (characters to draw..)
		/* Render the character */
		T2K_RenderGlyph( scaler, charCode, 0, 0, GREY_SCALE_BITMAP_HIGH_QUALITY, T2K_SCAN_CONVERT,  &errCode );
		assert( errCode == 0 );
		/* Now draw the character */
		MyDrawCharExample( scaler, ((x + 0x8000)>> 16) + (scaler->fLeft26Dot6 >> 6), ((y + 0x8000)>> 16) - (scaler->fTop26Dot6 >>6)  );
		x += scaler->xAdvanceWidth16Dot16;	/* advance the pen forward */
		/* Free up memory */
		T2K_PurgeMemory( scaler, 1, &errCode );
		assert( errCode == 0 );
	}
-------------------------
Q8)	How do I get grey-scale or monochrome output ?
A8) For monochrome output set 5th input parameter to T2K_RenderGlyph called greyScaleLevel equal to BLACK_AND_WHITE_BITMAP. 
	To get grey-scale output set 5th input parameter to T2K_RenderGlyph called greyScaleLevel equal to GREY_SCALE_BITMAP_HIGH_QUALITY.
-------------------------
Q9) My output device support grey-scale. Should I use it ?
A9) Yes for the best quality and end-user experience you should use grey-scale whenever possible.
-------------------------
Q10) How do I turn the T2K run-time hinting on or off.
A10) It is enabled by turning on the T2K_GRID_FIT bit in the 6th input parameterto T2K_RenderGlyph called cmd.
	 It is disabled by turning off the T2K_GRID_FIT bit in the 6th input parameterto T2K_RenderGlyph called cmd.
-------------------------
Q11) When should the T2K run-time hinting be on or off.
A11) For monochrome output your probably mostly want to turn it on.
     For a high quality display device such as a computer monitor you should probably turn it on.
     For a device such as a TV monitor you should turn it off.
-------------------------
Q12) I am using an interlacing TV as the output device. How do I make it look good.
A12) First turn off grid-fitting to get an image with less sharp transitions. (This also speeds up T2K)
	 Then turn on T2K_TV_MODE if you use integer metrics and do not use fractional positioning to improve the quality.
     Then you most likely also want to experiment with a simple filter to make the image
     more blurry. A simple 3*3 convolution is probably sufficient. You should
     probably "average" more in the y-direction than the x-direction to avoid the
     interlacing flicker. Alternatively your HW may have alrady have this built in.

-------------------------
Q13) A T2K call/method returned an error. What should the code do ?
A13) T2K automatically deletes all of its objects when it hits an error.
This means that all references to T2K objects become invalid, and can no
longer be used.
-------------------------
Q14) I noticed that there are a lot of asserts in the code...Why ?
A14) Asserts are in the code to detect/prevent programmer errors. The idea is that an
assert should never happen in real life. They are only there to catch programmer errors.
In a release build you need to turn off asserts in "CONFIG.H" to increase speed and
to reduce the code size. However in debug builds please leave it on, to ensure that
everything is working as intended.
-------------------------
Q15) How do I decide what mapping table to use in a TrueType or T2K font ?
A15) Use Set_PlatformID( scaler, ID ), and Set_PlatformSpecificID( scaler, ID ) 
To use the Unicode mapping used by Windows do the following:
Set_PlatformID( scaler, 3 );
Set_PlatformSpecificID( scaler, 1 );
-------------------------
Q16) Can you explain the second parameter called code to T2K_RenderGlyph ?
A16) Yes, this normally specifies the character code for the character you wish to render.
     However if you wish to use the glyph index instead then set the T2K_CODE_IS_GINDEX
     bit in the 6th input parameter to T2K_RenderGlyph called cmd. The glyph index is
     simply a number from 0 to N-1, assuming the font contains N glyphs.
     (N = T2K_GetNumGlyphsInFont( scaler );)
-------------------------
Q17) Does the outline winding direction matter in T2K.
A17) Yes Postscript outlines should use the correct winding direction and
     TrueType and T2K outlines also need to use the correct winding direction
     which is actually the opposite of the Postscript direction. It matters because
     the run-time-hinting process use this information to figure out where the
     black and white areas are. For TrueType and T2K the direction should be such
     that if you follow a contour in the direction of increasing point numbers
     then the black(inside) area should be on your right.
-------------------------
Q18) I need my fonts to be as small as possible. What should I do ?
A18) You need to contact Type Solutions to have them translated to
     T2K format.
-------------------------
Q19) Can you explain ALGORITHMIC_STYLES in CONFIG.H ?
A19) Yes it enables algorithmic styling.
	 The 5th parameter to New_sfntClassLogical(); T2K_AlgStyleDescriptor *styling
	 is normally set to NULL, but if ALGORITHMIC_STYLES is enabled you can set
	 it equal to an algoritmic style descriptor. Here is an example using the
	 algorithmic bolding provided by T2K.
	 
	 Original T2K Styling () ( "ENABLE_TT_HINTING" not defined ): ==>> 
	 			
						style.StyleFunc			= 	tsi_SHAPET_BOLD_GLYPH;
						style.StyleMetricsFunc	=	tsi_SHAPET_BOLD_METRICS;
						style.params[0] = 5L << 14;
						sfnt0 = New_sfntClassLogical( mem, fontType, 0, in, &style, &errCode );
						
			You can also write your own outline based style modifications and use them instead
			of the algorithmic bolding provided by T2K. Just model them after the Type Solutions
			code for algoritmic bolding in "SHAPET.c".
	
	Style with Hinting: ( "ENABLE_TT_HINTING" IS defined ): ==>> 
			
	 			The new style mechanism uses a different routine name, and different parameters.
	 			In the file shapet.h, shapet.c, see the discussion on the parameters for StyleFuncPost, and for
	 			writing your own StyleFuncPost routines..
	 			
	 			
		  		styling.StyleFuncPost			= (void *)	tsi_SHAPET_BoldItalic_GLYPH_Hinted;
				styling.StyleMetricsFunc	=	tsi_SHAPET_BOLD_METRICS;
				styling.params[0] = boldValue; 		/* Fixed point value 1.0 � boldValue � about 4.0: 1.33 is good */
													/* Use 1.0 = hexadecimal 0x10000L for no action (or don't use styling) */
				styling.params[1] = italicValue;	/* Fixed point value -2.0 � iitalicValue � about 2.0: .7 is good */
													/* Use  0  for no effect. Notice that slight negative values are good. */
 				(Use the same call to 	New_sfntClassLogical, as shown above )
 				
-------------------------
Q20) What is the story with USE_NON_ZERO_WINDING_RULE in CONFIG.H ?
A20) The recommended setting is to leave it on.  This determines what
kind of fill rule the T2K scan-converter will use. This enables
a non-zero winding rule, otherwise the scan-converter will use an even-odd
filling rule. For example the even-odd filling rule will turn an area
where two strokes overlap (rare) into white, but the non zero winding
rule will keep such areas black. For an embedded system where the fonts
are well build and you do not have overlapping strokes you will get a
small (proably less than 1%) speed-up by disabling this.
-------------------------
Q21) In the internal part of the options, there is an option called SAMPO_TESTING_T2K (!)
which should be disabled. The only thing it does is to enable ENABLE_WRITE and ENABLE_PRINTF.
But those two options are enabled unconditionally at the top of "don't touch" part. What is going on?
A21) :-) You should leave it off, since it is in the "don't touch" part.
When T2K is built as a font engine T2K_SCALER is defined.(Not when built as a translator)
 ifdef T2K_SCALER
actually does an  undef ENABLE_PRINTF and an  undef ENABLE_WRITE
Since the T2K font engine never does disk writes and does not use printf statements.
However, when the code is undergoing testing at Type Solutions the
T2K font engine may write to disk for logging purposes and use printf statements.
-------------------------
Q22) Are the functions used in the T2K_DOCUMENTATION_CODING_EXAMPLE part of t2k.h the only public APIs?
     Are there any other functions that might be useful to know? 
A22) Yes, but the idea is that you should only use functions/methods visible in T2K.H.
     For instance T2K_MeasureTextInX() may be useful for quickly determining lengths of text string.
     If you find a need to use something else then let us know and if it makes sense
     we may bring out a public way to do it.
     Do not rely on any function/methods outside of T2K since they may change from release to release.
-------------------------
Q23) I found that when a string contains a space, T2K_RenderGlyph returns NULL baseAddr. Why is that?
     Do I have to check the existence of space characters and advance x position accordingly?
A23) There is no bitmap to draw! Makes sense :-).
     Do not check for space, etc. just check for baseAddr == NULL instead 
	 In the future with T2KE check for ( baseARGB == NULL && baseAddr == NULL )
-------------------------
Q24) Are there functions for measuring widths and other metrics of strings
     (such as X11s XTextWidth, and XTextExtent) ?
A24) I think the closest to XTextWidth is T2K_MeasureTextInX in T2K.H
     (It measures the linear un-hinted width, it can not measure the hinted width without actually
     rendering the characters)
     There is no equivalent to X11s XTExtExtent, since this sort of function typically would need
     to be implemented on top of the font-bit-map cache. T2K does not cache the output data,
     so T2K clients typically implement a cache on top of T2K so that the second time a character is
     requested it can come directly from the cache without invoking T2K. This sort of cache makes things
     go fast. String wide functions should be implemented so that they request information from the
     cache, to avoid T2K having to recompute everything several times. 
	 T2K also has two early experimental functions called T2K_GetIdealLineWidth(), T2K_LayoutString().
	 They can help you to layout an entire line so that the total width is the ideal linear width, while
	 still using run-time hinted individual characters and metrics. At least is attempts to do this
	 by mostly putting the nonlinearities into the space characters between the words.
-------------------------
Q25) Can I edit T2K_BLACK_VALUE, and T2K_WHITE_VALUE so that I get a different range for the grey-scale ?
A25) No, you should not edit anything in T2K.H. They are there so that you can put in an assert in your
     code so that you can automatically detect if they are ever changed by Type Solutions in the future.
-------------------------
Q26) What does T2K_TV_MODE do?
A26) See future T2K release.
-------------------------
Q27) What next ?
A27) Go try the code. Good Luck! :-)
-------------------------


#endif /* T2K_DOCUMENTATION */

#ifdef T2K_DOCUMENTATION_CODING_EXAMPLE
	/* First configure T2K, please see "CONFIG.H" !!! */

	/* This shows a pseudo code example for how to use the T2K scaler. */
	tsiMemObject *mem = NULL;
	InputStream *in = NULL;
	sfntClass *font = NULL;
	T2K *scaler = NULL;
	int errCode;
	T2K_TRANS_MATRIX trans;
	T2K_AlgStyleDescriptor style;			
			

	/* Create a Memhandler object. */
	mem	= tsi_NewMemhandler( &errCode );
	assert( errCode == 0 );
		/* Point data1 at the font data */
		If ( TYPE 1 ) {
			if ( PC Type 1 ) {
				data1 = ExtractPureT1FromPCType1( data1, &size1 );
				/* data1 is not allocated just munged by this call ! */
			} else if ( Mac Type 1 ) {
				short refNum = OpenResFile( pascalName ); /* Open the resource with some Mac call */
				data1 = (unsigned char *)ExtractPureT1FromMacPOSTResources( mem, refNum, &size1 );
				CloseResFile( refNum ); /* Close the resource file with some Mac call */
				/* data1 IS allocated by the T2kMemory layer! */
			}
		}
		/* Please make sure you use the right New_InputStream call depending on who allocated data1,
		  and depending on if the font is in ROM/RAM or on the disk/server etc. */
		/* Create an InputStream object for the font data */
		in 	= New_InputStream( mem, data1, size1, &errCode ); /* if data allocated by the T2kMemory layer */
		assert( errCode == 0 );
	  	**** OR ****
		in 	= New_InputStream3( mem, data1, size1, &errCode ); /* otherwise do this if you allocated the data  */
		**** OR *****
		/* Allows you to leave the font on the disk, or remote server for instance (!) */
		in = New_NonRamInputStream( mem, fpID, ReadFileDataFunc, length, &errCode  ); 
		
		assert( errCode == 0 );
			/* Create an sfntClass object. (No algorithmic styling) */
			short fontType = FONT_TYPE_TT_OR_T2K; /* Or, set equal to FONT_TYPE_1 for type 1, FONT_TYPE_2 for CFF fonts */
			font = New_sfntClass( mem, fontType, in, NULL, &errCode );
			**** OR ****
			/* alternatively do this for formats that support multiple logical fonts within one file */
			font = New_sfntClassLogical( mem, fontType, logicalFontNumber, in, NULL, &errCode );
			
			/* Or if you wish to use algorithmic styling do this instead
			 * T2K_AlgStyleDescriptor style;
			 *
			 * style.StyleFunc 			= 	tsi_SHAPET_BOLD_GLYPH;
			 * style.StyleMetricsFunc	=	tsi_SHAPET_BOLD_METRICS;
			 * style.params[0] = 5L << 14; (* 1.25 *)
			 * font = New_sfntClass( mem, fontType, in, &style, &errCode );
			 */
			assert( errCode == 0 );
				/* Create a T2K font scaler object.  */
				scaler = NewT2K( font->mem, font, &errCode );
				assert( errCode == 0 );
					/* 12 point */
					trans.t00 = ONE16Dot16 * 12;
					trans.t01 = 0;
					trans.t10 = 0;
					trans.t11 = ONE16Dot16 * 12;
					/* Set the transformation */
					T2K_NewTransformation( scaler, true, 72, 72, &trans, true, &errCode );
					assert( errCode == 0 );
					loop {
						/* Create a character */
						T2K_RenderGlyph( scaler, charCode, 0, 0, BLACK_AND_WHITE_BITMAP, T2K_GRID_FIT | T2K_RETURN_OUTLINES  | T2K_SCAN_CONVERT, &errCode );
						assert( errCode == 0 );
						/* Now draw the char */
						/* Free up memory */
						T2K_PurgeMemory( scaler, 1, &errCode );
						assert( errCode == 0 );
					}
				/* Destroy the T2K font scaler object. */
				DeleteT2K( scaler, &errCode );
				assert( errCode == 0 );
			/* Destroy the sfntClass object. */
			Delete_sfntClass( font, &errCode );
			assert( errCode == 0 );
		/* Destroy the InputStream object. */
		Delete_InputStream( in, &errCode  );
		assert( errCode == 0 );
	/* Destroy the Memhandler object. */
	tsi_DeleteMemhandler( mem );

#endif /* T2K_DOCUMENTATION_CODING_EXAMPLE */


/************************************************************/
/************************************************************/
/************************************************************/
/************************************************************/
/************************************************************/
/************************************************************/
/***** HERE THE ACTUAL NON-DOCUMENTATION CONTENTS BEGIN *****/
/************************************************************/
/************************************************************/
/************************************************************/
/************************************************************/
/************************************************************/

tt_int32  IsUnhintableMatrix( T2K_TRANS_MATRIX *xfrm );

/* Pass data which is needed by hinting setup. */
	/* speicify the x, and y vectors after rotation. */
typedef struct {

   Frac2Dot14 x;
   Frac2Dot14 y;

} ShortFracVector;
	
typedef struct {
   ShortFracVector  xVec;
   ShortFracVector  yVec;

}  VectorSet;
 
/* Find the result of transforming the unit x,y vectors */
void ExtractUnitVectors( VectorSet *vs,F16Dot16 t00, F16Dot16 t01, F16Dot16 t02, F16Dot16 t03);

/* public getter functions */
#define T2K_FontHasKerningData( t ) ((t)->font != NULL && (t)->font->kern != NULL)
#define T2K_GetNumGlyphsInFont( t ) GetNumGlyphs_sfntClass( (t)->font )

/*
 * This is the structure/class for the T2K scaler.
 */
/* Pass data which is needed by hinting setup. */
typedef struct {
			
	T2K_TRANS_MATRIX trans;
	tt_int32 xRes;
	tt_int32 yRes;
	F16Dot16 pointSize;
	VectorSet unitVectors; /* unit vectors for above matrix transform */

} TrueTypeTransformData;


/* 
   declare a set of flags to control dropout.
   see masterDropOutControlFlags, below, in T2K structure.
   see routine FindDropoutControlFlags 
   also see "AllowDropoutControl" in the Config.h file, which sets up all definitions. 
*/
typedef tt_int32 DropoutControlFlags;
		
#define DisableScannerDropoutFlag	1
#define EnableScannerDropoutFlag	2
/* we need to shift by 2 when packing the 2 flags. */
#define HintDropoutShift 0
#define T2KDropoutShift 2
#define AntiAliasDropoutShift 4

 

typedef struct {
	/* private */
	tt_int32 stamp1;
	tsiMemObject *mem;
	F16Dot16 t00, t01;
	F16Dot16 t10, t11;
	int is_Identity;
	TrueTypeTransformData ttd; /* only for hinting implementation.*/

	
	/* public */
	tt_int32       	numGlyphs;
	
	/*** Begin font wide HORIZONTAL Metrics data */
	int			horizontalFontMetricsAreValid;
	F16Dot16	xAscender,	yAscender;
	F16Dot16	xDescender,	yDescender;
	F16Dot16	xLineGap,	yLineGap;
	F16Dot16	xMaxLinearAdvanceWidth, yMaxLinearAdvanceWidth;
	F16Dot16 	caretDx, caretDy; /* [0,K] for vertical */
	/*** End font wide HORIZONTAL Metrics data */
	
	/*** Begin font wide VERTICAL Metrics data */
	int			verticalFontMetricsAreValid;
	F16Dot16	vert_xAscender,		vert_yAscender;
	F16Dot16	vert_xDescender,	vert_yDescender;
	F16Dot16	vert_xLineGap,		vert_yLineGap;
	F16Dot16	vert_xMaxLinearAdvanceWidth, vert_yMaxLinearAdvanceWidth;
	F16Dot16 	vert_caretDx,		vert_caretDy; /* [0,K] for vertical */
	/*** End font wide VERTICAL Metrics data */
	
	/*** Begin glyph specific HORIZONTAL Metrics data */
	int			horizontalMetricsAreValid;
	F16Dot16	xAdvanceWidth16Dot16,				yAdvanceWidth16Dot16;
	F16Dot16	xLinearAdvanceWidth16Dot16,			yLinearAdvanceWidth16Dot16;
	F26Dot6		fTop26Dot6, fLeft26Dot6;			/* For positioning the top left corner of the bitmap. */
	/*** End glyph specific HORIZONTAL Metrics data */
	
	/*** Begin glyph specific VERTICAL Metrics data */
	int			verticalMetricsAreValid;
	F16Dot16	vert_xAdvanceWidth16Dot16,			vert_yAdvanceWidth16Dot16;
	F16Dot16	vert_xLinearAdvanceWidth16Dot16,	vert_yLinearAdvanceWidth16Dot16;
	F26Dot6		vert_fTop26Dot6, vert_fLeft26Dot6;	/* For positioning the top left corner of the bitmap. */
	/*** End glyph specific VERTICAL Metrics data */
	
	/*** Begin outline data */
	GlyphClass *glyph;
	/*** End outline data */
	
	/*** Begin bitmap data */
	tt_int32 width, height;
	tt_int32 rowBytes;
	unsigned char *baseAddr; /* unsigned char baseAddr[N], 	N = t->rowBytes * t->height */
#ifdef ENABLE_T2KE
	tt_uint32 *baseARGB;
#endif /* ENABLE_T2KE */
	int embeddedBitmapWasUsed; /* This is a public field set by T2K_RenderGlyph() */
	/*** End bitmap data */

	/* private */
	/* F16Dot16 xPointSize, yPointSize; */
	/* tt_int32 xRes, yRes; */
	tt_int32 xPixelsPerEm, yPixelsPerEm;
 	DropoutControlFlags masterDropOutControlFlags;
	F16Dot16 xPixelsPerEm16Dot16, yPixelsPerEm16Dot16;
	F16Dot16 xMul, yMul;
	tt_int32 ag_xPixelsPerEm, ag_yPixelsPerEm;
	char xWeightIsOne;
	int fontCategory;
	int enableSbits;			/* This is a private field set by T2K_NewTranformation */

	sfntClass *font;
	/* Hide the true data Types from our client */
	void *hintHandle; /* ag_HintHandleType hintHandle */
	/* void *globalHintsCache; Moved into sfntClass */
	
#ifdef LAYOUT_CACHE_SIZE
	tt_uint32 tag[LAYOUT_CACHE_SIZE];
	tt_int16 kernAndAdvanceWidth[ LAYOUT_CACHE_SIZE ];
#ifdef ENABLE_KERNING
		tt_int16 kern[ LAYOUT_CACHE_SIZE ];
	#endif /* ENABLE_KERNING */
#endif /* LAYOUT_CACHE_SIZE */

#ifdef ENABLE_TT_HINTING
	/* see "perFont" data structure, and TTHintFont.c module.
	  Inited to zero(NewT2K), created by NewTTHintFontForT2K, released by ReleaseTTHintFontForT2K 
	 */
	void * TTHintFontData; 
	
	/* see "perTransformation" data structure, and TTHintTrab.c module.
	  Inited to zero(NewT2K), 
	  	allocated by:AllocTTHintTranForT2K
	  	created by NewTTHintTranForT2K:
	  	released by ReleaseTTHintTranForT2K:
	 */
	void * TTHintTranData; 
#endif
	/*  ORIENTBOLD_STYLES */
	ContourData	theContourData;
 
	
	tt_int32 stamp2;
} T2K;

#ifdef ENABLE_TT_HINTING
void ApplyPostStyle(GlyphClass *glyph,	T2K *t);
#endif


#ifdef ENABLE_AUTO_GRIDDING
/*
 * The T2K scaler constructor.
 * For all T2K functions *errCode will be set to zero if no error was encountered
 * If you are not using any algorithmic styling then set styling = NULL
 */
T2K *NewT2K( tsiMemObject *mem, sfntClass *font, int *errCode );

/* Two optional functions to set prefered platform and/or prefered platform specific ID */
/* Invoke right after NewT2K(), t is of type (T2K *) */
#define Set_PlatformID( t, ID ) 			((t)->font->preferedPlatformID = (ID))
#define Set_PlatformSpecificID( t, ID ) 	((t)->font->preferedPlatformSpecificID = (ID))


#ifdef ENABLE_SBIT
/* T2K_FontSbitsExists( T2K *t ) is a Query method for checking if the font contains any sbits */
#define T2K_FontSbitsExists( t )		((t)->font->bloc != NULL)
/* T2K_FontSbitsAreEnabled( T2K *t ) is a Query method for checking if the sbits are enabled */
#define T2K_FontSbitsAreEnabled( t )	((t)->enableSbits != 0 )

/*
 * Query method to see if a particaluar glyph exists in sbit format for the current size.
 * If you need to use characterCode then map it to glyphIndex by using T2K_GetGlyphIndex() first.
 */
int T2K_GlyphSbitsExists( T2K *t, tt_uint16 glyphIndex, int *errCode  );

#endif /* ENABLE_SBIT */

/*
 * Set the transformation and x and y resolution.
 *
 * x & y point size is passed embedded in the Transformation as trans = pointSize * old-Transformation
 */
void T2K_NewTransformation( T2K *t, int doSetUpNow, tt_int32 xRes, tt_int32 yRes, T2K_TRANS_MATRIX *trans, int enableSbits, int *errCode );


/* New experimental stuff for T2KE format */
tt_int32 T2K_GetNumAxes(T2K *t);
F16Dot16 T2K_GetAxisGranularity(T2K *t, tt_int32 n);
F16Dot16 T2K_GetCoordinate(T2K *t, tt_int32 n );
void T2K_SetCoordinate(T2K *t, tt_int32 n, F16Dot16 value );
/* End experimental stuff */

/* Bits for the cmd field below */
#define T2K_GRID_FIT		0x01
#define T2K_SCAN_CONVERT	0x02
#define T2K_RETURN_OUTLINES	0x04
#define T2K_CODE_IS_GINDEX	0x08 /* Otherwise it is the charactercode */
#define T2K_USE_FRAC_PEN	0x10
#define T2K_SKIP_SCAN_BM	0x20 /* Everything works as normal, however we do _not_ generate the actual bitmap */
#define T2K_TV_MODE			0x40 /* Ideal for TV if you use integer metrics, and gray-scale (please turn off T2K_GRID_FIT) */
/* #define T2K_FLIP_GLYPH		0x80 NO longer used: Extract Glyph and flip it. */


/* For the greyScaleLevel field below */
#define BLACK_AND_WHITE_BITMAP 				0
#define GREY_SCALE_BITMAP_LOW_QUALITY		1
#define GREY_SCALE_BITMAP_MEDIUM_QUALITY	2
#define GREY_SCALE_BITMAP_HIGH_QUALITY		3 /* Recommended for grey-scale */
#define GREY_SCALE_BITMAP_HIGHER_QUALITY	4
#define GREY_SCALE_BITMAP_EXTREME_QUALITY	5 /* Slooooowest */

/* When doing grey-scale the scan-converter returns values in the range T2K_WHITE_VALUE -- T2K_BLACK_VALUE */
#define T2K_BLACK_VALUE 120
#define T2K_WHITE_VALUE 0

/* The Caller HAS to deallocate outlines && t->baseAddr with T2K_PurgeMemory( t, 1 ) */
/* fracPenDelta should be between 0 and 63, 0 represents the normal pixel alignment,
   16 represents a quarter pixel offset to the right,
   32 represents a half pixel offset of the character to the right,
   and -16 represents a quarter/4 pixel shift to the left. */
/* For Normal integer character positioning set fracPenDelta == 0 */
/* IPenPos = Trunc( fracPenPos );  FracPenDelta = fPenPos - IPenPos */
/* The bitmap data is relative to  IPenPos, NOT fracPenPos */
/*
 * The T2K call to render a character.
 */
void T2K_RenderGlyph( T2K *t, tt_int32 code, tt_int8 xFracPenDelta, tt_int8 yFracPenDelta, tt_uint8 greyScaleLevel, tt_uint8 cmd, int *errCode );

void t2k_SetStyling( sfntClass *t, T2K_AlgStyleDescriptor *styling );

#define MAX_PURGE_LEVEL 2
/*
 * Call after you are done with the output data from T2K_RenderGlyph().
 * Normally set level = 1.
 */
void T2K_PurgeMemory( T2K *t, int level, int *errCode );

/*
 * The T2K destructor.
 */
void DeleteT2K( T2K *t, int *errCode );

#endif

/* Transforms xInFUnits into 16Dot16 x and y values */
void T2K_TransformXFunits( T2K *t, short xValueInFUnits, F16Dot16 *x, F16Dot16 *y);
/* Transforms yInFUnits into 16Dot16 x and y values */
void T2K_TransformYFunits( T2K *t, short yValueInFUnits, F16Dot16 *x, F16Dot16 *y);



#ifdef ENABLE_KERNING
typedef struct {
	tt_uint16	left;
	tt_uint16	right;
	tt_int16	xKern; /* value in FUnits */
	tt_int16	yKern; /* value in FUnits */
} T2K_KernPair;

/*
 * Return value is a pointer to T2K_KernPair with *pairCountPtr entries.
 * The entries consist of all kern pairs between the the character with
 * the charCode character code combined with itself and all the members
 * of baseSet. (A character should only appear once in baseSet)
 * The caller *has* to deallocate the pointer, if != NULL, with
 * tsi_DeAllocMem( t->mem, pointer );
 */
T2K_KernPair *T2K_FindKernPairs( T2K *t, tt_uint16 *baseSet, int baseLength, tt_uint16 charCode, int *pairCountPtr );
#endif /* ENABLE_KERNING */		

#ifdef ENABLE_LINE_LAYOUT

#ifdef LINEAR_LAYOUT_EXAMPLE
	/* This is a pseudo-code example */
	totalWidth = T2K_MeasureTextInX( scaler, string16, kern, numChars);
	for ( i = 0;  (charCode = string16[i]) != 0; i++ ) {
		F16Dot16 xKern, yKern;
		
		/* Create a character */
		T2K_RenderGlyph( scaler, charCode, 0, 0, BLACK_AND_WHITE_BITMAP, T2K_GRID_FIT | T2K_RETURN_OUTLINES  | T2K_SCAN_CONVERT, &errCode );
		assert( errCode == 0 );
		T2K_TransformXFunits( scaler, kern[i], &xKern, &yKern );

		bm->baseAddr 		= (char *)scaler->baseAddr;
		bm->rowBytes 		= scaler->rowBytes;
		bm->bounds.left 	= 0;
		bm->bounds.top		= 0;
		bm->bounds.right	= scaler->width;
		bm->bounds.bottom	= scaler->height;
	
		MyDrawChar( graf, x + ( (scaler->fLeft26Dot6+(xKern>>10))>>6), y - (scaler->fTop26Dot6+(yKern>>10)>>6), bm );
		/* We keep x as 32.16 */
		x16Dot16 += scaler->xLinearAdvanceWidth16Dot16 + xKern; x += x16Dot16>>16; x16Dot16 &= 0x0000ffff;
		/* Free up memory */
		T2K_PurgeMemory( scaler, 1, &errCode );
		assert( errCode == 0 );
	}

/* LINEAR_LAYOUT_EXAMPLE */
#endif 

/* Returns the total pixel width fast, and computes the kern values */
tt_uint32 T2K_MeasureTextInX(T2K *t, const tt_uint16 *text, tt_int16 *xKernValuesInFUnits, tt_uint32 numChars );


#define T2K_X_INDEX		0
#define T2K_Y_INDEX		1
#define T2K_NUM_INDECES	2

typedef struct {
	/* input */
	tt_uint16   	charCode;
	tt_uint16   	glyphIndex;
	F16Dot16 	AdvanceWidth16Dot16[ T2K_NUM_INDECES ];
	F16Dot16 	LinearAdvanceWidth16Dot16[ T2K_NUM_INDECES ];
	F26Dot6	 	Corner[ T2K_NUM_INDECES ]; /* fLeft26Dot6, fTop26Dot6 */
	tt_int32       	Dimension[ T2K_NUM_INDECES ]; /* width, height */
} T2KCharInfo;


typedef struct {
	/* output */
	F16Dot16	BestAdvanceWidth16Dot16[ T2K_NUM_INDECES ];
} T2KLayout;

/*
 * Maps the characterCode to the glyph index.
 */
tt_uint16 T2K_GetGlyphIndex( T2K *t, tt_uint16 charCode );
/*
 * Before calling create a T2KCharInfo for each character on the line and initialize
 * all the fields. You can use the above T2K_GetGlyphIndex() to get the glyphIndex.
 * Computes the ideal lineWidth. The computation takes kerning into account.
 * Initializes out
 */
void T2K_GetIdealLineWidth( T2K *t, const T2KCharInfo cArr[], tt_int32 lineWidth[], T2KLayout out[] );
/*
 * You have to call T2K_GetIdealLineWidth() first to initalize out before calling this function.
 * Computes out so that the LineWidthGoal is satisfied while taking kerning into account.
 * Note: This is an early version of the function.
 */
void T2K_LayoutString( const T2KCharInfo cArr[], const tt_int32 LineWidthGoal[], T2KLayout out[] );
 /* ENABLE_LINE_LAYOUT */

/* Find the vector cross product. */
tt_int32 crossproduct(tt_int32 dx1, tt_int32 dy1, tt_int32 dx2, tt_int32 dy2);

#endif

#ifdef __cplusplus
}
  /* __cplusplus */
#endif
 /* __T2K_T2K__ */
#endif
