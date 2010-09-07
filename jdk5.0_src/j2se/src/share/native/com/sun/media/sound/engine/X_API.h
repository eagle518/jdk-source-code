/*
 * @(#)X_API.h	1.45 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**	X_API.h
**
**	This provides for platform specfic functions that need to be rewitten,
**	but will provide a API that is stable across all platforms
**
**	Confidential-- Internal use only
**
** Overview
**	Machine dependent code and equates
**
**	History	-
**	9/25/95		Created
**	10/15/95	Added XFiles
**	10/20/95	Added XMemory support
**	2/11/96		Added X_WEBTV system
**				Added XSound Manager stuff
**	2/17/96		Added more platform compile arounds
**	2/21/96		Changed XGetAndDetachResource to return a size
**	3/29/96		Added XPutLong & XPutShort
**	6/28/96		Added BeBox stuff
**	6/30/96		Changed font and re tabbed
**				Added XSetHardwareSampleRate and XGetHardwareSampleRate
**				Moved some types here ABS, DEBUG_STR
**				Added BeBox stuff and we are now dependent upon X_API.h to set
**				the machine type only
**	7/1/96		Added XCreateAccessCache
**				Added caching system to XFILERESOURCE
**	7/2/96		Merged Machine.h into X_API.h
**	7/3/96		Added XStrCmp
**	7/7/96		Added XStrnCmp
**	7/23/96		Added XMicroseconds
**	8/6/96		Added XGetIndexedType & XCountTypes
**	8/19/96		Added XGetCompressionName, XExpandMacADPCMto16BitLinear,
**				XExpandULawto16BitLinear
**	9/22/96		Added XRandom & XSeedRandom & XRandomRange
**				Changed XExpandMacADPCMto16BitLinear to XExpandMacADPCMtoXBitLinear
**				to support 8 bit expansion
**	10/8/96		Added XStrCpy
**	10/9/96		Removed PASCAL usage
**	10/11/96	Added XConvertNativeFileToXFILENAME
**				Added XFileSetPositionRelative
**				Added XDetermineByteOrder
**				Added XSwapLong & XSwapShort
**	10/15/96	Added XFileOpenResourceFromMemory
**	11/14/96	Added XGetResourceName & XGetNamedResource
**				Added XWaitMicroseocnds
**	12/4/96		Added XStrLen
**	12/15/96	Added X_SOLARIS
**	12/18/96	Added more Solaris stuff
**	12/19/96	Added create flag in XFileOpenForWrite
**	12/19/96	Added Sparc pragmas
**	12/30/96	Changed copyright
**	1/2/97		Moved USE_MOD_API and USE_STREAM_API into this file
**				Added XCtoPstr & XPtoCstr
**	1/12/97		Added XDecryptData and XEncryptData
**				Renamed DecompressPtr to XDecompressPtr
**	1/13/97		Added XDuplicateStr
**	1/20/97		Added XEncryptedStrCpy && XEncryptedStrLen && XLStrCmp
**	1/21/97		Added XLStrnCmp
**	1/24/97		Added LONG_TO_FIXED & FIXED_TO_LONG
**	1/28/97		Added some Navio changes
**				Added XGetIndexedResource
**	1/29/97		Changed XFileOpenResourceFromMemory to support resource duplication
**				Added XCompressPtr
**	1/30/97		Added XDecryptAndDuplicateStr
**	2/2/97		Added XStringToLong
**	2/5/97		Added XFileOpenForReadFromMemory
**	2/19/97		Added XStrStr
**	6/3/97		Added flag USE_HIGHLEVEL_FILE_API to enable/disable sound file decoding
**	6/4/97		Added JAVA_SOUND for moe
**	6/10/97		Removed XIMAState structure
**				Added some more JAVA_SOUND wrappers
**	6/12/97		bvk Added XGetResourceNameOnly( openResourceFiles[currentResFile], theType, theID, pName );
**				Doesn't load resource data, just headers.
**	6/13/97		bvk Added XCountResources(XFILE fileRef, long int theType);
**	6/18/97		Removed some extra windows header files
**	6/27/97		bvk Added XDeleteFileResource()
**					Added TRSH resource type
**					Added XBOOL XCleanResourceFile( XFILE fileRef )
**	7/14/97		Added X_WIN_HAE to X_PLATFORM types
**	7/17/97		Removed XIsVirtualMemoryAvailable & XLockMemory & XUnlockMemory. Because
**				its assumed that all memory is locked.
**				Removed XSetHardwareSampleRate & XGetHardwareSampleRate
**	7/28/97		Changed DEBUG_STR and compile files for X_SOLARIS
**	8/6/97		Moved USE_FLOAT from GenSnd.h
**	8/7/97		Added XDuplicateAndStripStr
**	8/18/97		Changed X_WIN_HAE to USE_HAE_EXTERNAL_API
**	9/2/97		Added HAE_VXD & HAE_STANDALONE as modifiers for the X_WIN95 platform
**	9/3/97		Added USE_FULL_RMF_SUPPORT to eliminate excess RMF support
**	9/29/97		Changed XSetBit & XClearBit & XTestBit to be unsigned long rather than long
**	11/10/97	Changed some preprocessor tests and flags to explicity test for flags rather
**				than assume
**	11/11/97	Added USE_DEVICE_ENUM_SUPPORT to support multiple devices
**	12/16/97	Moe: removed compiler warnings
**	12/17/97	Added memoryCacheEntry element to XFILENAME structure. Used for memory cache
**				searches. See PV_XGetNamedCacheEntry in X_API.c for details
**	12/18/97	Cleaned up some warnings and added some rounding devices. Changed XFIXED
**	1/8/98		MOE: added XSwapLongsInAccessCache()
**	1/9/98		Added XFileDelete
**	1/21/98		Changed the functions XGetShort & XGetLong & XPutShort & XPutLong
**				into macros that fall out for Motorola order hardware
**	1/26/98		Added XERR to various functions
**	1/27/98		Added H_ prefix to CODE_TYPE's
**	1/31/98		Moved XPI_Memblock structures to X_API.h, and moved the function
**				XIsOurMemoryPtr to X_API.h
**	2/1/98		Changed XFILENAME to allow for different sized names. Created constant
**				FILE_NAME_LENGTH that is defined per platform
**	2/7/98		Changed XFIXED back to an unsigned long to fix broken content
**	3/12/98		MOE: Subtly changed XExpandMacADPCMtoXBitLinear()'s parameter types
**	3/18/98		Added XIs8BitSupported
**	3/20/98		MOE: Renamed XExpandIMAtoXBitLinearStreamed()-->XExpandAiffImaStream()
**	3/20/98		MOE: Renamed XExpandMacADPCMtoXBitLinear()-->XExpandAiffIma()
**	3/20/98		MOE: Renamed XDecodeIMA4Bit()-->XExpandWavIma()
**	3/23/98		MOE: Changed _XGetShort() and _XGetLong() to accept const* pointers
**	3/23/98		MOE: Added _XGetShortIntel() and _XGetLongIntel()
**				Added XRESOURCE
**				Added new parameter to XCompressAiffIma & XAllocateCompressedAiffIma
**	3/24/98		Added HAE_STANDALONE for use when making standalone players
**	4/15/98		Fixed a bug with XGetXXXX that failed on Solaris because the
**				data was being access as a macro and it expected it to be long
**				word aligned and it wasn't!
**	4/20/98		Changed the size of the XPI_Memblock structure to accomodate
**				the SolarisOS 8 byte alignment performance gain.
**	4/27/98		Changed XCompressPtr to handle XCOMPRESSION_TYPE
**	4/27/98		MOE:  Changed XDecompressPtr(),
**				eliminated XDecompressSampleFormatPtr()
**	5/5/98		Added XGetTempXFILENAME
**	5/12/98		MOE: Changed all the "ExpandAiffIma" functions to accept a
**				predictorCache[] parameter, rather than an indexCache[]
**	6/18/98		Added XFileFreeResourceCache
**				Changed macro FILE_NAME_LENGTH to use _MAX_PATH for Windows and
**				added a missing include file
**	6/19/98		Added USE_CAPTURE_API
**	7/1/98		Changed various API to use the new XResourceType and XLongResourceID
**	7/6/98		Changed _XPutShort to pass a unsigned short rather than an unsigned long
**	7/10/98		Added XGetUniqueFileResourceID & XGetUniqueResourceID & XAddResource
**				Added XDeleteResource
**				Added XCountResourcesOfType
**				Added XCleanResource
**				Added XFileGetCurrentResourceFile
**	8/10/98		Added macros XFIXED_TO_UNSIGNED_LONG & UNSIGNED_LONG_TO_XFIXED
**	10/2/98		Added XExpandALawto16BitLinear
**	11/6/98		Added HAE_EDITOR for editor support
**	11/11/98	Renamed pName to pResourceName because of a MacOS macro conflict.
**				Added XReadPartialFileResource
**				Removed unused macros XGet/Put and placed them back into
**				real functions
**	12/22/98	Changed status of USE_VARIABLE_REVERB is various builds
**	2/8/99		Added XLStrStr & XStrCat
**	2/11/99		Removed MOD support when HAE_PLUGIN is defined
**	2/12/99		Added types USE_MPEG_ENCODER & USE_MPEG_DECODER
**	2/21/99		Added XStripStr & XFileCreateResourceCache
**	2/24/99		Added XGetShortCompressionName
**	2/28/99		Changed some default BeOS settings
**
**	JAVASOFT
**	01.26.97	$$kk: removed USE_8_BIT_OUTPUT and USE_MONO_OUTPUT undefs to get switchable output format
**	02.12.98	$$kk: removed USE_VARIABLE_REVERB to get variable reverb back
**	05.19.98:	$$kk: changed
**					#define XFIXED_TO_LONG(x)			(long)(((long)(x)) / XFIXED_1)
**				to
**				#define XFIXED_TO_LONG(x)			(long)(((XFIXED)(x)) / XFIXED_1)
**				because the long was wrapping and causing problems (negative sample rate)
**				for high sample rates (44100) in GM_AudioStreamSetRate
**	08.17,98:	$$kk: set FILE_NAME_LENGTH to 1024 for solaris because _MAX_PATH does not exist
**	2002-03-07	$$fb Itanium port
**	2002-03-14	$$fb added support for IA64, sparcv9 architectures
**			added explicit support for Linux with X_LINUX variable
**			changed X_WIN95 to X_WINDOWS
**	2003-08-22      added amd64, unified arch detection, unified pragma pack() usage
**	                removed superflous CODE_TYPE macro
*/
/*****************************************************************************/

