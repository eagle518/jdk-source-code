/*
 * @(#)HAE_API_SolarisOS.c	1.37 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**	HAE_API_SolarisOS.c
**
**	This provides platform specfic functions for Sun Solaris OS. This interface
**	for HAE uses the dev/audio device
**
**	Overview:
**		This works by creating another thread, aquires the sound device via
**		open("/dev/audio",O_WRONLY) call. Then allocating enough buffers, preps then,
**		and build buffers depending upon how much data has been written to the
**		audio device. The thread polls the current position in the audio stream
**		that has been written and when it falls below a buffer or two, more
**		are built in that thread.
**
**	History	-
**	12/1/97		Gleaned from the orignal GenSolarisTools.c file
**	1/9/98		Added HAE_FileDelete
**	2/13/98		Modified HAE_AquireAudioCard to handle different sample rates
**	3/17/98		Fixed a test for 16/8 bit support in PV_AudioFrameThread
**				Added HAE_Is8BitSupported
**
**	12.09.98	$$kk: added HAE_StartAudioOutput and HAE_StopAudioOutput to just 
**				start and stop the audio output thread.  changed HAE_AquireAudioCard
**				to not startthe thread automatically.  HAE_ReleaseAudioCard does stop
**				the thread automatically.
**
**	03.17.99	$$kk: removed HAE_StartAudioOutput and HAE_StopAudioOutput  and
**				reversed above changes.
**
**	05.12.99:	$$kk: added very simple AUDIODEV environment variable support.
**
**  2001-07-13: $$fb: changes related to using ctl interface for AUDIODEV devices
**                    added specific audiots (SunBlade) and Creative Labs SB Ultra audio driver
**                    minor clean ups
*/
/*****************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <errno.h>
#ifdef __linux__
#include <asm-sparc/audioio.h>
#include <malloc.h>
#else
#include <sys/audio.h> 
#include <sys/mixer.h> 
#include <sys/audioio.h>
#endif


// $$kk: 08.12.98 merge 

// $$fb: TODO: support multiple devices !
// $$fb: TODO: share the same functions of HAE_API_SolarisOS_Capture.c

// $$kk: 04.28.98: we *need* to use string.h, NOT strings.h; 
// the latter is not available on earlier versions of solaris
#include <string.h>

/* for GM_GetSyncTimeStamp() */
#include "GenSnd.h"
#include "HAE_API.h"
#include "Configure.h"
#include "Utilities.h"

#ifndef TRUE
#define TRUE		1
#endif
#ifndef FALSE
#define FALSE		0
#endif

// How many audio frames to generate at one time 
#define HAE_SOLARIS_FRAMES_PER_BLOCK	8
#define HAE_SOLARIS_SOUND_PERIOD	10	// sleep time in milliseconds, between position checks

// $$fb 2003-02-10: check raw written data
//#define USE_RAWDATA_CHECK

static void    *g_audioBufferBlock;
static INT32   g_audioByteBufferSize; // size of audio buffers in bytes

static int     g_waveDevice = 0;
static INT32   g_shutDownDoubleBuffer;
static INT32   g_activeDoubleBuffer;

static short int g_bitSize;
static short int g_channels;

// $$kk: this is used in HAE_SetDeviceID and HAE_GetDeviceID but was not defined
// $$fb: TODO: support multiple devices ??
INT32 g_currentDeviceID = 0;

// number of samples per audio frame to generate
INT32 g_audioFramesToGenerate;

// How many audio frames to generate at one time 
static unsigned int g_synthFramesPerBlock;	// setup upon runtime

static unsigned int g_audioPeriodSleepTime;	// setup upon runtime

// $$kk: 08.12.98 merge: added these 4 variables
// $$kk: 03.17.98: these variables describe device capabilities and requirements
int g_supports16        = -1;
int g_supports8         = -1;
int g_supportsStereo    = -1;
int g_convertUnsigned   = 0;


/* for syncronization of real time MIDI events */
INT64 g_checkpointMicros = 0;
INT64 g_checkpointSyncCount = 0;


