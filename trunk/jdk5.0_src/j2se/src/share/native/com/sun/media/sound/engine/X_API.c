/*
 * @(#)X_API.c	1.33 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**	X_API.c
**
**		This provides platform specfic functions
**
**	Confidential-- Internal use only
**
**	History	-
**	9/25/95		Created
**	12/14/95	Added XGetAndDetachResource
**	12/19/95	Added XSetBit & XTestBit
**	1/18/96		Spruced up for C++ extra error checking
**	1/28/96		Changed XGetAndDetachResource
**	2/3/96		Removed extra includes
**	2/11/96		Added XIsStereoSupported & XIs16BitSupported
**				Added XFixedDivide & XFixedMultiply
**				Started putting in platform defines around code functions
**	2/21/96		Changed XGetAndDetachResource to return a size
**	3/25/96		Modifed XExpandMace1to6 & XExpandMace1to3 to create silence if
**				not supported
**	3/29/96		Added XPutLong & XPutShort
**	4/20/96		Moved myFixedMultiply from GenSynth.c and moved all references
**				to XFixedMultiply
**				Moved myFixedDivide from GenSynth.c and moved all references
**				to XFixedDivide
**	4/21/96		Removed register usage in parameters
**	5/14/96		Fixed Odd address error in XIsOurMemoryPtr
**				Replaced XFixedMultiply and XFixedDivide with integer versions rather
**				than floating point
**	6/1/96		Added some resource management code
**	6/5/96		Added some Win95 stuff.
**	6/28/96		Added BeBox stuff
**	6/30/96		Changed font and re tabbed
**				Moved DecompressSampleFormatPtr from MacSpecificSMS
**				Moved XSetVolume & XGetVolume from MacSpecificSMS
**				Moved DecompressPtr from MacSpecificSMS
**	7/1/96		Added XCreateAccessCache
**				Changed XFILERESOURCEID to XFILERESOURCE_ID
**	7/2/96		Changed XFileOpenResource to support read only access
**	7/3/96		Added XStrCmp
**	7/4/96		Added error checking for opening read only resources
**	7/7/96		Added XStrnCmp
**	7/18/96		Modifed XFileOpenResource to read Cache resource
**				Fixed a NULL reference in PV_XFileValid
**				Fixed problem with cached resources and byte ordering
**	7/21/96		Fixed dumb bug in XGetFileResource that caused extra file scanning
**	7/23/96		Added XMicroseconds
**	8/6/96		Added XGetIndexedType & XCountTypes
**	8/12/96		Moved XExpandMace1to3 & XExpandMace1to6 to X_Compress.c
**	9/22/96		Added XRandom & XSeedRandom & XRandomRange
**	10/8/96		Added XStrCpy
**	10/11/96	Added XConvertNativeFileToXFILENAME
**				Added XFileSetPositionRelative
**				Added XDetermineByteOrder
**				Added XSwapLong & XSwapShort
**	10/15/96	Added XFileOpenResourceFromMemory
**	10/23/96	Changed XGetAndDetachResource to support our resource manager
**				if a file has been opened
**				Changed XFileClose to NULL out currentResourceFile when closing
**				file
**	11/7/96		Fixed a bug in XMicroseconds
**	11/8/96		Fixed bug in XStrCpy. Forgot to copy zero terminator
**	11/14/96	Added XGetResourceName & XGetNamedResource
**				Removed dependancy on MacSpecificSMS.h
**	11/26/96	Changed file read status native to MacOS from read/write
**				to read only
**	12/3/96		Removed C++ warnings on void * to char * conversion
**	12/4/96		Added XStrLen
**	12/15/96	Added X_SOLARIS
**	12/17/96	Fixed a bug in XBlockMove for WinOS.
**	12/18/96	Added all Solaris APIs
**	12/19/96	Added create flag in XFileOpenForWrite
**	12/30/96	Changed copyright
**	1/2/97		Added XCtoPstr & XPtoCstr
**	1/12/97		Renamed DecompressPtr to XDecompressPtr
**	1/13/97		Added XDuplicateStr
**	1/16/97		Fixed bug with XStrLen that added extra byte
**	1/20/97		Added XLStrCmp
**				Put in more support for MacOS native resource management
**	1/21/97		Added XLStrnCmp
**	1/23/97		Special cased XFixedMultiply to handle zero faster
**	1/24/97		Fixed XWaitMicroseocnds on WinOS to handle threads better
**	1/28/97		Changed XGetFileResource to not duplicate a resource if its
**				being loaded from memory
**				Added some Navio Code
**				Fixed a bug with XIsOurMemoryPtr (Thanks Jeff!). Forgot to
**				get the data blocks in a platform way.
**				Added XGetIndexedResource
**	1/29/97		Fixed some platform bugs with XGetIndexedFileResource
**				Added XCompressPtr
**	2/4/97		Fixed XBlockMove for Navio case. It was backwards
**	2/5/97		Added XFileOpenForReadFromMemory
**	2/6/97		Fixed a bug with XFileOpenResourceFromMemory that creates a cache,
**				its not needed for memory based resources
**	2/11/97		Modified XGetIndexedFileResource to support MacOS native resource manager
**	2/18/97		Fixed XFileOpenResource & XFileOpenResourceFromMemory to fail if file
**				is not a XFile resource
**	2/19/97		Added XStrStr
**	4/22/97		Fixed a bug with XGetPtrSize that referenced a NULL pointer
**
**	6/5/97		(Beatnik Editor Windows)
**				bvk - Changed the _creat call (in XFileOpenResource) to use the right
**				flags based on this goofy Microsoft C++ 5.0 (_S_IREAD | _S_IWRITE
**				instead of _O_RDWR).  Added an include (sys/stat.h).
**
**	6/12/97		bvk - Added XGetResourceNameOnly().  Copied/modified from other code.
**				Doesn't load the resource data, just checks resource headers.
**	6/13/97		bvk	- Added XCountFileResourcesOfType(XFILE fileRef, long int theType);
**
**	6/19/97		bvk - Fixed bug in XAddFileResource.  Wasn't flipping the 'next' value,
**					Fixed bug getting name in XGetResourceNameOnly().
**
**	6/22/97		bvk - Changed XGetPtrSize(XPTR data).  It used to innocently assume that
**					it could get the handle for any data pointer to get the size.  That
**					actually leaves a lot of room for error...
**
**	6/27/97		bvk - Added XDeleteFileResource.  This functions in two ways, either to delete
**					the resource immediately, or to just mark it as a 'TRSH' resource.
**					Then, trash collection occurs when XCleanResourceFile() is called.
**					For either case, resource deletion fails if the file has been opened as
**					read only.
**
**	7/02/97		bvk - Fixed bug in XGetResourceNameOnly().
**
**	7/03/97		bvk - Fixed bug in XCleanResourceFile() that was writing the # of resources wrong.
**
**	7/05/97		bvk - Modified the resource info caching system.  Read carefully:
**					Originally, the resource cache was only created if the res
**					file was opened ReadOnly.  Ostensibly, this was because the
**					resource map cache would be out of date should a resource add
**					or delete take place.
**
**					Now, the cache is used for all types of files.  It is updated
**					correctly should the res file have added or deleted resources.
**
**					I also rewrote some frequently called routines to use the cache.
**
**					I also disabled loading the pre-generated cache from the resource file.
**					Too many of my files didn't have 'em (and none of these routines write it out!)
**
**	5/3/97		Fixed a few potential problems that the MOT compiler found
**	5/13/97		Fixed extra byte swap with XGetIndexedFileResource. Thanks Moe!
**	6/20/97		Changed XGetHardwareVolume & XSetHardwareVolume for MacOS
**				to support news names for CW 2.0
**	6/30/97		Fixed a bug in XCompressPtr that forgot the extra four bytes! Thanks Moe!
**	7/14/97		Put in special case to XFixedDivide to handle zero values better and faster.
**				Put in support for new X_PLATFORM type, called X_WIN_HAE
**	7/17/97		Removed XIsVirtualMemoryAvailable & XLockMemory & XUnlockMemory. Because
**				its assumed that all memory is locked.
**				Removed XSetHardwareSampleRate & XGetHardwareSampleRate
**	7/18/97		Added XFileSetLength
**	7/21/97		Changed XGetResourceName to call XGetResourceNameOnly.
**	7/30/97		Removed references to creat because its not ANSI, now using the ANSI way which
**				is to call open with O_CREAT flag.
**	8/13/97		Fixed bug in XDuplicateAndStripStr that didn't increment counter the right way
**	8/18/97		Changed X_WIN_HAE to USE_HAE_EXTERNAL_API
**	9/3/97		Wired XGetHardwareVolume & XSetHardwareVolume to HAE_EXTERNAL_API
**				Wrapped a few more functions around USE_CREATION_API
**	9/29/97		Changed XSetBit & XClearBit & XTestBit to be unsigned long rather than long
**	10/2/97		Fixed a bug with XGetFileResource when trying to load a memory based resource
**				it still continued searching after it found what it was looking for.
**				Fixed XGetIndexedFileResource to support memory based resource files without
**				duplicating memory
**	10/14/97	Fixed bug with XFileOpenForWrite that didn't allow creation of files.
**	10/18/97	Modified XDisposePtr to mark blocks deallocated
**	11/19/97	Modified XNewPtr to allocate memory for performance
**	12/16/97	Moe: removed compiler warnings
**	12/17/97	bvk - Modified PV_XGetNamedCacheEntry().  Here's the problem:
**				Memory resource files don't have caches.  So how does a function
**				like XGetNamedResource(), that relies on PV_XGetNamedCacheEntry()
**				work?  IT DOESN'T.
**				PV_XGetNamedCacheEntry() now will search the whole file if for the
**				named resource if it is a memory file!
**	12/18/97	bvk - Modified XGetAndDetachResource so that in the case of memory files,
**				it truly creates a new buffer (so that calls to dispose of the resource
**				don't fail/nuke the res file).
**	1/8/98		MOE: added XSwapLongsInAccessCache()
**	1/9/98		Added XFileDelete
**	1/21/98		Changed the functions XGetShort & XGetLong & XPutShort & XPutLong
**				into macros that fall out for Motorola order hardware
**	1/26/98		Added XERR to various functions
**	1/31/98		Moved XPI_Memblock structures to X_API.h, and moved the function
**				XIsOurMemoryPtr to X_API.h
**	2/7/98		Changed XFIXED back to an unsigned long to fix broken content
**	2/9/98		Fixed bug with memory files and XFileClose in which the memory
**				block allocated outside of the resource file system was deallocated!
**	3/18/98		Added XIs8BitSupported
**	3/23/98		MOE: Changed _XGetShort() and _XGetLong() to accept const* pointers
**	3/23/98		MOE: Added _XGetShortIntel() and _XGetLongIntel()
**	3/23/98		MOE: Fixed big memory leak in XCompressPtr()
**	4/15/98		Removed the conditional compile for XGet/XPutXXX functions
**	4/20/98		Fixed a bug with XGetIndexedFileResource that return the wrong resource length
**				Fixed a alignment bug with Solaris only that caused memory to not be freed in
**				XIsOurMemoryPtr and XNewPtr. Did this by changing the size of the XPI_Memblock
**				structure to 16 bytes rather than 12.
**	4/27/98		Changed XCompressPtr to handle XCOMPRESSION_TYPE
**	4/27/98		MOE:  Changed XDecompressPtr() and XCompressPtr(),
**				eliminated XDecompressSampleFormatPtr()
**	5/7/98		Added XGetTempXFILENAME
**	5/12/98		Added some missing MacOS header files
**	6/18/98		Added XFileFreeResourceCache
**	7/1/98		Changed various API to use the new XResourceType and XLongResourceID
**	7/6/98		Fixed some type problems with XCompressPtr
**				Changed _XPutShort to pass a unsigned short rather than an unsigned long
**	7/10/98		Added XGetUniqueFileResourceID & XGetUniqueResourceID & XAddResource
**				Added XDeleteResource
**				Added XCountResourcesOfType
**				Added XCleanResource
**				Added XFileGetCurrentResourceFile
**	7/13/98		Fixed bug with XGetUniqueFileResourceID & XRandom
**	7/15/98		Fixed bug with XGetResourceName inwhich the name being returned was a
**				pascal string, rather than a C string
**	7/17/98		Fixed bug with XCleanResourceFile inwhich a errant buffer might be
**				deallocated twice
**	7/20/98		Fixed a problem with XConvertNativeFileToXFILENAME that didn't clear
**				the XFILENAME if there was a problem
**	9/10/98		Put XGetFileAsData into the common build code because HAE.cpp uses it,
**				and the new exporter API uses it.
**				Fixed XGetIndexedType & XCountTypes to handle a NULL file reference being passed
**				in meaning use from most recently opened file.
**	9/30/98		Added test to XNewPtr to fail if size passed in is zero.
**	11/11/98	Cleaned up some left over bark in XGetAndDetachResource. Renamed pName to pResourceName
**				because of a MacOS macro conflict.
**				Added XReadPartialFileResource
**				Removed unused macros XGet/Put and placed them back into
**				real functions
**	11/18/98	Fixed a bug with XFileClose in which a file that was closed
**				twice would crash rather than get ignored.
**	12/21/98	Fixed a memory leak in XCleanResourceFile
**	12/22/98	Added USE_FILE_CACHE to control cache usage
**				Modified XCleanResourceFile to delete the cache resource if there is one, because
**				it can become invalid after a clean or delete. Also modified XAddFileResource to
**				delete the file cache
**	1/5/98		Fixed a warning in XFileClose and changed copyrights
**	2/8/99		Added XLStrStr & XStrCat
**	2/21/99		Added XStripStr & XFileCreateResourceCache
**	3/1/99		Removed some warnings for BeOS compiles
**	2002-03-14	$$fb added support for IA64, sparcv9 architectures
**			added explicit support for Linux
*/
/*****************************************************************************/

#include "X_API.h"
#include "X_Formats.h"

#if X_PLATFORM == X_WEBTV
#include "Headers.h"
#include "ObjectStore.h"
#include "MemoryManager.h"
#endif

#if X_PLATFORM == X_WEBTV
#include "MacintoshUndefines.h"
#include <stdio.h>
#include <string.h>
#endif

#if X_PLATFORM == X_MACINTOSH
#include <Resources.h>
#include <Memory.h>
#include <Gestalt.h>
#include <ToolUtils.h>
#include <Sound.h>
#include <Timer.h>
#include <Folders.h>
#include <NumberFormatting.h>
#endif

#if X_PLATFORM == X_WEBTV
#include "MacintoshRedefines.h"
#endif

#if (X_PLATFORM == X_WINDOWS) || (X_PLATFORM == X_WIN_HARDWARE)
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <mmsystem.h>	// for timeGetTime()
#endif

#if USE_HAE_EXTERNAL_API == TRUE
#include "HAE_API.h"
#endif

#if X_PLATFORM == X_BE
#include <malloc.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#if (X_PLATFORM == X_SOLARIS) || (X_PLATFORM == X_LINUX)
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#endif

#if X_PLATFORM == X_NAVIO
#include <stdlib.h>
#include <kernel/Thread.h>
#endif



#define DEBUG_PRINT_RESOURCE		        0
#define USE_WIN32_FILE_IO			1
#define USE_FILE_CACHE				1	// if 1, then file cache is enabled

// Structures

#define MAX_OPEN_XFILES		5	// max number of open resource files

// Variables
static short int	resourceFileCount = 0;				// number of open resource files
static XFILE		openResourceFiles[MAX_OPEN_XFILES];

// Private functions

// Check for a valid file reference
static XBOOL PV_XFileValid(XFILE fileRef)
{
    XFILENAME	*pReference;
    XBOOL		valid;

    valid = FALSE;
    pReference = (XFILENAME *)fileRef;
    if (pReference)
	{
	    int			code;
#if USE_HAE_EXTERNAL_API == TRUE
	    code = HAE_IsBadReadPointer(&pReference->fileValidID, (INT32)sizeof(INT32));
#else
	    code = 0;	// ok
#if (X_PLATFORM == X_WINDOWS) || (X_PLATFORM == X_WIN_HARDWARE)
	    if (IsBadReadPtr(&pReference->fileValidID, (INT32)sizeof(INT32)))
		{
		    code = 1;	// bad
		}
#endif
#endif
	    if (code == 0 || code == 2)
		{
		    if (pReference->fileValidID == XPI_BLOCK_3_ID)
			{
			    valid = TRUE;
			}
		}
	}
    return valid;
}

// Given a valid XFILE, this will return the open index. Will return -1 if not valid
static short int PV_FindResourceFileReferenceIndex(XFILE fileRef)
{
    short int	count;

    for (count = 0; count < resourceFileCount; count++)
	{
	    if (openResourceFiles[count] == fileRef)
		{
		    return count;
		}
	}
    return -1;
}

static XBOOL PV_AddResourceFileToOpenFiles(XFILE fileRef)
{
    XBOOL		full;
    short int	count;

    full = TRUE;
    if (resourceFileCount < MAX_OPEN_XFILES)
	{
	    for (count = MAX_OPEN_XFILES-2; count >= 0; count--)
		{
		    openResourceFiles[count+1] = openResourceFiles[count];
		}
	    openResourceFiles[0] = fileRef;
	    resourceFileCount++;
	    full = FALSE;
	}
    return full;
}

static void PV_RemoveResourceFileFromOpenFiles(XFILE fileRef)
{
    short int	count;
    short int	found;

    found = -1;
    for (count = 0; count < resourceFileCount; count++)
	{
	    if (openResourceFiles[count] == fileRef)
		{
		    found = count;
		    break;
		}
	}
    if (found != -1)
	{
	    for (count = found; count < resourceFileCount-1; count++)
		{
		    openResourceFiles[count] = openResourceFiles[count+1];
		}
	    openResourceFiles[count] = 0;
	    resourceFileCount--;
	}
}

static INLINE XBOOL PV_IsAnyOpenResourceFiles(void)
{
    return (resourceFileCount) ? TRUE : FALSE;
}

// Functions

// convert a c string to a pascal string
void * XCtoPstr(register void *cstr)
{
    register char *	source;
    register char *	dest;
    register INT32	length;
    char			data[256];

    if (cstr)
	{
	    source = (char *)cstr;
	    dest = data+1;
	    length = 0;
	    while (*source)
		{
		    length++;
		    *dest++ = *source++;
		}
	    data[0] = (unsigned char)length;
	    XBlockMove(data, cstr, length + 1);
	}
    return cstr;
}

// convert a pascal string to a c string
void * XPtoCstr(register void *pstr)
{
    register char *	source;
    register char *	dest;
    register INT32	length;
    char			data[256];

    if (pstr)
	{
	    length = *((unsigned char *)pstr);
	    source = (char *)pstr;
	    source++;
	    dest = data;

	    while (length--)
		{
		    *dest++ = *source++;
		}
	    *dest = 0;
	    length = *((unsigned char *)pstr) + 1;
	    XBlockMove(data, pstr, length);
	}
    return pstr;
}


#if X_PLATFORM == X_MACINTOSH
static XBOOL PV_IsVirtualMemoryAvailable(void)
{
    static INT32		cachedResult = -1;
    INT32			feature;

    if (cachedResult == -1)
	{
	    feature = 0;
	    if (Gestalt(gestaltVMAttr, &feature) == noErr)
		{
		    if (feature & (1<<gestaltVMPresent))
			{
			    cachedResult = TRUE;
			}
		    else
			{
			    cachedResult = FALSE;
			}
		}
	}
    return (XBOOL)cachedResult;
}
#endif

// Determine if a data block was allocated with our memory allocation API
XPI_Memblock * XIsOurMemoryPtr(XPTR data)
{
    char			*pData;
    XPI_Memblock	*pBlock, *pBlockReturn;
#if USE_HAE_EXTERNAL_API == TRUE
    short int		code;
#endif
    pBlockReturn = NULL;
    if (data)
	{
	    pData = (char *)data;
	    pData -= sizeof(XPI_Memblock);
	    pBlock = (XPI_Memblock *)pData;
#if USE_HAE_EXTERNAL_API == TRUE
	    code = HAE_IsBadReadPointer(pBlock, (INT32)sizeof(XPI_Memblock));
	    if (code == 0 || code == 2)
#else
#if (X_PLATFORM == X_WINDOWS) || (X_PLATFORM == X_WIN_HARDWARE)
		if (!IsBadReadPtr(pBlock, (INT32)sizeof(XPI_Memblock)))
#endif
#endif
		    {
			if ( (XGetLong(&pBlock->blockID_one) == XPI_BLOCK_1_ID) &&
			     (XGetLong(&pBlock->blockID_two) == XPI_BLOCK_2_ID) )
			    {
				pBlockReturn = pBlock;
			    }
		    }
	}
    return pBlockReturn;
}