//#define USE_DEBUG
//#ifndef USE_TRACE
//#define USE_TRACE
//#endif


#ifndef __X_API__
#define __X_API__

// some common types

#if 0
#pragma mark ## X_PLATFORM DEFINES ##
#endif
// types for X_PLATFORM
#define X_UNDEFINED     0
#define X_MACINTOSH     1		// MacOS
#define X_WINDOWS       2		// Windows 95/98/ME/NT/2000/XP OS
#define X_WEBTV         3		// WebTV OS
#define X_BE            4		// BeOS
#define X_MAGIC         5		// MagicCap OS. Note: This probably won't work yet
#define X_SOLARIS       6		// SunOS
#define X_NAVIO         7		// NaviOS
#define X_WIN_HARDWARE  8		// Windows 95 only direct to Sound Blaster hardware
#define X_LINUX         9		// Linux

// **********************************
// Make sure you set the X_PLATFORM define correctly. Everything depends upon this
// flag being setup correctly.
// **********************************
//

#ifndef X_PLATFORM
// this is my hack for MacOS. You can define this outside of this file.
#define X_PLATFORM		X_MACINTOSH
#endif
#ifndef X_PLATFORM
#error "You need to define X_PLATFORM outside of the source. Use the types above."
#endif


// types for CPU_TYPE
#define k68000				0
#define kRISC				1
#define k80X86				2
#define kSPARC				3
#define kSPARCV9			4
#define kIA64				5
#define kAMD64				6

// if JavaSound is enabled, then set this flag
// $$kk: 08.12.98 merge
// $$kk: 04.27.98: note that we set this flag in the makefile
//#define JAVA_SOUND		1

// if Netscape Plugin or ActiveX is enabled, then set this flag
//#define HAE_PLUGIN		1

// if its a stand alone player, then set this flag
//#define HAE_STANDALONE	1

// if its a editor, then set this flag
//#define HAE_EDITOR		1

// following is needed for _LP64
#if ((X_PLATFORM == X_SOLARIS) || (X_PLATFORM == X_LINUX))
#include <sys/types.h>
#endif

// $$fb 2002-02-01: itanium port: needed for Windows types
#if X_PLATFORM == X_WINDOWS
#ifndef WIN32_EXTRA_LEAN
#define WIN32_EXTRA_LEAN
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif /* X_PLATFORM == X_WINDOWS */


/* ******** generic CPU_TYPE detection ********** */

/* legacy arch support */
#if (defined(i386) || defined(x86) || defined(_X86_))
#if (!defined(i586) && !defined(ia64) && !defined(amd64))
#define i586
#endif
#endif

/* first check for 64-bit arch, since for them
 * sometimes the 32-bit arch macros are set, too
 */
#if defined(ia64)
#define CPU_TYPE      kIA64
#elif defined(amd64)
#define CPU_TYPE      kAMD64
#elif (defined(sparc) && defined(_LP64))
#define CPU_TYPE      kSPARCV9
#elif defined(sparcv9)
#define CPU_TYPE      kSPARCV9
#elif defined(i586)
#define CPU_TYPE      k80X86
#elif defined(sparc)
#define CPU_TYPE      kSPARC
#else
#error Please add architecture to X_API.h
#endif


/* ************** ENDIANNESS selection ************** */
#if ( (CPU_TYPE == k80X86) || (CPU_TYPE == kIA64) || (CPU_TYPE == kAMD64) || defined(_LITTLE_ENDIAN) )
/* little endian */
#define X_WORD_ORDER   TRUE
#else
/* big endian */
#define X_WORD_ORDER   FALSE
#endif

/* ********** Alignment issues/pragma selection ******** */
#if ((CPU_TYPE == kIA64) || (CPU_TYPE == kSPARCV9) || (CPU_TYPE == kAMD64) || (X_PLATFORM == X_WINDOWS))
#define X_REQUIRE_64BIT_ALIGNMENT
#elif ( (CPU_TYPE == k80X86) || (CPU_TYPE == kSPARC) )
#define X_REQUIRE_32BIT_ALIGNMENT
#endif

/* for the file structures,
 * we need 1-byte struct alignment,
 * but not necessarily all compilers
 * support #pragma pack(1)
 */
#if ( (CPU_TYPE == k80X86)  || (CPU_TYPE == kIA64) || (CPU_TYPE == kSPARC) || (CPU_TYPE == kSPARCV9) || (CPU_TYPE == kAMD64) )
#define X_ALLOW_PRAGMA
#endif


#if X_PLATFORM != X_NAVIO
typedef unsigned char           UINT8;
typedef char                    INT8;
typedef short                   INT16;
typedef unsigned short          UINT16;
#ifdef _LP64
typedef int                     INT32;
typedef unsigned int            UINT32;
// $$fb 2002-02-07: itanium port: add UINT64 and INT64
typedef unsigned long           UINT64;
typedef long                    INT64;
#else /* _LP64 */
#if X_PLATFORM != X_WINDOWS
/* case for neither WIN32 nor WIN64 */
typedef long                    INT32;
typedef unsigned long           UINT32;
/* generic 64 bit ? */
typedef unsigned long long      UINT64;
typedef long long               INT64;
#endif /* !X_WINDOWS */
#endif /* _LP64 */
#endif /* !X_NAVIO */