// This file contains API's that need to be defined in order to get HAE (IgorAudio)
// to link and compile.


// $$fb: 2001-07-13: add ctl support for AUDIODEV environment variable
//       wouldn't it be great to share these utility functions for capture and playback ?
// isCtl:
//   0 -> the audio device is returned (e.g. /dev/audio)
//   1 -> the control device is returned (e.g. /dev/audioctl)
char* HAE_GetAudioDevPlay(INT32 deviceID, int isCtl) {
    static char ret[300]; // MAX_PATH
    char* audiodev=getenv("AUDIODEV");
    int l=297;
    char* dst, *src;
	
    // currently, ignore deviceID
    deviceID=deviceID;
    if (audiodev==NULL || audiodev[0]==0) {
	audiodev="/dev/audio";
    }
    if (!isCtl) {
	return audiodev;
    }
    // copy device name and append "ctl".
    // $$fb: for some reasons, kara doesn't use string.h functions. 
    // So I don't do it either.
    dst=ret; src=audiodev;
    while (*src && l>0) {
	*dst=*src; src++; dst++; l--;
    }
    *dst='c'; dst++;
    *dst='t'; dst++;
    *dst='l'; dst++;
    *dst=0;
    return ret;
}


// **** System setup and cleanup functions
// Setup function. Called before memory allocation, or anything serious. Can be used to 
// load a DLL, library, etc.
// return 0 for ok, or -1 for failure
int HAE_Setup(void) {
    // $$kk: 03.17.98: i am using this method to query device information for solaris.
    // here are the solaris drivers:
    // audiocs:		supports linear 16 bit data, mono or stereo
    //				does *not* support linear 8 bit data
    // sbpro:		sb16/AWE32 supports linear 8 and 16 bit data
    //				sbpro supports linear 8
    // dbri:		supports linear 16 bit data, mono or stereo
    //				does *not* support linear 8 bit data
    // audioamd:	supports no linear formats
    // audiots:		supports linear 16 bit data, mono or stereo
    //				does *not* support linear 8 bit data

    // we do not publically let people change the rendering format, so right now
    // we'll just do 16 if we can, otherwise 8.  this will have to be done more
    // carefully with the new api.

    int pseudoDevice = 0;
    INT32 error;
    audio_device_t deviceInfo;

    char sbproStr[] = "SUNW,sbpro";
    char sb16Str[] = "SUNW,sb16";
    char audiocsStr[] = "SUNW,CS4231";
    char dbriStr[] = "SUNW,dbri";
    char audioamdStr[] = "SUNW,am79c30";	
    char audiotsStr[] = "SUNW,audiots";	
    char SBUltraStr[] = "CREAF,SBUltra";

    pseudoDevice = open(HAE_GetAudioDevPlay(0, 1), O_RDONLY); // get ctl device

    if (pseudoDevice == -1) {
	//fprintf(stderr, "could not get pseudoDevice\n");
	/* $$fb this is an error, so return error code */
	return -1;
    }

    error = ioctl(pseudoDevice, AUDIO_GETDEV, &deviceInfo);

    close(pseudoDevice);

    if (error == -1) {
	//fprintf(stderr, "could ioctl AUDIO_GETDEV on pseudoDevice\n");
	/* $$fb this is an error, so return error code */
	return -1;
    }


    // choose some defaults
    g_supports16	= 1;
    g_supports8		= 0;
    g_supportsStereo	= 1;   
    g_convertUnsigned	= 0;


    if (strcmp(sbproStr, deviceInfo.name) == 0) {
	// sbpro does not support 16 bit
	// it does support 8 bit with unsigned conversion
	// it does support stereo
	//fprintf(stderr, "found sbpro hardware \n");

	g_supports16		= 0;
	g_supports8		= 1;
	g_supportsStereo	= 1;   
	g_convertUnsigned	= 1;

	return 0;
    }

    if (strcmp(sb16Str, deviceInfo.name) == 0) {
	// sb16 does support 16 bit
	// it does support 8 bit with unsigned conversion
	// it does support stereo
	//fprintf(stderr, "found sb16 hardware \n");

	g_supports16		= 1;
	g_supports8		= 1;
	g_supportsStereo	= 1;   
	g_convertUnsigned	= 1;

	return 0;
    }

    else if (strcmp(audiocsStr, deviceInfo.name) == 0) {
	// audiocs does support 16 bit
	// it does not support 8 bit linear
	// it does support stereo
	//fprintf(stderr, "found audiocs hardware \n");

	g_supports16		= 1;
	g_supports8		= 0;
	g_supportsStereo	= 1;   
	g_convertUnsigned	= 0;

	return 0;
    }

    else if (strcmp(dbriStr, deviceInfo.name) == 0) {
	// dbri does support 16 bit
	// it does not support 8 bit linear
	// it does support stereo
	//fprintf(stderr, "found dbri hardware \n");

	g_supports16		= 1;
	g_supports8		= 0;
	g_supportsStereo	= 1;   
	g_convertUnsigned	= 0;

	return 0;
    }

    else if (strcmp(audioamdStr, deviceInfo.name) == 0) {
	// audioamd does not support 16 bit
	// it does not support 8 bit linear
	// it does not support stereo
	//fprintf(stderr, "found audioamd hardware \n");

	g_supports16		= 0;
	g_supports8		= 0;
	g_supportsStereo	= 0;   
	g_convertUnsigned	= 0;

	return 0;
    }

    else if (strcmp(audiotsStr, deviceInfo.name) == 0) {
	// audiots does not support 8 bit linear
	//fprintf(stderr, "found audiots hardware \n");

	g_supports16		= 1;
	g_supports8		= 0;
	g_supportsStereo	= 1;   
	g_convertUnsigned	= 0;

	return 0;
    }
    else if (strcmp(SBUltraStr, deviceInfo.name) == 0) {
	// sb ultra does not support 8 bit linear
	//fprintf(stderr, "found audiots hardware \n");

	g_supports16		= 1;
	g_supports8		= 0;
	g_supportsStereo	= 1;   
	g_convertUnsigned	= 0;

	return 0;
    }

    // hardware not identified.  just assume defaults apply....
    //fprintf(stderr, "hardware not identified\n");
    return 0;
}

