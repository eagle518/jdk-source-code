/*
 * @(#)kcmsos.h	1.21 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)kcmsos.h	1.90 99/04/16

Contains:       This file contains operating system dependent
			definitions.  Most of the stuff in here is for
			defining common data types.  There is a
			section for each operating system.

		If you add new "generic" data types to this file, you
			should typedef them to provide at least the data type,
			pointer to the data type and pointer to a constant of
			the data type.  For example, the following shows the
			naming conventions that we are using:

			typedef type KpXxxx_t, FAR* KpXxxx_p, const FAR* KpConstXxxx_p;


			Created by PGT, May 2, 1991


	Written by:     Kodak CMS Engineering

*/

/*********************************************************************
 *********************************************************************
 *    COPYRIGHT (c) 1991-2003 Eastman Kodak Company.
 *    As  an  unpublished  work pursuant to Title 17 of the United
 *    States Code.  All rights reserved.
 *********************************************************************
 *********************************************************************
 */


/*********************************************************************
 *********************************************************************
 ** integer sizes **
 ** KPINT16		defined if integers are 16 bit
 ** KPINT32		defined if integers are 32 bit

 ** platforms **
 ** KPDOS		defined if dos
 ** KPWIN		defined if windows (16 or 32 bit)
 ** KPWIN16		defined if 16 bit windows (3.1)
 ** KPWIN32		defined if 32 bit windows
 ** KPMAC		defined if Macintosh
 ** KPSUN		defined if Sun
 ** KPSGI		defined if SGI
 ** KPUNIX_BSD	defined if BSD derived version of Unix (i.e. SunOS)
 ** KPUNIX_SYS5	defined if ATT System V derived version of Unix (i.e. Solaris)
 ** KPUNIX		defined if any version of Unix

 ** byte order **
 ** KPLSBFISRT	defined if least significant byte first (Intel)
 ** KPMSBFISRT	defined if most significant byte first (Motorola, Sun)

 ** compiler **
 ** KPMSC		defined if Microsoft C/C++
 ** KPMSMAC		defined if Microsoft C/C++ cross development for Mac
 ** KPMSMAC68K		defined if Microsoft C/C++ cross development for 68K Mac
 ** KPMSMACPPC		defined if Microsoft C/C++ cross development for PowerMac
 ** KPBC		defined if Borland C/C++
 ** KPWATCOM	defined if Watcom C/C++
 ** KPMPW		defined if MPW
 ** KPMAC68K	defined if Macintosh 68K (defined for MSVC++ 68K cross development too)
 ** KPMACPPC	defined if Macintosh Power PC (defined for MSVC++ PPC cross development too)
 ** KPMW		defined if Metrowerks
 ** KPMPPC		defined if Power PC - Metrowerks (and MPW PPCC?)
 ** KPTHINK		defined if Think
 ** KPHIGHC		defined if High C
 ** KPCENTERLINE defined if Centerline C
 ** KPSUN_ANSI  defined if Sun ANSI C compiler
 ** KPSOLARIS	defined if Sun Solaris C compiler
 *********************************************************************
 *********************************************************************
 */


#ifndef KCMSOS_HEADER
#define KCMSOS_HEADER