// $$fb 2002-02-01: itanium port: added UINT_PTR - an unsigned int with pointer size
// UINT_PTR is defined for 64-bit Windows header files,
// but not for older 32-bit header files or Solaris/Linux
#ifndef _WIN64
typedef unsigned long           UINT_PTR;
typedef long                    INT_PTR;
#endif


typedef void *                  XPTR;
typedef void *                  XRESOURCE;
typedef INT32                   XERR;
typedef unsigned char           XBYTE;
typedef UINT16                  XWORD;
typedef UINT32                  XDWORD;
typedef char                    XBOOL;

typedef unsigned char *	G_PTR;
typedef unsigned char	UBYTE;
typedef char			SBYTE;
typedef INT32			LOOPCOUNT;


// $$fb 2002-02-01: itanium port: added XFILE_HANDLE type, because HANDLE is 64 bit on Windows/Itanium
typedef INT_PTR                 XFILE_HANDLE;


// This is used to solve the 4 character constant problem on some compilers
#define FOUR_CHAR(ch1,ch2,ch3,ch4) \
			((((UINT32)(ch1)&0x0FFL)<<24L) + (((ch2)&0x0FFL)<<16L) + (((ch3)&0x0FFL)<<8L) + ((ch4)&0x0FFL))


#undef ABS
#define ABS(x)				(((x) < 0) ? -(x) : (x))

#define XMAKELONG(a, b)		((INT32)(((XWORD)(a)) | ((XDWORD)((XWORD)(b))) << 16L))
#define XLOWORD(l)			((XWORD)(l))
#define XHIWORD(l)			((XWORD)(((XDWORD)(l) >> 16L) & 0xFFFFL))

#define XMAX(a,b)			(((a) > (b)) ? (a) : (b))
#define XMIN(a,b)			(((a) < (b)) ? (a) : (b))

#undef TRUE
#undef FALSE
#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

#undef NULL
#ifndef NULL
#define NULL	0L
#endif

#if 0
#pragma mark ## X_PLATFORM == X_MACINTOSH ##
#endif
// **********************************
// MacOS via Sound Manager 3.1
#if X_PLATFORM == X_MACINTOSH
#include <Sound.h>
#include <Files.h>
#include <Types.h>
#define X_WORD_ORDER		FALSE
#define FAR
#define FP_OFF(x)			(x)
#if GENERATING68K == FALSE
#define CPU_TYPE		kRISC
#else
#define CPU_TYPE		k68000
#endif
#ifndef __MOTO__
#define INLINE				inline
#else
#define INLINE
#endif

#ifndef DEBUG_STR
#if (USE_DEBUG == 0) || !defined(USE_DEBUG)
#define DEBUG_STR(x)
#else
#include <stdio.h>
#endif

#if USE_DEBUG == 1
#define DEBUG_STR(x)	DebugStr((unsigned char *)(x))
#endif

#if USE_DEBUG == 2
extern short int drawDebug;
#ifdef __cplusplus
extern "C" {
#endif
    extern void DPrint(short int d, ...);
#ifdef __cplusplus
}
#endif
#define DEBUG_STR(x)	DPrint(drawDebug, "%s\r", x)
#endif

#if USE_DEBUG == 3
#define DEBUG_STR(x)
extern short int drawDebug;
#ifdef __cplusplus
extern "C" {
#endif
    extern void DPrint(short int d, ...);
#ifdef __cplusplus
}
#endif
#define DEBUG_STR2(x)	DPrint(drawDebug, "%s\r", x)
#else
#define DEBUG_STR2(x)
#endif
#endif

// API features
#if 0
#pragma mark ## Settings ##
#endif
#define USE_HAE_EXTERNAL_API			TRUE
#define USE_DROP_SAMPLE					TRUE
#define USE_TERP1						TRUE
#define USE_TERP2						TRUE
#define USE_8_BIT_OUTPUT				TRUE
#define USE_16_BIT_OUTPUT				TRUE
#define USE_MONO_OUTPUT					TRUE
#define USE_STEREO_OUTPUT				TRUE
#define USE_STREAM_API					TRUE
#define USE_CREATION_API				TRUE
#define USE_MOD_API						TRUE
#define USE_HIGHLEVEL_FILE_API			TRUE
#define USE_SMALL_MEMORY_REVERB			FALSE
#define USE_FLOAT						TRUE
#define USE_VARIABLE_REVERB				TRUE
//#define USE_NEW_EFFECTS					TRUE
//#define USE_MPEG_ENCODER				FALSE
//#define USE_MPEG_DECODER				TRUE
#define USE_FULL_RMF_SUPPORT			TRUE
#define USE_DEVICE_ENUM_SUPPORT			TRUE
#define USE_CAPTURE_API					TRUE

#if 0		// this is for my testing. Please keep set to 0!!!
#undef	USE_DROP_SAMPLE
#undef	USE_TERP1
#undef	USE_8_BIT_OUTPUT
#undef	USE_MONO_OUTPUT
#undef	USE_CREATION_API
#undef	USE_MOD_API
#undef	USE_HIGHLEVEL_FILE_API
#undef	USE_STREAM_API
#undef	USE_FULL_RMF_SUPPORT
#undef	USE_CAPTURE_API
#endif
#if HAE_EDITOR == TRUE
#undef	USE_MOD_API
#undef	USE_STREAM_API
#undef	USE_CAPTURE_API
#endif
#if HAE_STANDALONE == TRUE
#undef	USE_DROP_SAMPLE
#undef	USE_TERP1
#undef	USE_8_BIT_OUTPUT
#undef	USE_MONO_OUTPUT
#undef	USE_CREATION_API
#undef	USE_MOD_API
//#undef	USE_HIGHLEVEL_FILE_API
#undef	USE_STREAM_API
#undef	USE_CAPTURE_API
#endif
#if HAE_PLUGIN == TRUE
#undef	USE_MOD_API
#undef	USE_DROP_SAMPLE
#undef	USE_TERP1
#undef	USE_8_BIT_OUTPUT
#undef	USE_MONO_OUTPUT
#undef	USE_CREATION_API
#undef	USE_CAPTURE_API
#endif

#if CPU_TYPE == k68000
#undef USE_SMALL_MEMORY_REVERB
#define USE_SMALL_MEMORY_REVERB		TRUE
#undef USE_FLOAT
#undef USE_VARIABLE_REVERB
#endif
#endif


#if 0
#pragma mark ## X_PLATFORM == X_NAVIO ##
#endif
// **********************************
// NavioOS via direct hardware
#if X_PLATFORM == X_NAVIO
#ifndef DEBUG_STR
#define DEBUG_STR(x)
#endif
#define X_WORD_ORDER		TRUE		// do swap words
#define FAR
#define FP_OFF(x)			(x)
#define CPU_TYPE			k80X86
#define INLINE
#include <kernel/cFile.h>