// Allocates a block of ZEROED!!!! memory and locks it down
XPTR XNewPtr(INT32 size)
{
    char			*data;
    XPI_Memblock	*pBlock;

    if (size == 0)
	{
	    return NULL;
	}
    size += (INT32)sizeof(XPI_Memblock);
#if USE_HAE_EXTERNAL_API == TRUE
    data = (char *)HAE_Allocate(size);
#else
#if X_PLATFORM == X_MACINTOSH
    data = (Ptr)NewPtrClear(size);
    if (data)
	{
	    if (PV_IsVirtualMemoryAvailable())
		{
		    HoldMemory(data, size);
		    LockMemory(data, size);
		}
	}
#endif
#if X_PLATFORM == X_WIN_HARDWARE
    data = (char *)BobAllocate(size);
    if (data)
	{
	    //		LockMemory(data, size);
	    XSetMemory(data, size, 0);
	}
#endif
#if X_PLATFORM == X_WINDOWS
    data = (char *)GlobalAllocPtr(GHND, size);
    // the GHND flag includes the GMEM_ZEROINIT flag
    //	VirtualLock(data, size);
#endif
#if X_PLATFORM == X_WEBTV
    data = (char *)AllocateZero(size);
#endif
#if X_PLATFORM == X_BE
    data = (char *)malloc(size);
    if (data)
	{
	    XSetMemory(data, size, 0);
	}
#endif
#if (X_PLATFORM == X_SOLARIS) || (X_PLATFORM == X_LINUX)
    // $$kk: 10.14.97
    // changed this line as per Liang He's performance recommendations
    //	data = (char *)malloc((size_t)size);

    // $$kk: 10.23.97
    // changed th is line as per Liang He's performance revommendations.
    // buffer should be 8-byte aligned, not 4-byte aligned.
    //data = (char *)memalign(8, (size_t)size);

    data = (char *)memalign(8, (size_t)size + 8);
    if (data)
	{
	    XSetMemory(data, size, 0);
	}
#endif
#if X_PLATFORM == X_NAVIO
    data = (char *)malloc(size);
    if (data)
	{
	    XSetMemory(data, size, 0);
	}
#endif
#endif
    if (data)
	{
	    pBlock = (XPI_Memblock *)data;
	    XPutLong(&pBlock->blockID_one, XPI_BLOCK_1_ID);			// set our ID for this block
	    XPutLong(&pBlock->blockID_two, XPI_BLOCK_2_ID);
	    data += sizeof(XPI_Memblock);
	    pBlock->blockSize = size - (INT32)sizeof(XPI_Memblock);
	}
    return (XPTR)data;
}

void XDisposePtr(XPTR data)
{
    INT32			size;
    XPTR			osAllocatedData;
    XPI_Memblock	*pBlock;

    osAllocatedData = (XPTR)XIsOurMemoryPtr(data);
    if (osAllocatedData)
	{
	    size = XGetPtrSize(data);	// need to get the size before we translate the pointer

	    pBlock = (XPI_Memblock *)osAllocatedData;
	    XPutLong(&pBlock->blockID_one, (UINT32)XPI_DEAD_ID);			// set our ID for this block
	    XPutLong(&pBlock->blockID_two, (UINT32)XPI_DEAD_ID);			// to be dead. Used for tracking
#if USE_HAE_EXTERNAL_API == TRUE
	    HAE_Deallocate(osAllocatedData);
#else
#if X_PLATFORM == X_MACINTOSH
	    if (PV_IsVirtualMemoryAvailable())
		{
		    size = GetPtrSize((char *)osAllocatedData);
		    UnholdMemory(osAllocatedData, size);
		    UnlockMemory(osAllocatedData, size);
		}
	    DisposePtr((Ptr)osAllocatedData);
#endif
#if X_PLATFORM == X_WIN_HARDWARE
	    BobFree(osAllocatedData, size);
#endif
#if X_PLATFORM == X_WINDOWS
	    //		VirtualUnlock(osAllocatedData, XGetPtrSize(data));
	    GlobalFreePtr(osAllocatedData);
#endif
#if X_PLATFORM == X_WEBTV
	    FreeMemory(osAllocatedData);
#endif
#if X_PLATFORM == X_BE
	    free(osAllocatedData);
#endif
#if (X_PLATFORM == X_SOLARIS) || (X_PLATFORM == X_LINUX)
	    free(osAllocatedData);
#endif
#if X_PLATFORM == X_NAVIO
	    free(osAllocatedData);
#endif
#endif
	}
}

INT32 XGetPtrSize(XPTR data)
{
    INT32		size;
    XPTR		odata;

    size = 0;
    if (data)
	{
	    odata = (XPTR)XIsOurMemoryPtr(data);
	    if (odata)
		{
		    size = ((XPI_Memblock *)odata)->blockSize;
		}
	    else
		{
		    // then this block is not ours, so use the system to determine its real size
#if USE_HAE_EXTERNAL_API == TRUE
		    size = HAE_SizeOfPointer(data);
#else
#if X_PLATFORM == X_MACINTOSH
		    size = GetPtrSize((Ptr)data);
#endif
#if X_PLATFORM == X_WEBTV
		    size = MemorySize(data);
#endif
#if X_PLATFORM == X_WIN_HARDWARE
		    size = 0;
#endif
#if X_PLATFORM == X_WINDOWS
		    HANDLE	hData;	//bvk

		    hData = GlobalPtrHandle(data);
		    if (hData)
			{
			    size = GlobalSize(hData);
			}
#endif
#if X_PLATFORM == X_BE
		    size = 0;
#endif
#if (X_PLATFORM == X_SOLARIS) || (X_PLATFORM == X_LINUX)
		    size = 0;
#endif
#if X_PLATFORM == X_NAVIO
		    size = 0;
#endif
#endif
		}
	}
    return size;
}

// Typical block move, expect it must be able to handle overlapping source and
// destination pointers
void XBlockMove(XPTR source, XPTR dest, INT32 size)
{
    if (source && dest && size)
	{
#if USE_HAE_EXTERNAL_API == TRUE
	    HAE_BlockMove(source, dest, size);
#else
#if X_PLATFORM == X_MACINTOSH
	    BlockMoveData(source, dest, size);
#endif
#if X_PLATFORM == X_WEBTV
	    CopyMemory(source, dest, size);
#endif
#if (X_PLATFORM == X_WINDOWS) || (X_PLATFORM == X_WIN_HARDWARE)
	    memcpy(dest, source, size);
#endif
#if X_PLATFORM == X_BE
	    memcpy(dest, source, size);
#endif
#if (X_PLATFORM == X_SOLARIS) || (X_PLATFORM == X_LINUX)
	    memcpy(dest, source, (size_t)size);
#endif
#if X_PLATFORM == X_NAVIO
	    memcpy(dest, source, size);
#endif
#endif
	}
}

// set memory range with value
void XSetMemory(void *pAdr, INT32 len, char value)
{
    register char *pData;

    if (pAdr && (len > 0))
	{
	    pData = (char *)pAdr;
	    do
		{
		    *pData++ = value;
		    len--;
		}
	    while (len);
	}
}

// Given a pointer, and a bit number; this will set that bit to 1
void XSetBit(void *pBitArray, UINT32 whichbit)
{
    UINT32	byteindex, bitindex;
    unsigned char *byte;

    if (pBitArray)
	{
	    byteindex = whichbit / 8;
	    bitindex = whichbit % 8;
	    byte = &((unsigned char *)pBitArray)[byteindex];
	    *byte |= (1L << bitindex);
	}
}

// Given a pointer, and a bit number; this will set that bit to 0
void XClearBit(void *pBitArray, UINT32 whichbit)
{
    UINT32	byteindex, bitindex;
    unsigned char *byte;

    if (pBitArray)
	{
	    byteindex = whichbit / 8;
	    bitindex = whichbit % 8;
	    byte = &((unsigned char *)pBitArray)[byteindex];
	    *byte &= ~(1L << bitindex);
	}
}

// Given a pointer, and a bit number; this return the value of that bit
XBOOL XTestBit(void *pBitArray, UINT32 whichbit)
{
    register UINT32	byteindex, byte, bitindex;

    if (pBitArray)
	{
	    byteindex = whichbit / 8;
	    bitindex = whichbit % 8;
	    byte = ((unsigned char *)pBitArray)[byteindex];
	    return (byte & (1L << bitindex)) ? TRUE : FALSE;
	}
    else
	{
	    return FALSE;
	}
}

/* Sort an integer array from the lowest to the highest
*/
#if USE_CREATION_API == TRUE
void XBubbleSortArray(short int theArray[], short int theCount)
{
    register short int i, j, swapValue;

    for (i = 0; i < (theCount - 1); i++)
	{
	    for (j = i + 1; j < theCount; j++)
		{
		    if (theArray[i] > theArray[j])
			{
			    swapValue = theArray[i];
			    theArray[i] = theArray[j];
			    theArray[j] = swapValue;
			}
		}
	}
}
#endif	// USE_CREATION_API


// Does sound hardware support stereo output
XBOOL XIsStereoSupported(void)
{
#if USE_HAE_EXTERNAL_API == TRUE
    return HAE_IsStereoSupported();
#else
#if X_PLATFORM == X_MACINTOSH
    static INT32		cachedResult = -1;
    INT32			feature;

    if (cachedResult == -1)
	{
	    feature = 0;
	    cachedResult = FALSE;
	    if (Gestalt(gestaltSoundAttr, &feature) == noErr)
		{
		    if (feature & (1L<<gestaltStereoCapability))
			{
			    cachedResult = TRUE;
			}
		}
	}
    return (XBOOL)cachedResult;
#endif
#if ( (X_PLATFORM == X_WINDOWS) 			||	\
		  (X_PLATFORM == X_WIN_HARDWARE)	||	\
		  (X_PLATFORM == X_WEBTV)			||	\
		  (X_PLATFORM == X_BE)				||	\
		  (X_PLATFORM == X_SOLARIS)			||	\
		  (X_PLATFORM == X_LINUX)			||	\
		  (X_PLATFORM == X_NAVIO) )
    return TRUE;
#endif
#endif
}

// wait for a waitAmount of microseconds to pass
// CLS??: If this function is called from within the frame thread and
// JAVA_THREAD is non-zero, we'll probably crash.
void XWaitMicroseocnds(UINT32 waitAmount)
{
#if USE_HAE_EXTERNAL_API == TRUE
    HAE_WaitMicroseocnds(waitAmount);
#else
#if X_PLATFORM == X_BE
    snooze((double)waitAmount);		// wait microseconds
#endif
#if X_PLATFORM == X_NAVIO
    ThreadSleep( waitAmount/1000 );
#endif
#if X_PLATFORM == X_WINDOWS
    UINT32	ticks;

    ticks = XMicroseconds() + waitAmount;
    while (XMicroseconds() < ticks)
	{
	    Sleep(0);	// Give up the rest of this time slice to other threads
	}
#else
    UINT32	ticks;

    ticks = XMicroseconds() + waitAmount;
    while (XMicroseconds() < ticks) {}
#endif
#endif
}

#if X_PLATFORM == X_WINDOWS
static UINT32 PV_GetMicrosecondTick(void)
{
    return timeGetTime() * 1000;
#if 0
    LARGE_INTEGER	freq, tick;
    UINT32	ms;
    double			freq_d, tick_d;
    ms = 0;
    if (QueryPerformanceFrequency(&freq) == FALSE)
	{	// does not support this type of timer
	    ms = timeGetTime() * 1000;	// timeGetTime only returns milliseconds and we want microseconds
	}
    else
	{
	    freq_d = (double)freq.LowPart;
	    QueryPerformanceCounter(&tick);
	    tick_d = (double)tick.LowPart;
	    ms = (tick_d / freq_d * 1000.0);
	}

    return ms;
#endif

}
#endif

#if X_PLATFORM == X_WIN_HARDWARE
extern INT32 hardwareTicks;
#endif


// Returns microseconds since boot
// $$fb ????
// 1 second = 10 milliseconds = 1000 microseconds
UINT32 XMicroseconds(void)
{
#if USE_HAE_EXTERNAL_API == TRUE
    return HAE_Microseconds();
#else
#if X_PLATFORM == X_MACINTOSH
    static UINT32	starttick = 0;
    UnsignedWide			tick;

    if (starttick == 0)
	{
	    Microseconds(&tick);
	    starttick = tick.lo;
	}
    Microseconds(&tick);
    return tick.lo - starttick;
#endif
#if X_PLATFORM == X_WINDOWS
    static UINT32 starttick = 0;

    if (starttick == 0)
	{
	    starttick = PV_GetMicrosecondTick();
	}
    return (PV_GetMicrosecondTick() - starttick);
#endif
#if X_PLATFORM == X_BE
    static double betick = 0;

    if (betick == 0)
	{
	    betick = system_time();
	}
    return (UINT32) (system_time() - betick);
#endif
#if X_PLATFORM == X_WIN_HARDWARE
    static UINT32 starttick = 0;

    if (starttick == 0)
	{
	    starttick = hardwareTicks;
	}
    return (hardwareTicks - starttick);
#endif
#if (X_PLATFORM == X_SOLARIS) || (X_PLATFORM == X_LINUX)
    static hrtime_t solaristick = 0;

    if (solaristick == 0)
	{
	    solaristick = gethrtime();
	}
    /* ghrtime is in nanoseconds, we want microseconds. */
    return (UINT32) ((gethrtime() - solaristick) / 1000);
#endif
#endif
}

// Does sound hardware support 16 bit output
XBOOL XIs16BitSupported(void)
{
#if USE_HAE_EXTERNAL_API == TRUE
    return HAE_Is16BitSupported();
#else
#if X_PLATFORM == X_MACINTOSH
    static INT32		cachedResult = -1;
    INT32			feature;

    if (cachedResult == -1)
	{
	    feature = 0;
	    cachedResult = FALSE;
	    if (Gestalt(gestaltSoundAttr, &feature) == noErr)
		{
		    if (feature & (1L<<gestalt16BitAudioSupport))
			{
			    cachedResult = TRUE;
			}
		}
	}
    return (XBOOL)cachedResult;
#endif
#if ( (X_PLATFORM == X_WINDOWS) 			||	\
		  (X_PLATFORM == X_WIN_HARDWARE)	||	\
		  (X_PLATFORM == X_BE)				||	\
		  (X_PLATFORM == X_SOLARIS)			||	\
		  (X_PLATFORM == X_LINUX)			||	\
		  (X_PLATFORM == X_WEBTV)			||	\
		  (X_PLATFORM == X_NAVIO) )
    return TRUE;
#endif
#endif
}

// Does sound hardware support 8 bit output
XBOOL XIs8BitSupported(void)
{
#if USE_HAE_EXTERNAL_API == TRUE
    return HAE_Is8BitSupported();
#else
    return TRUE;
#endif
}

#define PV_USE_FLOAT		0
// do a 16.16 fixed point multiply
XFIXED XFixedMultiply(XFIXED factor1, XFIXED factor2)
{
    XFIXED	result;

    if (factor1 && factor2)
#if PV_USE_FLOAT
	{
	    double			product;

	    product = factor1;
	    product *= factor2;
	    product /= XFIXED_1;
	    result = (XFIXED)product;
	}
#else
    {
	result = (((factor1 >> 16L) & 0xFFFFL) * ((factor2 >> 16L) & 0xFFFFL)) << 16L;
	result += ((factor1 >> 16L) & 0xFFFFL) * ( factor2 & 0xFFFFL);
	result += (factor1 & 0xFFFFL) * ((factor2 >> 16L) & 0xFFFFL);
	result += (((factor1 & 0xFFFFL) * (factor2 & 0xFFFFL)) >> 16L) & 0xFFFFL;
    }
#endif
    else
	{
	    result = 0;
	}
    return result;
}

// do a 16.16 fixed point divide
XFIXED XFixedDivide(XFIXED dividend, XFIXED divisor)
{
    XFIXED	result;

    if (divisor && dividend)
#if PV_USE_FLOAT
	{
	    double	quotient;

	    quotient = dividend;
	    quotient /= divisor;
	    quotient *= XFIXED_1;
	    result = (XFIXED)quotient;
	}
#else
    {
	XFIXED		temp, rfactor, f2;
	int			i;

	f2 = divisor;
	temp = dividend;
	result = 0;
	rfactor = 0x10000L;
	for (i = 0; i < 16; i++)
	    {
		while ((temp >= f2) && (rfactor != 0) && (temp != 0))
		    {
			temp -= f2;
			result += rfactor;
		    }
		f2 = f2 >> 1L;
		rfactor = rfactor >> 1L;
	    }
    }
#endif
    else
	{
	    result = 0;
	}
    return result;
}

// given a pointer, get a short int ordered in an Intel way
unsigned short int XGetShortIntel(void const* pData)
{
    register unsigned char	*pByte;
    register unsigned short	data;

    pByte = (unsigned char *)pData;
    data = ((unsigned short)pByte[1] << 8) |
	(unsigned short)pByte[0];
    return data;
}
// given a pointer, get a long ordered in an Intel way
UINT32 XGetLongIntel(void const* pData)
{
    register unsigned char	*pByte;
    register UINT32	data;

    pByte = (unsigned char *)pData;
    data = ((UINT32)pByte[3] << 24L) |
	((UINT32)pByte[2] << 16L) |
	((UINT32)pByte[1] << 8L) |
	(UINT32)pByte[0];
    return data;
}

// given a pointer, get a short int ordered in a Motorola way
unsigned short int XGetShort(void const* pData)
{
    register unsigned char	*pByte;
    register unsigned short	data;

    pByte = (unsigned char *)pData;
    data = ((unsigned short int)pByte[0] << 8) |
	(unsigned short int)pByte[1];
    return data;
}
// given a pointer, get a long ordered in a Motorola way
UINT32 XGetLong(void const* pData)
{
    register unsigned char	*pByte;
    register UINT32	data;

    pByte = (unsigned char *)pData;
    data = ((UINT32)pByte[0] << 24L) |
	((UINT32)pByte[1] << 16L) |
	((UINT32)pByte[2] << 8L) |
	(UINT32)pByte[3];
    return data;
}
// given a pointer and a value, this with put a short in a ordered 68k way
void XPutShort(void *pData, unsigned short data)
{
    register unsigned char	*pByte;

    pByte = (unsigned char *)pData;
    pByte[0] = (UBYTE)((data >> 8) & 0xFF);
    pByte[1] = (UBYTE)(data & 0xFF);
}

// given a pointer and a value, this with put a long in a ordered 68k way
void XPutLong(void *pData, UINT32 data)
{
    register unsigned char	*pByte;

    pByte = (unsigned char *)pData;
    pByte[0] = (UBYTE)((data >> 24) & 0xFF);
    pByte[1] = (UBYTE)((data >> 16) & 0xFF);
    pByte[2] = (UBYTE)((data >> 8) & 0xFF);
    pByte[3] = (UBYTE)(data & 0xFF);
}

UINT32 XSwapLong(UINT32 value)
{
    UINT32	newValue;
    unsigned char	*pOld;
    unsigned char	*pNew;
    unsigned char 	temp;

    pOld = (unsigned char *)&value;
    pNew = (unsigned char *)&newValue;

    temp = pOld[0];
    pNew[3] = temp;
    temp = pOld[1];
    pNew[2] = temp;
    temp = pOld[2];
    pNew[1] = temp;
    temp = pOld[3];
    pNew[0] = temp;

    return newValue;
}

unsigned short XSwapShort(unsigned short value)
{
    unsigned short	newValue;
    unsigned char	*pOld;
    unsigned char	*pNew;
    unsigned char 	temp;

    pOld = (unsigned char *)&value;
    pNew = (unsigned char *)&newValue;

    temp = pOld[0];
    pNew[1] = temp;
    temp = pOld[1];
    pNew[0] = temp;

    return newValue;
}

// if TRUE, then motorola; if FALSE then intel
XBOOL XDetermineByteOrder(void)
{
    static INT32	value = 0x12345678;
    XBOOL		order;

    order = FALSE;
    if (XGetLong(&value) == 0x12345678)
	{
	    order = TRUE;
	}
    return order;
}

// given a native file spec (FSSpec for MacOS, and 'C' string for WinOS, fill in a XFILENAME
void XConvertNativeFileToXFILENAME(void *file, XFILENAME *xfile)
{
    if (xfile)
	{
	    XSetMemory(xfile, (INT32)sizeof(XFILENAME), 0);
	}
    if (file)
	{
#if USE_HAE_EXTERNAL_API == TRUE
	    {
		void	*dest;

		dest = &xfile->theFile;
		HAE_CopyFileNameNative(file, dest);
	    }
#else
#if X_PLATFORM == X_MACINTOSH
	    xfile->theFile = *((FSSpec *)file);
#endif
#if ( (X_PLATFORM == X_WINDOWS) 			||	\
		  (X_PLATFORM == X_WIN_HARDWARE)	||	\
		  (X_PLATFORM == X_BE)				||	\
		  (X_PLATFORM == X_SOLARIS)			||	\
		  (X_PLATFORM == X_LINUX)			||	\
		  (X_PLATFORM == X_NAVIO) )
	    XStrCpy((char *)xfile->theFile, (char *)file);
#endif
#endif
	}
}