#if defined(__cplusplus)
extern "C" {
#endif


/*
 *     *** Identify the compiler being used ***
 */
#if defined(_MSC_VER)
	#define KPMSC				_MSC_VER

	#if defined(_MAC)
		#define KPMSBFIRST	1
		#define	KPINT32		1
		#define	KPWIN32		1
		#define	KPMSMAC		1
	#elif defined(_M_ALPHA)	/* Alpha machines */
		#define KPINT32	1
		#define KPWIN32	1
		#define KPLSBFIRST	1
		#define KPALPHA		1
	#elif defined(_M_IX86) || defined(_WIN64)	/* Intel Platforms */
		#define KPINT32	1
		#define KPWIN32	1
		#define KPLSBFIRST	1
		#define KPINTEL	1
	#else				/* 16 bit compiler */
		#define KPINT16	1
		#if defined(_WINDOWS)
			#define KPWIN16	1
		#endif
		#define FLAT_FAR	_far
		#define FLAT_HUGE	_huge
		#define KPHUGE		__huge
		#define KPLSBFIRST			1
	#endif
	/* set proper Mac 68K or PPC */
	#if defined (_MPPC_)
		#define KPMSMACPPC	1
		#define KPMACPPC		1
	#elif defined (_68K_)
		#define KPMSMAC68K	1
		#define KPMAC68K		1
	#endif
#endif

#if defined(__BORLANDC__)
	#define KPBC				__BORLANDC__
	#define KPLSBFIRST			1

	#if defined(__WIN32__)
		#define KPINT32	1
		#define KPWIN32	1
	#else	/* 16 bit compiler */
		#define KPINT16	1
		#if defined (_Windows)
			#define KPWIN16	1
		#endif
		#define FLAT_FAR	_far
		#define FLAT_HUGE	_huge
		#define KPHUGE		huge
	#endif
#endif

#if defined(HIGH_C)
	#define KPHIGHC	1
	#define KPDOS	1

	#define KPLSBFIRST	1
#endif

#if defined(__386__)
	#define KPWATCOM	1

	#define KPLSBFIRST	1
	#define CNVRT_16PTR_TO_32PTR(x)	MK_FP32(x)
	#define FLAT_FAR				_far
	#define FLAT_HUGE				_far
#endif

#if defined (__MWERKS__) 			/* Metrowerks */	
	#define KPMW				1
	#define KPMAC				1

	#define KPINT32		1
	#define KPMSBFIRST	1
#if defined (TARGET_RT_MAC_MACHO)
	#define CHAR_P_DEFINED
#endif
#endif

#if defined(macintosh)
	#define KPMPW				1  /* MPW - 68K */
	#define	KPMAC				1

	#define KPINT32		1
	#define KPMSBFIRST	1
#endif

#if defined(THINK_C) || defined(SYMANTEC_C)
	#define KPTHINK				1
	#define	KPMAC				1

	#define KPMSBFIRST	1
#endif

#if defined (KPMAC)	
#if defined (powerc) || defined(__powerc)
	#define KPMACPPC			1  /* Power PC */
#else
	#define KPMAC68K			1  /* 68K */
#endif
#endif

#if defined (KPMW) || defined(KPMACPPC)
	#define KPMPPC				1  /* Power PC - Metrowerks */
#endif


#if defined(__sun) || defined (sun) /* Sun ANSI C Compiler */
	#define KPSUN_ANSI	1
	#define KPSOLARIS	1
	#define KPSUN		1
	#define KPUNIX_SYS5	1

	#define KPINT32		1
/* Turn on LSB software for x86 Solaris
 * builds of the code
 */
#include <sys/isa_defs.h>
#if defined (_LITTLE_ENDIAN)
        #define KPLSBFIRST      1
#else
        #define KPMSBFIRST      1
#endif

#if defined (SOLARIS_CMM)
	#define KP_USE_KBOOL
       	#define KP_POSIX_THREADS
#endif

#if !defined(SOLARIS_CMM) && !defined(JAVACMM)
	#define KP_USE_VERSIONSTRING
#endif

#endif

#if defined (linux)
	#define KPSUN_ANSI	1
	#define KPSUN		1
	#define KPLINUX		1
	#define KPUNIX_SYS5	1
	#define KP_POSIX_THREADS 1

	#define KPINT32		1
#if defined(i386) || defined(ia64) || defined(__amd64__)
	#define KPLSBFIRST	1
#else
	#define KPMSBFIRST	1
#endif
	#define KP_USE_VERSIONSTRING
#endif

#if defined(__sgi) && defined (__mips)  /* SGI C Compiler */
#if defined(SGIALL)
	#define KPSGIALL	1
#else
	#define KPSGI		1
#endif

    #define KPMIPS		1		/*	define this for now until all KPMIPS 
									occurances are changed to KPUNIX or
									KPSGI									*/
	#define KPUNIX_SYS5	1

	#define KPINT32		1
	#define KPMSBFIRST	1
#endif

#if defined(__CLCC__)  /* CenterLine C Compiler */
	#define KPCENTERLINE	1
	#define KPSUN		1
	#define KPUNIX_BSD	1

	#define KPINT32		1
	#define KPMSBFIRST	1
#endif

#if defined(__osf__)	/* CenterLine C Compiler - Digital Unix */
#define KPDU		1
#define KPUNIX_BSD	1

#define KPINT32		1
#define KPLSBFIRST	1
#endif

/* set generic Unix indicator */
#if defined (KPUNIX_BSD) || defined (KPUNIX_SYS5)
	#define KPUNIX	1
#endif

/* set generic Microsoft Windows indicator */
#if defined (KPWIN16) || defined (KPWIN32)
	#define KPWIN	1

/*	disable warnings generated by the Microsoft header files	*/
#pragma warning (disable: 4115)
#pragma warning (disable: 4201)
#pragma warning (disable: 4214) 

	#include <windows.h>
	#include <tchar.h>

#pragma warning (default: 4115)
#pragma warning (default: 4201)
#pragma warning (default: 4214) 
#endif

#if !defined (CALLBACK)
	#define CALLBACK
#endif

#if !defined (FAR)
	#define FAR
#endif

#if !defined (NEAR)
	#define NEAR
#endif

#if !defined (KPHUGE)
	#define KPHUGE
#endif

#if !defined (PASCAL)
	#define PASCAL
#endif


/*	The pointer data types are intended to be used whenever a complete
 *	32-bit address is needed.  This is particularly true when pointers
 *	are used with memory allocated using the standard CIPG allocation
 *	routines in memory.c.  For Microsoft Windows, the pointers are all
 *	FAR because of the way memory needs to be allocated. If using a
 *	32-bit compiler, this FAR and KPHUGE stuff is not needed. FAR and KPHUGE
 *	are not defined for 32-bit compilers.  In order to get a FAR pointer
 *	in the 32-bit flat model, we have the definition FLAT_FAR, which is
 *	always defined as FAR. The macro CNVRT_16PTR_TO_32PTR() converts
 *	16-bit FAR pointers to 32-bit FAR pointers. FLAT_HUGE is defined
 *	to be KPHUGE unless we are working in the 32-bit flat memory model for
 *	80386s.  Then it has the definition of FAR. It is used primarily for
 *	image data and this will need to be FAR in 32-bit flat model because
 *	it was allocated KPHUGE in Windows applications.
 */

#if !defined (CNVRT_16PTR_TO_32PTR)
	#define CNVRT_16PTR_TO_32PTR(x) (x)
#endif

#if !defined (FLAT_FAR)
	#define FLAT_FAR
#endif

#if !defined (FLAT_HUGE)
	#define FLAT_HUGE
#endif

/*
 *     *** Platform specific definitions ***
 */

/*
  Architecture dependent flags and types defined:

       KPMSBFIRST     Most Significant Byte First
               bit 0:  1 if bits-within-byte are MSB-first
               bit 1:  1 if bytes-within-word are MSB-first
               bit 2:  1 if words-within-longword are MSB-first
               bit 3:  1 if longwords-within-quadword are MSB-first
           Currently, only values of 0x0(DEC) or 0xF(mc68000) are supported.

       KP_ALIGN       Word Alignment
               bit 0:  1 if short words must be aligned on even addresses
               bit 1:  1 if long words must be aligned on double even addresses
               bit 2:  1 if quad words must be aligned on quad even addresses
           Currently, only values of 0x0(mc68000) or 0x3(sparc) are supported.

*/

#if defined(KPMSMAC)
	#define KP_ALIGN 0x0
	#define KcpFileDirSep	":"
#elif defined(KPWIN)
	#define KP_ALIGN 0x0
	#define KcpFileDirSep   "\\"
#elif defined(KPDOS)
	#define KP_ALIGN 0x0
	#define KcpFileDirSep   "\\"
#elif defined(KPMAC)
	#define KP_ALIGN 0x0
	#define KcpFileDirSep   ":"
#elif defined(KPSUN)
	#define KP_ALIGN 0x3
	#define KcpFileDirSep   "/"
#elif defined(KPSGI)
	#define KP_ALIGN 0x3
	#define KcpFileDirSep   "/"
#elif defined(KPSGIALL)
	#define KP_ALIGN 0x3
	#define KcpFileDirSep   "/"
#elif defined(KPDU)
	#define KP_ALIGN 0x3
	#define KcpFileDirSep   "/"
#endif

#if defined(KPMAC) || defined (KPMSMAC)
/*
 *     *** Macintosh Section ***
 */
#include <MacTypes.h>
#include <Components.h>

/* Obsolete conditional names commented out for CW6 6/20/95 - stanp
#ifndef USES68KINLINES
#if defined(KPMACPPC)
#define USES68KINLINES 0
#else
#define USES68KINLINES 1
#endif
#endif

#ifndef USESROUTINEDESCRIPTORS
#if defined(KPMACPPC)
#define USESROUTINEDESCRIPTORS 1
#else
#define USESROUTINEDESCRIPTORS 0
#endif
#endif
*/
/*
 *	BIG_ENUM_NUMBER is really needed by the MPW compiler to make sure all
 *	enums have a data size of int.  This is for compatibility with MacApp.
 */
#define BIG_ENUM_NUMBER         0xfffffff

/*
 *	The data type ioFileChar is used to hide file system dependencies.
 *	For most OS, this is a meaningless data type.  However, for the
 *	Macintosh, there are some special parameters which need to be
 *	passed to give extra non-portable information about a file.
 *  NOTE: ioFileChar is obsolete.  Any new implementations 
 *        should use 
 */
typedef struct {
	char    fileType[5];         /* type of file */
	char    creatorType[5];      /* creator of file */
	short   vRefNum;             /* vol. ref. num. for file dir. */
} ioFileChar;

/*
 *	The data type KpFileProps_t is used to hide file system dependencies.
 *	For most OS, this is a meaningless data type.  However, for the
 *	Macintosh, there are some special parameters which need to be
 *	passed to give extra non-portable information about a file.
 */
typedef struct {
	char    fileType[5];         /* type of file */
	char    creatorType[5];      /* creator of file */
	short   vRefNum;             /* vol. ref. num. for file dir. */
	long    dirID;              /* directory Id  for file dir. */
} KpFileProps_t;

typedef Handle          KpHandle_t;

/* The data type KpTChar_t is used to support MicroSoft Windows
   unicode character format. This is just defined here for compatibility. */
typedef char	KpTChar_t;

/* typedefs used to support Microsoft Windows registry database.  This should
be modified later to support the future Macintosh os */
typedef int	KpMainHive_t;
#define	KPLOCAL_MACHINE 0
#define	KPCURRENT_USER 0
#define	KPLOCAL_USERS 0

/*
 *     *** Microsoft Windows 32 Section ***
 */
#elif defined(KPWIN32)

#define WIN_NULL        0

typedef void FAR *KpHandle_t;

/* The data type KpTChar_t is used to support unicode. If _UNICODE 
   is defined, then the KpTChar_t becomes a 16 bit wide character and
   if _UNICODE is not defined, then the KpTChar_t is just a standard
   8 bit wide character. */
typedef _TCHAR	KpTChar_t;

/* defines for the registry hive database */
typedef HKEY				KpMainHive_t;
#define KPLOCAL_MACHINE		HKEY_LOCAL_MACHINE
#define KPCURRENT_USER		HKEY_CURRENT_USER

/*
 *	The data type ioFileChar is used to hide file system dependencies.
 *	For most OS, this is a meaningless data type.  However, for the
 *	Macintosh, there are some special parameters which need to be
 *	passed to give extra non-portable information about a file.
 *  NOTE: ioFileChar is obsolete.  Any new implementations 
 *        should use 
 */
typedef OFSTRUCT  ioFileChar;

/*
 *	The data type KpFileProps_t is used to hide file system dependencies.
 *	For most OS, this is a meaningless data type.  However, for the
 *	Macintosh, there are some special parameters which need to be
 *	passed to give extra non-portable information about a file.
 */
typedef OFSTRUCT  KpFileProps_t;

/*
 *	BIG_ENUM_NUMBER is really needed by the MPW compiler to make sure all
 *	enums have a data size of int.  This is for compatibility with MacApp.
 *	This definition is to get a value of proper size for this environment.
 */
#define BIG_ENUM_NUMBER         0x7fff

/*
 *	Define a variable to indicate we are building for an ICM.
 *	This variable is only needed for 32 bit code because that
 *	is all an ICM can be.
 */
#if defined(ICM_BASE) || defined(ICM_PRO)
	#define ICM	1
#endif					/* ICM_BASE or ICM_PRO */


/*
 *	If we are compiling for the MS ICM, redefine certain
 *	library function names so that we can use the kernel32
 *	library to link to instead of the C run time library.
 */
#if defined(ICM)

	#define strcat(x,y)	lstrcat((x),(y))
	#define strcmp(x,y)	lstrcmp((x),(y))
	#define strcpy(d,s)	lstrcpy((d),(s))
	#define strlen(s)	lstrlen((s))

	#if defined(isdigit)
		#undef isdigit
	#endif

	#if defined(isspace)
		#undef isspace
	#endif

	#define isdigit(c)	(((c) >= '0') && ((c) <= '9'))
	#define isspace(c)	(((c) =='\t') \
						|| ((c) == '\r') \
						|| ((c) == '\n') \
						|| ((c) == ' '))

		/* not quite right, but ok for ICM (?) */