// Cleanup function. Called after all memory and the last buffers are deallocated, and
// audio hardware is released. Can be used to unload a DLL, library, etc.
// return 0 for ok, or -1 for failure
int HAE_Cleanup(void) {
    return 0;
}

// **** Memory management
// allocate a block of locked, zeroed memory. Return a pointer
void * HAE_Allocate(UINT32 size) {
    // $$kk: 10.14.97
    // changed this line as per Liang He's performance recommendations
    //	data = (char *)malloc(size);

    // $$kk: 10.23.97
    // changed this line as per Liang He's performance revommendations.
    // buffer should be 8-byte aligned, not 4-byte aligned.
    //data = (char *)memalign(8, size);

    char *data;

    data = (char *)memalign(8, (size_t)(size + 8));
    if (data) {
	memset(data, 0, (size_t)size);		// clear memory
    }
    return data;
}

// dispose of memory allocated with HAE_Allocate
void HAE_Deallocate(void * memoryBlock) {
    if (memoryBlock) {
	free(memoryBlock);
    }
}

// Given a memory pointer and a size, validate of memory pointer is a valid memory address
// with at least size bytes of data avaiable from the pointer.
// This is used to determine if a memory pointer and size can be accessed without 
// causing a memory protection
// fault.
// return 0 for valid, or 1 for bad pointer, or 2 for not supported. 
int HAE_IsBadReadPointer(void *memoryBlock, UINT32 size) {
    return 2;
}

// this will return the size of the memory pointer allocated with HAE_Allocate. Return
// 0 if you don't support this feature
UINT32 HAE_SizeOfPointer(void * memoryBlock) {
    return 0;
}