// Read a file into memory and return an allocated pointer.
// 0 is ok, -1 failed to open, -2 failed to read, -3 failed memory
// if 0, then *pData is valid
XERR XGetFileAsData(XFILENAME *pResourceName, XPTR *ppData, INT32 *pSize)
{
    XFILE	ref;
    INT32	size;
    XERR	error;
    XPTR	pData;

    error = -3;	// failed memory
    pData = NULL;
    if (pResourceName && pSize && ppData)
	{
	    *pSize = 0;
	    *ppData = NULL;
	    ref = XFileOpenForRead(pResourceName);
	    if (ref)
		{
		    size = XFileGetLength(ref);
		    pData = XNewPtr(size);
		    if (pData)
			{
			    if (XFileRead(ref, pData, size))
				{
				    error = -2;	// failed to read
				    XDisposePtr(pData);
				    pData = NULL;
				}
			    else
				{
				    error = 0;	// ok
				}
			}
		    else
			{
			    error = -3;	// failed memory
			}
		    XFileClose(ref);
		    *pSize = size;
		}
	    else
		{
		    error = -1;	// failed open
		}
	    *ppData = pData;
	}
    return error;
}

#if USE_CREATION_API == TRUE
// Create a temporary file name and fill an XFILENAME structure. Return -1 for failure, or 0 for sucess.
XERR XGetTempXFILENAME(XFILENAME* xfilename)
{
#if X_PLATFORM == X_MACINTOSH
    short			vRefNum;
    INT32			theDirID;
    OSErr			err;
    FSSpec			fileSpec;
    char			name[64], name2[32];
    int				length;

    // point to the prefs folder
    err = FindFolder(kOnSystemDisk, kTemporaryFolderType, kCreateFolder, &vRefNum, &theDirID);
    if (err == noErr)
	{
	    XStrCpy(name, "HAE_TEMP_");
	    length = XStrLen(name);

	    NumToString(XMicroseconds() & 0x3FFF, (unsigned char *)name2);	// create a text plot based upon time
	    XStrCpy(&name[length], (char *)XPtoCstr(name2));				// concatinate it to name

	    // Create FSSpec structure from file name
	    XSetMemory(&fileSpec, (INT32)sizeof(FSSpec), 0);
	    err = FSMakeFSSpec(vRefNum, theDirID, (unsigned char *)XCtoPstr(name), &fileSpec);
	    if (err == noErr)
		{
		    XConvertNativeFileToXFILENAME(&fileSpec, xfilename);
		    return 0;	// success
		}
	}
#endif
#if (X_PLATFORM == X_WINDOWS) || (X_PLATFORM == X_WIN_HARDWARE)
    TCHAR			directory[MAX_PATH];
    TCHAR			path[MAX_PATH];
    unsigned int	length;

    length = GetTempPath(MAX_PATH, directory);
    if ((length != 0) && (length < MAX_PATH))
	{
	    if (GetTempFileName(directory, "HAE", 0, path) != 0)
		{
		    XConvertNativeFileToXFILENAME(path, xfilename);
		    return 0;	// success
		}
	}
#endif
    return -1;	// failure
}
#endif

XFILE XFileOpenResourceFromMemory(XPTR pResource, UINT32 resourceLength, XBOOL allowCopy)
{
    XFILENAME			*pReference;
    XFILERESOURCEMAP	map;
    short int			err;

    err = 0;
    pReference = (XFILENAME *)XNewPtr((INT32)sizeof(XFILENAME));
    if (pReference)
	{
	    pReference->pResourceData = pResource;
	    pReference->resMemLength = resourceLength;
	    pReference->resMemOffset = 0;
	    pReference->resourceFile = TRUE;
	    pReference->allowMemCopy = allowCopy;
	    pReference->fileValidID = XPI_BLOCK_3_ID;
	    if (pReference)
		{
		    if (PV_AddResourceFileToOpenFiles((XFILE)pReference))
			{	// can't open any more files
			    err = 1;
			}
		    else
			{
			    pReference->pCache = NULL;
				// since we are pointer based, we don't care about caching.

				// validate resource file
			    XFileSetPosition((XFILE)pReference, 0L);		// at start
			    if (XFileRead((XFILE)pReference, &map, (INT32)sizeof(XFILERESOURCEMAP)) == 0)
				{
				    if (XGetLong(&map.mapID) != XFILERESOURCE_ID)
					{
					    err = 2;
					}
				}
			    else
				{
				    err = 3;
				}
			}
		}
	    if (err)
		{
		    XDisposePtr(pReference);
		    pReference = NULL;
		}
	}
    return (XFILE)pReference;
}

XFILE XFileOpenResource(XFILENAME *file, XBOOL readOnly)
{
    XFILENAME			*pReference;
    XFILERESOURCEMAP	map;

    pReference = (XFILENAME *)XNewPtr((INT32)sizeof(XFILENAME));
    if (pReference)
	{
	    *pReference = *file;
	    pReference->resourceFile = TRUE;
	    pReference->fileValidID = XPI_BLOCK_3_ID;
	    pReference->pResourceData = NULL;
	    pReference->allowMemCopy = TRUE;
	    pReference->readOnly = readOnly;

#if USE_HAE_EXTERNAL_API == TRUE
	    if (readOnly)
		{
		    pReference->fileReference = HAE_FileOpenForRead((void *)&pReference->theFile);
		    if (pReference->fileReference == -1)
			{
			    XDisposePtr(pReference);
			    pReference = NULL;
			}
		}
	    else
		{
		    pReference->fileReference = HAE_FileOpenForReadWrite((void *)&pReference->theFile);
		    if (pReference->fileReference == -1)
			{
				// must not be there, so create it and prepare it as a resource file
			    HAE_FileCreate((void *)&pReference->theFile);
			    pReference->fileReference = HAE_FileOpenForReadWrite((void *)&pReference->theFile);
			    if (pReference->fileReference == -1)
				{
				    XDisposePtr(pReference);
				    pReference = NULL;
				}
			    else
				{
				    // prepare it as a resource file
				    XFileSetPosition((XFILE)pReference, 0L);		// at start
				    XPutLong(&map.mapID, XFILERESOURCE_ID);
				    XPutLong(&map.version, 1);
				    XPutLong(&map.totalResources, 0);
				    XFileWrite((XFILE)pReference, &map, (INT32)sizeof(XFILERESOURCEMAP));
				}
			}
		}
#else

#if X_PLATFORM == X_MACINTOSH
	    {
		short int			err;

		if (readOnly)
		    {
			err = FSpOpenDF(&pReference->theFile, fsRdPerm, &pReference->fileReference);
			if (err)
			    {
				XDisposePtr(pReference);
				pReference = NULL;
			    }
		    }
		else
		    {
			err = FSpOpenDF(&pReference->theFile, fsRdWrPerm, &pReference->fileReference);
			if (err)
			    {
				// must not be there, so create it and prepare it as a resource file
				FSpDelete(&pReference->theFile);
				err = FSpCreate(&pReference->theFile, 'MPLR', 'FLAT', 0);
				if (err == noErr)
				    {
					err = FSpOpenDF(&pReference->theFile, fsRdWrPerm, &pReference->fileReference);
					if (err)
					    {
						XDisposePtr(pReference);
						pReference = NULL;
					    }
					else
					    {
						// prepare it as a resource file
						XFileSetPosition((XFILE)pReference, 0L);		// at start
						XPutLong(&map.mapID, XFILERESOURCE_ID);
						XPutLong(&map.version, 1);
						XPutLong(&map.totalResources, 0);
						XFileWrite((XFILE)pReference, &map, (INT32)sizeof(XFILERESOURCEMAP));
					    }
				    }
				else
				    {
					XDisposePtr(pReference);
					pReference = NULL;
				    }
			    }
		    }
	    }
#endif

#if (X_PLATFORM == X_WINDOWS) || (X_PLATFORM == X_WIN_HARDWARE)
	    if (readOnly)
		{
		    pReference->fileReference = _open(pReference->theFile, _O_RDONLY | _O_BINARY);
		    if (pReference->fileReference == -1)
			{
			    XDisposePtr(pReference);
			    pReference = NULL;
			}
		}
	    else
		{
		    pReference->fileReference = _open(pReference->theFile, _O_RDWR | _O_BINARY);
		    if (pReference->fileReference == -1)
			{
				// must not be there, so create it and prepare it as a resource file
			    pReference->fileReference = _open(pReference->theFile, _O_CREAT | _O_RDWR | _O_BINARY | _O_TRUNC);
			    if (pReference->fileReference == -1)
				{
				    XDisposePtr(pReference);
				    pReference = NULL;
				}
			    else
				{
				    // prepare it as a resource file
				    XFileSetPosition((XFILE)pReference, 0L);		// at start
				    XPutLong(&map.mapID, XFILERESOURCE_ID);
				    XPutLong(&map.version, 1);
				    XPutLong(&map.totalResources, 0);
				    XFileWrite((XFILE)pReference, &map, (INT32)sizeof(XFILERESOURCEMAP));
				}
			}
		}
#endif

#if X_PLATFORM == X_BE
	    if (readOnly)
		{
		    pReference->fileReference = open(pReference->theFile, O_RDONLY | O_BINARY);
		    if (pReference->fileReference == -1)
			{
			    XDisposePtr(pReference);
			    pReference = NULL;
			}
		}
	    else
		{
		    pReference->fileReference = open(pReference->theFile, O_RDWR | O_BINARY);
		    if (pReference->fileReference == -1)
			{
				// must not be there, so create it and prepare it as a resource file
			    pReference->fileReference = open(pReference->theFile, O_CREAT | O_RDWR | O_BINARY | O_TRUNC);
			    if (pReference->fileReference == -1)
				{
				    XDisposePtr(pReference);
				    pReference = NULL;
				}
			    else
				{
				    // prepare it as a resource file
				    XFileSetPosition((XFILE)pReference, 0L);		// at start
				    XPutLong(&map.mapID, XFILERESOURCE_ID);
				    XPutLong(&map.version, 1);
				    XPutLong(&map.totalResources, 0);
				    XFileWrite((XFILE)pReference, &map, (INT32)sizeof(XFILERESOURCEMAP));
				}
			}
		}
#endif

#if (X_PLATFORM == X_SOLARIS) || (X_PLATFORM ==X_LINUX)
	    if (readOnly)
		{
		    pReference->fileReference = open(pReference->theFile, O_RDONLY);	// O_BINARY
		    if (pReference->fileReference == -1)
			{
			    XDisposePtr(pReference);
			    pReference = NULL;
			}
		}
	    else
		{
		    pReference->fileReference = open(pReference->theFile, O_RDWR);	// O_BINARY
		    if (pReference->fileReference == -1)
			{
				// must not be there, so create it and prepare it as a resource file
			    pReference->fileReference = open(pReference->theFile, O_CREAT | O_RDWR | O_TRUNC);	// O_BINARY
			    if (pReference->fileReference == -1)
				{
				    XDisposePtr(pReference);
				    pReference = NULL;
				}
			    else
				{
				    // prepare it as a resource file
				    XFileSetPosition((XFILE)pReference, 0L);		// at start
				    XPutLong(&map.mapID, XFILERESOURCE_ID);
				    XPutLong(&map.version, 1);
				    XPutLong(&map.totalResources, 0);
				    XFileWrite((XFILE)pReference, &map, (INT32)sizeof(XFILERESOURCEMAP));
				}
			}
		}
#endif
#if X_PLATFORM == X_NAVIO
	    if (readOnly)
		{
		    pReference->fileReference = FileOpen(pReference->theFile, FILE_O_RDONLY );
		    if (pReference->fileReference == NULL)
			{
			    XDisposePtr(pReference);
			    pReference = NULL;
			}
		}
	    else
		{
		    pReference->fileReference = FileOpen(pReference->theFile, FILE_O_RDWR );
		    if (pReference->fileReference == NULL)
			{
				// must not be there, so create it and prepare it as a resource file
			    pReference->fileReference = FileCreate(pReference->theFile);
			    if (pReference->fileReference == NULL)
				{
				    XDisposePtr(pReference);
				    pReference = NULL;
				}
			    else
				{
				    // prepare it as a resource file
				    XFileSetPosition((XFILE)pReference, 0L);		// at start
				    XPutLong(&map.mapID, XFILERESOURCE_ID);
				    XPutLong(&map.version, 1);
				    XPutLong(&map.totalResources, 0);
				    XFileWrite((XFILE)pReference, &map, (INT32)sizeof(XFILERESOURCEMAP));
				}
			}
		}
#endif
#endif
	    if (pReference)
		{
		    // success
		    if (PV_AddResourceFileToOpenFiles((XFILE)pReference))
			{	// can't open any more files
			    XDisposePtr(pReference);
			    pReference = NULL;
			}
		    else
			{
			    pReference->pCache = NULL;
#if USE_FILE_CACHE != 0
			    //				if (readOnly)
			    {
				// Try to open XFILECACHE_ID cache block.
				pReference->pCache = (XFILERESOURCECACHE *)XGetFileResource((XFILE)pReference,
											    XFILECACHE_ID, 0, NULL, NULL);
				if (pReference->pCache != NULL)
				    {
					XSwapLongsInAccessCache(pReference->pCache, TRUE);
				    }
				else
				    {
					pReference->pCache = XCreateAccessCache((XFILE)pReference);
				    }
			    }
#endif
				// validate resource file
			    XFileSetPosition((XFILE)pReference, 0L);		// at start
			    if (XFileRead((XFILE)pReference, &map, (INT32)sizeof(XFILERESOURCEMAP)) == 0)
				{
				    if (XGetLong(&map.mapID) != XFILERESOURCE_ID)
					{
					    XDisposePtr(pReference);
					    pReference = NULL;
					}
				}
			    else
				{
				    XDisposePtr(pReference);
				    pReference = NULL;
				}
			}
		}
	}
    return (XFILE)pReference;
}

XFILE XFileOpenForReadFromMemory(XPTR pMemoryBlock, UINT32 memoryBlockSize)
{
    XFILENAME	*pReference;

    pReference = (XFILENAME *)XNewPtr((INT32)sizeof(XFILENAME));
    if (pReference)
	{
	    pReference->pResourceData = pMemoryBlock;
	    pReference->resMemLength = memoryBlockSize;
	    pReference->resMemOffset = 0;
	    pReference->resourceFile = FALSE;
	    pReference->allowMemCopy = TRUE;
	    pReference->fileValidID = XPI_BLOCK_3_ID;
	    pReference->pCache = NULL;
	    pReference->fileReference = 0;
	}
    return (XFILE)pReference;
}

XFILE XFileOpenForRead(XFILENAME *file)
{
    XFILENAME	*pReference;

    pReference = (XFILENAME *)XNewPtr((INT32)sizeof(XFILENAME));
    if (pReference)
	{
	    *pReference = *file;
	    pReference->resourceFile = FALSE;
	    pReference->fileValidID = XPI_BLOCK_3_ID;
	    pReference->pResourceData = NULL;
	    pReference->allowMemCopy = TRUE;
	    pReference->pCache = NULL;
#if USE_HAE_EXTERNAL_API == TRUE
	    pReference->fileReference = HAE_FileOpenForRead((void *)&pReference->theFile);
	    if (pReference->fileReference == -1)
		{
		    XDisposePtr(pReference);
		    pReference = NULL;
		}
#else

#if X_PLATFORM == X_MACINTOSH
	    if (FSpOpenDF(&pReference->theFile, fsRdPerm, &pReference->fileReference))
		{
		    XDisposePtr(pReference);
		    pReference = NULL;
		}
#endif
#if (X_PLATFORM == X_WINDOWS) || (X_PLATFORM == X_WIN_HARDWARE)
	    pReference->fileReference = _open(pReference->theFile, _O_RDONLY | _O_BINARY);
	    if (pReference->fileReference == -1)
		{
		    XDisposePtr(pReference);
		    pReference = NULL;
		}
#endif
#if X_PLATFORM == X_BE
	    pReference->fileReference = open(pReference->theFile, O_RDONLY | O_BINARY);
	    if (pReference->fileReference == -1)
		{
		    XDisposePtr(pReference);
		    pReference = NULL;
		}
#endif
#if (X_PLATFORM == X_SOLARIS) || (X_PLATFORM == X_LINUX)
	    pReference->fileReference = open(pReference->theFile, O_RDONLY);	// O_BINARY
	    if (pReference->fileReference == -1)
		{
		    XDisposePtr(pReference);
		    pReference = NULL;
		}
#endif
#if X_PLATFORM == X_NAVIO
	    pReference->fileReference = FileOpen(pReference->theFile, FILE_O_RDONLY);
	    if (pReference->fileReference == NULL)
		{
		    XDisposePtr(pReference);
		    pReference = NULL;
		}
#endif
#endif
	}
    return (XFILE)pReference;
}

#if USE_CREATION_API == TRUE
XFILE XFileOpenForWrite(XFILENAME *file, XBOOL create)
{
    XFILENAME	*pReference;

    pReference = (XFILENAME *)XNewPtr((INT32)sizeof(XFILENAME));
    if (pReference)
	{
	    *pReference = *file;
	    pReference->resourceFile = FALSE;
	    pReference->fileValidID = XPI_BLOCK_3_ID;
	    pReference->pResourceData = NULL;
	    pReference->allowMemCopy = TRUE;
	    pReference->pCache = NULL;
#if USE_HAE_EXTERNAL_API == TRUE
	    if (create)
		{
		    pReference->fileReference = HAE_FileCreate((void *)&pReference->theFile);
		}
	    pReference->fileReference = HAE_FileOpenForReadWrite((void *)&pReference->theFile);
	    if (pReference->fileReference == -1)
		{
		    XDisposePtr(pReference);
		    pReference = NULL;
		}
#else
#if X_PLATFORM == X_MACINTOSH
	    if (create)
		{
		    FSpDelete(&pReference->theFile);
		    FSpCreate(&pReference->theFile, 'IGOR', 'IGOR', 0);
		    FSpCreateResFile(&pReference->theFile, 'IGOR', 'IGOR', 0);
		}
	    if (FSpOpenDF(&pReference->theFile, fsRdWrPerm, &pReference->fileReference))
		{
		    XDisposePtr(pReference);
		    pReference = NULL;
		}
#endif
#if (X_PLATFORM == X_WINDOWS) || (X_PLATFORM == X_WIN_HARDWARE)
	    if (create)
		{
		    pReference->fileReference = _open(pReference->theFile, _O_WRONLY | _O_CREAT | _O_TRUNC);
		}
	    else
		{
		    pReference->fileReference = _open(pReference->theFile, O_RDWR);
		}
	    if (pReference->fileReference == -1)
		{
		    XDisposePtr(pReference);
		    pReference = NULL;
		}
#endif
#if X_PLATFORM == X_BE
	    if (create)
		{
		    pReference->fileReference = open(pReference->theFile, O_WRONLY | O_CREAT | O_TRUNC);
		}
	    else
		{
		    pReference->fileReference = open(pReference->theFile, O_RDWR);
		}
	    if (pReference->fileReference == -1)
		{
		    XDisposePtr(pReference);
		    pReference = NULL;
		}
#endif
#if (X_PLATFORM == X_SOLARIS) || (X_PLATFORM == X_LINUX)
	    if (create)
		{
		    pReference->fileReference = open(pReference->theFile, O_WRONLY | O_CREAT | O_TRUNC);
		}
	    else
		{
		    pReference->fileReference = open(pReference->theFile, O_RDWR);
		}
	    if (pReference->fileReference == -1)
		{
		    XDisposePtr(pReference);
		    pReference = NULL;
		}
#endif
#if X_PLATFORM == X_NAVIO
	    pReference->fileReference = FileOpen(pReference->theFile, FILE_O_RDWR);
	    if (pReference->fileReference == NULL)
		{
		    XDisposePtr(pReference);
		    pReference = NULL;
		}
#endif
#endif
	}
    return (XFILE)pReference;
}
#endif

// delete file. 0 is ok, -1 for failure
XERR XFileDelete(XFILENAME *file)
{
#if USE_HAE_EXTERNAL_API == TRUE
    void	*dest;

    dest = &file->theFile;
    return HAE_FileDelete(dest);
#else
#if X_PLATFORM == X_MACINTOSH
    return (FSDelete(file->theFile) == noErr) ? 0 : -1;
#endif
#if (X_PLATFORM == X_WINDOWS) || (X_PLATFORM == X_WIN_HARDWARE)
    return _unlink(file->theFile);
#endif
#if X_PLATFORM == X_BE
    return unlink(file->theFile);
#endif
#if (X_PLATFORM == X_SOLARIS) || (X_PLATFORM == X_LINUX)
    return unlink(file->theFile);
#endif
#if X_PLATFORM == X_NAVIO
    return FileDelete(file->theFile);
#endif
#endif
}