#endif					/* ICM */

#include <stdlib.h>


/*
 *     *** MS-DOS/Microsoft Windows Section ***
 */
#elif defined(KPWIN16) || defined(KPDOS)

/*
 * Get a definition of APIENTRY so we can write stuff
 * for Win32 and yet be compatible with Win16
 */
#if !defined(APIENTRY)
	#define APIENTRY WINAPI
#endif

#if defined(KPDOS)   /* DOS!!! */
	typedef struct tagOFSTRUCT {  /*  to get ioFile.c to compile easily */
		int dvalue;
		int hvalue;
	} OFSTRUCT;

	typedef OFSTRUCT FAR *LPOFSTRUCT;
#endif


/*
 * Microsoft Windows definition of NULL is not ANSI compatible.
 */
#define WIN_NULL        0

typedef void FAR *KpHandle_t;

/*
 *	The data type ioFileChar is used to hide file system dependencies.
 *	For most OS, this is a meaningless data type.  However, for the
 *	Macintosh, there are some special parameters which need to be
 *	passed to give extra non-portable information about a file.
 *  NOTE: ioFileChar is obsolete.  Any new implementations 
 *        should use KpFileProps_t.
 */
typedef struct tagOFSTRUCT  ioFileChar;

/*
 *	The data type KpFileProps_t is used to hide file system dependencies.
 *	For most OS, this is a meaningless data type.  However, for the
 *	Macintosh, there are some special parameters which need to be
 *	passed to give extra non-portable information about a file.
 */
