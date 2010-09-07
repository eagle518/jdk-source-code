/*
 * @(#)HAE_API_WinOS.c	1.33 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**	HAE_API_WinOS.c
**
**	This provides platform specfic functions for Windows 95/NT. This interface
**	for HAE is for Windows 95/NT and uses the waveOut API or DirectSound to send
**	buffer slices through the multimedia system.
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
**	9/2/97		Fixed typo that referenced samples rather than bytes
**	9/3/97		Added support to detect stereo or 16 bit support
**	11/11/97	Added support for DirectSound.
**				Added HAE_MaxDevices & HAE_SetDeviceID & HAE_GetDeviceID & HAE_GetDeviceName
**	11/20/97	Modifed DirectSound implementation to handle differences for NT DirectSound
**	12/8/97		Added HAE_GetDeviceSamplesPlayedPosition
**	12/16/97	Modified device system to be real device numbers, and allowed for the user
**				to pass back and forth device speicific info, like the window
**	2/13/98		Modified HAE_AquireAudioCard to handle different sample rates
**	3/2/98		Changed HAE_WAVEOUT_FRAMES_PER_BLOCK to 6 from 5 to support even blocks for NT
**				Put an extra waveOutReset in PV_AudioWaveOutFrameThread
**	3/3/98		Changed HAE_Is16BitSupported & HAE_IsStereoSupported to walk through
**				valid devices rather than use the WAVE_MAPPER constant
**	3/17/98		Added HAE_Is8BitSupported
**	4/27/98		Added new compile time flag USE_DIRECTSOUND to eliminate DirectSound
**				define the USE_DIRECTSOUND flag as 0 and DirectSound will be compiled out
**	8/12/98		Fixed a compiler warning with HAE_WriteFile & HAE_ReadFile & PV_AudioWaveOutFrameThread
**
**	JAVASOFT
**	04.16.98	$$kk: NOTE: *must* define USE_DIRECTSOUND 0 in makefile rather than defining NO_DIRECT_SOUND
**				def to javasound win32 makefile.to def out direct sound stuff.  we *cannot* link with dsound.dll
**				or we get an unsatisfied link error on machines that don't have in on the system (#4110266).
**
**	05.06.98:	$$kk: made g_lastPos global and updated in HAE_AquireAudioCard to deal with
**				bug #4135246.
**	9/3/98		Renamed lastPos global to g_lastPos. Changed HAE_Is8BitSupported to actaully
**				test the audio hardware for 8 bit support.
**	9/13/98		Fixed a oversight in PV_IsNT that never cached the result
**	12/17/98	Added HAE_GetHardwareBalance & HAE_SetHardwareBalance
**	3/5/99		Changed context to threadContext to reflect what is really is used
**				in the system.
**
**	12.09.98	$$kk: added HAE_StartAudioOutput and HAE_StopAudioOutput to just
**				start and stop the audio output thread.  changed HAE_AquireAudioCard
**				to not startthe thread automatically.  HAE_ReleaseAudioCard does stop
**				the thread automatically.
**
**	12.09.98	$$kk: removed HAE_StartAudioOutput and HAE_StopAudioOutput and
**				reversed above changes
*/
/*****************************************************************************/

#ifndef WIN32_EXTRA_LEAN
#define WIN32_EXTRA_LEAN
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <mmsystem.h>
#include <windowsx.h>

//#include <stdio.h>
//#include <fcntl.h>
//#include <io.h>

/* for GM_GetSyncTimeStamp() */
#include "GenSnd.h"

#include "HAE_API.h"

#define USE_WIN32_FILE_IO				1

#ifndef USE_DIRECTSOUND
#define USE_DIRECTSOUND				1
#endif

#if USE_DIRECTSOUND
#include <dsound.h>
#endif

//#define DEBUG_OUTPUT

enum
{
    HAE_WAVEOUT	= 0,
    HAE_DIRECTSOUND = 1
};
typedef INT32 HAEDeviceID;

// current device ID. 0 is waveOut, 1 is directSound
static HAEDeviceID			g_currentDeviceID = HAE_WAVEOUT;
static short int			g_balance = 0;				// balance scale -256 to 256 (left to right)
static short int			g_unscaled_volume = 256;	// hardware volume in HAE scale

#define HAE_WAVEOUT_FRAMES_PER_BLOCK	6				// how much ms latancy can we handle (in ms)
#define HAE_WAVEOUT_SOUND_PERIOD		11				// sleep period between position checks (in ms)

static void					*g_audioBufferBlock[HAE_WAVEOUT_FRAMES_PER_BLOCK];
static INT32					g_audioByteBufferSize;			// size of audio buffers in bytes

static HWAVEOUT				g_waveDevice = 0;
static INT32					g_shutDownDoubleBuffer;
static INT32					g_activeDoubleBuffer;

// $$kk: 05.06.98: made lastPos a global variable
static INT32					g_lastPos;

// Format of output buffer
static WAVEFORMATEX			g_waveFormat;

// How many audio frames to generate at one time
#define HAE_DIRECTSOUND_FRAMES_PER_BLOCK	8
#define HAE_DIRECTSOUND_FRAMES_PER_BLOCK_NT	16
// How long in milliseconds to sleep between polling DirectSound play position?
#define HAE_DIRECTSOUND_SOUND_PERIOD		6
#define HAE_DIRECTSOUND_SOUND_PERIOD_NT		5

#if USE_DIRECTSOUND
// Structure to describe output sound buffer topology
struct DXSOUNDPOSDATA
{
    UINT32	uiBufferSize;			// Size of DX sound play buffer
    UINT32	uiBufferBlocks;			// Number of sub blocks in play buffer
    UINT32	uiWriteBlock;			// Current block to fill with audio samples
    UINT32	uiDestPos;				// current destination buffer offset
    UINT32	uiBlockSize;			// size of data block to copy
    UINT32	uiSynthFrameBytes;		// How many bytes per audio frame
    UINT32	uiSynthFramesPerBlock;	// How many frames to audio process at once
    BOOL			bWaveEnd;				// Signal to stop processing audio
    BOOL			bDone;					// Has last generated frame been played
};
typedef struct DXSOUNDPOSDATA	DXSOUNDPOSDATA;

// Output buffer sub block info
struct SOUNDPOSBLOCK
{
    UINT32	dwStart;				// Offset of block start
    UINT32	dwEnd;					// Offset to block end
    BOOL			bEndingBlock;			// Does this block contain final samples?
};
typedef struct SOUNDPOSBLOCK	SOUNDPOSBLOCK;

static LPDIRECTSOUND		g_pDirectSoundObject = NULL;
static LPDIRECTSOUNDBUFFER	g_pDirectSoundBuffer = NULL;
static LPDIRECTSOUNDBUFFER	g_pDirectSoundPrimaryBuffer = NULL;

// Thread used to poll DirectSound play position
static HANDLE				g_hthreadSoundMon = NULL;

// Window handle of Client application
static HWND					g_directSoundWindow = NULL;
// How is the output buffer divided for purposes of monitoring the play position
static INT32					g_nBufferSegments = 2;

// Output sound buffer record
static DXSOUNDPOSDATA		g_SPD;
// Are we currently generating audio frames?
static BOOL					g_bProcessingFrame = FALSE;
// Events to signal the start and end of a batch audio frame processing
static HANDLE				g_heventProcessFrameStart = NULL;
static HANDLE				g_heventProcessFrameFinish = NULL;
#endif	// USE_DIRECTSOUND

// number of samples per audio frame to generate
INT32						g_audioFramesToGenerate;

// How many audio frames to generate at one time
static unsigned int			g_synthFramesPerBlock;	// setup upon runtime

static unsigned int			g_audioPeriodSleepTime;	// setup upon runtime


/* for syncronization of real time MIDI events */
INT64 g_checkpointMicros = 0;
INT64 g_checkpointSyncCount = 0;


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
void * HAE_Allocate(UINT32 size)
{
    void	*data;

    data = 0;
    if (size)
	{
	    // the GHND flag includes the GMEM_ZEROINIT flag
	    // TEMP
	    // $$kk: 04.22.99: we are crashing sometimes on XNewPtr in STREAM_CREATE!!
	    // this extra bit of memory makes us run.  where are we overwriting our bounds??
	    // data = (void *)GlobalAllocPtr(GHND, size);
	    data = (void *)GlobalAllocPtr(GHND, (size+16));
	}
    return data;
}