void XFileClose(XFILE fileRef)
{
    XFILENAME	*pReference;

    pReference = (XFILENAME *)fileRef;
    if (PV_XFileValid(fileRef))
	{
	    pReference->fileValidID = (INT32)XPI_DEAD_ID;
	    if (pReference->pCache)
		{
		    XDisposePtr((XPTR)pReference->pCache);
		    pReference->pCache = NULL;
		}
	    if (pReference->pResourceData)
		{
		    pReference->pResourceData = NULL;	// clear memory file access
		}
	    else
		{
#if USE_HAE_EXTERNAL_API == TRUE
		    HAE_FileClose(pReference->fileReference);
#else
#if X_PLATFORM == X_MACINTOSH
		    FSClose(pReference->fileReference);
#endif
#if (X_PLATFORM == X_WINDOWS) || (X_PLATFORM == X_WIN_HARDWARE)
		    _close(pReference->fileReference);
#endif
#if X_PLATFORM == X_BE
		    close(pReference->fileReference);
#endif
#if (X_PLATFORM == X_SOLARIS) || (X_PLATFORM == X_LINUX)
		    close(pReference->fileReference);
#endif
#if X_PLATFORM == X_NAVIO
		    FileClose(pReference->fileReference);
#endif
#endif
		}
	    PV_RemoveResourceFileFromOpenFiles(fileRef);
	    XDisposePtr(pReference);
	}
}

XERR XFileRead(XFILE fileRef, void * buffer, INT32 bufferLength)
{
    XFILENAME	*pReference;
    INT32		newLength;
    XERR		err;

    pReference = (XFILENAME *)fileRef;
    if (PV_XFileValid(fileRef))
	{
	    if (pReference->pResourceData)
		{
		    err = 0;
		    newLength = pReference->resMemOffset + bufferLength;
		    if (newLength > pReference->resMemLength)
			{
			    bufferLength -= (newLength - pReference->resMemLength);
			    err = -1;	// eof
			}
		    XBlockMove((char *)pReference->pResourceData + pReference->resMemOffset,
			       buffer, bufferLength);
		    pReference->resMemOffset += bufferLength;
		    return err;
		}
	    else
		{
#if USE_HAE_EXTERNAL_API == TRUE
		    return (HAE_ReadFile(pReference->fileReference, buffer, bufferLength) == bufferLength) ? 0 : -1;
#else
#if X_PLATFORM == X_MACINTOSH
		    return (FSRead(pReference->fileReference, &bufferLength, buffer) == noErr) ? 0 : -1;
#endif
#if (X_PLATFORM == X_WINDOWS) || (X_PLATFORM == X_WIN_HARDWARE)
		    return (_read(pReference->fileReference, (char *)buffer, bufferLength) == bufferLength) ? 0 : -1;
#endif
#if X_PLATFORM == X_BE
		    return (read(pReference->fileReference, buffer, bufferLength) == bufferLength) ? 0 : -1;
#endif
#if (X_PLATFORM == X_SOLARIS) || (X_PLATFORM == X_LINUX)
		    return (read(pReference->fileReference, buffer, (size_t)bufferLength) == bufferLength) ? 0 : -1;
#endif
#if X_PLATFORM == X_NAVIO
		    return (FileRead(pReference->fileReference, buffer, bufferLength) == bufferLength) ? 0 : -1;
#endif
#endif
		}
	}
    return -1;
}

XERR XFileWrite(XFILE fileRef, void *buffer, INT32 bufferLength)
{
    XFILENAME	*pReference;

    pReference = (XFILENAME *)fileRef;
    if (PV_XFileValid(fileRef))
	{
	    if (pReference->pResourceData)
		{
		    return -1;	// can't write a memory resource
		}
	    else
		{
#if USE_HAE_EXTERNAL_API == TRUE
		    return (HAE_WriteFile(pReference->fileReference, buffer, bufferLength) == bufferLength) ? 0 : -1;
#else
#if X_PLATFORM == X_MACINTOSH
		    return (FSWrite(pReference->fileReference, &bufferLength, buffer) == noErr) ? 0 : -1;
#endif
#if (X_PLATFORM == X_WINDOWS) || (X_PLATFORM == X_WIN_HARDWARE)
		    return (_write(pReference->fileReference, (char *)buffer, bufferLength) == bufferLength) ? 0 : -1;
#endif
#if X_PLATFORM == X_BE
		    return (write(pReference->fileReference, buffer, bufferLength) == bufferLength) ? 0 : -1;
#endif
#if (X_PLATFORM == X_SOLARIS) || (X_PLATFORM == X_LINUX)
		    return (write(pReference->fileReference, buffer, (size_t)bufferLength) == bufferLength) ? 0 : -1;
#endif
#if X_PLATFORM == X_NAVIO
		    return (FileWrite(pReference->fileReference, buffer, bufferLength) == bufferLength) ? 0 : -1;
#endif
#endif
		}
	}
    return -1;
}

XERR XFileSetPosition(XFILE fileRef, INT32 filePosition)
{
    XFILENAME	*pReference;
    XERR		err;

    err = -1;
    pReference = (XFILENAME *)fileRef;
    if (PV_XFileValid(fileRef))
	{
	    if (pReference->pResourceData)
		{
		    if ( (filePosition >= 0) && (filePosition < pReference->resMemLength) )
			{
			    pReference->resMemOffset = filePosition;
			    err = 0;
			}
		}
	    else
		{
#if USE_HAE_EXTERNAL_API == TRUE
		    err = HAE_SetFilePosition(pReference->fileReference, filePosition);
#else
#if X_PLATFORM == X_MACINTOSH
		    err = SetFPos(pReference->fileReference, fsFromStart, filePosition);
#endif
#if (X_PLATFORM == X_WINDOWS) || (X_PLATFORM == X_WIN_HARDWARE)
		    err = (_lseek(pReference->fileReference, filePosition, SEEK_SET) == -1) ? -1 : 0;
#endif
#if X_PLATFORM == X_BE
		    err = (lseek(pReference->fileReference, filePosition, SEEK_SET) == -1) ? -1 : 0;
#endif
#if (X_PLATFORM == X_SOLARIS) || (X_PLATFORM == X_LINUX)
		    err = (lseek(pReference->fileReference, (off_t)filePosition, SEEK_SET) == -1) ? -1 : 0;
#endif
#if X_PLATFORM == X_NAVIO
		    err = (FileSeek(pReference->fileReference, FILE_SEEK_SET, filePosition) == -1) ? -1 : 0;
#endif
#endif
		}
	}
    return err;
}

INT32 XFileSetPositionRelative(XFILE fileRef, INT32 relativeOffset)
{
    INT32		pos;

    pos = XFileGetPosition(fileRef);
    if (pos != -1)
	{
	    pos = XFileSetPosition(fileRef, pos + relativeOffset);
	}
    return pos;
}

static XBYTE * PV_GetFilePositionFromMemoryResource(XFILE fileRef)
{
    XFILENAME	*pReference;
    XBYTE		*pos;

    pos = NULL;
    pReference = (XFILENAME *)fileRef;
    if (PV_XFileValid(fileRef))
	{
	    if (pReference->pResourceData)
		{
		    pos = ((XBYTE *)pReference->pResourceData) + pReference->resMemOffset;
		}
	}
    return pos;
}

INT32 XFileGetPosition(XFILE fileRef)
{
    XFILENAME	*pReference;
    INT32		pos;

    pos = -1;
    pReference = (XFILENAME *)fileRef;
    if (PV_XFileValid(fileRef))
	{
	    if (pReference->pResourceData)
		{
		    pos = pReference->resMemOffset;
		}
	    else
		{
#if USE_HAE_EXTERNAL_API == TRUE
		    pos = HAE_GetFilePosition(pReference->fileReference);
#else
#if X_PLATFORM == X_MACINTOSH
		    GetFPos(pReference->fileReference, &pos);
#endif
#if (X_PLATFORM == X_WINDOWS) || (X_PLATFORM == X_WIN_HARDWARE)
		    pos = _lseek(pReference->fileReference, 0, SEEK_CUR);
#endif
#if X_PLATFORM == X_BE
		    pos = lseek(pReference->fileReference, 0, SEEK_CUR);
#endif
#if (X_PLATFORM == X_SOLARIS) || (X_PLATFORM == X_LINUX)
		    pos = lseek(pReference->fileReference, 0, SEEK_CUR);
#endif
#if X_PLATFORM == X_NAVIO
		    pos = FileSeek(pReference->fileReference, FILE_SEEK_CUR, 0);
#endif
#endif
		}
	}
    return pos;
}

#if USE_CREATION_API == TRUE
INT32 XFileSetLength(XFILE fileRef, INT32 newSize)
{
    XFILENAME	*pReference;
    XERR		error;

    error = 0;
    pReference = (XFILENAME *)fileRef;
    if (PV_XFileValid(fileRef))
	{
	    if (pReference->pResourceData == NULL)
		{	// not a memory file
#if USE_HAE_EXTERNAL_API == TRUE
		    error = HAE_SetFileLength(pReference->fileReference, newSize);
#else
#if X_PLATFORM == X_MACINTOSH
		    error = SetEOF(pReference->fileReference, newSize);
#endif
#if (X_PLATFORM == X_WINDOWS) || (X_PLATFORM == X_WIN_HARDWARE)
		    _chsize(pReference->fileReference, newSize);
#endif
#if X_PLATFORM == X_BE
		    error = -1;
		    // deleting of resources will fail until this function is fixed
		    //			chsize(pReference->fileReference, newSize);
#endif
#if (X_PLATFORM == X_SOLARIS) || (X_PLATFORM == X_LINUX)
		    error = -1;
		    // deleting of resources will fail until this function is fixed
		    //			chsize(pReference->fileReference, newSize);
#endif
#if X_PLATFORM == X_NAVIO
#error "Need to define this function"
#endif
#endif
		}
	}
    return (error) ? -1 : 0;
}
#endif

INT32 XFileGetLength(XFILE fileRef)
{
    XFILENAME	*pReference;
    INT32		pos;

    pos = -1;
    pReference = (XFILENAME *)fileRef;
    if (PV_XFileValid(fileRef))
	{
	    if (pReference->pResourceData)
		{
		    pos = pReference->resMemLength;
		}
	    else
		{
#if USE_HAE_EXTERNAL_API == TRUE
		    pos = HAE_GetFileLength(pReference->fileReference);
#else
#if X_PLATFORM == X_MACINTOSH
		    GetEOF(pReference->fileReference, &pos);
#endif
#if (X_PLATFORM == X_WINDOWS) || (X_PLATFORM == X_WIN_HARDWARE)
		    pos = _lseek(pReference->fileReference, 0, SEEK_END);
		    _lseek(pReference->fileReference, 0, SEEK_SET);
#endif
#if X_PLATFORM == X_BE
		    pos = lseek(pReference->fileReference, 0, SEEK_END);
		    lseek(pReference->fileReference, 0, SEEK_SET);
#endif
#if (X_PLATFORM == X_SOLARIS) || (X_PLATFORM == X_LINUX)
		    pos = lseek(pReference->fileReference, 0, SEEK_END);
		    lseek(pReference->fileReference, 0, SEEK_SET);
#endif
#if X_PLATFORM == X_NAVIO
		    pos = FileSeek(pReference->fileReference, FILE_SEEK_END, 0);
		    FileSeek(pReference->fileReference, FILE_SEEK_SET, 0);
#endif
#endif
		}
	}
    return pos;
}

// search the cache for a particular item
static XFILE_CACHED_ITEM * PV_XGetCacheEntry(XFILE fileRef, XResourceType resourceType, XLongResourceID resourceID)
{
    XFILENAME			*pReference;
    INT32				count, total;
    XFILERESOURCECACHE	*pCache;
    XFILE_CACHED_ITEM	*pItem;

    pItem = NULL;
    pReference = (XFILENAME *)fileRef;
    if (PV_XFileValid(fileRef))
	{
	    pCache = pReference->pCache;
	    if (pCache)
		{
		    total = pCache->totalResources;
		    for (count = 0; count < total; count++)
			{
			    if (pCache->cached[count].resourceType == resourceType)
				{
				    if (pCache->cached[count].resourceID == resourceID)
					{
					    pItem = &pCache->cached[count];
					    break;
					}
				}
			}
		}
	}
    return pItem;
}

// search the cache for a particular item with a name
static XFILE_CACHED_ITEM * PV_XGetNamedCacheEntry(XFILE fileRef, XResourceType resourceType, void *cName)
{
    XFILENAME			*pReference;
    INT32				count, total;
    XFILERESOURCECACHE	*pCache;
    XFILE_CACHED_ITEM	*pItem;
    XERR				err=0;
    char				tempPascalName[256];
    INT32				savePos;
    XFILERESOURCEMAP	map;
    INT32				data, next;

    pItem = NULL;
    pReference = (XFILENAME *)fileRef;
    if (PV_XFileValid(fileRef))
	{
	    if (pReference->pResourceData && (pReference->allowMemCopy == FALSE) )
		{
		    //Yes sir!  We have a memory file!
		    //Gotta search the whole bleedin' thing!

		    XFileSetPosition(fileRef, 0L);		// at start
		    if (XFileRead(fileRef, &map, (INT32)sizeof(XFILERESOURCEMAP)) == 0)
			{
			    if (XGetLong(&map.mapID) == XFILERESOURCE_ID)
				{
				    next = (INT32)sizeof(XFILERESOURCEMAP);
				    total = XGetLong(&map.totalResources);
				    for (count = 0; (count < total) && (err == 0); count++)
					{
					    err = XFileSetPosition(fileRef, next);		// at start
					    if (err == 0)
						{
						    err = XFileRead(fileRef, &next, (INT32)sizeof(INT32));		// get next pointer
						    next = XGetLong(&next);
						    if (next != -1L)
							{
							    err = XFileRead(fileRef, &data, (INT32)sizeof(INT32));		// get type

							    if ((XResourceType)XGetLong(&data) == resourceType)
								{
								    pReference->memoryCacheEntry.resourceType = (XResourceType)XGetLong(&data);

								    err = XFileRead(fileRef, &data, (INT32)sizeof(INT32));		// get ID

								    pReference->memoryCacheEntry.resourceID = (XLongResourceID)XGetLong(&data);

								    err = XFileRead(fileRef, &tempPascalName[0], 1L);		// get name
								    if (tempPascalName[0])
									{
									    err = XFileRead(fileRef, &tempPascalName[1], tempPascalName[0]);

									    XPtoCstr(tempPascalName);
									    if (!XStrCmp((char *)tempPascalName, (char *)cName))
										{
										    pItem = &pReference->memoryCacheEntry;
										    break;
										}
									}
								}
							}
						    else
							{
							    err = -4;
							    //DebugStr("\pNext offset is bad");
							    break;
							}
						}
					    else
						{
						    err = -3;
						    //DebugStr("\pCan't set next position");
						    break;
						}
					}
				}
			}

		}
	    else
		{
		    savePos = XFileGetPosition(fileRef);
		    pCache = pReference->pCache;
		    if (pCache)
			{
			    total = pCache->totalResources;
			    for (count = 0; count < total; count++)
				{
				    if (pCache->cached[count].resourceType == resourceType)
					{
					    XFileSetPosition(fileRef, pCache->cached[count].fileOffsetName);
					    err = XFileRead(fileRef, &tempPascalName[0], 1L);		// get name
					    if (tempPascalName[0])
						{
						    err = XFileRead(fileRef, &tempPascalName[1], tempPascalName[0]);
						    if (XStrCmp((char *)cName, (char *)XPtoCstr(tempPascalName)) == 0)
							{
							    pItem = &pCache->cached[count];
							    break;
							}
						}
					}
				}
			}
		    XFileSetPosition(fileRef, savePos);
		}
	}
    return pItem;
}


// bvk 6/12/97
//	This scans the resource file for the header data and returns the name associated
//		with a type and id.  Taken pretty much verbatim from XGetFileResource (just
//		without the data loading).
// pResourceName is a pascal string
char *	XGetResourceNameOnly( XFILE fileRef, XResourceType resourceType, XLongResourceID resourceID, char *pResourceName )
{
    XFILENAME			*pReference;
    XERR				err;
    XFILERESOURCEMAP	map;
    INT32				data, next;
    INT32				count, total;
    char				tempPascalName[256];
    XFILE_CACHED_ITEM	*pCacheItem;

    err = 0;
    tempPascalName[0] = 0;
    pReference = (XFILENAME *)fileRef;
    if (PV_XFileValid(fileRef))
	{
	    // first do we have a cache?
	    if (pReference->pCache)
		{
		    // second check to see if its in the cache
		    pCacheItem = PV_XGetCacheEntry(fileRef, resourceType, resourceID);
		    if (pCacheItem)
			{
				// get name
			    if (pResourceName)
				{
				    XFileSetPosition(fileRef, pCacheItem->fileOffsetName);
				    err = XFileRead(fileRef, &tempPascalName[0], 1L);		// get name
				    if (tempPascalName[0])
					{
					    err = XFileRead(fileRef, &tempPascalName[1], tempPascalName[0]);
					    if (pResourceName)
						{
						    XBlockMove(tempPascalName, pResourceName, tempPascalName[0] + 1);
						}
					}
				}
			}
		    else
			{
			    err = -1;	// can't find it
			}
		}
	    else
		{
		    XFileSetPosition(fileRef, 0L);		// at start
		    if (XFileRead(fileRef, &map, (INT32)sizeof(XFILERESOURCEMAP)) == 0)
			{
			    if (XGetLong(&map.mapID) == XFILERESOURCE_ID)
				{
				    next = (INT32)sizeof(XFILERESOURCEMAP);
				    total = XGetLong(&map.totalResources);
				    for (count = 0; (count < total) && (err == 0); count++)
					{
					    err = XFileSetPosition(fileRef, next);		// at start
					    if (err == 0)
						{
						    err = XFileRead(fileRef, &next, (INT32)sizeof(INT32));		// get next pointer
						    next = XGetLong(&next);
						    if (next != -1L)
							{
							    err = XFileRead(fileRef, &data, (INT32)sizeof(INT32));		// get type
							    if ((XResourceType)XGetLong(&data) == resourceType)
								{
								    err = XFileRead(fileRef, &data, (INT32)sizeof(INT32));		// get ID
								    if ((XLongResourceID)XGetLong(&data) == resourceID)
									{
									    err = XFileRead(fileRef, &tempPascalName[0], 1L);		// get name
									    if (tempPascalName[0])
										{
										    err = XFileRead(fileRef, &tempPascalName[1], tempPascalName[0]);
										    if (pResourceName)
											{
											    XBlockMove(tempPascalName, pResourceName, tempPascalName[0] + 1 );
											    break;
											}
										}
									    err = XFileRead(fileRef, &data, (INT32)sizeof(INT32));		// get length
									    data = XGetLong(&data);		// get resource size
									}
								}
							}
						    else
							{
							    err = -4;
							    //DebugStr("\pNext offset is bad");
							    break;
							}
						}
					    else
						{
						    err = -3;
						    //DebugStr("\pCan't set next position");
						    break;
						}
					}
				}
			}
		}
	}
    return (err ? NULL : pResourceName);
}

