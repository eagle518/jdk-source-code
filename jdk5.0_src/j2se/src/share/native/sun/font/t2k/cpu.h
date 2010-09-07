/*
 * @(#)cpu.h	1.19 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "dtypes.h"

/* $Revision: 1.1 $ */
/* skia:
	The subsidiary defines for the various CPUs that skia supports
	by Cary Clark, Georgiann Delaney, Michael Fairman, Dave Good, Robert Johnson, Keith McGreggor, Oliver Steele, David Van Brink, Chris Yerga
	Copyright 1987 - 1991 Apple Computer, Inc.  All rights reserved.	*/

/*
	This file contains macros that conditionalize various aspects of skia for different environments.
	CPUs supported by this file are
	*** These should be renamed from cpuXXX to xxxPlatform
		cpuReference	the reference version.  Doesn't necessarily run on any actual hardware
		cpuMacClassic	any Macintosh with an 68000 or higher, and with Classic Quickdraw:
					upgraded 512KE, MacPlus, Classic Mac, Portable, 100
		cpuMacII		any Macintosh with a 68020 or higher, and with Color Quickdraw:
					Mac II, IIx, IIc, IIci, IIcx, IIfx, IIlc, IIsi, 140, 170, 700, 900, Classic II
		cpu29000		the GC 8¥24 card
		cpuNewton	the ARM on a Newton
		cpuIntel		the Intel ³ 80386
		cpuPrinter	PostScript eexec downloadable patch
		cpuPowerPC	the 601 PowerPC processor

	The cpu that the build is for should be #defined in project prefix, or from the command gxLine,
	by (for example) "-d cpuMacClassic -d buildOptionsIncludes".

	Each of these CPUs will enable various switches; the switches are:
		// software environment
			cpuMacintosh				the target is a Macintosh
			classicQuickdraw			is classic quickdraw resident? also true for the GC
			colorQuickdraw				is gxColor quickdraw resident?  also true for the GC
			pascalAware				the compiler supports pascal calling convention specifier "pascal" in prototypes
			externalWindows			the host should be consulted about the position and clip of ports in windows
			remoteServer				the client and server are in separate address spaces
			segmented					do extra checks to avoid calling functions in other files
			unix						the target is unix
			callOutSemaphores			call routines to lock and unlock the system gxFont cache

		// software environment macros
			CleanAddress				a macro to turn make a pointer 32-bit clean
			minimumStackSpace		leave this much room on the stack (used by NewTempPtr)
			local						this is the equivalent of static for machines that donÕt have a segment loader

		// cpu
			branchIsSlow				in fast code, avoid branches where possible
			multiplyIsSlow				in fast code, use adds and shifts if the multiplicand is less than or equal to this
			shiftIsSlow				in fast code, use adds in fast code if shiftand than or equal to this
			unrolled					in fast code, unroll loops by this much
			USEHALFWORDS			shorts are fast enough to be usable for text caches
			_LITTLE_ENDIAN			define if low byte of word or long is byte #0 when byte indexed

		// cpu and compiler
			asm68000				the target and compiler support inline 68000 assembly code
			asm68020				the target and compiler support inline 68020 assembly code
*/

/*#pragma once*/

