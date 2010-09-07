/*
 * @(#)HAE_API_WinVxD.c	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*****************************************************************************/
/*
**	HAE_API_WinVxD.c
**
**	This provides platform specfic functions for Windows 95. This interface
**	for HAE is for Windows 95 only and uses a vdx as a multimedia driver to send buffer slices
**	through to multimedia system.
**
**	Overview:
**		This works by creating another thread, aquires the sound card via
**		the WinOS waveOutOpen call. Then allocating enough buffers, preps then,
**		and build buffers depending upon how much data has been written to the
**		audio device. The thread polls the current position in the audio stream
**		that has been written and when it falls below a buffer or two, more
**		are built in that thread.
**
**	History	-
**	7/14/97		Created
**	7/18/97		Added Win32 file i/o functions rather than odd unix style ones
**	11/11/97	Added HAE_MaxDevices & HAE_SetDeviceID & HAE_GetDeviceID & HAE_GetDeviceName
**	12/16/97	Modified device system to be real device numbers, and allowed for the user
**				to pass back and forth device speicific info, like the window
**	3/17/98		Added HAE_Is8BitSupported
*/
/*****************************************************************************/

#ifndef WIN32_EXTRA_LEAN
#define WIN32_EXTRA_LEAN
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "R0File.h"
#include "R0CARD.H"
#include "R0MEM.H"

#include "HAE_API.h"

#define HAE_FRAMES_PER_BLOCK		1				// how much ms latancy can we handle (in ms)

static int					g_audioByteBufferSize;	// size of audio buffers in bytes

static int					g_waveDevice;
static long					g_audioFramesToGenerate;// number of samples per audio frame to generate
static char					g_shutDownDoubleBuffer;
static char					g_activeDoubleBuffer;

 // number of samples per audio frame to generate
long						g_audioFramesToGenerate;

// How many audio frames to generate at one time 
static short int			g_synthFramesPerBlock = HAE_FRAMES_PER_BLOCK;

static unsigned long		g_samplesPlayed = 0;


// This file contains API's that need to be defined in order to get HAE (IgorAudio)
// to link and compile.

// **** System setup and cleanup functions
// Setup function. Called before memory allocation, or anything serious. Can be used to 
// load a DLL, library, etc.
// return 0 for ok, or -1 for failure
int HAE_Setup(void)
{
    return 0;
}

// Cleanup function. Called after all memory and the last buffers are deallocated, and
// audio hardware is released. Can be used to unload a DLL, library, etc.
// return 0 for ok, or -1 for failure
int HAE_Cleanup(void)
{
    return 0;
}

// **** Memory management
// allocate a block of locked, zeroed memory. Return a pointer
void * HAE_Allocate(unsigned long size)
{
    void	*data;

    data = 0L;
    if (size)
	{
	    data = (char *)R0MEM_allocate(size);
	    if (data)
		{
		    R0MEM_lock(data, size);
		}
	}
    return data;
}

// dispose of memory allocated with HAE_Allocate
void HAE_Deallocate(void * memoryBlock)
{
    unsigned long	size;

    if (memoryBlock)
	{
	    size = R0MEM_sizeof(memoryBlock);
	    if (size)
		{
		    R0MEM_unlock(memoryBlock, size);
		}
	    R0MEM_free(memoryBlock);
	}
}

// Given a memory pointer and a size, validate of memory pointer is a valid memory address
// with at least size bytes of data avaiable from the pointer.
// This is used to determine if a memory pointer and size can be accessed without 
// causing a memory protection
// fault.
// return 0 for valid, or 1 for bad pointer, or 2 for not supported. 
int HAE_IsBadReadPointer(void *memoryBlock, unsigned long size)
{
    return (R0MEM_IsBadReadPtr(memoryBlock, size)) ? 1 : 0;
}

// this will return the size of the memory pointer allocated with HAE_Allocate. Return
// 0 if you don't support this feature
unsigned long HAE_SizeOfPointer(void * memoryBlock)
{
    unsigned long	size;
			
    size = 0;
    if (memoryBlock)
	{
	    size = R0MEM_sizeof(memoryBlock);
	}
    return size;
}

// block move memory. This is basicly memcpy, but its exposed to take advantage of
// special block move speed ups, various hardware has available.
void HAE_BlockMove(void * source, void * dest, unsigned long size)
{
    if (source && dest && size)
	{
	    memcpy(dest, source, size);
	}
}

// **** Audio Card modifiers
// Return 1 if stereo hardware is supported, otherwise 0.
int HAE_IsStereoSupported(void)
{
    return 1;
}

// Return 1, if sound hardware support 16 bit output, otherwise 0.
int HAE_Is16BitSupported(void)
{
    return 1;
}