// block move memory. This is basicly memcpy, but its exposed to take advantage of
// special block move speed ups, various hardware has available.
void HAE_BlockMove(void * source, void * dest, UINT32 size) {
    if (source && dest && size) {
	memcpy(dest, source, (size_t)size);
    }
}

// **** Audio Card modifiers
// Return 1 if stereo hardware is supported, otherwise 0.
// $$kk: 08.12.98 merge: changed this to return a real value 
int HAE_IsStereoSupported(void) {
    // $$kk: 06.28.99: we are sometimes calling these methods before InitGeneralSound,
    // which calls HAE_Setup().  we should probably not do this!  however, in the
    // meantime....
    if (g_supportsStereo < 0) {
	HAE_Setup();
    }

    return g_supportsStereo;
}

// Return 1, if sound hardware support 16 bit output, otherwise 0.
// $$kk: 08.12.98 merge: changed this to return a real value 
int HAE_Is16BitSupported(void) {
    // $$kk: 06.28.99: we are sometimes calling these methods before InitGeneralSound,
    // which calls HAE_Setup().  we should probably not do this!  however, in the
    // meantime....
    if (g_supports16 < 0) {
	HAE_Setup();
    }

    return g_supports16;
}


// Return 1, if sound hardware support 8 bit output, otherwise 0.
// some solaris drivers support 8bit u-law or a-law only, so they are not supported
// $$kk: 08.12.98 merge: changed this to return a real value 
int HAE_Is8BitSupported(void) {
    // $$kk: 06.28.99: we are sometimes calling these methods before InitGeneralSound,
    // which calls HAE_Setup().  we should probably not do this!  however, in the
    // meantime....
    if (g_supports8 < 0) {
	HAE_Setup();
    }

    return g_supports8;
}


// returned volume is in the range of 0 to 256
short int HAE_GetHardwareVolume(void) {
    return 256;
}

// theVolume is in the range of 0 to 256
void HAE_SetHardwareVolume(short int theVolume)
{
}

// **** Timing services
// return microseconds
UINT32 HAE_Microseconds(void) {
#ifdef __linux__
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000000UL) + tv.tv_usec;
#else
    static hrtime_t solaristick = 0;

    if (solaristick == 0)
	{
	    solaristick = gethrtime();
	}
    /* ghrtime is in nanoseconds, we want microseconds. */
    return (UINT32) ((gethrtime() - solaristick) / 1000);
#endif
}