typedef struct tagOFSTRUCT  KpFileProps_t;
/*
 *   BIG_ENUM_NUMBER is really needed by the MPW compiler
 *   to make sure all enums have a data size of int.  This
 *   is for compatibility with MacApp.  This definition is
 *   to get a value of proper size for this environment.
 */
#define BIG_ENUM_NUMBER         0x7fff

#include <stdlib.h>



#elif defined(KPSUN) || defined(KPDU)
/*
 *     *** Sun Microsystems Section ***
 */


#include <string.h>

#if defined (KPSOLARIS)
#include <synch.h>
#endif

#if defined(KPDU)
#ifndef NULL
#ifdef __cplusplus
#define NULL	0
#else
#define NULL	((void *)0)
#endif
#endif
#endif
/*
 *	BIG_ENUM_NUMBER is really needed by the MPW compiler
 *	to make sure all enums have a data size of int.  This
 *	is for compatibility with MacApp.  This definition is
 *	to get a value of proper size for this environment.
 */
#define BIG_ENUM_NUMBER         0xfffffff

/*
 *	The data type ioFileChar is used to hide file system dependencies.
 *	For most OS, this is a meaningless data type.  However, for the
 *	Macintosh, there are some special parameters which need to be
 *	passed to give extra non-portable information about a file.
 *  NOTE: ioFileChar is obsolete.  Any new implementations 
 *        should use 
 */