XERR XReadPartialFileResource(XFILE fileRef, XResourceType resourceType, XLongResourceID resourceID,
			      char *pResourceName,
			      XPTR *pReturnedBuffer, INT32 bytesToReadAndAllocate)
{
    XFILENAME			*pReference;
    XERR				err;
    XFILERESOURCEMAP	map;
    INT32				data, next;
    INT32				count, total;
    XPTR				pData;
    char				tempPascalName[256];
    XFILE_CACHED_ITEM	*pCacheItem;

    err = 0;
    pData = NULL;
    tempPascalName[0] = 0;
    pReference = (XFILENAME *)fileRef;

    if (PV_XFileValid(fileRef) && pReturnedBuffer && bytesToReadAndAllocate)
	{
	    // first do we have a cache?
	    if (pReference->pCache)
		{
		    // second check to see if its in the cache
		    pCacheItem = PV_XGetCacheEntry(fileRef, resourceType, resourceID);
		    if (pCacheItem)
			{
				// get name
			    if (pResourceName)
				{
				    XFileSetPosition(fileRef, pCacheItem->fileOffsetName);
				    err = XFileRead(fileRef, &tempPascalName[0], 1L);		// get name
				    if (tempPascalName[0])
					{
					    err = XFileRead(fileRef, &tempPascalName[1], tempPascalName[0]);
					    if (pResourceName)
						{
						    XBlockMove(tempPascalName, pResourceName, tempPascalName[0] + 1);
						}
					}
				}
				// get data
			    XFileSetPosition(fileRef, pCacheItem->fileOffsetData);

				// is data memory based?
			    if (pReference->pResourceData && (pReference->allowMemCopy == FALSE) )
				{	// don't bother coping data again, since its already in memory
				    // just return the pointer.
				    pData = PV_GetFilePositionFromMemoryResource(fileRef);
				    if (pData == NULL)
					{
					    err = -2;
					    //DebugStr("\pOut of memory; can't allocate resource");
					}
				}
			    else
				{
				    pData = XNewPtr(bytesToReadAndAllocate);
				    if (pData)
					{
					    err = XFileRead(fileRef, pData, bytesToReadAndAllocate);
					}
				    else
					{
					    err = -2;
					    //DebugStr("\pOut of memory; can't allocate resource");
					}
				}
			}
		    else
			{
			    err = -1;	// can't find it
			}
		}
	    else
		{
		    XFileSetPosition(fileRef, 0L);		// at start
		    if (XFileRead(fileRef, &map, (INT32)sizeof(XFILERESOURCEMAP)) == 0)
			{
			    if (XGetLong(&map.mapID) == XFILERESOURCE_ID)
				{
				    next = (INT32)sizeof(XFILERESOURCEMAP);
				    total = XGetLong(&map.totalResources);
				    for (count = 0; (count < total) && (err == 0); count++)
					{
					    err = XFileSetPosition(fileRef, next);		// at start
					    if (err == 0)
						{
						    err = XFileRead(fileRef, &next, (INT32)sizeof(INT32));		// get next pointer
						    next = XGetLong(&next);
						    if (next != -1L)
							{
							    err = XFileRead(fileRef, &data, (INT32)sizeof(INT32));		// get type
							    if ((XResourceType)XGetLong(&data) == resourceType)
								{
								    err = XFileRead(fileRef, &data, (INT32)sizeof(INT32));		// get ID
								    if ((XLongResourceID)XGetLong(&data) == resourceID)
									{
									    err = XFileRead(fileRef, &tempPascalName[0], 1L);		// get name
									    if (tempPascalName[0])
										{
										    err = XFileRead(fileRef, &tempPascalName[1], tempPascalName[0]);
										    if (pResourceName)
											{
											    XBlockMove(tempPascalName, pResourceName, tempPascalName[0] + 1);
											}
										}
									    err = XFileRead(fileRef, &data, (INT32)sizeof(INT32));		// get length
									    data = XGetLong(&data);		// get resource size

									    // get data
									    // is data memory based?
									    if (pReference->pResourceData && (pReference->allowMemCopy == FALSE) )
										{	// don't bother coping data again, since its already in memory
										    pData = PV_GetFilePositionFromMemoryResource(fileRef);
										    if (pData)
											{
											    err = 0;
											    break;
											}
										    else
											{
											    err = -2;
											    //DebugStr("\pOut of memory; can't allocate resource");
											}
										}
									    else
										{
										    pData = XNewPtr(bytesToReadAndAllocate);
										    if (pData)
											{
											    err = XFileRead(fileRef, pData, bytesToReadAndAllocate);
											    break;
											}
										    else
											{
											    err = -2;
											    //DebugStr("\pOut of memory; can't allocate resource");
											    break;
											}
										}
									}
								}
							}
						    else
							{
							    err = -4;
							    //DebugStr("\pNext offset is bad");
							    break;
							}
						}
					    else
						{
						    err = -3;
						    //DebugStr("\pCan't set next position");
						    break;
						}
					}
				}
			}
		}
	}
    else
	{
	    err = -1;
	}
    return err;
}


// Read from a resource a particular type and ID. Return the size and name and data block.
// fileRef is an open resource file
// resourceType is a vaild resource type
// resourceID is an ID
// pResourceName is a pascal string. pResourceName can be NULL.
// pReturnedResourceSize be filled with the size of the resource. pReturnedResourceSize can be NULL.
XPTR XGetFileResource(XFILE fileRef, XResourceType resourceType, XLongResourceID resourceID, void *pResourceName, INT32 *pReturnedResourceSize)
{
    XFILENAME			*pReference;
    XERR				err;
    XFILERESOURCEMAP	map;
    INT32				data, next;
    INT32				count, total;
    XPTR				pData;
    char				tempPascalName[256];
    XFILE_CACHED_ITEM	*pCacheItem;

    err = 0;
    pData = NULL;
    if (pReturnedResourceSize)
	{
	    *pReturnedResourceSize = 0;
	}
#if DEBUG_PRINT_RESOURCE
    XPutLong(tempPascalName, resourceType);
    tempPascalName[4] = 0;
    printf("GetResource %s %ld is ", tempPascalName, resourceID);
#endif
    tempPascalName[0] = 0;
    pReference = (XFILENAME *)fileRef;
    if (PV_XFileValid(fileRef))
	{
	    // first do we have a cache?
	    if (pReference->pCache)
		{
		    // second check to see if its in the cache
		    pCacheItem = PV_XGetCacheEntry(fileRef, resourceType, resourceID);
		    if (pCacheItem)
			{
				// get name
			    if (pResourceName)
				{
				    XFileSetPosition(fileRef, pCacheItem->fileOffsetName);
				    err = XFileRead(fileRef, &tempPascalName[0], 1L);		// get name
				    if (tempPascalName[0])
					{
					    err = XFileRead(fileRef, &tempPascalName[1], tempPascalName[0]);
					    if (pResourceName)
						{
						    XBlockMove(tempPascalName, pResourceName, tempPascalName[0] + 1);
						}
					}
				}
				// get data
			    XFileSetPosition(fileRef, pCacheItem->fileOffsetData);

				// is data memory based?
			    if (pReference->pResourceData && (pReference->allowMemCopy == FALSE) )
				{	// don't bother coping data again, since its already in memory
				    pData = PV_GetFilePositionFromMemoryResource(fileRef);
				    if (pData)
					{
					    if (pReturnedResourceSize)
						{
						    *pReturnedResourceSize = pCacheItem->resourceLength;
						}
					}
				    else
					{
					    err = -2;
					    //DebugStr("\pOut of memory; can't allocate resource");
					}
				}
			    else
				{
				    pData = XNewPtr(pCacheItem->resourceLength);
				    if (pData)
					{
					    err = XFileRead(fileRef, pData, pCacheItem->resourceLength);
					    if (pReturnedResourceSize)
						{
						    *pReturnedResourceSize = pCacheItem->resourceLength;
						}
					}
				    else
					{
					    err = -2;
					    //DebugStr("\pOut of memory; can't allocate resource");
					}
				}
			}
		    else
			{
			    err = -1;	// can't find it
			}
		}
	    else
		{
		    XFileSetPosition(fileRef, 0L);		// at start
		    if (XFileRead(fileRef, &map, (INT32)sizeof(XFILERESOURCEMAP)) == 0)
			{
			    if (XGetLong(&map.mapID) == XFILERESOURCE_ID)
				{
				    next = (INT32)sizeof(XFILERESOURCEMAP);
				    total = XGetLong(&map.totalResources);
				    for (count = 0; (count < total) && (err == 0); count++)
					{
					    err = XFileSetPosition(fileRef, next);		// at start
					    if (err == 0)
						{
						    err = XFileRead(fileRef, &next, (INT32)sizeof(INT32));		// get next pointer
						    next = XGetLong(&next);
						    if (next != -1L)
							{
							    err = XFileRead(fileRef, &data, (INT32)sizeof(INT32));		// get type
							    if ((XResourceType)XGetLong(&data) == resourceType)
								{
								    err = XFileRead(fileRef, &data, (INT32)sizeof(INT32));		// get ID
								    if ((XLongResourceID)XGetLong(&data) == resourceID)
									{
									    err = XFileRead(fileRef, &tempPascalName[0], 1L);		// get name
									    if (tempPascalName[0])
										{
										    err = XFileRead(fileRef, &tempPascalName[1], tempPascalName[0]);
										    if (pResourceName)
											{
											    XBlockMove(tempPascalName, pResourceName, tempPascalName[0] + 1);
											}
										}
									    err = XFileRead(fileRef, &data, (INT32)sizeof(INT32));		// get length
									    data = XGetLong(&data);		// get resource size

									    // get data
									    // is data memory based?
									    if (pReference->pResourceData && (pReference->allowMemCopy == FALSE) )
										{	// don't bother coping data again, since its already in memory
										    pData = PV_GetFilePositionFromMemoryResource(fileRef);
										    if (pData)
											{
											    if (pReturnedResourceSize)
												{
												    *pReturnedResourceSize = data;
												}
											    err = 0;
											    break;
											}
										    else
											{
											    err = -2;
											    //DebugStr("\pOut of memory; can't allocate resource");
											}
										}
									    else
										{
										    pData = XNewPtr(data);
										    if (pData)
											{
											    err = XFileRead(fileRef, pData, data);
											    if (pReturnedResourceSize)
												{
												    *pReturnedResourceSize = data;
												}
											    break;
											}
										    else
											{
											    err = -2;
											    //DebugStr("\pOut of memory; can't allocate resource");
											    break;
											}
										}
									}
								}
							}
						    else
							{
							    err = -4;
							    //DebugStr("\pNext offset is bad");
							    break;
							}
						}
					    else
						{
						    err = -3;
						    //DebugStr("\pCan't set next position");
						    break;
						}
					}
				}
			}
		}
	}
#if DEBUG_PRINT_RESOURCE
    printf((pData) ? "OK\n" : "BAD\n");
#endif
    return pData;
}


//bvk New!
//	Adds another cache entry to end of cache.

#if USE_CREATION_API == TRUE
static XBOOL PV_AddToAccessCache(XFILE fileRef, XFILE_CACHED_ITEM *cacheItemPtr )
{
    XFILENAME			*pReference;
    XFILERESOURCECACHE	*pCache,*newCache;
    INT32			resCount;
    XFILE_CACHED_ITEM	*pItem;

    pReference = (XFILENAME *)fileRef;
    if (PV_XFileValid(fileRef))
	{
	    pCache = pReference->pCache;
	    if (pCache)
		{
		    resCount = pCache->totalResources + 1;
		    newCache = (XFILERESOURCECACHE *)XNewPtr((INT32)sizeof(XFILERESOURCECACHE) +
							     ((INT32)sizeof(XFILE_CACHED_ITEM) * resCount));
		    if (newCache)
			{
			    XBlockMove(pCache, newCache, (INT32)sizeof(XFILERESOURCECACHE) +
				       ((INT32)sizeof(XFILE_CACHED_ITEM) * (resCount - 1)));

			    XDisposePtr(pCache);
			    pReference->pCache = newCache;
			    newCache->totalResources = resCount;
			    pItem = &newCache->cached[resCount - 1];
				// copy cache item
			    *pItem = *cacheItemPtr;
			    return TRUE;
			}
		}
	}
    return FALSE;
}
#endif	// USE_CREATION_API == TRUE

// This will scan through the resource file and return an pointer to an array of
XFILERESOURCECACHE * XCreateAccessCache(XFILE fileRef)
{
    XFILENAME			*pReference;
    XERR				err;
    XFILERESOURCEMAP	map;
    INT32				data, next;
    INT32				count, total;
    XFILERESOURCECACHE	*pCache;
    char				pPName[256];

    err = 0;
    pCache = NULL;
    pReference = (XFILENAME *)fileRef;
    if (PV_XFileValid(fileRef))
	{
	    XFileSetPosition(fileRef, 0L);		// at start
	    if (XFileRead(fileRef, &map, (INT32)sizeof(XFILERESOURCEMAP)) == 0)
		{
		    if (XGetLong(&map.mapID) == XFILERESOURCE_ID)
			{
			    next = (INT32)sizeof(XFILERESOURCEMAP);
			    total = XGetLong(&map.totalResources);
			    pCache = (XFILERESOURCECACHE *)XNewPtr((INT32)sizeof(XFILERESOURCECACHE) +
								   ((INT32)sizeof(XFILE_CACHED_ITEM) * total));
			    if (pCache)
				{
				    pCache->totalResources = total;
				    for (count = 0; (count < total) && (err == 0); count++)
					{
					    err = XFileSetPosition(fileRef, next);		// at start
					    if (err == 0)
						{
						    err = XFileRead(fileRef, &next, (INT32)sizeof(INT32));				// get next pointer
						    next = XGetLong(&next);
						    if (next != -1L)
							{
							    err = XFileRead(fileRef, &data, (INT32)sizeof(INT32));			// get type
							    pCache->cached[count].resourceType = (XResourceType)XGetLong(&data);

							    err = XFileRead(fileRef, &data, (INT32)sizeof(INT32));			// get ID
							    pCache->cached[count].resourceID = (XLongResourceID)XGetLong(&data);

							    pCache->cached[count].fileOffsetName = XFileGetPosition(fileRef);	// get name
							    err = XFileRead(fileRef, &pPName[0], 1L);
							    if (pPName[0])
								{
								    err = XFileRead(fileRef, &pPName[1], pPName[0]);
								}

							    err = XFileRead(fileRef, &data, (INT32)sizeof(INT32));			// get length
							    pCache->cached[count].resourceLength = XGetLong(&data);
							    pCache->cached[count].fileOffsetData = XFileGetPosition(fileRef);	// save data offset
							}
						    else
							{
							    err = -4;
							    //DebugStr("\pNext offset is bad");
							    break;
							}
						}
					    else
						{
						    err = -3;
						    //DebugStr("\pCan't set next position");
						    break;
						}
					}
				}
			}
		}
	}
    if (err)
	{
	    XDisposePtr((XPTR)pCache);
	    pCache = NULL;
	}
    return pCache;
}

void XSwapLongsInAccessCache(XFILERESOURCECACHE	*pCache, XBOOL inFileOrder)
{
#if X_WORD_ORDER != FALSE
    INT32				count;
    XFILE_CACHED_ITEM	*item;

    count = pCache->totalResources;
    pCache->totalResources = XGetLong(&pCache->totalResources);
    if (inFileOrder)
	{
	    count = pCache->totalResources;
	}

    item = pCache->cached;
    while (--count >= 0)
	{
	    item->resourceType = (XResourceType)XGetLong(&item->resourceType);
	    item->resourceID = (XLongResourceID)XGetLong(&item->resourceID);
	    item->resourceLength = XGetLong(&item->resourceLength);
	    item->fileOffsetName = XGetLong(&item->fileOffsetName);
	    item->fileOffsetData = XGetLong(&item->fileOffsetData);
	    item++;
	}
#else
    pCache;
    inFileOrder;
#endif
}

// Create a resource cache for a file
XERR XFileCreateResourceCache(XFILE fileRef)
{
    XERR		err;
    XFILENAME	*pReference;

    err = 0;
#if USE_FILE_CACHE != 0
    pReference = (XFILENAME *)fileRef;
    if (PV_XFileValid(fileRef))
	{
	    if (pReference->pCache)
		{
		    XFileFreeResourceCache(fileRef);
		}
	    // Try to open XFILECACHE_ID cache block.
	    pReference->pCache = (XFILERESOURCECACHE *)XGetFileResource((XFILE)pReference,
									XFILECACHE_ID, 0, NULL, NULL);
	    if (pReference->pCache != NULL)
		{
		    XSwapLongsInAccessCache(pReference->pCache, TRUE);
		}
	    else
		{
		    pReference->pCache = XCreateAccessCache((XFILE)pReference);
		}
	}
#endif
    return err;
}

// Free cache of a resource file
void XFileFreeResourceCache(XFILE fileRef)
{
    XFILENAME	*pReference;

    pReference = (XFILENAME *)fileRef;
    if (PV_XFileValid(fileRef))
	{
	    if (pReference->pCache)
		{
		    XDisposePtr((XPTR)pReference->pCache);
		    pReference->pCache = NULL;
		}
	}
}

static XBOOL PV_CheckForTypes(XResourceType *pTypes, INT32 total, XResourceType typeCheck)
{
    INT32	count;
    XBOOL	found;

    found = FALSE;
    for (count = 0; count < total; count++)
	{
	    if (pTypes[count] == typeCheck)
		{
		    found = TRUE;
		    break;
		}
	}
    return found;
}

#define MAX_XFILE_SCAN_TYPES		5120L

// Return the number of resource types included in file

//	bvk:  why doesn't this use the cache?

INT32 XCountTypes(XFILE fileRef)
{
    INT32				typeCount;
    XResourceType		lastResourceType;
    XFILENAME			*pReference;
    XERR				err;
    XFILERESOURCEMAP	map;
    INT32				data, next;
    INT32				count, total;
    INT32				*pTypes;

    err = 0;
    typeCount = 0;
    lastResourceType = 0;
#if X_PLATFORM == X_MACINTOSH
    if (PV_IsAnyOpenResourceFiles() == FALSE)
	{
	    // use native resource manager
	    return Count1Types() - 1;
	}
#endif
    if (PV_IsAnyOpenResourceFiles())
	{
	    if (fileRef == (XFILE)NULL)
		{	// then use first open file
		    fileRef = openResourceFiles[0];
		}
	    pTypes = (INT32 *)XNewPtr(((INT32)sizeof(INT32) * MAX_XFILE_SCAN_TYPES));
	    if (pTypes)
		{
		    pReference = (XFILENAME *)fileRef;
		    if (PV_XFileValid(fileRef))
			{
			    XFileSetPosition(fileRef, 0L);		// at start
			    if (XFileRead(fileRef, &map, (INT32)sizeof(XFILERESOURCEMAP)) == 0)
				{
				    if (XGetLong(&map.mapID) == XFILERESOURCE_ID)
					{
					    next = (INT32)sizeof(XFILERESOURCEMAP);
					    total = XGetLong(&map.totalResources);
					    for (count = 0; (count < total) && (err == 0); count++)
						{
						    err = XFileSetPosition(fileRef, next);		// at start
						    if (err == 0)
							{
							    err = XFileRead(fileRef, &next, (INT32)sizeof(INT32));		// get next pointer
							    next = XGetLong(&next);
							    if (next != -1L)
								{
								    err = XFileRead(fileRef, &data, (INT32)sizeof(INT32));		// get type
								    lastResourceType = (XResourceType)XGetLong(&data);
								    if (typeCount < MAX_XFILE_SCAN_TYPES)
									{
									    if (PV_CheckForTypes(pTypes, typeCount, lastResourceType) == FALSE)
										{
										    pTypes[typeCount] = lastResourceType;
										    typeCount++;
										}
									}
								    else
									{
									    err = -5;
									    break;
									}
								}
							    else
								{
								    err = -4;
								    //DebugStr("\pNext offset is bad");
								    break;
								}
							}
						    else
							{
							    err = -3;
							    //DebugStr("\pCan't set next position");
							    break;
							}
						}
					}
				}
			}
		    XDisposePtr((XPTR)pTypes);
		}
	}
    return typeCount;
}

// Return the type from the file based upon an index of 0 to XCountTypes
//	bvk:  why doesn't this use the cache?
XResourceType XGetIndexedType(XFILE fileRef, INT32 resourceIndex)
{
    INT32				typeCount;
    XResourceType		lastResourceType;
    XFILENAME			*pReference;
    XERR				err;
    XFILERESOURCEMAP	map;
    INT32				data, next;
    INT32				count, total;
    INT32				*pTypes;

#if X_PLATFORM == X_MACINTOSH
    if (PV_IsAnyOpenResourceFiles() == FALSE)
	{
	    // use native resource manager
	    ResType	type;

	    Get1IndType(&type, resourceIndex + 1);
	    return (XResourceType)type;
	}
#endif
    err = 0;
    typeCount = 0;
    lastResourceType = 0;
    if (PV_IsAnyOpenResourceFiles())
	{
	    if (fileRef == (XFILE)NULL)
		{	// then use first open file
		    fileRef = openResourceFiles[0];
		}
	    pTypes = (INT32 *)XNewPtr(((INT32)sizeof(INT32) * MAX_XFILE_SCAN_TYPES));
	    if (pTypes)
		{
		    pReference = (XFILENAME *)fileRef;
		    if (PV_XFileValid(fileRef))
			{
			    XFileSetPosition(fileRef, 0L);		// at start
			    if (XFileRead(fileRef, &map, (INT32)sizeof(XFILERESOURCEMAP)) == 0)
				{
				    if (XGetLong(&map.mapID) == XFILERESOURCE_ID)
					{
					    next = (INT32)sizeof(XFILERESOURCEMAP);
					    total = XGetLong(&map.totalResources);
					    for (count = 0; (count < total) && (err == 0); count++)
						{
						    err = XFileSetPosition(fileRef, next);		// at start
						    if (err == 0)
							{
							    err = XFileRead(fileRef, &next, (INT32)sizeof(INT32));		// get next pointer
							    next = XGetLong(&next);
							    if (next != -1L)
								{
								    err = XFileRead(fileRef, &data, (INT32)sizeof(INT32));		// get type
								    lastResourceType = XGetLong(&data);
								    if (typeCount < MAX_XFILE_SCAN_TYPES)
									{
									    if (PV_CheckForTypes(pTypes, typeCount, lastResourceType) == FALSE)
										{
										    pTypes[typeCount] = lastResourceType;
										    if (typeCount == resourceIndex)
											{
											    break;
											}
										    else
											{
											    typeCount++;
											}
										}
									}
								    else
									{
									    err = -5;
									    break;
									}
								}
							    else
								{
								    err = -4;
								    //DebugStr("\pNext offset is bad");
								    break;
								}
							}
						    else
							{
							    err = -3;
							    //DebugStr("\pCan't set next position");
							    break;
							}
						}
					}
				}
			}
		    XDisposePtr((XPTR)pTypes);
		}
	}
    return lastResourceType;
}