// Return 1, if sound hardware support 8 bit output, otherwise 0.
int HAE_Is8BitSupported(void)
{
    return 1;
}

// returned volume is in the range of 0 to 256
short int HAE_GetHardwareVolume(void)
{
    return 256;
}

// theVolume is in the range of 0 to 256
void HAE_SetHardwareVolume(short int theVolume)
{
    theVolume = theVolume;
}

// **** Timing services
// return microseconds
unsigned long HAE_Microseconds(void)
{
    return R0CARD_Microseconds();
}

// wait or sleep this thread for this many microseconds
void HAE_WaitMicroseocnds(unsigned long waitAmount)
{
    unsigned long	ticks;

    ticks = HAE_Microseconds() + waitAmount;
    while (HAE_Microseconds() < ticks) 
	{
	    //		Sleep(0);	// Give up the rest of this time slice to other threads
	}
}

// **** File support
// Create a file, delete orignal if duplicate file name.
// Return -1 if error


// Given the fileNameSource that comes from the call HAE_FileXXXX, copy the name
// in the native format to the pointer passed fileNameDest.
void HAE_CopyFileNameNative(void *fileNameSource, void *fileNameDest)
{
    char *dest;
    char *src;

    if (fileNameSource && fileNameDest)
	{
	    dest = (char *)fileNameDest;
	    src = (char *)fileNameSource;
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
	}
}

long HAE_FileCreate(void *fileName)
{
    return R0FILE_creat((char *)fileName, R0FILE_S_IREADWRITE);
}

long HAE_FileDelete(void *fileName)
{
    fileName;
    return -1;
}


// Open a file
// Return -1 if error, otherwise file handle
long HAE_FileOpenForRead(void *fileName)
{
    if (fileName)
	{
	    return R0FILE_open((char *)fileName, R0FILE_O_RDONLY, R0FILE_S_IREAD);
	}
    return -1;
}

long HAE_FileOpenForWrite(void *fileName)
{
    if (fileName)
	{
	    return R0FILE_open((char *)fileName, R0FILE_O_WRONLY, R0FILE_S_IWRITE);
	}
    return -1;
}

long HAE_FileOpenForReadWrite(void *fileName)
{
    if (fileName)
	{
	    return R0FILE_open((char *)fileName, R0FILE_O_RDWR, R0FILE_S_IREADWRITE);
	}
    return -1;
}

// Close a file
void HAE_FileClose(long fileReference)
{
    R0FILE_close(fileReference);
}

// Read a block of memory from a file.
// Return -1 if error, otherwise length of data read.
long HAE_ReadFile(long fileReference, void *pBuffer, long bufferLength)
{
    if (pBuffer && bufferLength)
	{
	    return R0FILE_read(fileReference, (char *)pBuffer, bufferLength);
	}
    return -1;
}

// Write a block of memory from a file
// Return -1 if error, otherwise length of data written.
long HAE_WriteFile(long fileReference, void *pBuffer, long bufferLength)
{
    if (pBuffer && bufferLength)
	{
	    return R0FILE_write(fileReference, (char *)pBuffer, bufferLength);
	}
    return -1;
}

// set file position in absolute file byte position
// Return -1 if error, otherwise 0.
long HAE_SetFilePosition(long fileReference, unsigned long filePosition)
{
    return (R0FILE_lseek(fileReference, filePosition, R0FILE_SEEK_SET) == -1) ? -1 : 0;
}

// get file position in absolute file bytes
unsigned long HAE_GetFilePosition(long fileReference)
{
    return R0FILE_lseek(fileReference, 0, R0FILE_SEEK_CUR);
}

// get length of file
unsigned long HAE_GetFileLength(long fileReference)
{
    unsigned long pos;

    pos = R0FILE_lseek(fileReference, 0, R0FILE_SEEK_END);
    R0FILE_lseek(fileReference, 0, R0FILE_SEEK_SET);
    return pos;
}

// set the length of a file. Return 0, if ok, or -1 for error
int HAE_SetFileLength(long fileReference, unsigned long newSize)
{
    return (R0FILE_chsize(fileReference, newSize) == newSize) ? 0 : -1;
}

static void PV_AudioHardwareCallback(void *pCardBuffer)
{
    short int		count;
    unsigned char	*pFillBuffer;

    pFillBuffer = (unsigned char *)pCardBuffer;
    if (pFillBuffer)
	{
	    if ( (g_activeDoubleBuffer) && (g_shutDownDoubleBuffer == FALSE) )
		{
		    for (count = 0; count < g_synthFramesPerBlock; count++)
			{
				// Generate one frame audio
			    HAE_BuildMixerSlice(NULL, pFillBuffer, g_audioByteBufferSize,
						g_audioFramesToGenerate);
			    pFillBuffer += g_audioByteBufferSize;
			    g_samplesPlayed += g_audioFramesToGenerate;
			}
		}
	    else
		{
		    g_activeDoubleBuffer = FALSE;
		}
	}
}