typedef long    ioFileChar;

/*
 *	The data type KpFileProps_t is used to hide file system dependencies.
 *	For most OS, this is a meaningless data type.  However, for the
 *	Macintosh, there are some special parameters which need to be
 *	passed to give extra non-portable information about a file.
 */
typedef long    KpFileProps_t;

typedef void    *KpHandle_t;

/* The data type KpTChar_t is used to support MicroSoft Windows
   unicode character format. This is just defined here for compatibility. */
typedef char	KpTChar_t;
 
#elif defined(KPSGI) || defined(KPSGIALL)
/*
 *     *** SGI/MIPS Section ***
 */

#include <string.h>
#if defined(KPSGI)
#include <sgidefs.h>
#endif

/*
 *	BIG_ENUM_NUMBER is really needed by the MPW compiler
 *	to make sure all enums have a data size of int.  This
 *	is for compatibility with MacApp.  This definition is
 *	to get a value of proper size for this environment.
 */
#define BIG_ENUM_NUMBER         0xfffffff

/*
 *	The data type ioFileChar is used to hide file system dependencies.
 *	For most OS, this is a meaningless data type.  However, for the
 *	Macintosh, there are some special parameters which need to be
 *	passed to give extra non-portable information about a file.
 *  NOTE: ioFileChar is obsolete.  Any new implementations 
 *        should use 
 */
typedef long    ioFileChar;

/*
 *	The data type KpFileProps_t is used to hide file system dependencies.
 *	For most OS, this is a meaningless data type.  However, for the
 *	Macintosh, there are some special parameters which need to be
 *	passed to give extra non-portable information about a file.
 */
typedef long    KpFileProps_t;

typedef void    *KpHandle_t;

/* The data type KpTChar_t is used to support MicroSoft Windows
   unicode character format. This is just defined here for compatibility. */
typedef char	KpTChar_t;

#else
/*****
#error *** OS Not Defined!! *** and in SunOS *** #error Not Defined!! ***
*****/
#endif                  /* OS Sections */

typedef void	FAR *KpGenericPtr_t;			/* KcmGenericPtr */
typedef void	KPHUGE *KpLargeBuffer_t;		/* largeBuffer */
typedef float	KpFloat32_t;					/* float32 */
typedef double	KpDouble64_t;					/* double64 */

typedef char   			KpChar_t;			/*  */
typedef unsigned char   KpUInt8_t;			/* u_int8 */
typedef unsigned short  KpUInt16_t;			/* u_int16 */
#if defined(KPSGI)
    typedef __uint32_t	KpUInt32_t;	
    typedef __int32_t   KpInt32_t;			/* int32 */
#else
#if defined (_LP64)
    typedef unsigned int    KpUInt32_t;
    typedef int             KpInt32_t;			/* int32 */
#else
    typedef unsigned long   KpUInt32_t;			/* u_int32 */
    typedef long            KpInt32_t;			/* int32 */
#endif
#endif /* KPSGI */
typedef char            KpInt8_t;			/* int8 */
typedef short           KpInt16_t;			/* int16 */
typedef short           KpBool_t;           /* int16, boolean */

/* intended initially to follow the 'natural' size of a
 * number, ultimately to be an ANSI bool.
 */
typedef KpUInt32_t	Kbool_t;

/* Now for the defines for the imported MD5 code */
/* These defines and declarations are from global.h where
 * MD5 code files are found */
#define UINT4 KpUInt32_t
#define UINT2 KpUInt16_t

/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;

#ifndef PROTOTYPES
#define PROTOTYPES 0
#endif

/* PROTO_LIST is defined depending on how PROTOTYPES is defined above.
If using PROTOTYPES, then PROTO_LIST returns the list, otherwise it
  returns an empty list.
 */
#if PROTOTYPES
#define PROTO_LIST(list) list
#else
#define PROTO_LIST(list) ()
#endif

/* End of MD5 global definitions */

#define KPTRUE	1
#define KPFALSE	0