// API features
#if 0
#pragma mark ## Settings ##
#endif
#define USE_HAE_EXTERNAL_API			FALSE
#define USE_CREATION_API				FALSE
#define USE_DROP_SAMPLE					TRUE
#define USE_TERP1						TRUE
#define USE_TERP2						TRUE
#define USE_8_BIT_OUTPUT				TRUE
#define USE_16_BIT_OUTPUT				TRUE
#define USE_MONO_OUTPUT					TRUE
#define USE_STEREO_OUTPUT				TRUE
#define USE_FLOAT						FALSE
#define USE_MOD_API						FALSE
#define USE_STREAM_API					TRUE
#define USE_HIGHLEVEL_FILE_API			TRUE
#define USE_SMALL_MEMORY_REVERB			TRUE
#define USE_VARIABLE_REVERB				FALSE
//#define USE_NEW_EFFECTS					TRUE
//#define USE_MPEG_ENCODER				FALSE
//#define USE_MPEG_DECODER				TRUE
#define USE_FULL_RMF_SUPPORT			TRUE
#define USE_DEVICE_ENUM_SUPPORT			FALSE
#define USE_CAPTURE_API					FALSE
#endif

#if 0
#pragma mark ## X_PLATFORM == X_WEBTV ##
#endif
// **********************************
// WebTVOS via direct hardware
#if X_PLATFORM == X_WEBTV
#ifndef DEBUG_STR
#define DEBUG_STR(x)
#endif
#define X_WORD_ORDER		FALSE		// don't swap words
#define FAR
#define FP_OFF(x)			(x)
#define CPU_TYPE			kRISC
#define INLINE

// API features
#if 0
#pragma mark ## Settings ##
#endif
#define USE_HAE_EXTERNAL_API			TRUE
#define USE_CREATION_API				FALSE
#define USE_DROP_SAMPLE					FALSE
#define USE_TERP1						FALSE
#define USE_TERP2						TRUE
#define USE_8_BIT_OUTPUT				FALSE
#define USE_16_BIT_OUTPUT				TRUE
#define USE_MONO_OUTPUT					FALSE
#define USE_STEREO_OUTPUT				TRUE
#define USE_FLOAT						TRUE
#define USE_MOD_API						FALSE
#define USE_HIGHLEVEL_FILE_API			FALSE
#define USE_STREAM_API					FALSE
#define USE_SMALL_MEMORY_REVERB			FALSE
#define USE_VARIABLE_REVERB				TRUE
//#define USE_NEW_EFFECTS					TRUE
//#define USE_MPEG_ENCODER				FALSE
//#define USE_MPEG_DECODER				TRUE
#define USE_FULL_RMF_SUPPORT			TRUE
#define USE_DEVICE_ENUM_SUPPORT			FALSE
#define USE_CAPTURE_API					FALSE
#endif

#if 0
#pragma mark ## X_PLATFORM == X_WINDOWS ##
#endif
// **********************************
// Windows 95 and NT via DirectX or waveOut
#if X_PLATFORM == X_WINDOWS
// $$fb 2002-02-01: itanium port: moved windows.h include to top
#include <stdlib.h>

#ifndef DEBUG_STR
#if !defined(USE_DEBUG)
#define DEBUG_STR(x)
#elif defined(USE_DEBUG)
#include <stdio.h>
#define DEBUG_STR(x)	printf(x); fflush(stdout)
#endif

#endif
#define FP_OFF(x)			(x)
#define INLINE				_inline

// API features
#if 0
#pragma mark ## Settings ##
#endif
#define USE_HAE_EXTERNAL_API			TRUE
#define USE_DROP_SAMPLE					TRUE
#define USE_TERP1						TRUE
#define USE_TERP2						TRUE
#define USE_8_BIT_OUTPUT				TRUE
#define USE_16_BIT_OUTPUT				TRUE
#define USE_MONO_OUTPUT					TRUE
#define USE_STEREO_OUTPUT				TRUE
#define USE_STREAM_API					TRUE
#define USE_CREATION_API				TRUE
#define USE_MOD_API						TRUE
#define USE_HIGHLEVEL_FILE_API			TRUE
#define USE_SMALL_MEMORY_REVERB			FALSE
#define USE_FLOAT						TRUE
#define USE_VARIABLE_REVERB				TRUE
//#define USE_NEW_EFFECTS					TRUE
//#define USE_MPEG_ENCODER				FALSE
//#define USE_MPEG_DECODER				TRUE
#define USE_FULL_RMF_SUPPORT			TRUE
#define USE_DEVICE_ENUM_SUPPORT			TRUE
#define USE_CAPTURE_API					TRUE

#if HAE_VXD
#undef	USE_FLOAT
#undef	USE_CREATION_API
#undef	USE_HIGHLEVEL_FILE_API
#undef	USE_STREAM_API
#undef	USE_CAPTURE_API
#undef	USE_MOD_API
#undef	USE_HAE_EXTERNAL_API
#define USE_HAE_EXTERNAL_API		TRUE
#define USE_NEW_EFFECTS				TRUE
#endif

#if HAE_EDITOR == TRUE
#undef	USE_MOD_API
#undef	USE_STREAM_API
#undef	USE_CAPTURE_API
#endif

#if HAE_STANDALONE == TRUE
#undef	USE_DROP_SAMPLE
#undef	USE_TERP1
#undef	USE_8_BIT_OUTPUT
#undef	USE_MONO_OUTPUT
#undef	USE_CREATION_API
#undef	USE_MOD_API
#undef	USE_HIGHLEVEL_FILE_API
#undef	USE_STREAM_API
#undef	USE_CAPTURE_API
#endif

#if HAE_PLUGIN == TRUE
#undef	USE_MOD_API
#undef	USE_DROP_SAMPLE
#undef	USE_TERP1
#undef	USE_8_BIT_OUTPUT
#undef	USE_MONO_OUTPUT
#undef	USE_CREATION_API
#undef	USE_CAPTURE_API
#endif
#endif /* WIN95 */


#if 0
#pragma mark ## X_PLATFORM == X_WIN_HARDWARE ##
#endif
// **********************************
// Windows 95 only and direct to Sound Blaster hardware
#if X_PLATFORM == X_WIN_HARDWARE
#ifndef WIN32_EXTRA_LEAN
#define WIN32_EXTRA_LEAN
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "Vxdintf.h"

#ifndef DEBUG_STR
#define DEBUG_STR(x)
#endif
#define X_WORD_ORDER		TRUE		// swap words
#define FP_OFF(x)			(x)
#define INLINE				_inline
#define CPU_TYPE			k80X86

// API features
#if 0
#pragma mark ## Settings ##
#endif
#define USE_HAE_EXTERNAL_API			FALSE
#define USE_CREATION_API				FALSE
#define USE_DROP_SAMPLE					TRUE
#define USE_TERP1						TRUE
#define USE_TERP2						TRUE
#define USE_8_BIT_OUTPUT				TRUE
#define USE_16_BIT_OUTPUT				TRUE
#define USE_MONO_OUTPUT					TRUE
#define USE_STEREO_OUTPUT				TRUE
#define USE_MOD_API						TRUE
#define USE_HIGHLEVEL_FILE_API			TRUE
#define USE_STREAM_API					TRUE
#define USE_SMALL_MEMORY_REVERB			FALSE
#define USE_VARIABLE_REVERB				TRUE
//#define USE_NEW_EFFECTS					TRUE
//#define USE_MPEG_ENCODER				FALSE
//#define USE_MPEG_DECODER				TRUE
#define USE_FLOAT						FALSE
#define USE_FULL_RMF_SUPPORT			TRUE
#define USE_DEVICE_ENUM_SUPPORT			FALSE
#define USE_CAPTURE_API					FALSE
#endif /* WIN_HARDWARE */


#if 0
#pragma mark ## X_PLATFORM == X_BE ##
#endif
// **********************************
// BeOS via low level audio stream threads
#if X_PLATFORM == X_BE
// Note compiler must be set to treat all files at C++, and char to
// be signed
#include <Be.h>
#ifndef DEBUG_STR
#define DEBUG_STR(x)
#endif
#define X_WORD_ORDER		B_HOST_IS_LENDIAN
#define FAR
#define FP_OFF(x)			(x)
//#if __POWERPC__
#define CPU_TYPE		kRISC
//#endif
//#if __INTEL__
//	#define CPU_TYPE		k80X86
//#endif
#define INLINE				inline