// Returns TRUE if the file is read only

static XBOOL PV_IsXFileLocked(XFILE fileRef)
{
    XFILENAME			*pReference;

    if (PV_XFileValid(fileRef))
	{
	    pReference = (XFILENAME *)fileRef;
	    return pReference->readOnly;
	}
    return FALSE;
}


#if USE_CREATION_API == TRUE
static XERR PV_CopyWithinFile( XFILE fileRef, INT32 offsetIn, INT32 offsetOut, INT32 size,
			       void *buffer, INT32 bufferSize)
{
    INT32	xferSize, bytesTransferred = 0;
    INT32	offset=0;
    XERR    err=0;

    if (buffer == NULL)
	{
	    //GACK!
	    return -1;
	}

    while (bytesTransferred < size)
	{
	    xferSize = XMIN(bufferSize, (size - bytesTransferred));

	    //Read data
	    err = XFileSetPosition(fileRef, offsetIn + offset);
	    if (err == 0)
		{
		    err = XFileRead(fileRef, buffer, xferSize );
		    if (err == 0)
			{
				//Write data
			    err = XFileSetPosition(fileRef, offsetOut + offset);
			    if (err == 0)
				{
				    err = XFileWrite(fileRef, buffer, xferSize );
				    if (err != 0)
					{
					    err = -5;
					    break;
					}
				}
			    else
				{
				    err = -4;
				    break;
				}
			}
		    else
			{
			    err = -3;
			    break;
			}

		    bytesTransferred += xferSize;
		    offset += xferSize;
		}
	    else
		{
		    err = -2;
		    break;
		}
	}
    return err;
}
#endif	// USE_CREATION_API == TRUE

#if USE_CREATION_API == TRUE
// Force a clean/update of the most recently opened resource file
XBOOL XCleanResource(void)
{
    XBOOL	err;

#if X_PLATFORM == X_MACINTOSH
    if (PV_IsAnyOpenResourceFiles() == FALSE)
	{
	    // use native resource manager
	    UpdateResFile(CurResFile());
	    return TRUE;
	}
#endif
    err = FALSE;
    if (PV_IsAnyOpenResourceFiles())
	{	// clean from the most recent open file
	    err = XCleanResourceFile(openResourceFiles[0]);
	}
    return err;
}

// bvk - NEW for Beatnik Windows
//
//	Scans the file for all resources of 'TRSH' type, and rewrites the file.
//	It does this by scanning through the file, maintaining two sets of offsets.
//	When a 'TRSH' resource is found, the function skips the resource and starts
//	copying over the trashed resource in place.  At the end, the file size is
//	reset and the appropriate header data changed.
//

#define XFER_BUFFER_SIZE	1024

XBOOL XCleanResourceFile( XFILE fileRef )
{
    XFILENAME			*pReference;
    XERR				err;
    XFILERESOURCEMAP	map;
    INT32				data, resType, resID, totalResSize;
    INT32                           fileSize, resInStart, resDataIn, resDataOut;
    INT32				nextIn, nextOut, count, inResTotal, outResTotal, resDataSize;
    char				pResourceName[256];
    XBOOL				isCompacting;
    void				*fileBuffer;

    isCompacting = FALSE;
    err = 0;
    fileBuffer = NULL;
    pReference = (XFILENAME *)fileRef;
    if (PV_XFileValid(fileRef))
	{
	    // first, do we have memory data?
	    if (pReference->pResourceData)
		{
		    return FALSE;
		}
	    // second, are we a read only file?
	    else if (PV_IsXFileLocked(fileRef))
		{
		    return FALSE;
		}
	    else
		{
		    // if we have a cache resource, delete it
		    XDeleteFileResource(fileRef, XFILECACHE_ID, 0, FALSE);

		    fileSize = XFileGetLength(fileRef);
		    XFileSetPosition(fileRef, 0L);		// at start
		    if (XFileRead(fileRef, &map, (INT32)sizeof(XFILERESOURCEMAP)) == 0)
			{
			    if (XGetLong(&map.mapID) == XFILERESOURCE_ID)
				{
				    nextOut = nextIn = (INT32)sizeof(XFILERESOURCEMAP);
				    inResTotal = XGetLong(&map.totalResources);
				    outResTotal = inResTotal;
				    for (count = 0; (count < inResTotal) && (err == 0); count++)
					{
					    err = XFileSetPosition(fileRef, nextIn);		// at start of input resource
					    if (err != 0)
						{
						    err = -4;
						    break;
						}
					    resInStart = nextIn;
					    err = XFileRead(fileRef, &nextIn, (INT32)sizeof(INT32));		// get next input pointer
					    if (err != 0)
						{
						    err = -5;
						    break;
						}
					    nextIn = XGetLong(&nextIn);
					    totalResSize = nextIn - resInStart;
					    if (nextIn != -1L)
						{
						    err = XFileRead(fileRef, &data, (INT32)sizeof(INT32));		// get type
						    if (err != 0)
							{
							    err = -6;
							    break;
							}
						    resType = XGetLong(&data);
						    if (resType == XFILETRASH_ID)
							{
							    //Since we have skipped a trash resource,
							    //now we must copy/compact every resource that follows!
							    if (isCompacting == FALSE)
								{
								    isCompacting = TRUE;
								    // do we have a cache?
								    if (pReference->pCache)
									{
									    //if we do, REMOVE IT.  It will be invalid
									    XFileFreeResourceCache(fileRef);
									}

								    fileBuffer = XNewPtr(XFER_BUFFER_SIZE);
								    if (fileBuffer == NULL)
									{
									    err = -7;
									    break;
									}
								}
							    //Note that the outputBuffer pointer (nextOut) is NOT being updated!
							    fileSize -= totalResSize;
							    outResTotal--;
							}
						    else
							{
							    //This wasn't a trash resource, so move the output pointer
							    //and/or copy the resource!
							    if (isCompacting)
								{
								    //Get remainder of input info
								    err = XFileRead(fileRef, &data, (INT32)sizeof(INT32));		// get ID
								    if (err != 0)
									{
									    err = -8;
									    break;
									}
								    resID = XGetLong(&data);
								    err = XFileRead(fileRef, &pResourceName[0], 1L);		// get name
								    if (pResourceName[0])
									{
									    err = XFileRead(fileRef, &pResourceName[1], pResourceName[0]);
									}
								    err = XFileRead(fileRef, &data, (INT32)sizeof(INT32));		// get length
								    if (err != 0)
									{
									    err = -9;
									    break;
									}
								    resDataSize = XGetLong(&data);		// get resource size
								    resDataIn = XFileGetPosition(fileRef);	// get current pos

								    //reposition to start of output block
								    err = XFileSetPosition(fileRef, nextOut);		// at start
								    if (err != 0)
									{
									    err = -10;
									    break;
									}

								    nextOut += totalResSize;	//move output pointer

								    //write out information data
								    XPutLong(&data, nextOut);
								    err = XFileWrite(fileRef, &data, (INT32)sizeof(INT32));
								    if (err != 0)
									{
									    err = -16;
									    break;
									}
								    XPutLong(&data, resType);
								    err = XFileWrite(fileRef, &data, (INT32)sizeof(INT32));		// put type
								    if (err != 0)
									{
									    err = -11;
									    break;
									}
								    XPutLong(&data, resID);
								    err = XFileWrite(fileRef, &data, (INT32)sizeof(INT32));		// put ID
								    if (err != 0)
									{
									    err = -12;
									    break;
									}
								    err = XFileWrite(fileRef, pResourceName, (((char *)pResourceName)[0])+1L);		// put name
								    if (err != 0)
									{
									    err = -13;
									    break;
									}
								    XPutLong(&data, resDataSize);
								    err = XFileWrite(fileRef, &data, (INT32)sizeof(INT32));				// put length
								    if (err != 0)
									{
									    err = -14;
									    break;
									}
								    resDataOut = XFileGetPosition(fileRef);	// get current pos

								    //Since we are filling for deleted resources, copy this
								    //res data back into this file over old data!
								    err = PV_CopyWithinFile( fileRef,
											     resDataIn, resDataOut, resDataSize,
											     fileBuffer, XFER_BUFFER_SIZE );
								    if (err != 0)
									{
									    err = -15;
									    break;
									}
								}
							    else
								{
								    nextOut += totalResSize;	//move output pointer
								}
							}
						}
					    else
						{
						    err = -3;
						    //DebugStr("\pCan't set next position");
						    break;
						}
					}
				}
			    else
				{
				    err = -3;
				}
			}
		    else
			{
			    err = -2;
			}
		}
	}
    else
	{
	    err = -1;
	}

    if (isCompacting && err == 0)
	{
	    XFileSetLength(fileRef, fileSize);

	    //Update the resource map
	    XFileSetPosition(fileRef, 0L);		// at start
	    XPutLong(&map.totalResources, outResTotal);
	    err = XFileWrite(fileRef, &map, (INT32)sizeof(XFILERESOURCEMAP));

	    XFileFreeResourceCache(fileRef);
#if USE_FILE_CACHE != 0
	    pReference->pCache = XCreateAccessCache(fileRef);
#endif
	}

    if (fileBuffer)
	{
	    XDisposePtr(fileBuffer);
	    fileBuffer = NULL;
	}
    return (err == 0) ? TRUE : FALSE;
}
#endif	// USE_CREATION_API == TRUE


// bvk - NEW for Beatnik Windows
//
//	Marks the selected resource as a 'TRSH' type w/ID of 0.  If collectTrash is TRUE,
//	writes out the file IN PLACE to remove dead space.  Does not work on read only or
//	memory mapped files.
//
//	Not the fastest thing in the world, but none of the resource functions are!
//
#if USE_CREATION_API == TRUE
XBOOL XDeleteFileResource(XFILE fileRef, XResourceType resourceType, XLongResourceID resourceID, XBOOL collectTrash )
{
    XFILENAME			*pReference;
    XERR				err=0;
    XFILERESOURCEMAP	map;
    INT32				data, next;
    INT32				count, total;
    INT32				whereType, whereID;
    XFILE_CACHED_ITEM	*pCachedItem;

    pReference = (XFILENAME *)fileRef;
    if (PV_XFileValid(fileRef))
	{
	    // first, do we have memory data?
	    if (pReference->pResourceData)
		{
		    return FALSE;
		}
	    // second, are we a read only file?
	    else if (PV_IsXFileLocked(fileRef))
		{
		    return FALSE;
		}

	    // do we have a cache?
	    if (pReference->pCache)
		{
		    pCachedItem = PV_XGetCacheEntry(fileRef, resourceType, resourceID);
		    if (pCachedItem)
			{
			    pCachedItem->resourceType = XFILETRASH_ID;
			    pCachedItem->resourceID = 0;
			    whereType = pCachedItem->fileOffsetName;
			    whereType -= ( (INT32)sizeof(resourceType) + (INT32)sizeof(resourceID) );
			    err = XFileSetPosition(fileRef, whereType );
			    if (err != -1)
				{
				    XPutLong(&data, XFILETRASH_ID);
				    err = XFileWrite(fileRef, &data, (INT32)sizeof(INT32));						// put type

				    if (err == 0)
					{
					    XPutLong(&data, 0);
					    err = XFileWrite(fileRef, &data, (INT32)sizeof(INT32));					// put ID
					}
				}
			    else
				{
				    err = -7;
				}
			}
		    else
			{
			    err = -6;
			}
		}
	    else
		{
		    XFileSetPosition(fileRef, 0L);		// at start
		    if (XFileRead(fileRef, &map, (INT32)sizeof(XFILERESOURCEMAP)) == 0)
			{
			    if (XGetLong(&map.mapID) == XFILERESOURCE_ID)
				{
				    next = (INT32)sizeof(XFILERESOURCEMAP);
				    total = XGetLong(&map.totalResources);
				    for (count = 0; (count < total) && (err == 0); count++)
					{
					    err = XFileSetPosition(fileRef, next);		// at start
					    if (err == 0)
						{
						    err = XFileRead(fileRef, &next, (INT32)sizeof(INT32));		// get next pointer
						    next = XGetLong(&next);
						    if (next != -1L)
							{
							    whereType = XFileGetPosition(fileRef);	// get current pos

							    err = XFileRead(fileRef, &data, (INT32)sizeof(INT32));		// get type
							    if ((XResourceType)XGetLong(&data) == resourceType)
								{
								    whereID = XFileGetPosition(fileRef);	// get current pos

								    err = XFileRead(fileRef, &data, (INT32)sizeof(INT32));		// get ID
								    if ((XLongResourceID)XGetLong(&data) == resourceID)
									{
									    //We found it!
									    //Now 'TRASH' everything
									    //td - added to properly overwrite existing id
									    err = XFileSetPosition(fileRef, whereType);

									    XPutLong(&data, XFILETRASH_ID);
									    err = XFileWrite(fileRef, &data, (INT32)sizeof(INT32));						// put type

									    if (err == 0)
										{
										    XPutLong(&data, 0);
										    err = XFileWrite(fileRef, &data, (INT32)sizeof(INT32));					// put ID
										    break;
										}
									    else
										{
										    err = -5;
										    break;
										}
									}
								}
							}
						    else
							{
							    err = -4;
							    //DebugStr("\pNext offset is bad");
							    break;
							}
						}
					    else
						{
						    err = -3;
						    //DebugStr("\pCan't set next position");
						    break;
						}
					}
				}
			}
		}
	}

    if (collectTrash)
	{
	    XCleanResourceFile( fileRef );
	}
    return (err == 0);
}
#endif	// USE_CREATION_API == TRUE


// return the number of resources of a particular type.
INT32 XCountResourcesOfType(XResourceType resourceType)
{
    XERR	err;

    err = -1;

#if X_PLATFORM == X_MACINTOSH
    if (PV_IsAnyOpenResourceFiles() == FALSE)
	{
	    return Count1Resources((ResType)resourceType);
	}
#endif
    if (PV_IsAnyOpenResourceFiles())
	{	// delete from the most recent open file
	    err = XCountFileResourcesOfType(openResourceFiles[0], resourceType);
	}
    return err;
}


// bvk - NEW for Beatnik Windows
//	For a given resource type, counts the number of resources of a type.
//	Does not check for duplicate IDs.
INT32 XCountFileResourcesOfType(XFILE fileRef, XResourceType theType)
{
    INT32				resCount;
    XResourceType		resourceType;
    XFILENAME			*pReference;
    XERR				err;
    XFILERESOURCEMAP	map;
    INT32				data, next;
    INT32				count, total;
    XFILERESOURCECACHE	*pCache;
    XFILE_CACHED_ITEM	*pCacheItem;

    err = 0;
    resCount = 0;
    resourceType = 0;
    pReference = (XFILENAME *)fileRef;
    if (PV_IsAnyOpenResourceFiles())
	{
	    if (PV_XFileValid(fileRef))
		{
		    if (pReference->pCache)
			{
			    pCache = pReference->pCache;

			    for (count = 0; count < pCache->totalResources; count++)
				{
				    pCacheItem = &pCache->cached[count];

				    if (theType == pCacheItem->resourceType)
					{
					    resCount++;
					}
				}
			}
		    else
			{
			    XFileSetPosition(fileRef, 0L);		// at start
			    if (XFileRead(fileRef, &map, (INT32)sizeof(XFILERESOURCEMAP)) == 0)
				{
				    if (XGetLong(&map.mapID) == XFILERESOURCE_ID)
					{
					    next = (INT32)sizeof(XFILERESOURCEMAP);
					    total = XGetLong(&map.totalResources);
					    for (count = 0; (count < total) && (err == 0); count++)
						{
						    err = XFileSetPosition(fileRef, next);		// at start
						    if (err == 0)
							{
							    err = XFileRead(fileRef, &next, (INT32)sizeof(INT32));		// get next pointer
							    next = XGetLong(&next);
							    if (next != -1L)
								{
								    err = XFileRead(fileRef, &data, (INT32)sizeof(INT32));		// get type
								    resourceType = (XResourceType)XGetLong(&data);
								    if (resourceType == theType )
									{
									    resCount++;
									}
								}
							    else
								{
								    err = -4;
								    //DebugStr("\pNext offset is bad");
								    break;
								}
							}
						    else
							{
							    err = -3;
							    //DebugStr("\pCan't set next position");
							    break;
							}
						}
					}
				}
			}
		}
	}
    return resCount;
}


// search through open resource files until the resourceType is found based upon the resource index
// pResourceName is a pascal string
XPTR XGetIndexedResource(XResourceType resourceType, XLongResourceID *pReturnedID, INT32 resourceIndex,
			 void *pResourceName, INT32 *pReturnedResourceSize)
{
    INT32	count;
    XPTR	pData;

    pData = NULL;
    if (PV_IsAnyOpenResourceFiles())
	{
	    for (count = 0; count < resourceFileCount; count++)
		{
		    pData = XGetIndexedFileResource(openResourceFiles[count], resourceType,
						    pReturnedID, resourceIndex, pResourceName, pReturnedResourceSize);
		    if (pData)
			{
			    break;
			}
		}
	}
#if X_PLATFORM == X_MACINTOSH
    if ((pData == NULL) && (PV_IsAnyOpenResourceFiles() == FALSE))
	{
	    // use native resource manager
	    ResType		type;
	    Handle		data;
	    short		shortID;
	    char		pPName[256];
	    INT32		total;

	    data = Get1IndResource(resourceType, resourceIndex + 1);
	    if (data)
		{
		    total = GetHandleSize(data);
		    if (pReturnedResourceSize)
			{
			    *pReturnedResourceSize = total;
			}
		    pData = XNewPtr(total);
		    if (pData)
			{
			    HLock(data);
			    XBlockMove(*data, pData, total);
			    HUnlock(data);

			    GetResInfo(data, &shortID, &type, (unsigned char *)pPName);
			    if (pReturnedID)
				{
				    *pReturnedID = shortID;
				}
			    if (pResourceName)
				{
				    XBlockMove(pPName, pResourceName, pPName[0]+1);
				}
			    ReleaseResource(data);
			}
		}
	}
#endif
    return pData;
}

// Get a resource based upon its entry count into the resource file. Return NULL if resourceIndex
// is out of range
// pResourceName is a pascal string
XPTR XGetIndexedFileResource(XFILE fileRef, XResourceType resourceType, XLongResourceID *pReturnedID,
			     INT32 resourceIndex, void *pResourceName, INT32 *pReturnedResourceSize)
{
    XFILENAME			*pReference;
    XERR				err;
    XFILERESOURCEMAP	map;
    INT32				data, next;
    INT32				count, total, typeCount;
    XPTR				pData;
    char				pPName[256];
    XFILERESOURCECACHE	*pCache;
    XFILE_CACHED_ITEM	*pCacheItem;

    pData = NULL;
    err = 0;
    if (pReturnedResourceSize)
	{
	    *pReturnedResourceSize = 0;
	}
    pPName[0] = 0;
    pReference = (XFILENAME *)fileRef;
    typeCount = 0;
    if (PV_XFileValid(fileRef))
	{
	    if (pReference->pCache)
		{
		    pCache = pReference->pCache;

		    for (count = 0; count < pCache->totalResources; count++)
			{
			    pCacheItem = &pCache->cached[count];

			    if (pCacheItem->resourceType == resourceType)
				{
				    if (resourceIndex == typeCount)
					{
					    *pReturnedID = pCacheItem->resourceID;

					    XFileSetPosition(fileRef, pCacheItem->fileOffsetName);
					    err = XFileRead(fileRef, &pPName[0], 1L);		// get name length
					    if (pPName[0])
						{
						    err = XFileRead(fileRef, &pPName[1], pPName[0]);
						}
					    // get data
					    XFileSetPosition(fileRef, pCacheItem->fileOffsetData);

					    // is data memory based?
					    if (pReference->pResourceData && (pReference->allowMemCopy == FALSE) )
						{	// don't bother coping data again, since its already in memory
						    pData = PV_GetFilePositionFromMemoryResource(fileRef);
						    if (pData)
							{
							    if (pReturnedResourceSize)
								{
								    *pReturnedResourceSize = pCacheItem->resourceLength;
								}
							}
						    else
							{
							    err = -2;
							    //DebugStr("\pOut of memory; can't allocate resource");
							}
						}
					    else
						{
						    pData = XNewPtr(pCacheItem->resourceLength);
						    if (pData)
							{
							    err = XFileRead(fileRef, pData, pCacheItem->resourceLength);
							    if (pReturnedResourceSize)
								{
								    *pReturnedResourceSize = (INT32)pCacheItem->resourceLength;
								}
							    break;
							}
						    else
							{
							    err = -2;
							    //DebugStr("\pOut of memory; can't allocate resource");
							    break;
							}
						}
					}
				    typeCount++;
				}
			}
		}
	    else
		{
		    XFileSetPosition(fileRef, 0L);		// at start
		    if (XFileRead(fileRef, &map, (INT32)sizeof(XFILERESOURCEMAP)) == 0)
			{
			    if (XGetLong(&map.mapID) == XFILERESOURCE_ID)
				{
				    next = (INT32)sizeof(XFILERESOURCEMAP);
				    total = XGetLong(&map.totalResources);
				    for (count = 0; (count < total) && (err == 0); count++)
					{
					    err = XFileSetPosition(fileRef, next);		// at start
					    if (err == 0)
						{
						    err = XFileRead(fileRef, &next, (INT32)sizeof(INT32));		// get next pointer
						    next = XGetLong(&next);
						    if (next != -1L)
							{
							    err = XFileRead(fileRef, &data, (INT32)sizeof(INT32));		// get type
							    if ((XResourceType)XGetLong(&data) == resourceType)
								{
								    if (resourceIndex == typeCount)
									{
									    err = XFileRead(fileRef, pReturnedID, (INT32)sizeof(INT32));		// get ID
									    *pReturnedID = (XLongResourceID)XGetLong(pReturnedID);
									    err = XFileRead(fileRef, &pPName[0], 1L);		// get name length
									    if (pPName[0])
										{
										    err = XFileRead(fileRef, &pPName[1], pPName[0]);
										}
									    err = XFileRead(fileRef, &data, (INT32)sizeof(INT32));		// get length
									    data = XGetLong(&data);		// get resource size
									    // get data
									    // is data memory based?
									    if (pReference->pResourceData && (pReference->allowMemCopy == FALSE) )
										{	// don't bother coping data again, since its already in memory
										    pData = PV_GetFilePositionFromMemoryResource(fileRef);
										    if (pData)
											{
											    if (pReturnedResourceSize)
												{
												    *pReturnedResourceSize = data;
												}
											    err = 0;
											    break;
											}
										    else
											{
											    err = -2;
											    //DebugStr("\pOut of memory; can't allocate resource");
											}
										}
									    else
										{
										    pData = XNewPtr(data);
										    if (pData)
											{
											    err = XFileRead(fileRef, pData, data);
											    if (pReturnedResourceSize)
												{
												    *pReturnedResourceSize = data;
												}
											    break;
											}
										    else
											{
											    err = -2;
											    //DebugStr("\pOut of memory; can't allocate resource");
											    break;
											}
										}
									}
								    typeCount++;
								}
							}
						    else
							{
							    err = -4;
							    //DebugStr("\pNext offset is bad");
							    break;
							}
						}
					    else
						{
						    err = -3;
						    //DebugStr("\pCan't set next position");
						    break;
						}
					}
				}
			}
		}
	}
    if (pResourceName)
	{
	    XBlockMove(pPName, pResourceName, pPName[0] + 1);
	}
    return pData;
}