// dispose of memory allocated with HAE_Allocate
void HAE_Deallocate(void * memoryBlock)
{
    if (memoryBlock)
	{
	    GlobalFreePtr(memoryBlock);
	}
}

// Given a memory pointer and a size, validate of memory pointer is a valid memory address
// with at least size bytes of data avaiable from the pointer.
// This is used to determine if a memory pointer and size can be accessed without
// causing a memory protection
// fault.
// return 0 for valid, or 1 for bad pointer, or 2 for not supported.
int HAE_IsBadReadPointer(void *memoryBlock, UINT32 size)
{
    return (IsBadReadPtr(memoryBlock, size)) ? 1 : 0;
}

// this will return the size of the memory pointer allocated with HAE_Allocate. Return
// 0 if you don't support this feature
UINT32 HAE_SizeOfPointer(void * memoryBlock)
{
    UINT32	size;
    HANDLE			hData;

    size = 0;
    if (memoryBlock)
	{
	    hData = GlobalPtrHandle(memoryBlock);
	    if (hData)
		{
		    size = (UINT32) GlobalSize(hData);
		}
	}
    return size;
}

// block move memory. This is basicly memcpy, but its exposed to take advantage of
// special block move speed ups, various hardware has available.
void HAE_BlockMove(void * source, void * dest, UINT32 size)
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
    MMRESULT	theErr;
    int			status;
    WAVEOUTCAPS	info;
    UINT		count, maxDevs;
    static int	cachedStatus = -1;

    if (cachedStatus == -1)
	{
	    status = 0;
	    maxDevs = waveOutGetNumDevs();
	    for (count = 0; count < maxDevs; count++)
		{
		    theErr = waveOutGetDevCaps(count, &info, sizeof(WAVEOUTCAPS));
		    if (theErr == MMSYSERR_NOERROR)
			{
			    if (info.wChannels > 1)
				{
				    status = 1;	// stereo supported
				}
			}
		}
	    cachedStatus = status;
	}
    return cachedStatus;
}

// Return 1, if sound hardware support 16 bit output, otherwise 0.
int HAE_Is16BitSupported(void)
{
    MMRESULT	theErr;
    int			status, fcount;
    UINT		count, maxDevs;
    WAVEOUTCAPS	info;
    static int	caps[] = {	WAVE_FORMAT_1M16, WAVE_FORMAT_1S16, WAVE_FORMAT_2M16,
				WAVE_FORMAT_2S16, WAVE_FORMAT_4M16, WAVE_FORMAT_4S16};
    static int	cachedStatus = -1;

    if (cachedStatus == -1)
	{
	    status = 0;
	    maxDevs = waveOutGetNumDevs();
	    for (count = 0; count < maxDevs; count++)
		{
		    theErr = waveOutGetDevCaps(count, &info, sizeof(WAVEOUTCAPS));
		    if (theErr == MMSYSERR_NOERROR)
			{
			    for (fcount = 0; fcount < 6; fcount++)
				{
				    if (info.dwFormats & caps[fcount])
					{
					    status = 1;		// 16 bit supported
					    break;
					}
				}
			}
		}
	    cachedStatus = status;
	}
    return cachedStatus;
}

// Return 1, if sound hardware support 8 bit output, otherwise 0.
int HAE_Is8BitSupported(void)
{
    MMRESULT	theErr;
    int			status, fcount;
    UINT		count, maxDevs;
    WAVEOUTCAPS	info;
    static int	caps[] = {	WAVE_FORMAT_1M08, WAVE_FORMAT_1S08, WAVE_FORMAT_2M08,
				WAVE_FORMAT_2S08, WAVE_FORMAT_4M08, WAVE_FORMAT_4S08};
    static int	cachedStatus = -1;

    if (cachedStatus == -1)
	{
	    status = 0;
	    maxDevs = waveOutGetNumDevs();
	    for (count = 0; count < maxDevs; count++)
		{
		    theErr = waveOutGetDevCaps(count, &info, sizeof(WAVEOUTCAPS));
		    if (theErr == MMSYSERR_NOERROR)
			{
			    for (fcount = 0; fcount < 6; fcount++)
				{
				    if (info.dwFormats & caps[fcount])
					{
					    status = 1;		// 8 bit supported
					    break;
					}
				}
			}
		}
	    cachedStatus = status;
	}
    return cachedStatus;
}

// returned balance is in the range of -256 to 256. Left to right. If you're hardware doesn't support this
// range, just scale it.
short int HAE_GetHardwareBalance(void)
{
    return g_balance;
}

// 'balance' is in the range of -256 to 256. Left to right. If you're hardware doesn't support this
// range, just scale it.
void HAE_SetHardwareBalance(short int balance)
{
    // pin balance to box
    if (balance > 256)
	{
	    balance = 256;
	}
    if (balance < -256)
	{
	    balance = -256;
	}
    g_balance = balance;
    HAE_SetHardwareVolume(g_unscaled_volume);
}

// returned volume is in the range of 0 to 256
short int HAE_GetHardwareVolume(void)
{
    // Note this might be an implementation issue. One way (A), just returns the passed in volume the
    // HAE user has set, the other way (B) always looks at the hardware to adjust for outside changes.
    // B has the disadvantage of scaling problems with repeated calls to get and set.
#if 1
    // A
    return g_unscaled_volume;
#else
    // B
    MMRESULT	theErr;
    DWORD		volume;

    volume = 256;
    theErr = waveOutGetVolume((HWAVEOUT)WAVE_MAPPER, &volume);
    if (theErr == MMSYSERR_NOERROR)
	{
	    volume = (volume & 0xFFFF) / 256;	// scale down to 0 to 256
	}
    g_unscaled_volume = volume;
    return (short int)volume;
#endif
}

// newVolume is in the range of 0 to 256
void HAE_SetHardwareVolume(short int newVolume)
{
    MMRESULT		theErr;
    UINT32	volume;
    UINT32	lbm, rbm;

    // pin volume
    if (newVolume > 256)
	{
	    newVolume = 256;
	}
    if (newVolume < 0)
	{
	    newVolume = 0;
	}
    g_unscaled_volume = newVolume;

    // calculate balance multipliers
    if (g_balance > 0)
	{
	    lbm = 256 - g_balance;
	    rbm = 256;
	}
    else
	{
	    lbm = 256;
	    rbm = 256 + g_balance;
	}
    rbm = (((UINT32)newVolume * rbm) * 65535) / 65536;	// scale down to 0 to 65535
    lbm = (((UINT32)newVolume * lbm) * 65535) / 65536;	// scale down to 0 to 65535

    volume = (rbm << 16L) | lbm;
    theErr = waveOutSetVolume((HWAVEOUT) -1 /*WAVE_MAPPER*/, volume);
}

// **** Timing services
// return microseconds
UINT32 HAE_Microseconds(void)
{
    //$$fb prevent overflow: startTick not in microseconds
    static UINT32 starttick = 0;

    if (starttick == 0)
	{
	    starttick = timeGetTime();
	}
    return (UINT32) ((timeGetTime() - starttick) * 1000L);
}

// wait or sleep this thread for this many microseconds
// CLS??: If this function is called from within the frame thread and
// JAVA_THREAD is non-zero, we'll probably crash.
void HAE_WaitMicroseocnds(UINT32 waitAmount)
{
    UINT32	ticks;

    ticks = HAE_Microseconds() + waitAmount;
    while (HAE_Microseconds() < ticks)
	{
	    Sleep(0);	// Give up the rest of this time slice to other threads
	}
}