// wait or sleep this thread for this many microseconds
void HAE_WaitMicroseocnds(UINT32 waitAmount) {
    UINT32	ticks;

    ticks = HAE_Microseconds() + waitAmount;
    while (HAE_Microseconds() < ticks) 
	{
	    // $$kk: 09.01.98: this is a *guaranteed* core dump!
	    // i'm not sure if there's some other way we should "yield" here,
	    // but this is certainly not it....
	    // HAE_SleepFrameThread(NULL, 1);	// Give up the rest of this time slice to other threads
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
    int		file;

    file = open((char *)fileName, O_CREAT | O_RDWR | O_TRUNC);
    if (file != -1)
	{
	    close(file);
	}
    return (file != -1) ? 0 : -1;
}

INT32 HAE_FileDelete(void *fileName)
{
    if (fileName)
	{
	    if (unlink((char *)fileName) == 0)
		{
		    return 0;
		}
	}
    return -1;
}


// Open a file
// Return -1 if error, otherwise file handle
XFILE_HANDLE HAE_FileOpenForRead(void *fileName)
{
    if (fileName)
	{
	    return open((char *)fileName, O_RDONLY);
	}
    return -1;
}

XFILE_HANDLE HAE_FileOpenForWrite(void *fileName)
{
    if (fileName)
	{
	    return open((char *)fileName, O_CREAT | O_RDWR | O_TRUNC);
	}
    return -1;
}

XFILE_HANDLE HAE_FileOpenForReadWrite(void *fileName)
{
    if (fileName)
	{
	    return open((char *)fileName, O_RDWR);
	}
    return -1;
}

// Close a file
void HAE_FileClose(XFILE_HANDLE fileReference)
{
    close(fileReference);
}

// Read a block of memory from a file.
// Return -1 if error, otherwise length of data read.
INT32 HAE_ReadFile(XFILE_HANDLE fileReference, void *pBuffer, INT32 bufferLength)
{
    if (pBuffer && bufferLength)
	{
	    return (INT32)read(fileReference, (char *)pBuffer, (size_t)bufferLength);
	}
    return -1;
}

// Write a block of memory from a file
// Return -1 if error, otherwise length of data written.
INT32 HAE_WriteFile(XFILE_HANDLE fileReference, void *pBuffer, INT32 bufferLength)
{
    if (pBuffer && bufferLength)
	{
	    return (INT32)write(fileReference, (char *)pBuffer, (size_t)bufferLength);
	}
    return -1;
}

// set file position in absolute file byte position
// Return -1 if error, otherwise 0.
INT32 HAE_SetFilePosition(XFILE_HANDLE fileReference, UINT32 filePosition)
{
    return (lseek(fileReference, (off_t)filePosition, SEEK_SET) == -1) ? -1 : 0;
}

// get file position in absolute file bytes
UINT32 HAE_GetFilePosition(XFILE_HANDLE fileReference)
{
    return (UINT32)lseek(fileReference, 0, SEEK_CUR);
}

// get length of file
UINT32 HAE_GetFileLength(XFILE_HANDLE fileReference)
{
    UINT32 pos;

    pos = (UINT32)lseek(fileReference, 0, SEEK_END);
    lseek(fileReference, 0, SEEK_SET);
    return pos;
}

// set the length of a file. Return 0, if ok, or -1 for error
int HAE_SetFileLength(XFILE_HANDLE fileReference, UINT32 newSize)
{
    //	return chsize(fileReference, newSize);
    return -1;
}

#ifdef USE_RAWDATA_CHECK
static XFILE_HANDLE debugrawfile = 0;
#endif

// $$kk: 08.12.98 merge: changed this method to do convert to unsigned data if required by audio hardware 
void PV_AudioWaveOutFrameThread(void* context) {
    audio_info_t sunAudioHeader;
    char *pFillBuffer;
    INT32 count, currentPos, lastPos, sampleFrameSize;
    UINT32 startTime, stopTime, fillTime;
    int i;
    int rc;
    int bytesWritten;
    int bytesToWrite;


    ioctl(g_waveDevice, AUDIO_GETINFO, &sunAudioHeader);

    // calculate sample size for convertion of bytes to sample frames
    sampleFrameSize = 1;
		
    if (g_bitSize == 16) {
	sampleFrameSize *= 2;
    }

    if (g_channels == 2) {
	sampleFrameSize *= 2;
    }


    lastPos = sunAudioHeader.play.samples - ((g_audioByteBufferSize * HAE_SOLARIS_FRAMES_PER_BLOCK * 2) / sampleFrameSize);

    if (g_audioBufferBlock) {
	while ( (g_activeDoubleBuffer) && (g_shutDownDoubleBuffer == FALSE) ) {

	    /* put sync count and XMicroseconds into relation */
	    /* could be improved by using actual device sample count */
	    g_checkpointMicros = XMicroseconds();
	    g_checkpointSyncCount = GM_GetSyncTimeStamp();

	    // Generate HAE_SOLARIS_FRAMES_PER_BLOCK frames of audio
	    pFillBuffer = (char *)g_audioBufferBlock;	
	    for (count = 0; count < HAE_SOLARIS_FRAMES_PER_BLOCK; count++) {
				// Generate one frame audio
		HAE_BuildMixerSlice(context, pFillBuffer, g_audioByteBufferSize,
				    g_audioFramesToGenerate);

		pFillBuffer += g_audioByteBufferSize;

		if (g_shutDownDoubleBuffer) {
		    break;	// time to quit
		}
	    }

	    // $$kk
	    // for some solaris drivers, we must supply unsigned data when rendering 8 bit data
	    if (g_convertUnsigned && (g_bitSize == 8)) {
		pFillBuffer = (char *)g_audioBufferBlock;	
		for (i = 0; i < (g_audioByteBufferSize * HAE_SOLARIS_FRAMES_PER_BLOCK); i++) {
		    *pFillBuffer = (*pFillBuffer >= 0) ? (0x80 | *pFillBuffer) : (0x7F & *pFillBuffer);
		    pFillBuffer++;
		}
	    }
	
	    // $$jb: Changing the write() loop to handle cases when the 
	    // device is unavailable, or we can't write our entire buffer
	    bytesWritten = 0;
	    bytesToWrite = (g_audioByteBufferSize * HAE_SOLARIS_FRAMES_PER_BLOCK);
	    while( bytesToWrite > 0 )  {
		//$$fb don't write when it's time to quit.
		if( g_shutDownDoubleBuffer) {
		    break;
		}
		rc = write(g_waveDevice, ((char *)g_audioBufferBlock+bytesWritten), (size_t)bytesToWrite);
		if ( rc > 0 ) {
#ifdef USE_RAWDATA_CHECK
		    if (debugrawfile) {
			HAE_WriteFile(debugrawfile, ((char *)g_audioBufferBlock+bytesWritten), rc);
		    }
#endif
		    bytesWritten += rc;
		    bytesToWrite -= rc;
		} else {
                                // $$jb:  This happens when the device buffers cannot
                                // be written to.  Make sure we're not shutting down and 
                                // sleep a bit so that we don't completely hog the CPU
		    if( g_shutDownDoubleBuffer == FALSE ) {
			HAE_SleepFrameThread(context, HAE_SOLARIS_SOUND_PERIOD);
		    } else {
			break;
		    }
		} 
	    }


	    // O.k. We're done for now.
	    // Let the rest of the system know we're done ....

	    ioctl(g_waveDevice, AUDIO_GETINFO, &sunAudioHeader);
	    currentPos = sunAudioHeader.play.samples;

	    // $$jb: We have successfully written all our bytes.  
	    // If we encountered a problem while writing, play.error will be 1.
	    // This should be reset.
	    if( sunAudioHeader.play.error != 0 ) {
		AUDIO_INITINFO(&sunAudioHeader);
		sunAudioHeader.play.error = 0;
		ioctl(g_waveDevice, AUDIO_SETINFO, &sunAudioHeader);
	    }

			
	    // $$kk: 03.21.00: make sure we sleep at least once so that other threads can run.
	    // this is part of the fix for bug #4318062: "MixerSourceLine.drain hangs after
	    // repeated use."
	    //while ((currentPos < lastPos) && (g_shutDownDoubleBuffer == FALSE))
	    do {
		HAE_SleepFrameThread(context, HAE_SOLARIS_SOUND_PERIOD);		// in ms
				
		ioctl(g_waveDevice, AUDIO_GETINFO, &sunAudioHeader);
		currentPos = sunAudioHeader.play.samples;

                                // $$jb: Removing the bit of code that breaks out
                                // of this timing loop on sunAudioHeader.play.error != 0.
	    }
	    while ((currentPos < lastPos) &&
		   (lastPos - currentPos < (1 << 28)) && /* see note A */
		   (g_shutDownDoubleBuffer == FALSE));

	    // Note A: $$ay: Additional safeguard for wraparound of sample
	    // ------  count from 1 << 32 - 1.  Make sure the difference is
	    //         not a huge value
			
	    lastPos += (g_audioByteBufferSize * HAE_SOLARIS_FRAMES_PER_BLOCK) / sampleFrameSize;
	    // ... and reschedule ourselves.	
	}

	g_activeDoubleBuffer = FALSE;
    }
}


// Return the number of 11 ms buffer blocks that are built at one time.
int HAE_GetAudioBufferCount(void) {
    return g_synthFramesPerBlock;
}

// Return the number of bytes used for audio buffer for output to card
INT32 HAE_GetAudioByteBufferSize(void) {
    return g_audioByteBufferSize;
}


// **** Audio card support
// Aquire and enabled audio card
// return 0 if ok, -1 if failed
int HAE_AquireAudioCard(void *context, UINT32 sampleRate, UINT32 channels, UINT32 bits) {
    int flag;
    short int count;
    INT32 error;
    audio_info_t sunAudioHeader;
    char* pAudioDev = HAE_GetAudioDevPlay(g_currentDeviceID, 0);
	
    flag = 0;
    g_activeDoubleBuffer = FALSE;
    g_shutDownDoubleBuffer = TRUE;

    g_audioFramesToGenerate = HAE_GetMaxSamplePerSlice();	// get number of frames per sample rate slice

    // we're going to build this many buffers at a time
    g_synthFramesPerBlock = HAE_SOLARIS_FRAMES_PER_BLOCK;
    g_audioPeriodSleepTime = HAE_SOLARIS_SOUND_PERIOD;
    g_bitSize = bits;
    g_channels = channels;
    if (bits == 8) {
	g_audioByteBufferSize = ((INT32)sizeof(char) * g_audioFramesToGenerate);
    } else {
	g_audioByteBufferSize = ((INT32)sizeof(short int) * g_audioFramesToGenerate);
    }
    g_audioByteBufferSize *= channels;

    flag = 1;
    // allocate buffer blocks
    g_audioBufferBlock = HAE_Allocate(g_audioByteBufferSize * HAE_SOLARIS_FRAMES_PER_BLOCK);
    if (g_audioBufferBlock) {
	// try to open wave device
	// $$kk: 12.17.97: need O_NONBLOCK flag to be compatible with windows
#ifdef __linux__
            g_waveDevice = open(pAudioDev,O_WRONLY);
#else
            g_waveDevice = open(pAudioDev,O_WRONLY|O_NONBLOCK);
#endif

	if (g_waveDevice > 0) {

	    /* set to multiple open */
	    if (ioctl(g_waveDevice, AUDIO_MIXER_MULTIPLE_OPEN, NULL) >= 0) {
		TRACE1("HAE_AquireAudioCard: %s set to multiple open\n", pAudioDev);
	    } else {
		ERROR1("HAE_AquireAudioCard: ioctl AUDIO_MIXER_MULTIPLE_OPEN failed on %s!\n", pAudioDev);
	    }

	    AUDIO_INITINFO(&sunAudioHeader);
	    // $$kk: 12.17.97: need AUDIO_GETINFO ioctl to get this to work on solaris x86 
	    // add next 1 line
	    error = ioctl(g_waveDevice, AUDIO_GETINFO, &sunAudioHeader);

	    // $$kk: 03.16.98: not valid to call AUDIO_SETINFO ioctl with all the fields from AUDIO_GETINFO,
	    // so let's try init'ing again....
	    AUDIO_INITINFO(&sunAudioHeader);

	    // Set rendering format of the sun device.
	    sunAudioHeader.play.sample_rate = sampleRate;
	    sunAudioHeader.play.precision = bits;
	    sunAudioHeader.play.channels = channels;
	    sunAudioHeader.play.encoding = AUDIO_ENCODING_LINEAR;
		
	    error = ioctl(g_waveDevice, AUDIO_SETINFO, &sunAudioHeader);

	    if (error == 0) {
		g_shutDownDoubleBuffer = FALSE;
		g_activeDoubleBuffer = TRUE;	// must enable process, before thread begins


				/* Spin threads for device service and possibly
				 * stream service.
				 */
				// create thread to manage and render audio frames
		error = HAE_CreateFrameThread(context, PV_AudioWaveOutFrameThread);

		if (error == 0) {	// ok
		    flag = 0;
#ifdef USE_RAWDATA_CHECK
		    { 
			char* fname = "javasound_debug_output.pcm";
			debugrawfile = HAE_FileOpenForWrite(fname);
		    }
#endif

		} else {
		    flag = 1;
		    g_activeDoubleBuffer = FALSE;
		}
	    }
	}
    }

    if (flag) {	// something failed
	HAE_ReleaseAudioCard(context);
    }
    return flag;
}

// Release and free audio card.
// return 0 if ok, -1 if failed.
int HAE_ReleaseAudioCard(void *context) {
    int ctr = 50;
    g_shutDownDoubleBuffer = TRUE;	// signal thread to stop
    HAE_DestroyFrameThread(context);

    // $$fb 2002-04-17: wait until PV_AudioWaveOutFrameThread exits
    // fix for 4498848 Sound causes crashes on Linux
    ctr=50;
    while (g_activeDoubleBuffer && --ctr) {
	TRACE1("Waiting %d...\r", ctr);
	// the following call MUST allow the FrameThread to continue 
	// (i.e. to exit the PV_AudioWaveOutFrameThread function)
	HAE_SleepFrameThread(context, HAE_SOLARIS_SOUND_PERIOD);
    }
    if (!ctr) {
	ERROR0("Timed out waiting for frame thread to die!\n");
    }

    if (g_waveDevice) {
	close(g_waveDevice);
	g_waveDevice = 0;
    }

    if (g_audioBufferBlock) {
	HAE_Deallocate(g_audioBufferBlock);
	g_audioBufferBlock = NULL;
    }
#ifdef USE_RAWDATA_CHECK
    HAE_FileClose(debugrawfile); debugrawfile = 0;
#endif
    return 0;
}

// return device position in samples
UINT32 HAE_GetDeviceSamplesPlayedPosition(void) {
    audio_info_t	sunAudioHeader;
    UINT32	pos;

    pos = 0;
    if (g_waveDevice) {
	ioctl(g_waveDevice, AUDIO_GETINFO, &sunAudioHeader);
	pos = sunAudioHeader.play.samples;
    }
    return pos;
}

// number of devices. ie different versions of the HAE connection. DirectSound and waveOut
// return number of devices. ie 1 is one device, 2 is two devices.
// NOTE: This function needs to function before any other calls may have happened.
INT32 HAE_MaxDevices(void) {

    /* let's see if HAE_Setup can be done successfully... */
    if (g_supports16 >= 0 || HAE_Setup() == 0) {
	return 1; // only AUDIODEV (defaulting to /dev/audio)
    }
    return 0;
}

// set the current device. device is from 0 to HAE_MaxDevices()
// NOTE:	This function needs to function before any other calls may have happened.
//			Also you will need to call HAE_ReleaseAudioCard then HAE_AquireAudioCard
//			in order for the change to take place. deviceParameter is a device specific
//			pointer. Pass NULL if you don't know what to use.
void HAE_SetDeviceID(INT32 deviceID, void *deviceParameter) {
    if (deviceID < HAE_MaxDevices()) {
	g_currentDeviceID = deviceID;
    }
}

// return current device ID, and fills in the deviceParameter with a device specific
// pointer. It will pass NULL if there is nothing to use.
// NOTE: This function needs to function before any other calls may have happened.
INT32 HAE_GetDeviceID(void *deviceParameter) {
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
void HAE_GetDeviceName(INT32 deviceID, char *cName, UINT32 cNameLength) {
    char		*data;
    //$$fb this name is not nice. Shouldn't be static but reflect the actually used device
    static char *names[] = {
#ifdef __linux__
        "LinuxOS,dev/audio,multi threaded",
#else
        "SolarisOS,dev/audio,multi threaded",
#endif
    };
    if (cName && cNameLength) {
	if (deviceID < HAE_MaxDevices()) {
	    // can't do string functions here, it might be bad to do.
	    data = names[deviceID];
	    while (*data && (cNameLength > 0)) {
		*cName = *data;
		cName++;
		data++;
		cNameLength--;
	    }
	    *cName = 0;
	}
    }
}


// EOF of HAE_API_SolarisOS.c