// API features
#if 0
#pragma mark ## Settings ##
#endif
#define USE_HAE_EXTERNAL_API			TRUE
#define USE_CREATION_API				TRUE
#define USE_DROP_SAMPLE					TRUE
#define USE_TERP1						TRUE
#define USE_TERP2						TRUE
#define USE_8_BIT_OUTPUT				TRUE
#define USE_16_BIT_OUTPUT				TRUE
#define USE_MONO_OUTPUT					TRUE
#define USE_STEREO_OUTPUT				TRUE
#define USE_MOD_API						FALSE
#define USE_HIGHLEVEL_FILE_API			FALSE
#define USE_STREAM_API					FALSE
#define USE_SMALL_MEMORY_REVERB			FALSE
#define USE_VARIABLE_REVERB				TRUE
#define USE_NEW_EFFECTS					TRUE
//#define USE_MPEG_ENCODER				FALSE
//#define USE_MPEG_DECODER				TRUE
#define USE_FLOAT						TRUE
#define USE_FULL_RMF_SUPPORT			TRUE
#define USE_DEVICE_ENUM_SUPPORT			FALSE
#define USE_CAPTURE_API					FALSE
#endif /* BE */

#if 0
#pragma mark ## X_PLATFORM == X_SOLARIS ##
#endif
// **********************************
// SunOS via dev/audio thread
#if X_PLATFORM == X_SOLARIS
#ifndef DEBUG_STR
#ifdef  USE_DEBUG
#include <stdio.h>
#define DEBUG_STR(x) {\
                 fprintf(stderr, x); fprintf(stderr, "\n");\
                 }
#else
#define DEBUG_STR(x)
#endif /* USE_DEBUG */
#endif /* DEBUG_STR */
#define FAR
#define FP_OFF(x)			(x)
#define INLINE

// API features
#if 0
#pragma mark ## Settings ##
#endif
#define USE_HAE_EXTERNAL_API			TRUE

// $$kk: 12.16.98: this was FALSE; i need it to be TRUE for the
// soundbank stuff.  is it ok to change it??
#define USE_CREATION_API				TRUE

#define USE_DROP_SAMPLE					TRUE
#define USE_TERP1						TRUE
#define USE_TERP2						TRUE
#define USE_8_BIT_OUTPUT				TRUE
#define USE_16_BIT_OUTPUT				TRUE
#define USE_MONO_OUTPUT					TRUE
#define USE_STEREO_OUTPUT				TRUE
#define USE_MOD_API						FALSE
#define USE_HIGHLEVEL_FILE_API			TRUE
#define USE_STREAM_API					TRUE
#define USE_SMALL_MEMORY_REVERB			FALSE
#define USE_VARIABLE_REVERB				TRUE
//#define USE_NEW_EFFECTS					TRUE
//#define USE_MPEG_ENCODER				FALSE
//#define USE_MPEG_DECODER				TRUE
#define USE_FLOAT						TRUE
#define USE_FULL_RMF_SUPPORT			TRUE
#define USE_DEVICE_ENUM_SUPPORT			FALSE
#define USE_CAPTURE_API					TRUE

#if HAE_EDITOR == TRUE
#undef	USE_MOD_API
#undef	USE_STREAM_API
#undef	USE_CAPTURE_API
#endif
#if HAE_STANDALONE == TRUE
#undef	USE_DROP_SAMPLE
#undef	USE_TERP1
#undef	USE_8_BIT_OUTPUT
#undef	USE_MONO_OUTPUT
#undef	USE_CREATION_API
#undef	USE_MOD_API
//#undef	USE_HIGHLEVEL_FILE_API
#undef	USE_STREAM_API
#endif
#if HAE_PLUGIN == TRUE
#undef	USE_MOD_API
#undef	USE_DROP_SAMPLE
#undef	USE_TERP1
#undef	USE_8_BIT_OUTPUT
#undef	USE_MONO_OUTPUT
#undef	USE_CREATION_API
#endif
#endif

/* $$fb 2002-03-11: added X_LINUX type */
// **********************************
// Linux via dev/dsp thread
#if X_PLATFORM == X_LINUX
#ifndef DEBUG_STR
#ifdef  USE_DEBUG
#include <stdio.h>
#define DEBUG_STR(x) {\
                 fprintf(stderr, x); fprintf(stderr, "\n");\
                 }
#else
#define DEBUG_STR(x)
#endif /* USE_DEBUG */
#endif /* DEBUG_STR */
#define FAR
#define FP_OFF(x)			(x)
#define INLINE    inline

// API features
#if 0
#pragma mark ## Settings ##
#endif
#define USE_HAE_EXTERNAL_API		TRUE
#define USE_CREATION_API		TRUE
#define USE_DROP_SAMPLE			TRUE
#define USE_TERP1			TRUE
#define USE_TERP2			TRUE
#define USE_8_BIT_OUTPUT		TRUE
#define USE_16_BIT_OUTPUT		TRUE
#define USE_MONO_OUTPUT			TRUE
#define USE_STEREO_OUTPUT		TRUE
#define USE_MOD_API			FALSE
#define USE_HIGHLEVEL_FILE_API		TRUE
#define USE_STREAM_API			TRUE
#define USE_SMALL_MEMORY_REVERB		FALSE
#define USE_VARIABLE_REVERB		TRUE
//#define USE_NEW_EFFECTS		TRUE
//#define USE_MPEG_ENCODER		FALSE
//#define USE_MPEG_DECODER		TRUE
#define USE_FLOAT			TRUE
#define USE_FULL_RMF_SUPPORT		TRUE
#define USE_DEVICE_ENUM_SUPPORT		FALSE
#define USE_CAPTURE_API			TRUE

#if HAE_EDITOR == TRUE
#undef	USE_MOD_API
#undef	USE_STREAM_API
#undef	USE_CAPTURE_API
#endif
#if HAE_STANDALONE == TRUE
#undef	USE_DROP_SAMPLE
#undef	USE_TERP1
#undef	USE_8_BIT_OUTPUT
#undef	USE_MONO_OUTPUT
#undef	USE_CREATION_API
#undef	USE_MOD_API
//#undef	USE_HIGHLEVEL_FILE_API
#undef	USE_STREAM_API
#endif
#if HAE_PLUGIN == TRUE
#undef	USE_MOD_API
#undef	USE_DROP_SAMPLE
#undef	USE_TERP1
#undef	USE_8_BIT_OUTPUT
#undef	USE_MONO_OUTPUT
#undef	USE_CREATION_API
#endif
#endif

/* $$fb 2002-03-11: take JAVA_SOUND stuff out of the system-specific sections
 * Java Sound should have always the same set of features. Having it in one
 * place makes it easier to maintain. */
#if JAVA_SOUND == TRUE
#undef	USE_DROP_SAMPLE
#undef	USE_TERP1

// $$kk: 01.26.87: need these to be defined to get switchable output format
//	#undef	USE_8_BIT_OUTPUT
//	#undef	USE_MONO_OUTPUT

// $$kk: 12.16.98: need this for the soundbank stuff
// $$kk: 03.17.99: actually, shouldn't need this anymore
#undef	USE_CREATION_API

// $$kk: 02.12.98: need this to get variable reverb back
//	#undef	USE_VARIABLE_REVERB

#undef	USE_MOD_API
#undef	USE_HIGHLEVEL_FILE_API

// $$kk: 12.16.98: need this for the soundbank stuff
//	#undef	USE_FULL_RMF_SUPPORT
#endif

typedef UINT32				XFIXED;

