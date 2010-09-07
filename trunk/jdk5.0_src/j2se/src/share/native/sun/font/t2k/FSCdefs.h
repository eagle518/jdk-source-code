/*
 * @(#)FSCdefs.h	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

 /*
	File:		FSCdefs.h

	Copyright:	© 1988-1992 by Apple Computer, Inc., all rights reserved.

*/

#ifndef fontScalerDefinitionIncludes
#define fontScalerDefinitionIncludes

#ifndef cpuIncludes
	
#endif

#define RELEASE_MEM_FRAG
#ifndef cpuPrinter
/*  #define SBIT_SUPPORT */
#endif
#define DESCARTES

#ifdef pascalAware
#define PASCAL	pascal
#else
#define PASCAL
#endif

#if !(defined(__pink__) && defined(Taligent_TRUETYPEFILE))
#ifdef THINK_C
	typedef char tt_int8;				/* THINK doesn't like the "signed" keyword */
#else
#ifndef TTHintIncludes 
		typedef signed char tt_int8;		/* explicitly make tt_int8 a signed quantity */
#endif
#endif
	#ifndef TTHintIncludes 
		typedef unsigned char tt_uint8;
		typedef short tt_int16;
		typedef unsigned short tt_uint16;
		typedef long tt_int32;
		typedef unsigned long tt_uint32;
	#endif
#endif

typedef short FWord;
typedef unsigned short uFWord;
#ifndef TTHintIncludes 
	typedef long F26Dot6;
#endif

#if !(defined(__pink__) && defined(Taligent_TRUETYPEFILE))
typedef void (*voidFunc) ();
#endif
typedef void*	VoidPtr;

/* Level 1 function pointers for gxFont scaler input record
*/
typedef void*	(*GetSFNTFunc)(long, long, long);
typedef void	(*ReleaseSFNTFunc)(void*);

/*	Level 2 function pointers for gxFont scaler input record
*/
typedef void*	(*GetSfntFunc)(long, long, long);
typedef void	(*ReleaseSfntFunc)(long, void*);
typedef void	(*ScalerErrorFunc)(long, long);
typedef long	(*InterpErrorFunc)(long);

#ifdef INTEL
typedef int	LoopCount;
typedef int	ArrayIndex;
#else
typedef tt_int16	LoopCount;		/* short gives us a DBF */
typedef tt_int32	ArrayIndex;		/* avoids EXT.L */
#endif 


#ifdef INT_SIZE_IS_4		
	/* 32-bit int */
 /* These two groups do something different, which is correct? */
#define USHORTMUL(a, b)	((tt_uint32)((tt_uint16)(a)*(tt_uint16)(b)))
#define SHORTMUL(a,b)	(tt_int32)((tt_int16)(a) * (tt_int16)(b))
#define SHORTDIV(a,b)	(tt_int32)((tt_int16)(a) / (tt_int16)(b))

#else				 

#define USHORTMUL(a, b)	((tt_uint32)((tt_uint32)(a)*(tt_uint32)(b)))
#define SHORTMUL(a,b)	(tt_int32)((tt_int32)(a) * (tt_int32)(b))
#define SHORTDIV(a,b)	(tt_int32)((tt_int32)(a) / (tt_int32)(b))

#endif


/* d is half of the denumerator */
#define FROUND( x, n, d, s ) \
	    x = SHORTMUL(x, n); x += d; x >>= s;

/* <3> */
#define SROUND( x, n, d, halfd ) \
    if ( x < 0 ) { \
	    x = -x; x = SHORTMUL(x, n); x += halfd; x /= d; x = -x; \
	} else { \
	    x = SHORTMUL(x, n); x += halfd; x /= d; \
	}

#endif