#if USE_CREATION_API == TRUE
// get unique ID from most recent open resource file
XERR XGetUniqueResourceID(XResourceType resourceType, XLongResourceID *pReturnedID)
{
    XERR	err;

    err = -1;
#if X_PLATFORM == X_MACINTOSH
    if (PV_IsAnyOpenResourceFiles() == FALSE)
	{
	    *pReturnedID = UniqueID(resourceType);
	    return 0;
	}
#endif
    if (PV_IsAnyOpenResourceFiles())
	{	// pull from the most recent open file
	    err = XGetUniqueFileResourceID(openResourceFiles[0], resourceType, pReturnedID);
	}
    return err;
}

// Given a resource file, and a type, scan through the file and return a unique and unused XLongResourceID. Will
// return 0 if ok, -1 if failure
XERR XGetUniqueFileResourceID(XFILE fileRef, XResourceType resourceType, XLongResourceID *pReturnedID)
{
    XFILENAME			*pReference;
    XERR				err;
    XFILERESOURCECACHE	*pCache;
    INT32				count, total, idCount, next, data;
    XLongResourceID		*pIDs;
    XFILERESOURCEMAP	map;

    err = -1;
    pReference = (XFILENAME *)fileRef;
    if (PV_XFileValid(fileRef) && pReturnedID)
	{
	    // theory is we are going to collect all the IDs from either the cache or the file, then walk
	    // through them and figure out the best new ID
	    *pReturnedID = 0;
	    idCount = 0;
	    pIDs = NULL;

	    // scan from cache and collect them pesky IDs
	    pCache = pReference->pCache;
	    if (pCache)
		{
		    total = pCache->totalResources;
		    pIDs = (XLongResourceID *)XNewPtr((INT32)sizeof(XLongResourceID) * total);
		    if (pIDs)
			{
			    err = 0;
			    for (count = 0; count < total; count++)
				{
				    // same type
				    if (pCache->cached[count].resourceType == resourceType)
					{
					    pIDs[idCount] = pCache->cached[count].resourceID;
					    idCount++;
					}
				}
			}
		}
	    else
		{
		    err = XFileSetPosition(fileRef, 0L);		// at start
		    if (XFileRead(fileRef, &map, (INT32)sizeof(XFILERESOURCEMAP)) == 0)
			{
			    if (XGetLong(&map.mapID) == XFILERESOURCE_ID)
				{
				    next = (INT32)sizeof(XFILERESOURCEMAP);
				    total = XGetLong(&map.totalResources);
				    pIDs = (XLongResourceID *)XNewPtr((INT32)sizeof(XLongResourceID) * total);
				    if (pIDs)
					{
					    for (count = 0; (count < total) && (err == 0); count++)
						{
						    err = XFileSetPosition(fileRef, next);		// at start
						    if (err == 0)
							{
							    err = XFileRead(fileRef, &next, (INT32)sizeof(INT32));		// get next pointer
							    next = XGetLong(&next);
							    if (next != -1L)
								{
								    err = XFileRead(fileRef, &data, (INT32)sizeof(INT32));		// get type
								    if ((XResourceType)XGetLong(&data) == resourceType)
									{
									    err = XFileRead(fileRef, &data, (INT32)sizeof(INT32));		// get ID
									    pIDs[idCount] = (XLongResourceID)XGetLong(&data);
									    idCount++;
									}
								}
							    else
								{
								    err = -4;
								    //DebugStr("\pNext offset is bad");
								    break;
								}
							}
						    else
							{
							    err = -3;
							    //DebugStr("\pCan't set next position");
							    break;
							}
						}
					}
				}
			    else
				{
				    err = -2;
				}
			}
		    else
			{
			    err = -1;
			}
		}
	    // ok, we've now got a list of ID's. Pick a random number and search for a match, if there's
	    // a match, try again.
	    if (pIDs && (err == 0))
		{
		    XBOOL				good, failureCount;
		    XLongResourceID		newID;

		    good = FALSE;
		    failureCount = 0;
		    while (good == FALSE)
			{
			    newID = (XLongResourceID)XRandom();	// we only pick numbers within 0-32767. This is on purpose.
			    // numbers outside of this range are reserved
			    good = TRUE;
			    for (count = 0; count < idCount; count++)
				{
				    if (pIDs[count] == newID)
					{
					    good = FALSE;
					    break;
					}
				}
			    failureCount++;
			    if (good == FALSE)
				{
				    if (failureCount > idCount)	// we've looked through all possible IDs, time to bail
					{
					    err = -1;
					    good = TRUE;
					}
				}
			    else
				{
				    *pReturnedID = newID;
				}
			}
		}
	    else
		{
		    err = -1;	// I guess not
		}
	    XDisposePtr(pIDs);
	}
    return err;
}

// Add a resource to the most recently open resource file.
//		resourceType is a type
//		resourceID is an ID
//		pResourceName is a pascal string
//		pData is the data block to add
//		length is the length of the data block
XERR	XAddResource(XResourceType resourceType, XLongResourceID resourceID, void *pResourceName, void *pData, INT32 length)
{
    XERR	err;

    err = -1;
#if X_PLATFORM == X_MACINTOSH
    if (PV_IsAnyOpenResourceFiles() == FALSE)
	{
	    Handle	dataBlock;

	    dataBlock = NewHandle(length);
	    if (dataBlock)
		{
		    HLock(dataBlock);
		    BlockMove(pData, *dataBlock, length);
		    HUnlock(dataBlock);
		    AddResource(dataBlock, (ResType)resourceType, resourceID, (unsigned char *)pResourceName);
		}
	    return (ResError() == noErr) ? 0 : -1;
	}
#endif
    if (PV_IsAnyOpenResourceFiles())
	{	// add to the most recent open file
	    err = XAddFileResource(openResourceFiles[0], resourceType, resourceID, pResourceName, pData, length);
	}
    return err;
}


// Delete a resource from the most recently open resource file.
//		resourceType is a type
//		resourceID is an ID
//		collectTrash if TRUE will force an update, otherwise it will happen when the file is closed
XBOOL XDeleteResource(XResourceType resourceType, XLongResourceID resourceID, XBOOL collectTrash)
{
#if X_PLATFORM == X_MACINTOSH
    if (PV_IsAnyOpenResourceFiles() == FALSE)
	{
	    Handle	dataBlock;

	    dataBlock = Get1Resource(resourceType, resourceID);
	    if (dataBlock)
		{
		    RemoveResource(dataBlock);
		    if (collectTrash)
			{
			    UpdateResFile(CurResFile());
			}
		}
	    return (ResError() == noErr) ? TRUE : FALSE;
	}
#endif
    if (PV_IsAnyOpenResourceFiles())
	{	// delete from the most recent open file
	    return XDeleteFileResource(openResourceFiles[0], resourceType, resourceID, collectTrash);
	}
    return FALSE;
}


// Add a resource to a particular file
//		fileRef is the open file
//		resourceType is a type
//		resourceID is an ID
//		pResourceName is a pascal string
//		pData is the data block to add
//		length is the length of the data block
XERR XAddFileResource(XFILE fileRef, XResourceType resourceType, XLongResourceID resourceID, void *pResourceName, void *pData, INT32 length)
{
    XFILENAME			*pReference;
    XERR				err;
    XFILERESOURCEMAP	map;
    INT32				data;
    char				fakeName[2];
    INT32				next;
    INT32                           nextsave;
    XFILE_CACHED_ITEM	cacheItem;

#if X_PLATFORM == X_MACINTOSH
    if (PV_IsAnyOpenResourceFiles() == FALSE)
	{
	    // use native resource manager
	    Handle	theData;

	    theData = NewHandle(length);
	    if (theData)
		{
		    HLock(theData);
		    XBlockMove(pData, *theData, length);
		    HUnlock(theData);
		    AddResource(theData, resourceType, (short)resourceID, (unsigned char *)pResourceName);
		}
	    return 0;
	}
#endif
    err = -1;
    pReference = (XFILENAME *)fileRef;
    if (PV_XFileValid(fileRef))
	{
	    // the cache will updated, and the file based cache will be deleted
	    if (pData && length)
		{
		    // if we have a cache resource, delete it
		    XDeleteFileResource(fileRef, XFILECACHE_ID, 0, FALSE);

		    XFileSetPosition(fileRef, 0L);		// at start
		    if (XFileRead(fileRef, &map, (INT32)sizeof(XFILERESOURCEMAP)) == 0)
			{
			    if (XGetLong(&map.mapID) == XFILERESOURCE_ID)
				{
				    XFileSetPosition(fileRef, XFileGetLength(fileRef));						// at end

				    nextsave = XFileGetPosition(fileRef);		// save for later change
				    next = -1;
				    err = XFileWrite(fileRef, &next, (INT32)sizeof(INT32));

				    XPutLong(&data, (UINT32)resourceType);
				    err = XFileWrite(fileRef, &data, (INT32)sizeof(INT32));						// put type
				    XPutLong(&cacheItem.resourceType, (UINT32)resourceType);

				    if (err == 0)
					{
					    XPutLong(&data, (UINT32)resourceID);
					    err = XFileWrite(fileRef, &data, (INT32)sizeof(INT32));					// put ID
					    XPutLong(&cacheItem.resourceID, (UINT32)resourceID);

					    if (err == 0)
						{
						    data = (INT32)XFileGetPosition(fileRef);	// get name pos
						    XPutLong(&cacheItem.fileOffsetName, data);

						    if (pResourceName)
							{
							    err = XFileWrite(fileRef, pResourceName, (((char *)pResourceName)[0])+1L);		// put name
							}
						    else
							{
							    fakeName[0] = 0;
							    err = XFileWrite(fileRef, fakeName, 1L);		// put name
							}
						    XPutLong(&data, length);
						    err = XFileWrite(fileRef, &data, (INT32)sizeof(INT32));				// put length
						    XPutLong(&cacheItem.resourceLength, length);

						    if (err == 0)
							{
							    err = XFileWrite(fileRef, pData, length);					// put data block

							    next = (INT32)XFileGetPosition(fileRef);	// get current pos

							    if (err == 0)
								{
								    XFileSetPosition(fileRef, 0L);		// at start
								    data = XGetLong(&map.totalResources) + 1;
								    XPutLong(&map.totalResources, data);
								    err = XFileWrite(fileRef, &map, (INT32)sizeof(XFILERESOURCEMAP));

								    data = (INT32)XFileGetPosition(fileRef);	// get data pos
								    XPutLong(&cacheItem.fileOffsetData, data);

								    nextsave = XFileSetPosition(fileRef, nextsave);		// save for later change
								    //bvk 06/19/97
								    //You flip all of the other things, you should flip this!
								    XPutLong(&data, next);	//bvk

								    err = XFileWrite(fileRef, &data, (INT32)sizeof(INT32)); //bvk

								    // Now we add this to the cache!
								    if (pReference->pCache)
									{
									    PV_AddToAccessCache(fileRef, &cacheItem );
									}
								}
							}
						}
					}
				}
			}
		}
	}
    return err;
}
#endif	// USE_CREATION_API == TRUE


// get a resource type from the name. cName is a C string
XPTR XGetNamedResource(XResourceType resourceType, void *cName, INT32 *pReturnedResourceSize)
{
    XPTR				pData;
    XFILE_CACHED_ITEM	*pCacheItem;
    char				pResourceName[256];
    short int			count;

    pData = NULL;
    if (pReturnedResourceSize)
	{
	    *pReturnedResourceSize = 0;
	}
#if X_PLATFORM == X_MACINTOSH
    {
	Handle	theData;
	INT32	size;

	// first look inside any open resource files
	if (PV_IsAnyOpenResourceFiles())
	    {
		for (count = 0; count < resourceFileCount; count++)
		    {
			pCacheItem = PV_XGetNamedCacheEntry(openResourceFiles[count], resourceType, cName);
			if (pCacheItem)
			    {
				pData = XGetFileResource(openResourceFiles[count], pCacheItem->resourceType,
							 pCacheItem->resourceID,
							 pResourceName, pReturnedResourceSize);
				// we found our resource
				break;
			    }
		    }
	    }
	if (pData == NULL)
	    //		if ((pData == NULL) && (PV_IsAnyOpenResourceFiles() == FALSE))
	    {
		// use native resource manager
		XStrCpy(pResourceName, (char *)cName);
		XCtoPstr(pResourceName);
		theData = Get1NamedResource(resourceType, (unsigned char *)pResourceName);
		if (theData)
		    {
			size = GetHandleSize(theData);
			pData = XNewPtr(size);
			if (pData)
			    {
				HLock(theData);
				XBlockMove(*theData, pData, size);
				HUnlock(theData);
				ReleaseResource(theData);
				if (pReturnedResourceSize)
				    {
					*pReturnedResourceSize = size;
				    }
			    }
		    }
	    }
    }
#else
    if (PV_IsAnyOpenResourceFiles())
	{
	    for (count = 0; count < resourceFileCount; count++)
		{
		    pCacheItem = PV_XGetNamedCacheEntry(openResourceFiles[count], resourceType, cName);
		    if (pCacheItem)
			{
			    pData = XGetFileResource(openResourceFiles[count], pCacheItem->resourceType,
						     pCacheItem->resourceID,
						     pResourceName, pReturnedResourceSize);
				// we found our resource
			    break;
			}
		    else
			{
				// search through without cache. NEED TO BUILT THIS!!
			}
		}
	}
#endif
    return pData;
}

// Get just the resource name from resourceType and resourceID.
// The cName is a 'C' string which is returned
void XGetResourceName(XResourceType resourceType, XLongResourceID resourceID, void *cName)
{
    char		szPName[256];
    short int	count;

    if (cName)
	{
	    ((char *)cName)[0] = 0;
#if X_PLATFORM == X_MACINTOSH
	    // first look in any open resource files
	    if (PV_IsAnyOpenResourceFiles())
		{
		    for (count = 0; count < resourceFileCount; count++)
			{
			    szPName[0] = 0;
			    XGetResourceNameOnly(openResourceFiles[count], resourceType, resourceID, szPName);
			    if (szPName[0])
				{
				    XPtoCstr(szPName);
				    XStrCpy((char *)cName, szPName);
				    // we found data
				    break;
				}
			}
		}
	    if (((char *)cName)[0] == 0)
		{	// use native resource manager
		    short int		theID;
		    UINT32	theType;
		    Handle			theData;

		    SetResLoad(FALSE);
		    theData = Get1Resource(resourceType, (short int)resourceID);
		    SetResLoad(TRUE);
		    if (theData)
			{
			    GetResInfo(theData, &theID, &theType, (unsigned char *)cName);
			    XPtoCstr(cName);
			    DetachResource(theData);
			    DisposeHandle(theData);
			}
		}
#else
	    for (count = 0; count < resourceFileCount; count++)
		{
		    szPName[0] = 0;
		    XGetResourceNameOnly(openResourceFiles[count], resourceType, resourceID, szPName);
		    if (szPName[0])
			{
			    XPtoCstr(szPName);
			    XStrCpy((char *)cName, szPName);
				// we found data
			    break;
			}
		}
#endif
	}
}

// Get a resource and detach it from the resource manager. Which means that you'll need to call XDisposePtr
// to free the memory
XPTR XGetAndDetachResource(XResourceType resourceType, XLongResourceID resourceID, INT32 *pReturnedResourceSize)
{
#if X_PLATFORM == X_MACINTOSH
    Handle		theData;
    XPTR		pData;
    XPTR		pNewData;
    INT32		size;
    char		szPName[256];
    short int	count;
    XFILE		fileRef;
    XFILENAME	*pReference;

    pData = NULL;
    if (pReturnedResourceSize)
	{
	    *pReturnedResourceSize = 0;
	}
    // first look in any open resource files
    if (PV_IsAnyOpenResourceFiles())
	{
	    for (count = 0; count < resourceFileCount; count++)
		{
		    pData = XGetFileResource(openResourceFiles[count], resourceType, resourceID, szPName, &size);
		    if (pData)
			{
			    fileRef = openResourceFiles[count];
			    pReference = (XFILENAME *)fileRef;
			    if (pReference->pResourceData && (pReference->allowMemCopy) )
				{
				    //In the case of a memory file, we have to create a new block to return.
				    pNewData = XNewPtr(size);
				    if (pNewData)
					{
					    XBlockMove(pData, pNewData, size);
					    pData = pNewData;
					}
				    else
					{
					    pData = NULL;	//not detaching is NO SUCCESS!
					}
				}
			    if (pReturnedResourceSize)
				{
				    *pReturnedResourceSize = size;
				}
				// we found data
			    break;
			}
		}
	}
    if (pData == NULL)
	//	if ((pData == NULL) && (PV_IsAnyOpenResourceFiles() == FALSE))
	{	// use native resource manager
	    theData = Get1Resource(resourceType, (short int)resourceID);
	    if (theData)
		{
		    size = GetHandleSize(theData);
		    pData = XNewPtr(size);
		    if (pData)
			{
			    HLock(theData);
			    XBlockMove(*theData, pData, size);
			    HUnlock(theData);
			    ReleaseResource(theData);
			    if (pReturnedResourceSize)
				{
				    *pReturnedResourceSize = size;
				}
			}
		}
	}
    return pData;
#else	// X_PLATFORM == X_MACINTOSH
    char		szPName[256];
    INT32		lSize = 0;
    XPTR		pData = NULL;
    XPTR		pNewData;
    short int	count;
    XFILE		fileRef;
    XFILENAME	*pReference;

    for (count = 0; count < resourceFileCount; count++)
	{
	    pData = XGetFileResource(openResourceFiles[count], resourceType, resourceID, szPName, &lSize);

	    if (pData)
		{
		    fileRef = openResourceFiles[count];
		    pReference = (XFILENAME *)fileRef;
		    if (pReference->pResourceData && (pReference->allowMemCopy) )
			{
				//In the case of a memory file, we have to create a new block to return.
			    pNewData = XNewPtr( lSize );
			    if (pNewData)
				{
				    XBlockMove(pData, pNewData, lSize );
				    pData = pNewData;
				}
			    else
				{
				    pData = NULL;	//not detaching is NO SUCCESS!
				}
			}

		    if (pReturnedResourceSize)
			{
			    *pReturnedResourceSize = lSize;
			}
		    // we found data
		    break;
		}
	}
    return pData;
#endif	//	X_PLATFORM == X_MACINTOSH
}