/* can be used as time stamp for interactive MIDI events */
INT64 XGetRealTimeSyncCount() {
    INT64 ret = g_checkpointSyncCount
	+ (XMicroseconds() - g_checkpointMicros)
	+ (HAE_GetSliceTimeInMicroseconds() * HAE_GetAudioBufferCount());
    return ret;
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

INT32 HAE_FileCreate(void *fileName)
{
#if USE_WIN32_FILE_IO == 0
    int		file;

    file = _creat((char *)fileName, _S_IREAD | _S_IWRITE | _O_RDWR | _O_TRUNC);
    if (file != -1)
	{
	    _close(file);
	}
    return (file != -1) ? 0 : -1;
#else
    HANDLE	newFile;

    newFile = CreateFile((LPCTSTR)fileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL,
			 CREATE_ALWAYS,
			 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
			 NULL);
    CloseHandle(newFile);
    return newFile ? 0 : -1;
#endif
}

INT32 HAE_FileDelete(void *fileName)
{
    if (fileName)
	{
	    if (DeleteFile((LPCTSTR)fileName))
		{
		    return 0;
		}
	}
    return -1;
}


// Open a file
// Return -1 if error, otherwise file handle
// $$fb 2002-02-01: itanium port
XFILE_HANDLE HAE_FileOpenForRead(void *fileName)
{
    if (fileName)
	{
#if USE_WIN32_FILE_IO == 0
	    return (XFILE_HANDLE) _open((char *)fileName, _O_RDONLY | _O_BINARY);
#else
	    return (XFILE_HANDLE) CreateFile((LPCTSTR)fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
					     FILE_ATTRIBUTE_READONLY | FILE_FLAG_RANDOM_ACCESS,
					     NULL);
#endif
	}
    return -1;
}

// $$fb 2002-02-01: itanium port
XFILE_HANDLE HAE_FileOpenForWrite(void *fileName)
{
    if (fileName)
	{
#if USE_WIN32_FILE_IO == 0
	    return (XFILE_HANDLE) _open((char *)fileName, _O_WRONLY | _O_CREAT | _O_TRUNC | _O_BINARY);
#else
	    return (XFILE_HANDLE) CreateFile((LPCTSTR)fileName, GENERIC_WRITE, 0, NULL, CREATE_NEW | TRUNCATE_EXISTING,
					     FILE_FLAG_RANDOM_ACCESS,
					     NULL);
#endif
	}
    return -1;
}

// $$fb 2002-02-01: itanium port
XFILE_HANDLE HAE_FileOpenForReadWrite(void *fileName)
{
    if (fileName)
	{
#if USE_WIN32_FILE_IO == 0
	    return (XFILE_HANDLE) _open((char *)fileName, _O_RDWR | _O_BINARY);
#else
	    return (XFILE_HANDLE) CreateFile((LPCTSTR)fileName, GENERIC_READ | GENERIC_WRITE, 0, NULL,
					     OPEN_EXISTING,
					     FILE_FLAG_RANDOM_ACCESS,
					     NULL);
#endif
	}
    return -1;
}

// Close a file
// $$fb 2002-02-01: itanium port
void HAE_FileClose(XFILE_HANDLE fileReference)
{
#if USE_WIN32_FILE_IO == 0
    _close(fileReference);
#else
    CloseHandle((HANDLE)fileReference);
#endif
}

// Read a block of memory from a file.
// Return -1 if error, otherwise length of data read.
// $$fb 2002-02-01: itanium port
INT32 HAE_ReadFile(XFILE_HANDLE fileReference, void *pBuffer, INT32 bufferLength)
{
    if (pBuffer && bufferLength)
	{
#if USE_WIN32_FILE_IO == 0
	    return _read(fileReference, (char *)pBuffer, bufferLength);
#else
	    {
		DWORD	readFromBuffer;
		return ReadFile((HANDLE)fileReference, (LPVOID)pBuffer,
				bufferLength, &readFromBuffer,
				NULL) ? (INT32)readFromBuffer : -1;
	    }
#endif
	}
    return -1;
}

// Write a block of memory from a file
// Return -1 if error, otherwise length of data written.
// $$fb 2002-02-01: itanium port
INT32 HAE_WriteFile(XFILE_HANDLE fileReference, void *pBuffer, INT32 bufferLength)
{
    if (pBuffer && bufferLength)
	{
#if USE_WIN32_FILE_IO == 0
	    return _write(fileReference, (char *)pBuffer, bufferLength);
#else
	    {
		DWORD	writtenFromBuffer;
		return WriteFile((HANDLE)fileReference, (LPVOID)pBuffer,
				 bufferLength, &writtenFromBuffer,
				 NULL) ? (INT32)writtenFromBuffer : -1;
	    }
#endif
	}
    return -1;
}

// set file position in absolute file byte position
// Return -1 if error, otherwise 0.
// $$fb 2002-02-01: itanium port
INT32 HAE_SetFilePosition(XFILE_HANDLE fileReference, UINT32 filePosition)
{
#if USE_WIN32_FILE_IO == 0
    return (_lseek(fileReference, filePosition, SEEK_SET) == -1) ? -1 : 0;
#else
    return (
	    (SetFilePointer((HANDLE)fileReference, filePosition, NULL, FILE_BEGIN) == 0xFFFFFFFFL)
	    ? -1 : 0);
#endif
}

// get file position in absolute file bytes
// $$fb 2002-02-01: itanium port
UINT32 HAE_GetFilePosition(XFILE_HANDLE fileReference)
{
#if USE_WIN32_FILE_IO == 0
    return _lseek(fileReference, 0, SEEK_CUR);
#else
    return SetFilePointer((HANDLE)fileReference, 0, NULL, FILE_CURRENT);
#endif
}

// get length of file
// $$fb 2002-02-01: itanium port
UINT32 HAE_GetFileLength(XFILE_HANDLE fileReference)
{
    UINT32 pos;

#if USE_WIN32_FILE_IO == 0
    pos = _lseek(fileReference, 0, SEEK_END);
    _lseek(fileReference, 0, SEEK_SET);
#else
    pos = GetFileSize((HANDLE)fileReference, NULL);
    if (pos == 0xFFFFFFFFL)
	{
	    pos = 0;
	}
#endif
    return pos;
}

// set the length of a file. Return 0, if ok, or -1 for error
// $$fb 2002-02-01: itanium port
int HAE_SetFileLength(XFILE_HANDLE fileReference, UINT32 newSize)
{
#if USE_WIN32_FILE_IO == 0
    return _chsize(fileReference, newSize);
#else
    int error;

    error = -1;
    if (HAE_SetFilePosition(fileReference, newSize) == 0)
	{
	    error = SetEndOfFile((HANDLE)fileReference) ? 0 : -1;
	}
    return error;
#endif
}

// Determine if we should use DirectSound or just fall back to the waveOut API.
// return 0 for waveOut, or 1 for DirectSound
static HAEDeviceID PV_UseDirectSound(void)
{
    /*
      // this method does not seem to work
      WAVEOUTCAPS	info;
      int			status;
      MMRESULT	theErr;

      status = HAE_WAVEOUT;
      theErr = waveOutGetDevCaps(WAVE_MAPPER, &info, sizeof(WAVEOUTCAPS));
      if (theErr == MMSYSERR_NOERROR)
      {
      if ((info.dwSupport & WAVECAPS_DIRECTSOUND) == WAVECAPS_DIRECTSOUND)
      {
      status = HAE_DIRECTSOUND;
      }
      }
      return status;
*/
    return g_currentDeviceID;	// HAE_WAVEOUT is waveOut, HAE_DIRECTSOUND is DirectSound

}

static int PV_IsNT(void)
{
    static int	firstTime = TRUE;
    static int	underNT;

    if (firstTime)
	{
	    underNT = (GetVersion() < 0x80000000) ? TRUE : FALSE;
	    firstTime = FALSE;
	}
    return underNT;
}

// When using DirectSound, we need a window handle (!). So this function looks
// around and waits around until it can find an active window handle.
// This has to be done for Windows only, because the message system needs
// a window handle to process messages. If we can't find one, bail with an
// error.
#if USE_DIRECTSOUND
static HWND PV_GetMostActiveWindow(void)
{
    short int	count;
    HWND		theWindow;

    theWindow = NULL;
    // we're going to count to 100, and wait for a window to become active by
    // sleeping in this thread then checking to see if anything has changed.
    for (count = 0; count < 100; count++)
	{
	    theWindow = GetForegroundWindow();
	    if (theWindow == NULL)
		{
		    theWindow = GetActiveWindow();
		}
	    if (theWindow == NULL)
		{
		    theWindow = GetFocus();
		}
	    if (theWindow)
		{	// found something, so break out of here
		    break;
		}
	    Sleep(10);	// sleep 10 milliseconds
	}
    return theWindow;
}
#endif	// USE_DIRECTSOUND

#ifdef DEBUG_OUTPUT
static hasWrittenDebugFile=0;
#endif

static void PV_AudioWaveOutFrameThread(void* threadContext) {

    WAVEHDR			waveHeader[HAE_WAVEOUT_FRAMES_PER_BLOCK];
    INT32			waveHeaderCount;
    MMTIME			audioStatus;

    // $$kk: 08.12.98 merge: changed g_lastPos to global variable
    //$$kk: 05.06.98: i am making g_lastPos global to fix P1 bug #4135246, in
    // which the jdk samples sometimes do not get audio.  depending
    // on the order of events when the audio device is opened and
    // closed, the frame thread may get stuck in the wait().
    INT32			count, currentPos, /*g_lastPos,*/ error;
    char			*pFillBuffer;
#ifdef DEBUG_OUTPUT
    INT32 fh = 0;
#endif

    waveOutReset(g_waveDevice);		// stop all audio before preparing headers
    memset(&waveHeader, 0, sizeof(WAVEHDR) * HAE_WAVEOUT_FRAMES_PER_BLOCK);
    memset(&audioStatus, 0, (INT32)sizeof(MMTIME));
    audioStatus.wType = TIME_BYTES;	// get byte position

    error = waveOutGetPosition(g_waveDevice, &audioStatus, sizeof(MMTIME));
    g_lastPos = audioStatus.u.cb - (g_audioByteBufferSize * g_synthFramesPerBlock * 2);

    // now write out all of the data built
    for (count = 0; count < HAE_WAVEOUT_FRAMES_PER_BLOCK; count++) {
	waveHeader[count].lpData = (char *)g_audioBufferBlock[count];
	waveHeader[count].dwBufferLength = g_audioByteBufferSize * g_synthFramesPerBlock;
	waveHeader[count].dwFlags 		= 0;
	waveHeader[count].dwLoops 		= 0;
	error = waveOutPrepareHeader(g_waveDevice, &waveHeader[count], (INT32)sizeof(WAVEHDR));
    }

#ifdef DEBUG_OUTPUT
    if (!hasWrittenDebugFile) {
	HAE_FileCreate("E:\\output.pcm");
	fh = HAE_FileOpenForReadWrite("E:\\output.pcm");
	if (fh != 0) {
	    printf("Opened debug output file\n");
	    hasWrittenDebugFile = 1;
	} else {
	    printf("Opened NOT debug output file\n");
	}
    }
#endif
    waveHeaderCount = 0;
    while ( (g_activeDoubleBuffer) && (g_shutDownDoubleBuffer == FALSE) ) {
	/* put sync count and XMicroseconds into relation */
	/* could be improved by using actual device sample count */
	g_checkpointMicros = XMicroseconds();
	g_checkpointSyncCount = GM_GetSyncTimeStamp();
	pFillBuffer = (char *)g_audioBufferBlock[waveHeaderCount];
	for (count = 0; count < HAE_WAVEOUT_FRAMES_PER_BLOCK; count++) {
	    // Generate one frame audio
	    HAE_BuildMixerSlice(threadContext, pFillBuffer, g_audioByteBufferSize,
				g_audioFramesToGenerate);
	    pFillBuffer += g_audioByteBufferSize;

	    if (g_shutDownDoubleBuffer) {
		break;	// time to quit
	    }
	}

	if (g_shutDownDoubleBuffer == FALSE) {
	    if (PV_IsNT() == FALSE) {		// under Win95?
		waveHeader[waveHeaderCount].dwFlags &= WHDR_DONE;	// must do, or buffers stop on Win95
	    }

#ifdef DEBUG_OUTPUT
	    if (fh != NULL) {
		HAE_WriteFile(fh, waveHeader[waveHeaderCount].lpData, waveHeader[waveHeaderCount].dwBufferLength);
	    }
#endif
	    error = waveOutWrite(g_waveDevice, &waveHeader[waveHeaderCount], (INT32)sizeof(WAVEHDR));

	    // $$kk: 10.20.99: fix for bug #
	    // on some soundblaster cards, we get an error return of 33, or WAVERR_STILLPLAYING,
	    // which is not documented as an error returned from this call.  to avoid hanging in
	    // the while loop below, we don't increment g_lastPos if an error is returned.
	    if (!error) {

		// $$kk: 08.12.98 merge
		// $$kk: 05.06.98: i am incrementing g_lastPos right after the write operation
		// to lessen the chance of a device open/close messing us up in the wait loop.
		g_lastPos += (g_audioByteBufferSize * g_synthFramesPerBlock);
	    }

	    waveHeaderCount++;
	    if (waveHeaderCount == HAE_WAVEOUT_FRAMES_PER_BLOCK) {
		waveHeaderCount = 0;
	    }
	    error = waveOutGetPosition(g_waveDevice, &audioStatus, sizeof(MMTIME));

	    // $$kk: 08.20.99: added this to fix / work around bug #4256157: Player hangs on systems
	    // with Sound Blaster cards, under WIN 95 & 98.  on affected systems, the device sample
	    // count sometimes inexplicably drops to 0; without this change, we hang in this loop.
	    //if(audioStatus.u.cb < currentPos)
	    //{
	    //fprintf(stderr, "\n\n\n**************** CURRENT LASTPOS TOO HIGH!! (1) *****************\n\n\n");
	    //fprintf(stderr, "g_lastPos: %d, audioStatus.u.cb: %d, (g_audioByteBufferSize * g_synthFramesPerBlock): %d\n", g_lastPos, audioStatus.u.cb, (g_audioByteBufferSize * g_synthFramesPerBlock));

	    // $$kk: 10.20.99: should no longer need this
	    // g_lastPos = audioStatus.u.cb - (g_audioByteBufferSize * g_synthFramesPerBlock * 2);
	    //}


	    //if (error != 0) fprintf(stderr, "\n\n\n**************** ERROR *****************\n\n\n");
	    currentPos = audioStatus.u.cb;

	    // $$kk: 03.21.00: make sure we sleep at least once so that other threads can run.
	    // this is part of the fix for bug #4318062: "MixerSourceLine.drain hangs after
	    // repeated use."
	    // while ((currentPos < g_lastPos) && (g_shutDownDoubleBuffer == FALSE))
	    do {
		HAE_SleepFrameThread(threadContext, HAE_WAVEOUT_SOUND_PERIOD);		// in ms
		error = waveOutGetPosition(g_waveDevice, &audioStatus, sizeof(MMTIME));

		// $$kk: 08.20.99: added this to fix / work around bug #4256157: Player hangs on systems
		// with Sound Blaster cards, under WIN 95 & 98.  on affected systems, the device sample
		// count sometimes inexplicably drops to 0; without this change, we hang in this loop.
		//if(audioStatus.u.cb < currentPos)
		//{
		//fprintf(stderr, "\n\n\n**************** CURRENT LASTPOS TOO HIGH!! (2) *****************\n\n\n");
		//fprintf(stderr, "g_lastPos: %d, audioStatus.u.cb: %d, (g_audioByteBufferSize * g_synthFramesPerBlock * 2): %d\n", g_lastPos, audioStatus.u.cb, (g_audioByteBufferSize * g_synthFramesPerBlock));

		// $$kk: 10.20.99: should no longer need this
		// g_lastPos = audioStatus.u.cb - (g_audioByteBufferSize * g_synthFramesPerBlock * 2);
		//}

		currentPos = audioStatus.u.cb;
		//if (error != 0) fprintf(stderr, "\n\n\n**************** ERROR *****************\n\n\n");
	    } while ((currentPos < g_lastPos) &&
		     (g_lastPos - currentPos < (1 << 28)) && /* See Note A */
		     (g_shutDownDoubleBuffer == FALSE));

	    // Note A: $$ay: Additional safeguard for wraparound of sample
	    // ------  count from 1 << 32 - 1.  Make sure the difference is
	    //         not a huge value

	    // $$kk: 08.12.98 merge
	    // $$kk: 05.06.98: moved this line up.
	    //g_lastPos += (g_audioByteBufferSize * g_synthFramesPerBlock);

	}
    }
#ifdef DEBUG_OUTPUT
    if (fh != 0) {
	HAE_FileClose(fh);
	printf("Closed debug file.\n");
	fh = 0;
    }
#endif

    waveOutReset(g_waveDevice);		// stop all audio before unpreparing headers

    for (count = 0; count < HAE_WAVEOUT_FRAMES_PER_BLOCK; count++) {
	error = waveOutUnprepareHeader(g_waveDevice, &waveHeader[count], (INT32)sizeof(WAVEHDR));
    }
    g_activeDoubleBuffer = FALSE;
}



// Return the number of 11 ms buffer blocks that are built at one time.
int HAE_GetAudioBufferCount(void)
{
    return g_synthFramesPerBlock;
}

// Return the number of bytes used for audio buffer for output to card
INT32 HAE_GetAudioByteBufferSize(void)
{
    return g_audioByteBufferSize;
}


/*
static HRESULT PV_ConvertDSErrorCodeToSomethingReal(HRESULT hr)
{
	switch (hr)
	{
		default:
			hr = 1;
			break;
		case DS_OK:
			hr = 2;
			break;
		case DSERR_ALLOCATED:
			hr = 3;
			break;
		case DSERR_CONTROLUNAVAIL:
			hr = 4;
			break;
		case DSERR_INVALIDPARAM:
			hr = 5;
			break;
		case DSERR_INVALIDCALL:
			hr = 6;
			break;
		case DSERR_GENERIC:
			hr = 7;
			break;
		case DSERR_PRIOLEVELNEEDED:
			hr = 8;
			break;
		case DSERR_OUTOFMEMORY:
			hr = 9;
			break;
		case DSERR_BADFORMAT:
			hr = 10;
			break;
		case DSERR_UNSUPPORTED:
			hr = 11;
			break;
		case DSERR_NODRIVER:
			hr = 12;
			break;
		case DSERR_ALREADYINITIALIZED:
			hr = 13;
			break;
		case DSERR_NOAGGREGATION:
			hr = 14;
			break;
		case DSERR_BUFFERLOST:
			hr = 15;
			break;
		case DSERR_OTHERAPPHASPRIO:
			hr = 16;
			break;
		case DSERR_UNINITIALIZED:
			hr = 17;
			break;
	}
	return hr;
}
*/


// DWORD PV_AudioDirectSoundFrameThread (LPVOID lpv)
//
// This is the thread function that fills the current block of the
// output buffer.
//
// This thread will sleep until the "g_heventProcessFrameStart" event
// is set by the PV_MonitorDirectSoundPosition thread.  After the current
// block has been filled, the "g_heventProcessFrameFinish" event is
// set and the thread goes back to sleep.
//
// The call to ProcessSampleFrame is what drives the synth engine.
// Each call to this function generates "g_SPD.uiSynthFrameBytes"
// bytes of audio data.  HAE_BuildMixerSlice will be called
// g_SPD.uiSynthFramesPerBlock times to fill the current block.
//
// If the flag "g_SPD.bWaveEnd" is set then PV_AudioDirectSoundFrameThread will fill
// the current block with silence instead of calling generating
// more audio samples.
#if USE_DIRECTSOUND
static void PV_AudioDirectSoundFrameThread(void* threadContext)
{
    HRESULT		error;
    LPVOID		lpWrite;
    DWORD		dwWriteLen;
    DWORD		count;
    char		*pFillBuffer;

    while ( (g_activeDoubleBuffer) && (g_shutDownDoubleBuffer == FALSE) )
	{
	    g_bProcessingFrame = FALSE;
	    // Wait for someone to wake us up.
	    WaitForSingleObject(g_heventProcessFrameStart, INFINITE);
	    g_bProcessingFrame = TRUE;

	    // Lock the current block of the DirectSound buffer.
	    error = IDirectSoundBuffer_Lock(g_pDirectSoundBuffer,
					    g_SPD.uiDestPos, g_SPD.uiBlockSize,
					    &lpWrite, &dwWriteLen, NULL, NULL, 0);

	    if (error == DS_OK)
		{
		    // If we're to generate more audio samples
		    if (!g_SPD.bWaveEnd)
			{
				// For each frame of the current block
			    for (count = 0; count < g_SPD.uiSynthFramesPerBlock; count++)
				{
				    pFillBuffer = ((char *)lpWrite) + count * g_SPD.uiSynthFrameBytes;

				    // Generate new audio samples, putting the directly
				    // into the output buffer.
				    HAE_BuildMixerSlice(threadContext, pFillBuffer, g_audioByteBufferSize, g_audioFramesToGenerate);
				    if (g_shutDownDoubleBuffer)
					{
					    break;
					}
				}
			}
		    else
			{
				// Fill the current block with appropriate silence value/
			    memset(lpWrite, (g_waveFormat.wBitsPerSample == 16) ? 0 : 0x80, dwWriteLen);
			}
		    // Unlock the block of SoundBuffer were we writing.
		    error = IDirectSoundBuffer_Unlock(g_pDirectSoundBuffer,
						      lpWrite, dwWriteLen, NULL, 0);

		    // Advance to the next block in the output buffer
		    g_SPD.uiWriteBlock++;
		    // We're circular, so when we reach the end of the buffer,
		    // start back at the beginning.
		    if (g_SPD.uiWriteBlock > (g_SPD.uiBufferBlocks - 1))
			{
			    g_SPD.uiWriteBlock = 0;
			}
		    // Calculate the new offsets.
		    g_SPD.uiDestPos = g_SPD.uiWriteBlock * g_SPD.uiBlockSize;
		}

	    // Set the ProcessFrameFinish event before going back to sleep.
	    SetEvent(g_heventProcessFrameFinish);
	}

    g_activeDoubleBuffer = FALSE;
}



// DWORD PV_MonitorDirectSoundPosition(LPVOID lpv)
//
// This is the main thread routine for our engine.
//
// Here's a synopsis of what's going on:
// - Divide the output buffer into g_nBufferSegments.
// - Poll the DirectSound play cursor.
// - When the play cursor crosses the transition
//   from iPrevBlock to iCurBlock, wake up the PV_AudioDirectSoundFrameThread thread
//   to fill the next block.
// - Wait for PV_AudioDirectSoundFrameThread to finish
// - Sleep so that we aren't hogging the CPU with our polling
// - Wake up and poll the play cursor again.
//
// When the g_SPD.bWaveEnd flag is set, indicating that we
// should stop:
// - Mark the block we were polling in when the flag was set.
// - Wake up the PV_AudioDirectSoundFrameThread thread to fill the last block.
// - Let the polling engine know that the last block has been
//   filled by setting the bWaitingForLastBlock flag.
// - Wait for the last block to be played by continuing to poll
//   until the play cursor has passed through the last block.
//
// Note:
//   The block indecies used in this polling thread are not the same
//   as the indecies used in the PV_AudioDirectSoundFrameThread thread.  They are
//   offset by one because the first block of the sound buffer is
//   filled with silence.  This allows the engine to generate the
//   first audio frames while this silent block is playing.
//
//   There is a problem with this design though.  If PV_AudioDirectSoundFrameThread takes
//   more time to process the current block than it takes to play the
//   previous block, the polling loop thread will have missed the next
//   block transition.  This will result in having to wait until the same
//   transition comes around again before more audio is generated.  The
//   blocks after the missed transition will not be filled with new data
//   therefore old audio blocks will be replayed.

static DWORD WINAPI PV_MonitorDirectSoundPosition(LPVOID lpv)
{
    BOOL			bWaitingForLastBlock;
    DWORD			dwPlayPos, dwWritePos;
    int				iPrevBlock;
    int				iCurBlock;
    SOUNDPOSBLOCK	*pSoundBlock;
    DWORD			dwCurPos;
    DWORD			count;

    lpv = lpv;
    dwCurPos = 0;
    iPrevBlock = 0;
    iCurBlock = 0;
    bWaitingForLastBlock = FALSE;
    pSoundBlock = (SOUNDPOSBLOCK *)HAE_Allocate(sizeof(SOUNDPOSBLOCK) * g_SPD.uiBufferBlocks);
    if (pSoundBlock)
	{
	    // Calculate the boundaries for each of the blocks
	    for (count = 0; count < g_SPD.uiBufferBlocks; count++)
		{
		    pSoundBlock[count].dwStart = dwCurPos;
		    pSoundBlock[count].dwEnd = dwCurPos + g_SPD.uiBlockSize;
		    pSoundBlock[count].bEndingBlock = FALSE;
		    dwCurPos += g_SPD.uiBlockSize;
		}

	    // While the engine is not done
	    while(!g_SPD.bDone)
		{
		    // Give up some of our time slice
		    Sleep(g_audioPeriodSleepTime);

		    // Get the current play position
		    IDirectSoundBuffer_GetCurrentPosition(g_pDirectSoundBuffer, &dwPlayPos, &dwWritePos);

		    // If the play position has entered the current block and it hasn't
		    // reached the end of the block yet
		    if (dwPlayPos >= pSoundBlock[iCurBlock].dwStart &&
			dwPlayPos < pSoundBlock[iCurBlock].dwEnd)
			{
				// Wake up PV_AudioDirectSoundFrameThread to generate a block of audio
			    SetEvent(g_heventProcessFrameStart);
				// Wait for PV_AudioDirectSoundFrameThread to finish
			    WaitForSingleObject(g_heventProcessFrameFinish, INFINITE);

				// If the this is the last block to be generated
				// and we haven't marked it yet
			    if (g_SPD.bWaveEnd && !bWaitingForLastBlock)
				{
				    // Mark the current block as the last block
				    pSoundBlock[iCurBlock].bEndingBlock = TRUE;
				    bWaitingForLastBlock = TRUE;
				}

				// If we're waiting for the last block to play
			    if (bWaitingForLastBlock)
				{
				    iPrevBlock = iCurBlock - 1;
				    if (iPrevBlock < 0)
					{
					    iPrevBlock = g_SPD.uiBufferBlocks - 1;
					}
				    // If the current block is the last block
				    if (pSoundBlock[iPrevBlock].bEndingBlock)
					{
					    // End the thread and stop playing sound
					    g_SPD.bDone = TRUE;
					    IDirectSoundBuffer_Stop(g_pDirectSoundBuffer);
					}
				}

				// Go to the next block.
			    iCurBlock++;
				// If it's the last in the list wrap back to the first.
			    if ((UINT)iCurBlock > g_SPD.uiBufferBlocks - 1)
				{
				    iCurBlock = 0;
				}
			}
		}
	}

    // Get rid of our list of sound blocks
    HAE_Deallocate(pSoundBlock);
    return 0;
}

// void PV_ClearDirectSoundBuffer ()
//
// Files the DirectSound buffer with silence.

static void PV_ClearDirectSoundBuffer(void)
{
    HRESULT		error;
    LPVOID		lpWrite;
    DWORD		dwWriteLen;

    if (g_pDirectSoundBuffer)
	{
	    error = IDirectSoundBuffer_Lock(g_pDirectSoundBuffer, 0,
					    g_SPD.uiBlockSize * g_SPD.uiBufferBlocks,
					    &lpWrite, &dwWriteLen, NULL, NULL,
					    0);
	    if (error == DS_OK)
		{
		    memset(lpWrite, (g_waveFormat.wBitsPerSample == 16) ? 0 : 0x80, dwWriteLen);
		    error = IDirectSoundBuffer_Unlock(g_pDirectSoundBuffer,
						      lpWrite, dwWriteLen, NULL, 0);
		}
	}
}

// int PV_CreateDirectSoundThreads()
//
// Inititialize the SOUNDPOSDATA structure and create the threads
// and events for our engine
// returns -1 if failure, or 0 for success
static int PV_CreateDirectSoundThreads(void* threadContext)
{
    int		status;
    DWORD	threadID;

    status = -1;
    g_heventProcessFrameStart = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (g_heventProcessFrameStart)
	{
	    g_heventProcessFrameFinish = CreateEvent(NULL, FALSE, FALSE, NULL);
	    if (g_heventProcessFrameFinish)
		{
		    // Create the monitor thread
		    g_hthreadSoundMon = CreateThread(NULL, 0L,
						     PV_MonitorDirectSoundPosition, 0L,
						     0L, &threadID);
		    if (g_hthreadSoundMon)
			{
			    if (HAE_CreateFrameThread(threadContext, PV_AudioDirectSoundFrameThread) >= 0)
				{
				    // THREAD_PRIORITY_HIGHEST
				    // THREAD_PRIORITY_NORMAL
				    // THREAD_PRIORITY_TIME_CRITICAL
				    SetThreadPriority(g_hthreadSoundMon, THREAD_PRIORITY_TIME_CRITICAL);
				    status = 0;
				}
			}
		}
	}
    return status;
}


// setup direct sound, create a primary buffer and set format to match.
// Return 0 if success, or -1 if it fails
static int PV_SetupDirectSound(void* threadContext, int sampleRate, int channels, int bitSize)
{
    int				status;
    HRESULT			error;
    DSBUFFERDESC	dsbd;

    status = -1;
    memset(&g_waveFormat, 0, (DWORD)sizeof(WAVEFORMATEX));

    if (g_directSoundWindow == NULL)
	{
	    g_directSoundWindow = PV_GetMostActiveWindow();
	}
    if (g_directSoundWindow)
	{
	    // Use standard PCM audio samples

	    g_waveFormat.wFormatTag = WAVE_FORMAT_PCM;

	    g_waveFormat.nSamplesPerSec = sampleRate;
	    g_waveFormat.nChannels = channels;
	    g_waveFormat.wBitsPerSample = bitSize;

	    // Calculate size in bytes of an audio sample
	    g_waveFormat.nBlockAlign =  g_waveFormat.nChannels * g_waveFormat.wBitsPerSample / 8;
	    g_waveFormat.nAvgBytesPerSec = g_waveFormat.nSamplesPerSec * g_waveFormat.nBlockAlign;
	    g_waveFormat.cbSize = 0;

	    // Try and create a DirectSound object
	    if (DirectSoundCreate(NULL, &g_pDirectSoundObject, NULL) == DS_OK)
		{
		    // First thing that must be done after creating object
		    // Set level to PRIORITY so that we can change format of primary buffer
		    error = IDirectSound_SetCooperativeLevel(g_pDirectSoundObject,
							     g_directSoundWindow, DSSCL_PRIORITY);
		    if (error == DS_OK)
			{
				// Create a primary buffer and set its format to that of our synth engine
			    dsbd.dwSize = sizeof(dsbd);
			    dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;

			    dsbd.dwBufferBytes = 0;
			    dsbd.dwReserved = 0;
			    dsbd.lpwfxFormat = NULL;
			    error = IDirectSound_CreateSoundBuffer(g_pDirectSoundObject, &dsbd, &g_pDirectSoundPrimaryBuffer, NULL);

			    if ( (error == DS_OK) && (g_pDirectSoundPrimaryBuffer) )
				{
				    // By setting the primary buffer to have the same format as our generated
				    // synth data, there will be no extra overhead when DirectSound mixes
				    // our "Secondary" buffer into the "Primary" buffer.
				    error = IDirectSoundBuffer_SetFormat(g_pDirectSoundPrimaryBuffer,
									 &g_waveFormat);
				    if (error == DS_OK)
					{
					    // Now we can create the secondary sound buffer we will use to output sound
					    dsbd.dwSize = sizeof(dsbd);
					    dsbd.dwFlags = DSBCAPS_STICKYFOCUS;
					    dsbd.dwFlags |= DSBCAPS_GETCURRENTPOSITION2;
					    //						dsbd.dwFlags |= DSBCAPS_GLOBALFOCUS;
					    dsbd.dwBufferBytes = g_waveFormat.nBlockAlign *
						g_audioFramesToGenerate *
						g_synthFramesPerBlock *
						g_nBufferSegments;
					    dsbd.dwReserved = 0;
					    dsbd.lpwfxFormat = &g_waveFormat;

					    error = IDirectSound_CreateSoundBuffer(g_pDirectSoundObject, &dsbd, &g_pDirectSoundBuffer, NULL);
					    if ( (error == DS_OK) && (g_pDirectSoundPrimaryBuffer) )
						{
						    // Make sure we start at the beginning of the buffer
						    error = IDirectSoundBuffer_SetCurrentPosition(g_pDirectSoundBuffer, 0);
						    if (error == DS_OK)
							{
							    // Start by writing an empty block
							    PV_ClearDirectSoundBuffer();

							    g_SPD.uiSynthFramesPerBlock = g_synthFramesPerBlock;
							    // Number of bytes per audio frame
							    g_SPD.uiSynthFrameBytes = g_audioFramesToGenerate * g_waveFormat.nBlockAlign;
							    // Initial output buffer offset to start wrtting audio data
							    g_SPD.uiDestPos = g_SPD.uiSynthFrameBytes * g_SPD.uiSynthFramesPerBlock;
							    // Total number of bytes generated on each pass through PV_AudioDirectSoundFrameThread
							    g_SPD.uiBlockSize = g_SPD.uiSynthFrameBytes * g_SPD.uiSynthFramesPerBlock;
							    // Total number of blocks in output buffer
							    g_SPD.uiBufferBlocks = g_nBufferSegments;
							    // Total number of bytes in output buffer
							    g_SPD.uiBufferSize = g_SPD.uiBlockSize * g_nBufferSegments;
							    // Index of next block to we written
							    g_SPD.uiWriteBlock = 1;
							    g_SPD.bWaveEnd = FALSE;
							    g_SPD.bDone = FALSE;

							    // Create thread to poll sound play position
							    status = PV_CreateDirectSoundThreads(threadContext);
							    if (status == 0)
								{
								    // Start playing whatever is in the output buffer (silence initially)
								    error = IDirectSoundBuffer_Play(g_pDirectSoundBuffer,
												    0,0,DSBPLAY_LOOPING);
								    if (error == DS_OK)
									{
									    status = 0;
									}
								    else
									{
									    status = -1;
									}
								}
							}
						}
					}
				}
			}
		}
	}
    return status;
}

static void PV_CleanupDirectSound(void)
{
    if (g_pDirectSoundBuffer)
	{
	    IDirectSoundBuffer_Stop(g_pDirectSoundBuffer);
	}
    if (g_pDirectSoundPrimaryBuffer)
	{
	    IDirectSoundBuffer_Release(g_pDirectSoundPrimaryBuffer);
	    g_pDirectSoundPrimaryBuffer = NULL;
	}
    if (g_pDirectSoundBuffer)
	{
	    IDirectSoundBuffer_Release(g_pDirectSoundBuffer);
	    g_pDirectSoundBuffer = NULL;
	}
    if (g_pDirectSoundObject)
	{
	    IDirectSoundBuffer_Release(g_pDirectSoundObject);
	    g_pDirectSoundObject = NULL;
	}
    if (g_heventProcessFrameStart)
	{
	    CloseHandle(g_heventProcessFrameStart);
	    g_heventProcessFrameStart = NULL;
	}
    if (g_heventProcessFrameFinish)
	{
	    CloseHandle(g_heventProcessFrameFinish);
	    g_heventProcessFrameFinish = NULL;
	}
    g_directSoundWindow = NULL;
}
#endif	// USE_DIRECTSOUND

// **** Audio card support
// Aquire and enabled audio card
// return 0 if ok, -1 if failed
int HAE_AquireAudioCard(void *threadContext, UINT32 sampleRate, UINT32 channels, UINT32 bits)
{
    int				flag;
    short int		count;
    INT32			bufferSize;
    INT32			error;
    WAVEFORMATEX	waveFormat;

    flag = 0;
    g_activeDoubleBuffer = FALSE;
    g_shutDownDoubleBuffer = TRUE;

    g_audioFramesToGenerate = HAE_GetMaxSamplePerSlice();	// get number of frames per sample rate slice

    // use DirectSound or waveOut API
    if (PV_UseDirectSound() == HAE_WAVEOUT)
	{	// use waveOut API
	    // we're going to build this many buffers at a time
	    g_synthFramesPerBlock = HAE_WAVEOUT_FRAMES_PER_BLOCK;

	    // Use waveOut API
	    waveFormat.wFormatTag = WAVE_FORMAT_PCM;

	    waveFormat.nSamplesPerSec = sampleRate;
	    waveFormat.nChannels = (unsigned short)channels;
	    waveFormat.wBitsPerSample = (unsigned short)bits;

	    // Calculate size in bytes of an audio sample
	    waveFormat.nBlockAlign =  waveFormat.nChannels * waveFormat.wBitsPerSample / 8;
	    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	    waveFormat.cbSize = 0;
	    g_waveFormat = waveFormat;

	    if (waveFormat.wBitsPerSample == 8)
		{
		    bufferSize = (sizeof(char) * g_audioFramesToGenerate);
		}
	    else
		{
		    bufferSize = (sizeof(short int) * g_audioFramesToGenerate);
		}
	    bufferSize *= waveFormat.nChannels;
	    g_audioByteBufferSize = bufferSize;


	    // allocate buffer blocks
	    flag = TRUE;
	    for (count = 0; count < HAE_WAVEOUT_FRAMES_PER_BLOCK; count++)
		{
		    g_audioBufferBlock[count] = HAE_Allocate(g_audioByteBufferSize * HAE_WAVEOUT_FRAMES_PER_BLOCK);
		    if (g_audioBufferBlock[count] == NULL)
			{
			    flag = FALSE;
			    break;
			}
		    // $$jb:11.10.99: Fix for 4266439: loud pop
		    // if the device is open in an 8-bit mode, samples are unsigned,
		    // so fill the bufferes with 0x80 as silence instead of 0x00
		    if(waveFormat.wBitsPerSample == 8) {
			memset(g_audioBufferBlock[count], 0x80, (g_audioByteBufferSize * HAE_WAVEOUT_FRAMES_PER_BLOCK) );
		    }
		}

	    // try to open wave device
	    if (flag)
		{
		    error = waveOutOpen(&g_waveDevice,	WAVE_MAPPER, &waveFormat,
					0L, 0L,			CALLBACK_NULL
					//				| WAVE_FORMAT_DIRECT
					//				| WAVE_ALLOWSYNC
					//				| WAVE_MAPPED
					);
		    if (error == 0)
			{
			    g_shutDownDoubleBuffer = FALSE;
			    g_activeDoubleBuffer = TRUE;	// must enable process, before thread begins


				// $$kk: 08.12.98 merge
				// $$kk: 05.06.98: added this whole block.
				// we need to reset the g_lastPos each time the device gets acquired.
				// otherwise we may get stuck in the wait loop because we never count
				// up to the right sample position.
			    {
				MMTIME			audioStatus;
				memset(&audioStatus, 0, (INT32)sizeof(MMTIME));
				audioStatus.wType = TIME_BYTES;	// get byte position

				error = waveOutGetPosition(g_waveDevice, &audioStatus, sizeof(MMTIME));

				g_lastPos = audioStatus.u.cb - (g_audioByteBufferSize * g_synthFramesPerBlock * 2);
			    }


				// create thread that builds and sends buffers to the wave device
			    if (HAE_CreateFrameThread(threadContext, PV_AudioWaveOutFrameThread) >= 0)
				{
				    flag = 0;
				    //$$fb enable high resolution time
				    timeBeginPeriod(1);
				}
			    else
				{
				    flag = 1;
				    g_activeDoubleBuffer = FALSE;
				}
			}
		    else
			{
			    flag = 1;
			}
		}
	}
#if USE_DIRECTSOUND
    else
	{	// use DirectSound API
	    // we're going to build this many buffers at a time
	    if (PV_IsNT() == FALSE)		// under Win95?
		{
		    g_synthFramesPerBlock = HAE_DIRECTSOUND_FRAMES_PER_BLOCK;
		    g_audioPeriodSleepTime = HAE_DIRECTSOUND_SOUND_PERIOD;
		}
	    else
		{
		    g_synthFramesPerBlock = HAE_DIRECTSOUND_FRAMES_PER_BLOCK_NT;
		    g_audioPeriodSleepTime = HAE_DIRECTSOUND_SOUND_PERIOD_NT;
		}
	    g_shutDownDoubleBuffer = FALSE;
	    g_activeDoubleBuffer = TRUE;	// must enable process, before thread begins
	    g_audioByteBufferSize = g_audioFramesToGenerate * channels * (bits / 8);

	    flag = PV_SetupDirectSound(threadContext, sampleRate, channels, bits);
	    if (flag != 0)
		{
		    // something failed
		    flag = 1;
		    g_activeDoubleBuffer = FALSE;
		} else {
		    //$$fb enable high resolution time
		    timeBeginPeriod(1);
		}
	}
#endif	// USE_DIRECTSOUND
    if (flag)
	{	// something failed
	    HAE_ReleaseAudioCard(threadContext);
	}

    return flag;
}

// Release and free audio card.
// return 0 if ok, -1 if failed.
int HAE_ReleaseAudioCard(void *threadContext)
{
    INT32		count;
    HAEDeviceID	status;

    g_shutDownDoubleBuffer = TRUE;	// signal the frame thread to stop
    status = PV_UseDirectSound();
#if USE_DIRECTSOUND
    if (status == HAE_DIRECTSOUND)
	{	// DirectSound
	    // Did we create the SoundMon thread?
	    if (g_hthreadSoundMon)
		{
		    // Signal thread to stop generating audio
		    g_SPD.bWaveEnd = TRUE;

		    // wake up threads
		    SetEvent(g_heventProcessFrameStart);
		    SetEvent(g_heventProcessFrameFinish);

		    // Start by writing an empty block
		    PV_ClearDirectSoundBuffer();
		    // wait for thread to end and only wait 500 ms
		    count = WaitForSingleObject(g_hthreadSoundMon, 500);
		    // If thread takes too long, kill it
		    if (count == WAIT_TIMEOUT)
			{
			    TerminateThread(g_hthreadSoundMon, 0);
			    g_hthreadSoundMon = NULL;
			}
		    CloseHandle(g_hthreadSoundMon);
		    g_hthreadSoundMon = NULL;

		    //$$fb disable high resolution time
		    timeEndPeriod(1);
		}
	    PV_CleanupDirectSound();
	}
#endif	// USE_DIRECTSOUND
    HAE_DestroyFrameThread(threadContext);


    // $$kk: 08.20.99: removed calls to waveOutReset here.
    // added code to check for exiting of the frame proc thread.

    while(g_activeDoubleBuffer)
	{
	    // $$kk: 08,20.99: wait for thread to complete
	    HAE_SleepFrameThread(threadContext, HAE_WAVEOUT_SOUND_PERIOD);
	}

    // use DirectSound or waveOut API
    if (status == HAE_WAVEOUT)
	{	// waveOut
	    if (g_waveDevice)
		{
		    while (waveOutClose(g_waveDevice) == WAVERR_STILLPLAYING)
			{
				// $$kk: 08,20.99: just wait instead
			    HAE_SleepFrameThread(threadContext, HAE_WAVEOUT_SOUND_PERIOD);
			}
		    g_waveDevice = 0;
		    //$$fb disable high resolution time
		    timeEndPeriod(1);
		}
	    for (count = 0; count < HAE_WAVEOUT_FRAMES_PER_BLOCK; count++)
		{
		    HAE_Deallocate(g_audioBufferBlock[count]);
		    g_audioBufferBlock[count] = NULL;
		}
	}

    return 0;
}

// return device position in samples since the device was opened
UINT32 HAE_GetDeviceSamplesPlayedPosition(void)
{
    HAEDeviceID		status;
    int				error, frameSize;
    MMTIME			audioStatus;
    UINT32	pos;

    pos = 0;
    frameSize = g_waveFormat.nChannels * g_waveFormat.wBitsPerSample / 8;
    status = PV_UseDirectSound();
#if USE_DIRECTSOUND
    if (status == HAE_DIRECTSOUND)
	{	// DirectSound
	    UINT32	dwPlayPos, dwWritePos;

	    IDirectSoundBuffer_GetCurrentPosition(g_pDirectSoundBuffer, &dwPlayPos, &dwWritePos);
	    // return in samples
	    pos = dwPlayPos / frameSize;
	    // this function does not work!!!!
	}
#endif	// USE_DIRECTSOUND

    if (status == HAE_WAVEOUT)
	{	// waveOut
	    memset(&audioStatus, 0, (INT32)sizeof(MMTIME));
	    audioStatus.wType = TIME_BYTES;	// get byte position

	    error = waveOutGetPosition(g_waveDevice, &audioStatus, sizeof(MMTIME));
	    // return in samples
	    pos = audioStatus.u.cb / frameSize;
	}
    return pos;
}

// number of devices. ie different versions of the HAE connection. DirectSound and waveOut
// return number of devices. ie 1 is one device, 2 is two devices.
// NOTE: This function needs to function before any other calls may have happened.
INT32 HAE_MaxDevices(void)
{
#if USE_DIRECTSOUND
    return 2;	// waveOut and DirectSound
#else
    if (waveOutGetNumDevs() > 0) {
    	return 1;
    }
    return 0;
#endif
}

// set the current device. device is from 0 to HAE_MaxDevices()
// NOTE:	This function needs to function before any other calls may have happened.
//			Also you will need to call HAE_ReleaseAudioCard then HAE_AquireAudioCard
//			in order for the change to take place.
void HAE_SetDeviceID(INT32 deviceID, void *deviceParameter)
{
    deviceParameter = deviceParameter;
    if (deviceID < HAE_MaxDevices())
	{
	    g_currentDeviceID = (HAEDeviceID)deviceID;
	    switch (g_currentDeviceID)
		{
		case HAE_WAVEOUT:		// waveout
		    break;
#if USE_DIRECTSOUND
		case HAE_DIRECTSOUND:	// directsound
		    if (deviceParameter)
			{
			    g_directSoundWindow = (HWND)deviceParameter;
			}
		    break;
#endif
		}
	}
}

// return current device ID
// NOTE: This function needs to function before any other calls may have happened.
INT32 HAE_GetDeviceID(void *deviceParameter)
{
    if (deviceParameter)
	{
	    *(INT32 *)deviceParameter = 0;
	}
    switch (g_currentDeviceID)
	{
	case HAE_WAVEOUT:		// waveout
	    break;
#if USE_DIRECTSOUND
	case HAE_DIRECTSOUND:	// directsound
	    if (deviceParameter)
		{
		    *(INT32 *)deviceParameter = (INT32)g_directSoundWindow;
		}
	    break;
#endif
	}
    return g_currentDeviceID;
}

// NOTE:	This function needs to function before any other calls may have happened.
//			Format of string is a zero terminated comma delinated C string.
//			"platform,method,misc"
//	example	"MacOS,Sound Manager 3.0,SndPlayDoubleBuffer"
//			"WinOS,DirectSound,multi threaded"
//			"WinOS,waveOut,multi threaded"
//			"WinOS,VxD,low level hardware"
//			"WinOS,plugin,Director"
void HAE_GetDeviceName(INT32 deviceID, char *cName, UINT32 cNameLength)
{
    char		*data;
    static char *names[] =
    {	"WinOS,waveOut,multi threaded"
#if USE_DIRECTSOUND
	,"WinOS,DirectSound,multi threaded"
#endif
    };
    if (cName && cNameLength)
	{
	    if (deviceID < HAE_MaxDevices())
		{
		    // can't do string functions here, it might be bad to do.
		    data = names[deviceID];
		    while (*data && (cNameLength > 0))
			{
			    *cName = *data;
			    cName++;
			    data++;
			    cNameLength--;
			}
		    *cName = 0;
		}
	}
}


// EOF of HAE_API_WinOS.c