/*	Special integer types for use where a 'short' would provide the
 *	needed range of values.  This type is defined a 'int' because that
 *	will generate small fast code on both 32 bit and 16 bit machines.
 *	We have a special type to indicate our intent for the range of
 *	values.  Also one could redefine this as 'short' for a 32 bit
 *	implementation to test that it would work in a 16 bit implementation.
 */

typedef int     KpIndex_t;				/* kcpindex_t */


/*
 *      The data type KcmPixelBuffer is used to pass pixel data
 *      to the API and Color Processor.  Its defined here due to
 *      file system dependencies.
 */
typedef char KPHUGE		*KpPixelBuffer_t;	/* KcmPixelBuffer */

/*
 * Pointer Data Types for the most complete specification of a pointer.
 * These are mostly for PCs where a special definition is needed for some
 * pointers to be sure they can address any data on the machine.  Thus,
 * FAR translates to FAR on PCs and to nothing on anything else.
 */
typedef KpInt32_t
				FAR* KpInt32_long_ptr,				/* int32_long_ptr */
				FAR* KpInt32_p,						/* int32_p */
				FAR* FAR* KpInt32_h;				/* int32_h */
typedef const KpInt32_t FAR* KpConstInt32_p;		/* pointer to const int32 */

typedef KpInt16_t
				FAR* KpInt16_long_ptr,				/* int16_long_ptr */
				FAR* KpInt16_p,						/* int16_p */
				FAR* FAR* KpInt16_h;				/* int16_h */
typedef const KpInt16_t FAR* KpConstInt16_p;		/* pointer to const int16 */

typedef KpInt8_t
				FAR* KpInt8_long_ptr,				/* int8_long_ptr */
				FAR* KpInt8_p,						/* int8_p */
				FAR* FAR* KpInt8_h;					/* int8_h */
typedef const KpInt8_t FAR* KpConstInt8_p;			/* pointer to const int8 */


typedef char
				FAR* KpChar_long_ptr,				/* char_long_ptr */
				FAR* KpChar_p,						/* char_p */
				FAR* FAR* KpChar_h;					/* char_h */
typedef const char FAR* KpConstChar_p;				/* pointer to const char */

typedef KpTChar_t
				FAR* KpTChar_long_ptr,				/* TChar (ASCII/UNICODE) */
				FAR* KpTChar_p,						/* pointer to TChar */
				FAR* FAR* KpTChar_h;				/* "handle" to TChar */
typedef const KpTChar_t FAR* KpConstTChar_p;		/* pointer to const TChar */

typedef KpUInt32_t
				FAR* KpUInt32_long_ptr,				/* u_int32_long_ptr */
				FAR* KpUInt32_p,					/* u_int32_p */
				FAR* FAR* KpUInt32_h;				/* u_int32_h */
typedef const KpUInt32_t FAR* KpConstUInt32_p;		/* pointer to const UInt32 */

typedef KpUInt16_t
				FAR* KpUInt16_long_ptr,				/* u_int16_long_ptr */
				FAR* KpUInt16_p,					/* u_int16_p */
				FAR* FAR* KpUInt16_h;				/* u_int16_h */
typedef const KpUInt16_t FAR* KpConstUInt16_p;		/* pointer to const UInt16 */

typedef KpUInt8_t
				FAR* KpUInt8_long_ptr,				/* u_int8_long_ptr */
				FAR* KpUInt8_p,						/* u_int8_p */
				FAR* FAR* KpUInt8_h;				/* u_int8_h */
typedef const KpUInt8_t FAR* KpConstUInt8_p;		/* pointer to const UInt8 */


typedef KpFloat32_t
				FAR* KpFloat32_long_ptr,			/* float32_long_ptr */
				FAR* KpFloat32_p,					/* float32_p */
				FAR* FAR* KpFloat32_h;				/* float32_h */
typedef const KpFloat32_t FAR* KpConstFloat32_p;	/* pointer to const Float32 */

typedef KpDouble64_t
				FAR* KpDouble64_long_ptr,			/* double64_long_ptr */
				FAR* KpDouble64_p,					/* double64_p */
				FAR* FAR* KpDouble64_h;				/* double64_h */
typedef const KpDouble64_t FAR* KpConstDouble64_p;	/* pointer to const double64 */

typedef ioFileChar
		  		FAR* ioFileChar_p,
				FAR* FAR* ioFileChar_h;
typedef const ioFileChar FAR* ioConstFileChar_p;	/* pointer to const ioFileChar */

typedef KpFileProps_t
		  		FAR* KpFileProps_p,
				FAR* FAR* KpFileProps_h;
typedef const KpFileProps_t FAR* KpConstFileProps_p;	/* pointer to const KpFileProps_t */

typedef KpBool_t
				FAR* KpBool_p,					/* Boolean pointer */
				FAR* FAR* KpBool_h;				/* Boolean handle */
typedef const KpBool_t FAR* KpConstBool_p;		/* pointer to const Boolean */


