/*
 * @(#)math_types.h	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


/* $Revision: 1.1 $ */
/* graphics:
	math types
	by Cary Clark, Georgiann Delaney, Herb Derby, Michael Fairman, Pablo Fernicola, Dave Good, Josh Horwich, Barton House, Robert Johnson, Keith McGreggor, Mike Reed, Oliver Steele, David Van Brink, Chris Yerga
	Copyright 1987 - 1994 Apple Computer, Inc.  All rights reserved.	*/

/*#pragma once*/

#include <sys/types.h>

#ifndef mathTypesIncludes
	#define mathTypesIncludes

	#ifndef __TYPES__
		#include <Types.h>
#if !defined(__java2d__)
		#ifdef appleInternal
			#undef nil
		#endif
#endif /* __pink__*/
	#endif

	#ifndef define_true_false_boolean
		#define define_true_false_boolean
		typedef unsigned char boolean;
	#endif

	#if !defined(__cplusplus) || defined(__java2d__)
		#define fixed Fixed
	#endif

	struct gxPoint {
		Fixed x;
		Fixed y;
	};

	#ifndef __cplusplus
		typedef struct gxPoint gxPoint;
	#endif

#ifdef _LP64
	typedef int fract;
#else
	typedef long fract;
#endif
	typedef unsigned short gxColorValue;
	
	struct gxPolar {
		Fixed radius;
		Fixed angle;
	};

#if !defined(__powerc) && !defined (ppcinterfaces)	
	struct wide {
		tt_int32 hi;
		tt_uint32 lo;
	};
	
 	typedef struct wide wide;
#endif
	
	struct gxMapping {
		Fixed map[3][3];
	};
	
	#ifndef __cplusplus
		typedef struct gxPolar gxPolar;
		typedef struct gxMapping gxMapping;
	#endif

#if !defined(__powerc) || !defined(__FIXMATH__)
	#define fixed1		((Fixed) 0x00010000)	/* fixed 1.0 */
	#define fract1		((fract) 0x40000000)	/* fract 1.0 */
#endif

	#define gxColorValue1		((gxColorValue) 0xFFFF)	/* gxColorValue 1.0 */
	#define gxPositiveInfinity		((Fixed) 0x7FFFFFFF)	/* for fixed and fract */
	#define gxNegativeInfinity	((Fixed) 0x80000000)	/* for fixed and fract */

#endif
