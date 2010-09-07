/*
 * @(#)config.h	1.18 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * CONFIG.H
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
/***********************************************/
#ifndef __T2K_CONFIG__
#define __T2K_CONFIG__

#include <stdlib.h>

#ifdef UNUSED
#undef UNUSED
#endif
#define UNUSED(x) x


/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
/**** **** **** BEGIN configuration defines  #1 --- #15  **** **** ****/
/* The T2K client has to define the meaning of these 3 functions */
/*** #1 ***/
#define CLIENT_MALLOC( size )			malloc( size )
/* #define CLIENT_MALLOC( size )			AllocateTaggedMemoryNilAllowed(n,"t2k") */
/*** #2 ***/
#define CLIENT_FREE( ptr )			free( ptr )
/* #define CLIENT_FREE( ptr )				FreeTaggedMemory(p,"t2k") */
/*** #3 ***/
#define CLIENT_REALLOC( ptr, newSize )	        realloc( ptr, newSize )
/* #define CLIENT_REALLOC( ptr, newSize )	ReallocateTaggedMemoryNilAllowed(ptr, size, "t2k") */

/*** #4 ***/
/* Here the client can optionally redefine assert, by adding two lines according to the below example  */
/* #undef assert  (line1) */
/* Just leave it for some clients, OR	*/
/* #define assert(cond) 				CLIENT_ASSERT( cond ), OR for a _FINAL_ build _ALWAYS_ define as NULL 	*/
/* #define assert(cond) 				NULL					*/


/*** Start of optional features #5 --- #15 ***/
/* The optional features increase ROM/RAM needs, so only enable them if you are using them */
/*
 * This enables the following T2K functions/methods:
 * T2K_GetGlyphIndex(),T2K_MeasureTextInX(),T2K_GetIdealLineWidth,T2K_LayoutString()
 */
/*** #5 ***/
/* #define 								ENABLE_LINE_LAYOUT */
#define 								ENABLE_LINE_LAYOUT

/*** #6 ***/
/* #define 								ENABLE_KERNING */
/* #define 								ENABLE_KERNING */

/*** #7 we consume 8 * somesize bytes for the cache ***/
/* #define 								LAYOUT_CACHE_SIZE somesize */
/*
 * This just speeds up T2K_MeasureTextInX()
 * It only makes sense to enable if ENABLE_LINE_LAYOUT and ENABLE_KERNING is enabled
 * and you are using T2K_MeasureTextInX().
 */
#define 								LAYOUT_CACHE_SIZE 149

/*** #8 ***/
/* See more info in T2K.H */
#define 								ALGORITHMIC_STYLES

/*** #9 Always enable if you need Type 1 font support ***/
/* #define 								ENABLE_T1 */
#define 								ENABLE_T1

/*** #10 If you have enabled Type 1 support and also need Mac specific Type 1 then also enable this ***/
/* #define 								ENABLE_MAC_T1 */
/* #define 								ENABLE_MAC_T1 */

/*** #11 Always enable if you need CFF font support ***/
/* #define 								ENABLE_CFF */
#define 								ENABLE_CFF

/*** #12 Always enable if you need to be able to read entropy encoded T2K fonts (for compact Kanji fonts) ***/
/* #define 								ENABLE_ORION */


/*** #13 enable if you need non RAM/ROM resident fonts. Allows you to leave the fonts on the disk/server etc. ***/
/* #define 								ENABLE_NON_RAM_STREAM */
#define 								ENABLE_NON_RAM_STREAM

/*** #14 enable if you want to use a non-zero winding rule in the scan-converter instead of even-odd fill ***/
/* The recommended setting is to leave it on */
/* See info in T2K.H */
/* #define USE_NON_ZERO_WINDING_RULE */
#define USE_NON_ZERO_WINDING_RULE

#ifdef ALGORITHMIC_STYLES
#ifdef USE_NON_ZERO_WINDING_RULE
#define ORIENTBOLD_STYLES
	/* Orientation checking is only used for non-zero winding rule. */
	/* Otherwise, it should always be turned on!. */
#endif
#endif



/*** #15 Only enable if you need embedded bitmap font support ***/
/* #define 								ENABLE_SBIT */
#define 								ENABLE_SBIT

/* #XX Research on a future T2K format, DO NOT ENABLE!!! */
/* #define ENABLE_T2KE */


/*** End of optional features ***/
/**** **** **** END configuration defines  #1 --- #15    **** **** ****/
/* The T2K client is not supposed to change anything else in here beside items #1 -- #14 */
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/


/* Below we just have internal, non-user-configurable stuff */


#define ENABLE_WRITE
#define ENABLE_PRINTF
/* #define SAMPO_TESTING_T2K , should not be defined in a release going out from Type Solutions, Inc. */

/* Only ONE can and has to be defined at a time. They can NEVER be simultaneously defined  */
#define ENABLE_AUTO_GRIDDING
/* autogridding needs to be on for TT_HINTING */
#define ENABLE_TT_HINTING
/* #define ENABLE_AUTO_HINTING */

/* #define T2K_SCALER */
#define T2K_SCALER


/* #define ENABLE_T2KE_EDITING */


#ifdef ENABLE_WRITE
#undef ENABLE_WRITE
#endif
#ifdef ENABLE_PRINTF
#undef ENABLE_PRINTF
#endif



#ifdef SAMPO_TESTING_T2K
#define ENABLE_WRITE
#define ENABLE_PRINTF
#endif /* SAMPO_TESTING_T2K */


#ifdef ENABLE_T1
	#define T1_OR_T2_IS_ENABLED	
#endif

#ifdef ENABLE_CFF
	#ifndef T1_OR_T2_IS_ENABLED
		#define T1_OR_T2_IS_ENABLED
	#endif
#endif

/* Set to determine if spot size is to be implemented. Also see "useSpotSize" */	
#define use_engine_characteristics_in_hints
#define USESPOTSIZE 0x00010000L
#ifdef use_engine_characteristics_in_hints
	extern  long	useSpotSize;
#endif


/* The stuff below is for drop out control and UIDebug is for
   debugging on the Mac. */

#ifdef debugging
#ifdef __MWERKS__
#define UIDebug 
#endif
#endif

/* define use of dropout control:
   Note: all  dropout control is disabled when AllowDropoutControl is
   NOT defined.   
   Note: if UIDebug is turned on, then the hinted dropout control can be forced on.
 	
*/

#define xAllowDropoutControl
#ifdef AllowDropoutControl
 
/*  below for hinted code, no dropout is performed if both axes are too small
    Note: if anti-aliasing is used with hinting, then the anti-aliasing scanner 
    method is used- see below. For t2k scanning, see makeBits 
*/
 		 
#define MinHintedDropoutControl  4	
/*  at this size (down to MinHintedDropoutControl), the dropout is forced on. */
#define MaxCriticalHintedDropoutControl 9	
/* Above this value, dropout control is always turned off. */
#define MaxHintedDropoutControl 200
/* T2K is used for non-antialiased and non-hinted processing */
#define MinT2KDropoutControl  4	
#define MaxCriticalT2KDropoutControl 9	
#define MaxT2KDropoutControl 200

/* AntiAlias dropout applies to all forms of antialiasing including
   code that is hinted (or not). 
*/
#define MinAntiAliasDropoutControl  3	
#define MaxCriticalAntiAliasDropoutControl 4	
#define MaxAntiAliasDropoutControl 50

#endif


/****      End of configuration defines     ****/
/***********************************************/
#endif /* __T2K_CONFIG__ */