// The type XFIXED is an unsigned value, but the the calculations allow for negative numbers. If you changes this
// from long to unsigned, then all the fade API's will fail. If you need the extra bit for an unsigned value
// use the unsigned macros
#define XFIXED_1					65536L
#define LONG_TO_XFIXED(x)			(XFIXED)((((INT32)(x)) * XFIXED_1))
#define UNSIGNED_LONG_TO_XFIXED(x)	(XFIXED)((((UINT32)(x)) * XFIXED_1))
#define RATIO_TO_XFIXED(a,b)		(LONG_TO_XFIXED(a) / (b))

// $$kk: 08.12.98 merge
// $$kk: 05.19.98: changed this line.  it was wrapping and biting me
// on high sample rates (44100) in GM_AudioStreamSetRate.
// $$kk: 04.06.99: steve hales swears this problem is because i should
// be using XFIXED_TO_UNSIGNED_LONG instead of XFIXED_TO_LONG, so i am
// changing it back.
// #define XFIXED_TO_LONG(x)			(long)(((XFIXED)(x)) / XFIXED_1)
#define XFIXED_TO_LONG(x)			(INT32)(((INT32)(x)) / XFIXED_1)

#define XFIXED_TO_UNSIGNED_LONG(x)	(UINT32)(((UINT32)(x)) / XFIXED_1)
#define XFIXED_TO_SHORT(x)			((short)((x) / XFIXED_1))
#define XFIXED_TO_LONG_ROUNDED(x)	XFIXED_TO_LONG((x) + XFIXED_1 / 2)
#define XFIXED_TO_SHORT_ROUNDED(x)	XFIXED_TO_SHORT((x) + XFIXED_1 / 2)

//#if USE_FLOAT == TRUE
#define FLOAT_TO_XFIXED(x)		((XFIXED)((double)(x) * XFIXED_1))
#define XFIXED_TO_FLOAT(x)		((double)(x) / XFIXED_1)
//#endif

// This fixes a newly discovered bug in which there is a loss of persiscion when calculating
// and playing back midi files with the internal sequencer. By changing the appropiate
// variables to floats the problem goes away. This needs to be recoded for fixed point math
// to preserve the engine's non use of floating point
#if USE_FLOAT == FALSE
typedef UINT32	UFLOAT;
typedef INT32	IFLOAT;
#else
typedef float	UFLOAT;
typedef float	IFLOAT;
#endif

#ifdef USE_TRACE
#define TRACE_STR DEBUG_STR
#else
#define TRACE_STR(x)
#endif


#ifdef __cplusplus
extern "C" {
#endif


    // Memory Manager

    // Every block of data allocated with XNewPtr will contain this structure before the pointer that is
    // passed to the user.
    struct XPI_Memblock
    {
	INT32	blockID_one;		// ID that this is our block. part 1
	INT32	blockSize;			// block size
	INT32	blockID_two;		// ID that this is our block. part 2
	INT32	alignment8;			// used for alignment to 8 byte boundries
    };
    typedef struct XPI_Memblock XPI_Memblock;

#define XPI_BLOCK_1_ID		FOUR_CHAR('I','G','O','R')		//	'IGOR'
#define XPI_BLOCK_2_ID		FOUR_CHAR('G','S','N','D')		//	'GSND'
#define XPI_BLOCK_3_ID		FOUR_CHAR('F','L','A','T')		//	'FLAT'
#define XPI_DEAD_ID			FOUR_CHAR(0xDE,0xAD,0xFF,0xFF)	//	Dead block

    // This function is used to see of a particular memory block has been allocated with
    // XNewPtr. If not this function will return NULL, otherwise the new pointer
    // reference, which will be the real memory allocated with the host allocate memory function.
    XPI_Memblock * XIsOurMemoryPtr(XPTR data);

    XPTR	XNewPtr(INT32 size);
    void	XDisposePtr(XPTR data);
    INT32	XGetPtrSize(XPTR data);
    void	XBlockMove(XPTR source, XPTR dest, INT32 size);
    void	XSetMemory(void *pAdr, INT32 len, char value);
    void	XBubbleSortArray(short int *theArray, short int theCount);
    void	XSetBit(void *pBitArray, UINT32 whichbit);
    void	XClearBit(void *pBitArray, UINT32 whichbit);
    XBOOL	XTestBit(void *pBitArray, UINT32 whichbit);

    UINT32 XMicroseconds(void);
    void XWaitMicroseocnds(UINT32 waitAmount);
    /* get difference of MusicGlobals->syncCount and XMicroseconds */
    INT64 XGetRealTimeSyncCount();


    // Resource Manager

    typedef INT32			XResourceType;
    typedef INT32			XLongResourceID;
    typedef INT16			XShortResourceID;

    struct XFILE_CACHED_ITEM
    {
	XResourceType	resourceType;		// resource type
	XLongResourceID	resourceID;			// resource ID
	INT32			resourceLength;		// resource ID
	INT32			fileOffsetName;		// file offset from 0 to resource name
	INT32			fileOffsetData;		// file offset from 0 to resource data
    };
    typedef struct XFILE_CACHED_ITEM		XFILE_CACHED_ITEM;

    struct XFILERESOURCECACHE
    {
	INT32				totalResources;
	XFILE_CACHED_ITEM	cached[1];
    };
    typedef struct XFILERESOURCECACHE		XFILERESOURCECACHE;

#if X_PLATFORM == X_MACINTOSH
#define FILE_NAME_LENGTH	63
#endif

#if ((X_PLATFORM == X_SOLARIS || X_PLATFORM == X_LINUX))
    // $$kk: 08.12.98: what is the real value?  is 1024 correct??
#define FILE_NAME_LENGTH	1024
#endif

#if ( (X_PLATFORM == X_WINDOWS) 			||	\
	  (X_PLATFORM == X_WIN_HARDWARE)	||	\
	  (X_PLATFORM == X_NAVIO) )
#define FILE_NAME_LENGTH	_MAX_PATH
#endif
#if X_PLATFORM == X_WEBTV
#define FILE_NAME_LENGTH	128
#endif

#if X_PLATFORM == X_BE
#define FILE_NAME_LENGTH	128
#endif

    struct XFILENAME
    {
	// public platform specific
#if X_PLATFORM == X_MACINTOSH
	XFILE_HANDLE				fileReference;
	FSSpec				theFile;
#endif
#if ( (X_PLATFORM == X_WINDOWS) 			||	\
	  (X_PLATFORM == X_WIN_HARDWARE)	||	\
	  (X_PLATFORM == X_WEBTV)			||	\
	  (X_PLATFORM == X_BE)				||	\
	  (X_PLATFORM == X_SOLARIS)			||	\
	  (X_PLATFORM == X_LINUX)			||	\
	  (X_PLATFORM == X_NAVIO) )
	/* $$fb 2002-02-14: itanium port */
	XFILE_HANDLE				fileReference;
	char				theFile[FILE_NAME_LENGTH];	// "C" string name for path
#endif

	// private variables. Zero out before calling functions
	INT32				fileValidID;
	XBOOL				resourceFile;

	XPTR				pResourceData;	// if file is memory based
	INT32				resMemLength;	// length of memory resource file
	INT32				resMemOffset;	// current offset of memory resource file
	XBOOL				readOnly;		// TRUE then file is read only
	XBOOL				allowMemCopy;	// if TRUE, when a memory based resource is
	// read, a copy will be created otherwise
	// its just a pointer into the larger memory resource
	// file
	XFILE_CACHED_ITEM	memoryCacheEntry;
	XFILERESOURCECACHE	*pCache;		// if file has been cached this will point to it
    };
    typedef struct XFILENAME	XFILENAME;
    typedef void *				XFILE;