/* signed 15.16 */
typedef KpInt32_t
				KpF15d16_t,						/* Fixed_t */
				FAR* KpF15d16_p,				/* Fixed_p */
				FAR* FAR* KpF15d16_h;			/* Fixed_h */
typedef const KpInt32_t FAR* KpConstF15d16_p;	/* pointer to const F15d16 */
#define KpF15d16Scale	65536.0
#define KpF15d16FromDouble(d) ((KpF15d16_t) ((d) * KpF15d16Scale))
#define KpF15d16ToDouble(d) (((double) (d)) / KpF15d16Scale)

/* unsigned signed 1.15 */
typedef KpInt16_t	KpF1d15_t;
#define KpF1d15Scale	32768.0
#define KpF1d15FromDouble(d) ((KpF1d15_t) ((d) * KpF1d15Scale))
#define KpF1d15ToDouble(d) (((double) (d)) / KpF1d15Scale)

/* unsigned signed 8.8 */
typedef KpInt16_t	KpF8d8_t;
#define KpF8d8Scale	256.0
#define KpF8d8FromDouble(d) ((KpF8d8_t) ((d) * KpF8d8Scale))
#define KpF8d8ToDouble(d) (((double) (d)) / KpF8d8Scale)


/* XYZ color definition */
typedef struct {
	KpF15d16_t	X;
	KpF15d16_t	Y;
	KpF15d16_t	Z;
} KpF15d16XYZ_t;

typedef struct {
	KpF1d15_t	X;
	KpF1d15_t	Y;
	KpF1d15_t	Z;
} KpF1d15XYZ_t;

typedef struct {
	KpUInt32_t	Count;
	KpUInt16_t	FAR *Data;
} KpResponse_t;


/*
 *  The KPARGS macro is for defining function prototypes for ANSI C
 *	and just type declarations otherwise.
 *
 *	This has been changed to always produce the prototype!
 */
#define KPARGS(s)         s


#if !defined(KPSTRICT)
	typedef KpHandle_t		KcmHandle;
	typedef KpGenericPtr_t	KcmGenericPtr;
	typedef KpLargeBuffer_t	largeBuffer;
	typedef KpFloat32_t		float32;
	typedef KpDouble64_t	double64;

	/*
	 *	because the fut library in fut_arch.h uses #defines instead
	 *	of typedefs to define u_int8, int16 and int32, we are using
	 *	#defines here as using typedefs and later including KCMPTlib.h
	 *	causes problems with MPW C.
	 */

	#if !defined(U_INT8_DEFINED) && !defined(u_int8)
		typedef unsigned char   u_int8;
		#define U_INT8_DEFINED 1
	#endif

	#if !defined(U_INT16_DEFINED) && !defined(u_int16)
		typedef unsigned short  u_int16;
		#define U_INT16_DEFINED
	#endif

	#if !defined(U_INT32_DEFINED) && !defined(u_int32)
	#if defined (_LP64)
		typedef unsigned int    u_int32;
	#else
		typedef unsigned long   u_int32;
	#endif
		#define U_INT32_DEFINED
	#endif

	#if !defined(INT8_DEFINED) && !defined(int8)
		typedef char            int8;
		#define INT8_DEFINED
	#endif

	#if !defined(INT16_DEFINED) && !defined(int16)
		typedef short           int16;
		#define INT16_DEFINED
	#endif

	#if !defined(INT32_DEFINED) && !defined(int32)
		#if defined(KPSGI)
		    typedef __int32_t int32;
		    typedef __uint32_t uint32;
		#else
		#if defined (_LP64)
		    typedef int			int32;
		    typedef unsigned int	uint32;
		#else
		    typedef long		int32;
		    typedef unsigned long	uint32;
		#endif
		#endif /* KPSGI */
		#define INT32_DEFINED
	#endif


	/*	Special integer types for use where a 'short' would provide the
	 *	needed range of values.  This type is defined a 'int' because that
	 *	will generate small fast code on both 32 bit and 16 bit machines.
	 *	We have a special type to indicate our intent for the range of
	 *	values.  Also one could redefine this as 'short' for a 32 bit
	 *	implementation to test that it would work in a 16 bit implementation.
	 */

	typedef int     kcpindex_t;


	/*
	 *      The data type KcmPixelBuffer is used to pass pixel data
	 *      to the API and Color Processor.  Its defined here due to
	 *      file system dependencies.
	 */
	typedef char KPHUGE		*KcmPixelBuffer;

	/*
	 * Pointer Data Types for the most complete specification of a pointer.
	 * These are mostly for PCs where a special definition is needed for some
	 * pointers to be sure they can address any data on the machine.  Thus,
	 * FAR translates to FAR on PCs and to nothing on anything else.
	 */
	typedef int32   FAR* int32_long_ptr, FAR* int32_p, FAR* FAR* int32_h;
	typedef int16   FAR* int16_long_ptr, FAR* int16_p, FAR* FAR* int16_h;
	typedef int8    FAR* int8_long_ptr, FAR* int8_p, FAR* FAR* int8_h;