// get current most recently opened resource file, or NULL if nothing is open
XFILE XFileGetCurrentResourceFile(void)
{
    if (PV_IsAnyOpenResourceFiles())
	{
	    return openResourceFiles[0];
	}
    return (XFILE)NULL;
}

// make sure this resource file is first in the scan list
void XFileUseThisResourceFile(XFILE fileRef)
{
    short int	fileCount;
    XFILE		currentFirst;

    if (PV_XFileValid(fileRef))
	{
	    fileCount = PV_FindResourceFileReferenceIndex(fileRef);
	    if (fileCount != -1)
		{
		    currentFirst = openResourceFiles[0];
		    openResourceFiles[0] = fileRef;
		    openResourceFiles[fileCount] = currentFirst;
		}
	}
}

// theVolume is in the range of 0 to X_FULL_VOLUME
void XSetHardwareVolume(short int theVolume)
{
#if USE_HAE_EXTERNAL_API == TRUE
    HAE_SetHardwareVolume(theVolume);
#else
#if X_PLATFORM == X_MACINTOSH
    INT32	newVolume;

    if (theVolume < 0)
	{
	    theVolume = 0;
	}
    if (theVolume > X_FULL_VOLUME)
	{
	    theVolume = X_FULL_VOLUME;
	}

    /* Scale volume to match the Macintosh hardware
     */
    newVolume = theVolume;
    newVolume |= (newVolume << 16L);
    SetDefaultOutputVolume(newVolume);
#else
    theVolume;
#endif
#endif
}

// returned volume is in the range of 0 to X_FULL_VOLUME
short int XGetHardwareVolume(void)
{
#if USE_HAE_EXTERNAL_API == TRUE
    return HAE_GetHardwareVolume();
#else
#if X_PLATFORM == X_MACINTOSH
    short	theRealVolume;
    INT32	theLongRealVolume;

    GetDefaultOutputVolume(&theLongRealVolume);
    theRealVolume = theLongRealVolume & 0xFFFFL;
    return theRealVolume;
#else
    return 0;
#endif
#endif
}

// Decompress a 'csnd' format sound.
// First byte is a type.
// Next three bytes are a length.
// Type 0 is Delta LZSS compression
void * XDecompressPtr(void* pData, UINT32 dataSize, XBOOL ignoreType)
{
    UINT32  		theTotalSize;
    XCOMPRESSION_TYPE	theType;
    XPTR				theNewData;

    theNewData = NULL;
    if (pData && dataSize)
	{
	    theTotalSize = XGetLong(pData);
	    theType = ignoreType ? X_RAW
		: (XCOMPRESSION_TYPE)(theTotalSize >> 24L);
	    theTotalSize &= 0x00FFFFFFL;
	    theNewData = XNewPtr(theTotalSize);
	    if (theNewData)
		{
		    switch (theType)
			{
			case X_RAW:
			    LZSSUncompress((unsigned char*)pData + sizeof(INT32),
					   dataSize - (UINT32)sizeof(INT32),
					   (unsigned char*)theNewData,
					   theTotalSize);
			    break;
			case X_MONO_8:
			    LZSSUncompressDeltaMono8((unsigned char*)pData + sizeof(INT32),
						     dataSize - (UINT32)sizeof(INT32),
						     (unsigned char*)theNewData,
						     theTotalSize);
			    break;
			case X_STEREO_8:
			    LZSSUncompressDeltaStereo8((unsigned char*)pData + sizeof(INT32),
						       dataSize - (UINT32)sizeof(INT32),
						       (unsigned char*)theNewData,
						       theTotalSize);
			    break;
			case X_MONO_16:
			    LZSSUncompressDeltaMono16((unsigned char*)pData + sizeof(INT32),
						      dataSize - (UINT32)sizeof(INT32),
						      (short*)theNewData,
						      theTotalSize);
			    break;
			case X_STEREO_16:
			    LZSSUncompressDeltaStereo16((unsigned char*)pData + sizeof(INT32),
							dataSize - (UINT32)sizeof(INT32),
							(short*)theNewData,
							theTotalSize);
			    break;
			default:
			    XDisposePtr(theNewData);
			    theNewData = NULL;
			    break;
			}
		}
	}
    return theNewData;
}

// Given a block of data and a size, this will compress it and return a new pointer. The
// original pointer is not deallocated, and the new pointer must be deallocated when finished.
// Will return NULL if cannot compress. First 4 bytes is uncompressed length for all types
// except X_RAW. All other types the first byte is the type and the following 3 bytes are
// the length of the uncompressed data. No data larger than 256 MB.
#if USE_CREATION_API == TRUE
void * XCompressPtr(void *pData, UINT32 dataSize,
		    UINT32 *pNewSize, XCOMPRESSION_TYPE type)
{
    UINT32	                newSize;
    XPTR			compressedData;
    char			*realData;

    realData = NULL;
    newSize = -1L;
    if (type != X_RAW)
	{
	    if (dataSize > 0x00FFFFFF)
		{
		    dataSize = 0;	// too big, bail
		}
	}

    if (pData && dataSize)
	{
	    compressedData = XNewPtr(dataSize);
	    if (compressedData)
		{
		    switch (type)
			{
			case X_RAW:
			    LZSSCompress((unsigned char*)pData, dataSize,
					 (unsigned char*)compressedData, &newSize);
			    break;
			case X_MONO_8:
			    LZSSCompressDeltaMono8((unsigned char*)pData, dataSize,
						   (unsigned char*)compressedData, &newSize);
			    break;
			case X_STEREO_8:
			    LZSSCompressDeltaStereo8((unsigned char*)pData, dataSize,
						     (unsigned char*)compressedData, &newSize);
			    break;
			case X_MONO_16:
			    LZSSCompressDeltaMono16((short*)pData, dataSize,
						    (unsigned char*)compressedData, &newSize);
			    break;
			case X_STEREO_16:
			    LZSSCompressDeltaStereo16((short*)pData, dataSize,
						      (unsigned char*)compressedData, &newSize);
			    break;
			}
		    if (newSize != -1L)
			{	// compression succeeded
			    newSize += (UINT32)sizeof(INT32);
			    realData = (char *)XNewPtr(newSize);
			    if (realData)
				{
				    /* evil cast but we're not losing anything because  */
				    /* we made sure dataSize <= 0xFFFFFF above. -sbohne */
				    XPutLong(realData, (INT32)dataSize);
				    if (type != X_RAW)	//Moe sez: Yuck!
					{
					    *realData = (char)type;
					}
				    XBlockMove(compressedData, realData + sizeof(INT32), newSize - (UINT32)sizeof(INT32));
				}
			}
		    XDisposePtr(compressedData);
		}
	}
    if (pNewSize)
	{
	    *pNewSize = newSize;
	}
    return realData;
}
#endif	// USE_CREATION_API == TRUE


char * XDuplicateStr(char *src)
{
    char *dup;

    dup = NULL;
    if (src)
	{
	    dup = (char *)XNewPtr(XStrLen(src)+1);
	    if (dup)
		{
		    XStrCpy(dup, src);
		}
	}
    return dup;
}

// Strip characters below 32 in place
void XStripStr(char *pString)
{
    char *pNew;

    pNew = XDuplicateAndStripStr(pString);
    if (pNew)
	{
	    XStrCpy(pString, pNew);
	}
    XDisposePtr(pNew);
}


// Duplicate and string characters below 32
char * XDuplicateAndStripStr(char *src)
{
    short int length;									// must be signed
    char	*cStrippedString, *cSource, *cDest;

    cStrippedString = NULL;
    // strip out undesirable characters, give them some walking money
    length = XStrLen(src);
    if (length)
	{
	    cStrippedString = (char *)XNewPtr(length+1);
	    if (cStrippedString)
		{
		    cDest = cStrippedString;
		    cSource = src;
		    while (*cSource)
			{
			    if (*cSource >= 32)
				{
				    *cDest++ = *cSource;
				}
			    cSource++;
			}
		    *cDest = 0;
		}
	}
    return cStrippedString;
}

// standard strcpy
// Copies 'C' string src into dest
char * XStrCpy(char *dest, char *src)
{
    char *sav;

    sav = dest;
    if (src == NULL)
	{
	    src = "";
	}
    if (dest)
	{
	    while (*src)
		{
		    *dest++ = *src++;
		}
	    *dest = 0;
	}
    return sav;
}

static short int PV_LowerCase(short int c)
{
    return( ( ((c >= 'A') && (c <= 'Z')) ? c | 0x20 : c) );
}

// string search.
char * XStrStr(char *source, char *pattern)
{
    unsigned char * s1;
    unsigned char * p1;
    unsigned char firstc, c1, c2;

    if (source == NULL)
	{
	    source = "";
	}
    if (pattern == NULL)
	{
	    pattern = "";
	}
    s1 = (unsigned char *)source;
    p1 = (unsigned char *)pattern;

    if (!(firstc = *p1++))
	{
	    return((char *) s1);
	}
    while((c1 = *s1) != 0)
	{
	    s1++;
	    if (c1 == firstc)
		{
		    const unsigned char * s2 = s1;
		    const unsigned char * p2 = p1;

		    while ((c1 = *s2++) == (c2 = *p2++) && c1) {};

		    if (!c2)
			{
			    return((char *) s1 - 1);
			}
		}
	}
    return(NULL);
}

// string search, but ignore case
char * XLStrStr(char *source, char *pattern)
{
    unsigned char * s1;
    unsigned char * p1;
    unsigned char firstc, c1, c2;

    if (source == NULL)
	{
	    source = "";
	}
    if (pattern == NULL)
	{
	    pattern = "";
	}
    s1 = (unsigned char *)source;
    p1 = (unsigned char *)pattern;

    if (!(firstc = *p1++))
	{
	    return((char *) s1);
	}
    while((c1 = *s1) != 0)
	{
	    s1++;
	    if (PV_LowerCase(c1) == PV_LowerCase(firstc))
		{
		    const unsigned char * s2 = s1;
		    const unsigned char * p2 = p1;

		    while (PV_LowerCase(c1 = *s2++) == PV_LowerCase(c2 = *p2++) && PV_LowerCase(c1)) {};

		    if (!c2)
			{
			    return((char *) s1 - 1);
			}
		}
	}
    return(NULL);
}

char * XStrCat(char * dest, const char * source)
{
    const	char * p = source;
    char * q = dest;
    if (dest)
	{
	    if (source == NULL)
		{
		    source = "";
		}
	    while ((*q++) != 0) {};

	    q--;

	    while ((*q++ = *p++) != 0) {};
	}
    return dest;
}

short int XStrLen(char *src)
{
    short int len;

    len = -1;
    if (src == NULL)
	{
	    src = "";
	}
    do
	{
	    len++;
	} while  (*src++);

    return len;
}

// standard strcmp
short int XStrCmp(const char *s1, const char *s2)
{
    if (s1 == NULL)
	{
	    s1 = "";
	}
    if (s2 == NULL)
	{
	    s2 = "";
	}

    while (1)
	{
	    if (*s1 == *s2)
		{
		    if (*s1 == 0)
			{
			    return 0;
			}
		    else
			{
			    s1++;
			    s2++;
			}
		}
	    else if (*s1 > *s2)
		{
		    return 1;
		}
	    else
		{
		    return -1;
		}
	}
}

// standard strcmp, but ignore case
short int XLStrCmp(const char *s1, const char *s2)
{
    if (s1 == NULL)
	{
	    s1 = "";
	}
    if (s2 == NULL)
	{
	    s2 = "";
	}

    while (1)
	{
	    if (PV_LowerCase(*s1) == PV_LowerCase(*s2))
		{
		    if (*s1 == 0)
			{
			    return 0;
			}
		    else
			{
			    s1++;
			    s2++;
			}
		}
	    else if (PV_LowerCase(*s1) > PV_LowerCase(*s2))
		{
		    return 1;
		}
	    else
		{
		    return -1;
		}
	}
}

// Standard strncmp, but ignore case. Compares zero terminated s1 with non zero terminated s2 with s2 having
// a length of n
short int XLStrnCmp(register const char *s1, register const char *s2, register INT32 n)
{
    if (s1 == NULL)
	{
	    s1 = "";
	}
    if (s2 == NULL)
	{
	    s2 = "";
	}

    if (n)
	{
	    do
		{
		    if (PV_LowerCase(*s1) != PV_LowerCase(*s2++))
			{
			    return (*(unsigned char *)s1 - *(unsigned char *)--s2);
			}
		    if (*s1++ == 0)
			{
			    break;
			}
		} while (--n != 0);
	}
    return 0;
}
// Standard strncmp. Compares zero terminated s1 with non zero terminated s2 with s2 having
// a length of n
short int XStrnCmp(register const char *s1, register const char *s2, register INT32 n)
{
    if (s1 == NULL)
	{
	    s1 = "";
	}
    if (s2 == NULL)
	{
	    s2 = "";
	}

    if (n)
	{
	    do
		{
		    if (*s1 != *s2++)
			{
			    return (*(unsigned char *)s1 - *(unsigned char *)--s2);
			}
		    if (*s1++ == 0)
			{
			    break;
			}
		} while (--n != 0);
	}
    return 0;
}

// This will convert a string to a base 10 long value
INT32 XStrnToLong(char *pData, INT32 length)
{
    INT32	result, num, count;
    char	data[12];

    result = 0;
    num = 0;
    for (count = 0; count < length; count++)
	{
	    if (*pData != 0x20)
		{
		    if ( ((*pData) >= '0') && ((*pData) <= '9') )
			{
			    data[num++] = *pData;
			    if (num > 11)
				{
				    break;
				}
			}
		    else
			{
			    break;
			}
		}
	    pData++;
	}
    if (num)
	{
	    for (count = 0; count < num; count++)
		{
		    result *= 10;
		    result += data[count] - '0';
		}
	}
    return result;
}



short int XMemCmp(const void * src1, const void * src2, INT32 n)
{
    const	unsigned char * p1;
    const	unsigned char * p2;


    for (p1 = (const unsigned char *) src1, p2 = (const unsigned char *) src2, n++; --n;)
	{
	    if (*p1++ != *p2++)
		{
		    return((*--p1 < *--p2) ? -1 : +1);
		}
	}
    return 0;
}



#if USE_CREATION_API == TRUE
/*
 *	pseudo-random number generator
 *
 */
static UINT32 seed = 1;

// return a pseudo-random number in the range of 0-32767
short int XRandom(void)
{
    seed = seed * 1103515245 + 12345;

    return (short int)((seed >> 16L) & 0x7FFFL);		// high word of long, remove high bit
}


/*
 *  seed pseudo-random number generator
 *
 */

void XSeedRandom(UINT32 n)
{
    seed = n;
}

short int XRandomRange(short int max)
{
    static char	setup = 0;

    if (setup == 0)
	{
	    XSeedRandom(XMicroseconds());
	    setup = 1;
	}
    return XRandom() % max;
}
#endif	// USE_CREATION_API

#if USE_FULL_RMF_SUPPORT == TRUE
/****************************************************************************
**
** XIsWinInMac() determines whether a Macintosh character has an
** equivalent in the Windows character set.
**
** XIsMacInWin() determines whether a Windows character has an
** equivalent in the Macintosh character set.
**
** XTranslateWinToMac() provides the Macintosh-equivalent of a Windows
** character code.
**
** XTranslateMacToWin() provides the Windows-equivalent of a Macintosh
** character code.
**
*****************************************************************************/
////////////////////////////////////////////////// DATA:

static const unsigned char macToWinTable[128] =
{
    0xC4,	// A-umlaut
    0xC5,	// A-circle
    0xC7,	// C-cedire
    0xC9,	// E-acute
    0xD1,	// N-tilde
    0xD6,	// O-umlaut
    0xDC,	// U-umlaut
    0xE1,	// a-acute
    0xE0,	// a-grave
    0xE2,	// a-circumflex
    0xE4,	// a-umlaut
    0xE3,	// a-tilde
    0xE5,	// a-circle
    0xE7,	// c-cedire
    0xE9,	// e-acute
    0xE8,	// e-grave

    0xEA,	// e-circumflex
    0xEB,	// e-umlaut
    0xED,	// i-acute
    0xEC,	// i-grave
    0xEE,	// i-circumflex
    0xEF,	// i-umlaut
    0xF1,	// n-tilde
    0xF3,	// o-acute
    0xF2,	// o-grave
    0xF4,	// o-circumflex
    0xF6,	// o-umlaut
    0xF5,	// o-tilde
    0xFA,	// u-acute
    0xF9,	// u-grave
    0xFB,	// u-circumflex
    0xFC,	// u-umlaut

    0x86,	// little up arrow (dagger substituted)
    0xB0,	// degrees
    0xA2,	// cents
    0xA3,	// pounds
    0xA7,	// section
    0x95,	// bullet
    0xB6,	// paragraph
    0xDF,	// german double-s
    0xAE,	// registered
    0xA9,	// copyright (same in both sets)
    0x99,	// tm
    0xB4,	// acute
    0xA8,	// umlaut
    0xB1,	// not equals (plus-minus substituted)
    0xC6,	// AE
    0xD8,	// O-slash

    0xBF,	// infinity (upsidedown-? substituted)
    0xB1,	// plus-minus
    '<',	// less-than or equal-to
    '>',	// greater-than or equal-to
    0xA5,	// yen
    0xB5,	// mu
    'd',	// delta
    'S',	// SIGMA
    'P',	// PI
    'p',	// pi
    0x83,	// f without bar (fi substituted)
    0xAA,	// small a with bar
    0xBA,	// small o with bar
    'O',	// omega
    0xE6,	// ae
    0xF8,	// o-slash

    0xBF,	// upsidedown-?
    0xA1,	// upsidedown-!
    0xAC,	// rho(?)
    '/',	// integral
    0x83,	// fi(?)
    '~',	// approx=
    'D',	// DELTA
    0xAB,	// euro double-open-quote
    0xBB,	// euro double-close-quote
    0x85,	// ellipsis
    0xA0,	// non-breaking space
    0xC0,	// A-grave
    0xC3,	// A-tilde
    0xD5,	// O-tilde
    0x8C,	// OE
    0x9C,	// oe

    0x96,	// dash
    0x97,	// long dash
    0x93,	// open "
    0x94,	// close "
    0x91,	// open '
    0x92,	// close '
    0xF7,	// divide by
    '$',	// wordstar diamond
    0xFF,	// y-umlaut
    0x9F,	// Y-umlaut
    '/',	// some other kinda slash?
    0xA4,	// can't remember what this is called
    0x8B,	// euro single-open-quote
    0x9B,	// euro single-close-quote
    'f',	// fi
    'f',	// fl

    0x87,	// little up/down arrow (double dagger substituted)
    0xB7,	// center dot
    0x82,	// baseline single-quote
    0x84,	// baseline double-quote
    0x89,	// per-thousand
    0xC2,	// A-circumflex
    0xCA,	// E-circumflex
    0xC1,	// A-acute
    0xCB,	// E-umlaut
    0xC8,	// E-grave
    0xCD,	// I-acute
    0xCE,	// I-circumflex
    0xCF,	// I-umlaut
    0xCC,	// I-grave
    0xD3,	// O-acute
    0xD4,	// O-circumflex

    '@',	// apple
    0xD2,	// O-grave
    0xDA,	// U-acute
    0xDB,	// U-circumflex
    0xD9,	// U-grave
    'i',	// dotless i
    0x88,	// circumflex
    0x98,	// tilde
    0x8F,	// bar
    '\'',	// scoop accent
    0xB0,	// single dot accent (circle accent substituted)
    0xB0,	// circle accent
    0xB8,	// cedire
    0x98,	// another tilde(?)
    '.',	// backwards cedire
    '\'',	// another scoop accent
};


////////////////////////////////////////////////// FUNCTIONS:

XBOOL XIsWinInMac(char ansiChar)
{
    return (XTranslateWinToMac(ansiChar) == (char)0xF0) ? (XBOOL)FALSE
	: (XBOOL)TRUE;
}

XBOOL XIsMacInWin(char macChar)
{
    char		ansiChar = XTranslateMacToWin(macChar);

    return (XTranslateWinToMac(ansiChar) != macChar) ? (XBOOL)FALSE
	: (XBOOL)TRUE;
}

char XTranslateWinToMac(char ansiChar)
{
    if ((unsigned char) ansiChar < 0x80)
	{
	    return ansiChar;
	}
    else
	{
	    int			macChar;

	    macChar = 0x80;
	    while (--macChar >= 0)
		{
		    if ((char)macToWinTable[macChar] == ansiChar)
			{
			    return (char)(macChar + 0x80);
			}
		}
	    return (char)0xF0;	// apple character
	}
}

char XTranslateMacToWin(char macChar)
{
    if ((unsigned char) macChar < 0x80)
	{
	    return macChar;
	}
    else
	{
	    return (char)macToWinTable[macChar - 0x80];
	}
}
#endif	// USE_FULL_RMF_SUPPORT

// EOF of X_API.c