#define XFILERESOURCE_ID	FOUR_CHAR('I','R','E','Z')	// IREZ
#define XFILECACHE_ID		FOUR_CHAR('C','A','C','H')	// CACH
#define XFILETRASH_ID		FOUR_CHAR('T','R','S','H')	// TRSH

    struct XFILERESOURCEMAP
    {
	INT32		mapID;
	INT32		version;
	INT32		totalResources;
    };
    typedef struct XFILERESOURCEMAP		XFILERESOURCEMAP;

    // $$fb 2002-02-07: define UINT64 and INT64 above

    typedef UINT64 SAMPLE_COUNT;
    typedef INT64 SAMPLE_COUNT_SIGNED;

    // Resource Entry
    //	long	nextentry
    //	long	resourceType
    //	long	resourceID
    //	pascal	string resourceName
    //	long	resourceLength
    //	data block

    // Resource file works as follows:
    //	MAP
    //	ENTRIES
    //
    // You can assume that at the end of the file length you can tack new resources. Just update the map


    // Create a temporary file name and fill an XFILENAME structure. Return -1 for failure, or 0 for sucess.
    XERR XGetTempXFILENAME(XFILENAME* xfilename);

    // Open file as a resource file. Pass TRUE to 'readOnly' for read only.
    XFILE	XFileOpenResource(XFILENAME *file, XBOOL readOnly);

    // Open file as a read only resource file from a memory pointer. Assumes memory block
    // is an exact copy of the resource file format. Don't dispose of pResource until you
    // have closed the file. If allowCopy is TRUE, then when resources are read new copies
    // will be created, otherwise just a pointer into the mapped resource file
    XFILE	XFileOpenResourceFromMemory(XPTR pResource, UINT32 resourceLength, XBOOL allowCopy);

    // Open file as a read only file from a memory pointer. Don't dispose of pMemoryBlock until you
    // have closed the file.
    XFILE XFileOpenForReadFromMemory(XPTR pMemoryBlock, UINT32 memoryBlockSize);

    // open file for reading and writing. Direct access.
    XFILE	XFileOpenForRead(XFILENAME *file);
    XFILE	XFileOpenForWrite(XFILENAME *file, XBOOL create);

    // close file. Direct or resource
    void	XFileClose(XFILE fileRef);
    void	XFileUseThisResourceFile(XFILE fileRef);

    // get current most recently opened resource file, or NULL if nothing is open
    XFILE	XFileGetCurrentResourceFile(void);

    // delete file. 0 is ok, -1 for failure
    XERR XFileDelete(XFILENAME *file);

    // Read a file into memory and return an allocated pointer.
    // 0 is ok, -1 failed to open, -2 failed to read, -3 failed memory
    // if 0, then *pData is valid
    XERR XGetFileAsData(XFILENAME *pResourceName, XPTR *pData, INT32 *pSize);

    void XConvertNativeFileToXFILENAME(void *file, XFILENAME *xfile);

    XFILERESOURCECACHE* XCreateAccessCache(XFILE fileRef);
    void XSwapLongsInAccessCache(XFILERESOURCECACHE	*pCache, XBOOL inFileOrder);

    // Create a resource cache for a file
    XERR XFileCreateResourceCache(XFILE fileRef);

    // Free cache of a resource file
    void XFileFreeResourceCache(XFILE fileRef);

    // search through open resource files
    void	XGetResourceName(XResourceType resourceType, XLongResourceID resourceID, void *cName);
    XPTR	XGetNamedResource(XResourceType resourceType, void *cName, INT32 *pReturnedResourceSize);
    XPTR	XGetAndDetachResource(XResourceType resourceType, XLongResourceID resourceID, INT32 *pReturnedResourceSize);
    XPTR	XGetIndexedResource(XResourceType resourceType, XLongResourceID *pReturnedID, INT32 resourceIndex,
				    void *pResourceName, INT32 *pReturnedResourceSize);

    // get a unique ID for a particular file to be used as a resource ID
    XERR	XGetUniqueFileResourceID(XFILE fileRef, XResourceType resourceType, XLongResourceID *pReturnedID);
    XERR	XGetUniqueResourceID(XResourceType resourceType, XLongResourceID *pReturnedID);

    // Add a resource to the most recently open resource file.
    //		resourceType is a type
    //		resourceID is an ID
    //		pResourceName is a pascal string
    //		pData is the data block to add
    //		length is the length of the data block
    XERR	XAddResource(XResourceType resourceType, XLongResourceID resourceID, void *pResourceName, void *pData, INT32 length);

    // Delete a resource from the most recently open resource file.
    //		resourceType is a type
    //		resourceID is an ID
    //		collectTrash if TRUE will force an update, otherwise it will happen when the file is closed
    XBOOL	XDeleteResource(XResourceType theType, XLongResourceID resourceID, XBOOL collectTrash );

    // return the number of resources of a particular type.
    INT32	XCountResourcesOfType(XResourceType resourceType);

    // Force a clean/update of the most recently opened resource file
    XBOOL	XCleanResource(void);

    // Add a resource to a particular file
    //		fileRef is the open file
    //		resourceType is a type
    //		resourceID is an ID
    //		pResourceName is a pascal string
    //		pData is the data block to add
    //		length is the length of the data block
    XERR	XAddFileResource(XFILE fileRef, XResourceType resourceType, XLongResourceID resourceID, void *pResourceName, void *pData, INT32 length);
    XPTR	XGetFileResource(XFILE fileRef, XResourceType resourceType, XLongResourceID resourceID, void *pResourceName, INT32 *pReturnedResourceSize);
    XPTR	XGetIndexedFileResource(XFILE fileRef, XResourceType resourceType, XLongResourceID *pReturnedID, INT32 resourceIndex,
					void *pResourceName, INT32 *pReturnedResourceSize);

    XERR XReadPartialFileResource(XFILE fileRef, XResourceType resourceType, XLongResourceID resourceID,
				  char *pReturnedResourceName,
				  XPTR *pReturnedBuffer, INT32 bytesToReadAndAllocate);

    XResourceType XGetIndexedType(XFILE fileRef, INT32 resourceIndex);
    INT32	XCountTypes(XFILE fileRef);
    INT32	XCountFileResourcesOfType(XFILE fileRef, XResourceType theType);

    char	*XGetResourceNameOnly( XFILE fileRef, XResourceType theType, XLongResourceID theID, char *pResourceName );
    XBOOL	XDeleteFileResource(XFILE fileRef, XResourceType theType, XLongResourceID resourceID, XBOOL collectTrash );
    XBOOL	XCleanResourceFile( XFILE fileRef );

    // File Manager
    XERR	XFileRead(XFILE fileRef, void * buffer, INT32 bufferLength);
    XERR	XFileWrite(XFILE fileRef, void *buffer, INT32 bufferLength);
    XERR	XFileSetPosition(XFILE fileRef, INT32 filePosition);
    INT32	XFileGetPosition(XFILE fileRef);
    INT32	XFileSetPositionRelative(XFILE fileRef, INT32 relativeOffset);
    INT32	XFileGetLength(XFILE fileRef);
    INT32	XFileSetLength(XFILE refRef, INT32 newSize);

    // standard string functions
    short int	XStrCmp(const char *string1, const char *string2);
    short int	XStrnCmp(const char *string1, const char *string2, INT32 n);
    char *		XStrCpy(char *dest, char *src);
    char *		XStrStr(char *source, char *pattern);
    short int	XStrLen(char *src);
    char *		XStrCat(char * dest, const char * source);

    // standard string functions, but ignore case
    short int	XLStrCmp(const char *string1, const char *string2);
    short int	XLStrnCmp(const char *string1, const char *string2, INT32 n);
    char *		XLStrStr(char *source, char *pattern);

    short int	XMemCmp(const void * src1, const void * src2, INT32 n);
    char *		XDuplicateStr(char *src);
    // Duplicate and string characters below 32
    char *		XDuplicateAndStripStr(char *src);
    void		XStripStr(char *pString);
    // This will convert a string to a base 10 long value
    INT32		XStrnToLong(char *pData, INT32 length);

    enum
    {
	X_SOURCE_ENCRYPTED = 0,				// source encrypted, destination is not encrypted
	X_SOURCE_DEST_ENCRYPTED				// source and destination encrypted
    };
    // standard strcpy, but with crypto controls
    char		*XEncryptedStrCpy(char *dest, char *src, short int copy);
    short int	XEncryptedStrLen(char *src);
    char		*XDecryptAndDuplicateStr(char *src);



    // convert a c string to a pascal string
    void * XCtoPstr(void *cstr);

    // convert a pascal string to a c string
    void * XPtoCstr(void *pstr);

    XFIXED	XFixedDivide(XFIXED divisor, XFIXED dividend);
    XFIXED	XFixedMultiply(XFIXED prodA, XFIXED prodB);

    // if TRUE, then motorola; if FALSE then intel
    XBOOL				XDetermineByteOrder(void);

    // NOTE:
    //	!!!!	These can't be turned into macros because the code accesses bytes
    //			that are not byte aligned. We need to get them byte by byte to prevent
    //			CPU's from failing.
    unsigned short 		XGetShortIntel(void const* address);
    UINT32  		XGetLongIntel(void const* address);
    unsigned short 		XGetShort(void const* address);
    UINT32  		XGetLong(void const* address);
    void				XPutShort(void *address, unsigned short value);
    void				XPutLong(void *address, UINT32 value);

    // These will swap bytes no matter the byte order
    unsigned short		XSwapShort(unsigned short value);
    UINT32  		XSwapLong(UINT32 value);

    // Type 0 - Delta encoded LZSS

    typedef enum
    {	X_RAW			= 0xFF,
	X_MONO_8		= 0,
	X_STEREO_8		= 1,
	X_MONO_16		= 2,
	X_STEREO_16		= 3
    } XCOMPRESSION_TYPE;

    // First byte is a compression type.
    // Next 3 bytes is uncompressed length.
    void*	XDecompressPtr(void * pData, UINT32 dataSize, XBOOL ignoreType);

    void	LZSSUncompress(unsigned char* src, UINT32 srcBytes,
			       unsigned char* dst, UINT32 dstBytes);
    void	LZSSUncompressDeltaMono8(unsigned char* src, UINT32 srcBytes,
					 unsigned char* dst, UINT32 dstBytes);
    void	LZSSUncompressDeltaStereo8(unsigned char* src, UINT32 srcBytes,
					   unsigned char* dst, UINT32 dstBytes);
    void	LZSSUncompressDeltaMono16(unsigned char* src, UINT32 srcBytes,
					  short* dst, UINT32 dstBytes);
    void	LZSSUncompressDeltaStereo16(unsigned char* src, UINT32 srcBytes,
					    short* dst, UINT32 dstBytes);