#define cpuReference 1
#ifndef cpuIncludes
	#define cpuIncludes

	#if !defined(cpuReference) && !defined(cpuMacClassic) && !defined(cpuMacII) && !defined(cpu29000) && !defined(cpuNewton) && !defined(cpuIntel) && !defined(cpuPrinter) && !defined(cpuPowerPC)
		#error "One of the supported cpuXXX options must be defined in the project prefix."
	#endif

	#if !defined(cpuReference) && !defined(appleInternal)
		#error "You must define appleInternal in the project prefix in order to use the private version of the public interface files."
	#endif


	#ifdef cpuMacClassic
		#define cpuMacintosh
	#endif


	#ifdef cpuMacII
		#define cpuMacintosh
		#define colorQuickdraw

		#define CleanAddress(x)	xCleanAddress(x)
		void *xCleanAddress(void *address);

		#ifdef THINK_C
			#define asm68020
		#endif
	#endif


	#ifdef cpuPowerPC
		#define cpuMacintosh
		#define colorQuickdraw

		#define CleanAddress(x)	xCleanAddress(x)
		void *xCleanAddress(void *address);
	#endif


	#ifdef cpuMacintosh
		#define classicQuickdraw
		#define pascalAware
		#define externalWindows
		#define segmented

		#define minimumStackSpace	2048
		#define local

		#ifndef __powerc
			#define branchIsSlow
		#endif
		#define multiplyIsSlow	5
		#define shiftIsSlow		2
		#define unrolled		256
		#define USEHALFWORDS

		#ifdef THINK_C
			#define asm68000
		#endif
		
		#define FOND		/* to support the FOND gxFont type */
	#endif

	
	#ifdef cpu29000
		#define classicQuickdraw
		#define colorQuickdraw
		#define remoteServer
	#endif


	#ifdef cpuNewton
		/*** donÕt you guys have any environment conditionals? */
	#endif

	#ifdef cpuIntel
		#define multiplyIsSlow	5
		#define littleEndian
		#define USEHALFWORDS
	#endif

	#ifdef cpuPrinter
		/*** the printer does not have any conditionals ***/
	#endif
		
	/* these environment macros must default to something */
	#ifndef minimumStackSpace
		#define minimumStackSpace 2048
	#endif
	#ifndef local
		#define local static
	#endif
	#ifndef CleanAddress
		#define CleanAddress(x) (x)
	#endif

	/* portable types */
	#ifdef applec
		#define fastInt short
	#endif
	#ifndef fastInt
		/* Could just use int, which is supposed to be the fastest type, but MPW mis-defines it. */
		#define fastInt int
	#endif
	typedef signed char schar;

	#if !defined(THINK_C) && !defined(__MWERKS__)
		/* used in #ifs */
		#define __option(d) 0
	#endif

	/* Macros to do byte swapping when used on littleEndian machines and do NOTHING on others */
#ifdef _LITTLE_ENDIAN
#ifdef __cplusplus
extern "C" 
{
#endif
		extern unsigned short swapWord (unsigned short);
		extern unsigned short swapWordPtr (unsigned short**);
		extern tt_uint32 swapLong (tt_uint32);
#ifdef __cplusplus
}
#endif

		#define SWAPW(a)       swapWord(a)
		#define SWAPL(a)       swapLong(a)
		#define SWAPWINC(a)    swapWordPtr((unsigned short**)&a)
		#define SWAPLINC(a)		swapLong(*(a)++)
		#define DELTASWAPPED(a,d)		(((a) = SWAP_ (a) d), ((a) = SWAP_ (a)))
		
		#define SWAP_(a)	\
			(sizeof(a) == 1 ? (a) : \
			 sizeof(a) == 2 ? SWAPW(a) : \
			 sizeof(a) == 4 ? SWAPL(a) : \
			 (a))
	#else
		#define SWAPW(a)			(a)
		#define SWAPWINC(a)		(*(a)++)
		#define SWAPLINC(a)		(*(a)++)
		#define SWAPL(a)			(a)
		#define DELTASWAPPED(a,d)	((a) = (a) d)
		#define SWAP_(a)			(a)
	#endif

	#if THINK_C >= 5
		#ifdef asm68020
			#pragma options(mc68020)
		#endif
		#ifndef linkedIn
			#if __option(mc68881)
				#error "Please turn off 68881 code generation"
				/* If 68881 code generation is on, Think C generates setjmp/longjmp instructions
				  * that save/restore the FPU registers, taking more space for the jmp_buf, more
				  * code space for the instructions, and more time.  We don't actually use the FPU
				  * registers except in certain math routines, which are never jmped into or out of,
				  * so 68881 should be off when setjmp.h is included in order to get the short forms
				  * of the routines.  We could simply turn it off here, but then we'd get bitten if
				  * somebody had 68881 on for the project and inadvertantly included setjmp.h before
				  * cpu.h, or didn't include cpu.h at all.
				  *
				  * Allow people to run with the extra baggage in the linkedIn version, since nobody
				  * else is seeing that object code anyway and it may be useful to get native FPU
				  * instructions in the non-core part of the project.
				  */
			#endif
			#ifndef mutatorApp
				#if __option(separate_strs)
					#error "Please turn off Separate Strings"
					/* init.a doesn't know how to load Think C's 'STRS' resource; moreover, separate
					  * strings shouldn't be necessary for the core only.
					  */
				#endif
			#endif
		#endif
	#endif

#endif