// Return the number of buffer blocks that are built at one time. The value returned
// from this function and HAE_GetSliceTimeInMicroseconds will give you the amount
// of lantancy
int HAE_GetAudioBufferCount(void)
{
    return g_synthFramesPerBlock;
}

// Return the number of bytes used for audio buffer for output to card
long HAE_GetAudioByteBufferSize(void)
{
    return g_audioByteBufferSize;
}

// **** Audio card support
// Aquire and enabled audio card
// return 0 if ok, -1 if failed
int HAE_AquireAudioCard(void *context, unsigned long sampleRate, unsigned long channels, unsigned long bits)
{
    int				flag;
    long			bufferSize;
    short int		bufferTime;

    flag = 0;
    g_activeDoubleBuffer = FALSE;
    g_shutDownDoubleBuffer = TRUE;

    g_audioFramesToGenerate = HAE_GetMaxSamplePerSlice();	// get number of frames per sample rate slice
    g_synthFramesPerBlock = HAE_FRAMES_PER_BLOCK;
    bufferTime = (HAE_GetSliceTimeInMicroseconds() / 1000) * g_synthFramesPerBlock;

    if (bits == 8)
	{
	    bufferSize = (sizeof(char) * g_audioFramesToGenerate);
	}
    else
	{
	    bufferSize = (sizeof(short int) * g_audioFramesToGenerate);
	}
    bufferSize *= channels;
    g_audioByteBufferSize = bufferSize;

    // try and configure card to match our audio output

    g_waveDevice = R0CARD_AcquireSoundCard(sampleRate, channels, bits,
					   bufferTime, &g_audioByteBufferSize,
					   PV_AudioHardwareCallback);


    if (g_waveDevice)
	{
	    g_shutDownDoubleBuffer = FALSE;
	    g_activeDoubleBuffer = TRUE;	// must enable process, before thread begins
	    flag = 0;	// ok

	}
    else
	{
	    flag = -1;	// failed;
	    HAE_ReleaseAudioCard(context);
	}
    return flag;
}

// Release and free audio card.
// return 0 if ok, -1 if failed.
int HAE_ReleaseAudioCard(void *context)
{
    context = context;
    g_shutDownDoubleBuffer = TRUE;	// signal thread to stop

    if (g_waveDevice)
	{
	    // Stop audio, shut down hardware, unregister interrupt callback
	    R0CARD_ReleaseSoundCard(g_waveDevice);
	    g_waveDevice = 0;
	}
}

// return device position in samples
unsigned long HAE_GetDeviceSamplesPlayedPosition(void)
{
    return g_samplesPlayed;
}


// number of devices. ie different versions of the HAE connection. DirectSound and waveOut
// return number of devices. ie 1 is one device, 2 is two devices.
// NOTE: This function needs to function before any other calls may have happened.
long HAE_MaxDevices(void)
{
    return 1;	// waveOut
}

// set the current device. device is from 0 to HAE_MaxDevices()
// NOTE:	This function needs to function before any other calls may have happened.
//			Also you will need to call HAE_ReleaseAudioCard then HAE_AquireAudioCard
//			in order for the change to take place.
void HAE_SetDeviceID(long deviceID, void *deviceParameter)
{
    deviceID;
    deviceParameter;
}

// return current device ID
// NOTE: This function needs to function before any other calls may have happened.
long HAE_GetDeviceID(void *deviceParameter)
{
    deviceParameter;
    return 0;
}

// NOTE:	This function needs to function before any other calls may have happened.
//			Format of string is a zero terminated comma delinated C string.
//			"platform,method,misc"
//	example	"MacOS,Sound Manager 3.0,SndPlayDoubleBuffer"
//			"WinOS,DirectSound,multi threaded"
//			"WinOS,waveOut,multi threaded"
//			"WinOS,VxD,low level hardware"
//			"WinOS,plugin,Director"
void HAE_GetDeviceName(long deviceID, char *cName, unsigned long cNameLength)
{
    static char		id[] ={"WinOS,VxD,low level hardware"};
    unsigned long	length;

    if (cName && cNameLength)
	{
	    cName[0] = 0;
	    if (deviceID == 0)
		{
		    length = sizeof(id) + 1;
		    if (length > cNameLength)
			{
			    length = cNameLength;
			}
		    HAE_BlockMove((void *)cName, (void *)id, length);
		}
	}
}



// EOF of HAE_API_WinVxD.c