#if USE_CREATION_API != FALSE
    // Given a block of data and a size, this will compress it and return a new pointer. The
    // original pointer is not deallocated, and the new pointer must be deallocated when finished.
    // Will return NULL if cannot compress. First 4 bytes is uncompressed length for all types
    // except X_RAW. All other types the first byte is the type and the following 3 bytes are
    // the length of the uncompressed data. No data larger than 256 MB.
    void*	XCompressPtr(void *pData, UINT32 dataSize,
			     UINT32 *pNewSize, XCOMPRESSION_TYPE type);

    void	LZSSCompress(unsigned char* src, UINT32 srcBytes,
			     unsigned char* dst, UINT32* pDstBytes);
    void	LZSSCompressDeltaMono8(unsigned char* src, UINT32 srcBytes,
				       unsigned char* dst, UINT32* pDstBytes);
    void	LZSSCompressDeltaStereo8(unsigned char* src, UINT32 srcBytes,
					 unsigned char* dst, UINT32* pDstBytes);
    void	LZSSCompressDeltaMono16(short* src, UINT32 srcBytes,
					unsigned char* dst, UINT32* pDstBytes);
    void	LZSSCompressDeltaStereo16(short* src, UINT32 srcBytes,
					  unsigned char* dst, UINT32* pDstBytes);
#endif

    void	XPhase8BitWaveform(unsigned char * pByte, INT32 size);

    // Sound Support
    XBOOL	XIs8BitSupported(void);
    XBOOL	XIs16BitSupported(void);
    XBOOL	XIsStereoSupported(void);

#define X_FULL_VOLUME	256		// full volume (1.0)

    short int		XGetHardwareVolume(void);
    void			XSetHardwareVolume(short theVolume);

    void		XGetCompressionName(INT32 compressionType, void *cName);
    void		XGetShortCompressionName(INT32 compressionType, void *cName);

    // Mac ADPCM compression (IMA 4 to 1)
    XPTR		XAllocateCompressedAiffIma(void const* src, XDWORD srcBitsPerSample,
						   XDWORD frameCount, XDWORD channelCount);
    void		XCompressAiffIma(void const* src, XDWORD srcBitsPerSample, XBYTE* dst,
					 XDWORD frameCount, XDWORD channelCount);
    // Mac ADPCM decompression (IMA 4 to 1)
    void		XExpandAiffIma(XBYTE const* src, XDWORD srcBytesPerBlock,
				       void* dst, XDWORD dstBitsPerSample,
				       XDWORD frameCount, XDWORD channelCount);

    XDWORD		XExpandAiffImaStream(XBYTE const* src, XDWORD srcBytesPerBlock,
					     void *dst, XDWORD dstBitsPerSample,
					     XDWORD srcBytes, XDWORD channelCount,
					     short predictorCache[2]);

    // This is used for WAVE files
    XDWORD		XExpandWavIma(XBYTE const* src, XDWORD srcBytesPerBlock,
				      void* pbDst, XDWORD dstBitsPerSample,
				      XDWORD srcBytes, XDWORD channelCount);

    // u law decompression
    void		XExpandULawto16BitLinear(unsigned char *pSource, short int *pDest, INT32 frames, INT32 channels);

    // a law decompression
    void		XExpandALawto16BitLinear(unsigned char *pSource, short int *pDest, INT32 frames, INT32 channels);

    // MACE decompression
    void		XExpandMace1to6(void *inBuffer, void *outBuffer, INT32 cnt,
					void * inState, void * outState,
					UINT32 numChannels, UINT32 whichChannel);
    void		XExpandMace1to3(void *inBuffer, void *outBuffer, INT32 cnt,
					void * inState, void * outState,
					UINT32 numChannels, UINT32 whichChannel);

    // Decrypt a block of data. This should be U.S. munitions safe. ie below 40 bit
    void		XDecryptData(void *pData, UINT32 size);
    // Encrypt a block of data. This should be U.S. munitions safe. ie below 40 bit
    void		XEncryptData(void *pData, UINT32 size);

    // Random numbers
    short int	XRandom(void);					// return pseudo-random from 0 to 32767
    void		XSeedRandom(UINT32 n);	// set pseudo-random generator
    short int	XRandomRange(short int max);	// return pseudo-random from 0 to max - 1

    // Character translation functions

    // XIsWinInMac() determines whether a Macintosh character has an equivalent in the Windows character set.
    XBOOL	XIsWinInMac(char ansiChar);

    // XIsMacInWin() determines whether a Windows character has an equivalent in the Macintosh character set.
    XBOOL	XIsMacInWin(char macChar);

    // XTranslateWinToMac() provides the Macintosh-equivalent of a Windows character code.
    char	XTranslateWinToMac(char ansiChar);

    // XTranslateMacToWin() provides the Windows-equivalent of a Macintosh character code.
    char	XTranslateMacToWin(char macChar);



#ifdef __cplusplus
}
#endif

#endif	// __X_API__