#if !defined(CHAR_P_DEFINED) && !defined(char_p)
	typedef char    FAR* char_long_ptr, FAR* char_p, FAR* FAR* char_h;
#endif
	typedef u_int32 FAR* u_int32_long_ptr, FAR* u_int32_p, FAR* FAR* u_int32_h;
	typedef u_int16 FAR* u_int16_long_ptr, FAR* u_int16_p, FAR* FAR* u_int16_h;
	typedef u_int8  FAR* u_int8_long_ptr, FAR* u_int8_p, FAR* FAR* u_int8_h;
	typedef float32 FAR* float32_long_ptr, FAR* float32_p, FAR* FAR* float32_h;
	typedef double	FAR* double_p, FAR* FAR* double_h;

	typedef KpF15d16_t	Fixed_t, FAR* Fixed_p, FAR* FAR* Fixed_h;	/* signed 15.16 */


	/*
	 *  The ARGS macro is for defining function prototypes for ANSI C
	 *  and just type declarations otherwise.
	 *
	 *	This has been changed to always produce the prototype.
	 */
#ifndef ARGS
	#define ARGS(s)         s
#endif
#endif

/* Thread Memory Storage Types */

struct KpThreadMemHdl_tag { char dontuse; };
typedef struct KpThreadMemHdl_tag FAR* KpThreadMemHdl_t; 

#if defined (KPSOLARIS) || defined (KPLINUX)
#if defined (KP_POSIX_THREADS)
#include "pthread.h"
#else
#include "thread.h"
#endif
#endif 

/* Thread specific storage needs critical section control */
typedef struct {
	KpInt32_t	SyncFlag;
#if defined (KPWIN32)
	CRITICAL_SECTION CriticalFlag;
#elif defined (KPSOLARIS) || defined (KPLINUX)
	KpInt32_t	ThreadId;
	KpUInt32_t	Count;
#if defined (KP_POSIX_THREADS)
	pthread_mutex_t CriticalFlag;
#else
	mutex_t CriticalFlag;
#endif
#else
	KpInt32_t CriticalFlag;
#endif
} KpCriticalFlag_t;

/* Definition for Generic Rectangle Coordinates to be Platform Independent */

typedef struct	KpRect_tag {
							KpInt32_t	left;	/* left top x coord */
							KpInt32_t	top;	/* left top y coord */
							KpInt32_t	right;	/* bottom right x coord */
							KpInt32_t	bottom;	/* bottom right y coord */
						   } KpRect_t, FAR *KpRect_p;

typedef KpHandle_t KpRect_h;

/* data structures needed for SpInitCMS */
#if defined(KPWIN)
typedef KpHandle_t (*KpAllocBufferHandleProc_t)(KpInt32_t);
typedef KpGenericPtr_t (*KpLockBufferProc_t)(KpHandle_t);
typedef KpInt32_t (*KpUnlockBufferProc_t)(KpHandle_t);
typedef KpHandle_t (*KpGetHandleFromPtrProc_t)(KpGenericPtr_t);
typedef KpInt32_t (*KpGetBufferSizeProc_t)(KpHandle_t);
typedef KpGenericPtr_t (*KpReAllocBufferPtrProc_t)(KpGenericPtr_t, KpInt32_t);
typedef void (*KpFreeBufferProc_t)(KpHandle_t);

typedef struct
{
	KpUInt32_t structSize;			/* size of this structure according to user */
	KpAllocBufferHandleProc_t KpAllocBufferHandle;
	KpLockBufferProc_t KpLockBuffer;
	KpUnlockBufferProc_t KpUnlockBuffer;
	KpGetHandleFromPtrProc_t KpGetHandleFromPtr;
	KpGetBufferSizeProc_t KpGetBufferSize;
	KpReAllocBufferPtrProc_t KpReAllocBufferPtr;
	KpFreeBufferProc_t KpFreeBuffer;
}KpMemoryData_t;

#else

typedef struct
{
	KpUInt32_t structSize;			/* size of this structure according to user */
	KpHandle_t	(*KpAllocBufferHandle)(KpInt32_t);
	KpGenericPtr_t	(*KpLockBuffer)(KpHandle_t);
	KpInt32_t	(*KpUnlockBuffer)(KpHandle_t);
	KpHandle_t	(*KpGetHandleFromPtr)(KpGenericPtr_t);
	KpInt32_t	(*KpGetBufferSize)(KpHandle_t);
	KpGenericPtr_t	(*KpReAllocBufferPtr)(KpGenericPtr_t, KpInt32_t);
	void	(*KpFreeBuffer)(KpHandle_t);
} KpMemoryData_t;
#endif

#if defined(__cplusplus)
}
#endif

 /* misc macros */
 
#ifndef MIN
#define MIN(a,b) ((a < b) ? a : b)
#endif

#ifndef MAX
#define MAX(a,b) ((a > b) ? a : b)
#endif

#endif                  /* KCMSOS_HEADER */

     
