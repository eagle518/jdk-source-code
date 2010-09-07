/*
 * @(#)HAE_API_LinuxOS.c	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**	HAE_API_LinuxOS.c
**
**	This provides platform specfic functions for Linux. This interface
**	for HAE uses the dev/dsp device
**
**	Overview:
**		This works by creating another thread, aquires the sound device via
**		open("/dev/dsp",O_WRONLY) call. Then allocating enough buffers, preps then,
**		and build buffers depending upon how much data has been written to the
**		audio device. The thread polls the current position in the audio stream
**		that has been written and when it falls below a buffer or two, more
**		are built in that thread.
**
**	History	-
**	2002-03-14	$$fb cleaned up endian-ness support
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
#include <linux/soundcard.h>
#include <malloc.h>

// $$kk: 08.12.98 merge 
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
#define HAE_LINUX_FRAMES_PER_BLOCK		8
#define HAE_LINUX_SOUND_PERIOD			10	// sleep time in milliseconds, between position checks

static void					*g_audioBufferBlock;
static long					g_audioByteBufferSize;			// size of audio buffers in bytes

// $$ay: Shared globals between this file and HAE_API_LinuxOS_Capture.c
//       Linux OSS sound API allows opening the sound device only once.
//       Hence it has to be opened for both playback and capture simultaneously
//       Also the API only supports semi-FullDuplex (if Duplex is supported by the
//       driver). Which means you can playback and capture only in one format at a time.

#define MAXFORMATS (6*2*4*2)  // 6 sample rates, 2 channels, 3 encodings, 2 bit sizes

/* $$fb 2002-03-13: facilitate porting by defining the 16-bit format here */
#if X_WORD_ORDER == TRUE
#define LINUX_FORMAT16	AFMT_S16_LE
#else
#define LINUX_FORMAT16	AFMT_S16_BE
#endif


int    g_waveDevice = 0; // $$ay: Shared for playback and capture
int    g_openForPlayback = 0; 
int    g_openForCapture = 0;
int    g_queriedFormats = 0;
int    g_supportsDuplex = 0;
int    g_maxFormats = 0;
int    g_supEncodings[MAXFORMATS];
int    g_supSampleRates[MAXFORMATS];
int    g_supChannels[MAXFORMATS];
int    g_supBits[MAXFORMATS];


static long					g_shutDownDoubleBuffer;
static long					g_activeDoubleBuffer;

static short int			g_bitSize;
static short int			g_channels;

// $$kk: this is used in HAE_SetDeviceID and HAE_GetDeviceID but was not defined
long						g_currentDeviceID = 0;

// number of samples per audio frame to generate
long						g_audioFramesToGenerate;

// How many audio frames to generate at one time 
static unsigned int			g_synthFramesPerBlock;	// setup upon runtime

static unsigned int			g_audioPeriodSleepTime;	// setup upon runtime

// $$kk: 08.12.98 merge: added these 4 variables
// $$kk: 03.17.98: these variables describe device capabilities and requirements
int							g_supports16			= -1;
int							g_supports8				= -1;
int							g_supportsStereo		= -1;
int							g_convertUnsigned		= 0;

/* for syncronization of real time MIDI events */
INT64 g_checkpointMicros = 0;
INT64 g_checkpointSyncCount = 0;


// This file contains API's that need to be defined in order to get HAE (IgorAudio)
// to link and compile.

// **** System setup and cleanup functions
// Setup function. Called before memory allocation, or anything serious. Can be used to 
// load a DLL, library, etc.
// return 0 for ok, or -1 for failure
// $$kk: 08.12.98 merge: added code to query for audio device driver capabilities here.
// $$ay: 01.26.00 Added code to query available formats (playback and capture)
int HAE_Setup(void)
{
    // we do not publically let people change the rendering format, so right now
    // we'll just do 16 if we can, otherwise 8.  this will have to be done more
    // carefully with the new api.
    
    int pseudoDevice = 0;
    long error;
    int formats;
    int caps;
    int stereoChannel;
    int defaultChannels;
    int nFormat = 0;
    int iFormats, iChannels, iSampleRates;
    int SampleRates[] = { 8000, 11025, 16000, 22050, 32000, 44100 };
    int Formats[] = {
	AFMT_MU_LAW,
	AFMT_A_LAW,
	AFMT_S8,
	LINUX_FORMAT16,
    };
    // In the same order of Formats[] array.
    int JSEncodings[] = { ULAW, ALAW, PCM, PCM };
    
    if (g_queriedFormats)
	return 0;

    pseudoDevice = open("/dev/dsp", O_RDONLY | O_NONBLOCK);
    
    if (pseudoDevice == -1) {
	//fprintf(stderr, "could not get pseudoDevice: errno=%d\n", errno);
	return 0;
    }
    
    // Set to Full Duplex mode
    //error = ioctl(pseudoDevice, SNDCTL_DSP_SETDUPLEX, 0);
    
    error = ioctl(pseudoDevice, SNDCTL_DSP_GETFMTS, &formats);
    if (error < 0) {
	//perror("SNDCTL_DSP_GETFMTS");
    }
    g_supports8 = (formats & (AFMT_U8|AFMT_S8)) != 0;

    //$$fb we do not support unsigned 16 bit
    //g_supports16 = (formats & (AFMT_U16_LE | AFMT_S16_LE)) != 0;
    g_supports16 = (formats & LINUX_FORMAT16) != 0;
    g_convertUnsigned = (formats & AFMT_S8) != 0;

    /* Read channels. */
    error = ioctl(pseudoDevice, SOUND_PCM_READ_CHANNELS,
		  &defaultChannels);
    if (error < 0) {
	//perror("SOUND_PCM_READ_CHANNELS");
    }
    /* Check if stereo */
    stereoChannel = 2;
    error = ioctl(pseudoDevice, SOUND_PCM_WRITE_CHANNELS, &stereoChannel);
    if (error < 0) {
	//perror("SOUND_PCM_WRITE_CHANNELS");
    }
    /* Restore channels. */
    error = ioctl(pseudoDevice, SOUND_PCM_WRITE_CHANNELS,&defaultChannels);
    if (error < 0) {
	//perror("SOUND_PCM_WRITE_CHANNELS");
    }
    g_supportsStereo = (stereoChannel == 2);

    // Check for Semi Full Duplex support
    error = ioctl(pseudoDevice, SNDCTL_DSP_GETCAPS, &caps);
    if (!(error < 0) && (caps & DSP_CAP_DUPLEX))
	g_supportsDuplex = 1;
    
    for (iFormats = 0; iFormats < 4; iFormats++) {
	int format = Formats[iFormats];
	for (iChannels = 0; iChannels < 2; iChannels++) {
	    for (iSampleRates = 0; iSampleRates < 6; iSampleRates++) {
		int speed = SampleRates[iSampleRates];
		error = ioctl(pseudoDevice, SNDCTL_DSP_RESET, 0);
		//if (error < 0) perror("SNDCTL_DSP_RESET");
		error = ioctl(pseudoDevice, SNDCTL_DSP_SETFMT, &format);
		if (error < 0) continue;
		error = ioctl(pseudoDevice, SNDCTL_DSP_STEREO, &iChannels);
		if (error < 0) continue;
		error = ioctl(pseudoDevice, SNDCTL_DSP_SPEED, &speed);
		if (error < 0) continue;
		// Supported!
		//printf("Sup encoding %d\n", JSEncodings[iFormats]);
		g_supChannels[nFormat] = iChannels + 1;
		g_supSampleRates[nFormat] = speed;
		g_supBits[nFormat] = (iFormats == 3) ? 16 : 8;
		g_supEncodings[nFormat] = JSEncodings[iFormats];
		nFormat++;
	    }
	}
    }
    g_maxFormats = nFormat;
    ioctl(pseudoDevice, SNDCTL_DSP_RESET, 0);
    
    close(pseudoDevice);
    
    g_queriedFormats = 1;
    
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
    // $$kk: 10.14.97
    // changed this line as per Liang He's performance recommendations
    //	data = (char *)malloc(size);

    // $$kk: 10.23.97
    // changed this line as per Liang He's performance revommendations.
    // buffer should be 8-byte aligned, not 4-byte aligned.
    //data = (char *)memalign(8, size);

    char *data;

    data = (char *)memalign(8, size + 8);
    if (data)
	{
	    memset(data, 0, size);		// clear memory
	}
    return data;
}

// dispose of memory allocated with HAE_Allocate
void HAE_Deallocate(void * memoryBlock)
{
    if (memoryBlock)
	{
	    free(memoryBlock);
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
    return 2;
}

// this will return the size of the memory pointer allocated with HAE_Allocate. Return
// 0 if you don't support this feature
UINT32 HAE_SizeOfPointer(void * memoryBlock)
{
    return 0;
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
// $$kk: 08.12.98 merge: changed this to return a real value 
int HAE_IsStereoSupported(void)
{
    // $$kk: 06.28.99: we are sometimes calling these methods before InitGeneralSound,
    // which calls HAE_Setup().  we should probably not do this!  however, in the
    // meantime....
    if (g_supportsStereo < 0)
	{
	    HAE_Setup();
	}

    return g_supportsStereo;
}

// Return 1, if sound hardware support 16 bit output, otherwise 0.
// $$kk: 08.12.98 merge: changed this to return a real value 
int HAE_Is16BitSupported(void)
{
    // $$kk: 06.28.99: we are sometimes calling these methods before InitGeneralSound,
    // which calls HAE_Setup().  we should probably not do this!  however, in the
    // meantime....
    if (g_supports16 < 0)
	{
	    HAE_Setup();
	}

    return g_supports16;
}


// Return 1, if sound hardware support 8 bit output, otherwise 0.
// some solaris drivers support 8bit u-law or a-law only, so they are not supported
// $$kk: 08.12.98 merge: changed this to return a real value 
int HAE_Is8BitSupported(void)
{
    // $$kk: 06.28.99: we are sometimes calling these methods before InitGeneralSound,
    // which calls HAE_Setup().  we should probably not do this!  however, in the
    // meantime....
    if (g_supports8 < 0)
	{
	    HAE_Setup();
	}

    return g_supports8;
}


// returned volume is in the range of 0 to 256
short int HAE_GetHardwareVolume(void)
{
    int theVolume;
    ioctl(g_waveDevice, SOUND_MIXER_READ_VOLUME, &theVolume);
    return theVolume;
}

// theVolume is in the range of 0 to 256
void HAE_SetHardwareVolume(short int theVolume)
{
    ioctl(g_waveDevice, SOUND_MIXER_WRITE_VOLUME, &theVolume);
}

// **** Timing services
// return microseconds
UINT32 HAE_Microseconds(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000000UL) + tv.tv_usec;
}    

// wait or sleep this thread for this many microseconds
void HAE_WaitMicroseocnds(UINT32 waitAmount)
{
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
	    return read(fileReference, (char *)pBuffer, bufferLength);
	}
    return -1;
}

// Write a block of memory from a file
// Return -1 if error, otherwise length of data written.
INT32 HAE_WriteFile(XFILE_HANDLE fileReference, void *pBuffer, INT32 bufferLength)
{
    if (pBuffer && bufferLength)
	{
	    return write(fileReference, (char *)pBuffer, bufferLength);
	}
    return -1;
}

// set file position in absolute file byte position
// Return -1 if error, otherwise 0.
INT32 HAE_SetFilePosition(XFILE_HANDLE fileReference, UINT32 filePosition)
{
    return (lseek(fileReference, filePosition, SEEK_SET) == -1) ? -1 : 0;
}

// get file position in absolute file bytes
UINT32 HAE_GetFilePosition(XFILE_HANDLE fileReference)
{
    return lseek(fileReference, 0, SEEK_CUR);
}

// get length of file
UINT32 HAE_GetFileLength(XFILE_HANDLE fileReference)
{
    unsigned long pos;

    pos = lseek(fileReference, 0, SEEK_END);
    lseek(fileReference, 0, SEEK_SET);
    return pos;
}

// set the length of a file. Return 0, if ok, or -1 for error
int HAE_SetFileLength(XFILE_HANDLE fileReference, UINT32 newSize)
{
    //	return chsize(fileReference, newSize);
    return -1;
}

// $$kk: 08.12.98 merge: changed this method to do convert to unsigned data if required by audio hardware 
void PV_AudioWaveOutFrameThread(void* context)
{
    char			*pFillBuffer;
    long			count, currentPos, lastPos, sampleFrameSize;
    int				i;
    int rc;
    int bytesWritten;
    int bytesToWrite;
    //int avail;
    count_info audio_info;
    
    ioctl(g_waveDevice, SNDCTL_DSP_GETOPTR, &audio_info);

    // calculate sample size for convertion of bytes to sample frames
    sampleFrameSize = 1;
		
    if (g_bitSize == 16) {
	sampleFrameSize *= 2;
    }

    if (g_channels == 2) {
	sampleFrameSize *= 2;
    }

    // $$ay - sample count is in bytes for linux and not in samples
    lastPos = audio_info.bytes - ((g_audioByteBufferSize * HAE_LINUX_FRAMES_PER_BLOCK * 2));

    if (g_audioBufferBlock) {
	while ((g_activeDoubleBuffer) && (g_shutDownDoubleBuffer == FALSE)) {

	    /* put sync count and XMicroseconds into relation */
	    /* could be improved by using actual device sample count */
	    g_checkpointMicros = XMicroseconds();
	    g_checkpointSyncCount = GM_GetSyncTimeStamp();

	    // Generate HAE_LINUX_FRAMES_PER_BLOCK frames of audio
	    pFillBuffer = (char *)g_audioBufferBlock;	
	    for (count = 0; count < HAE_LINUX_FRAMES_PER_BLOCK; count++) {
		// Generate one frame audio
		HAE_BuildMixerSlice(context, pFillBuffer,
				    g_audioByteBufferSize,
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
		for (i = 0; i < (g_audioByteBufferSize * HAE_LINUX_FRAMES_PER_BLOCK); i++) {
		    *pFillBuffer = (*pFillBuffer >= 0) ? (0x80 | *pFillBuffer) : (0x7F & *pFillBuffer);
		    pFillBuffer++;
		}
	    }
	    
	    // $$jb: Changing the write() loop to handle cases when the 
	    // device is unavailable, or we can't write our entire buffer
	    bytesWritten = 0;
	    bytesToWrite = (g_audioByteBufferSize * HAE_LINUX_FRAMES_PER_BLOCK);

	    while ( bytesToWrite > 0 ) {
#ifdef TODO		
		// $$ay:  AARGH!!! Linux forces read to be non-blocking when opened for DUPLEX
		if (!g_openForCapture && g_supportsDuplex) {
		    int k, avail;
		    
		    ioctl(g_waveDevice, SNDCTL_DSP_GETBLKSIZE, &avail);
		    k = read(g_waveDevice, dummyBuffer, avail);
		    //printf("AvailToRead = %d, Read = %d\n", avail, k);
		    //k = read(g_waveDevice, dummyBuffer, sizeof(dummyBuffer));
		    //printf("Read = %d\n", k);
		}
		ioctl(g_waveDevice, SNDCTL_DSP_GETBLKSIZE, &avail);
		if (bytesToWrite > avail)
		    rc = write(g_waveDevice, ((char *)g_audioBufferBlock+bytesWritten), avail);
		else
		    rc = write(g_waveDevice, ((char *)g_audioBufferBlock+bytesWritten), bytesToWrite);
		//printf("Wrote %d bytes\n", rc);
#endif
		
		//$$fb don't write when it's time to quit.
		if( g_shutDownDoubleBuffer) {
		    break;
		}
		rc = write(g_waveDevice, ((char *)g_audioBufferBlock+bytesWritten), bytesToWrite);
		if ( rc > 0 ) {
		    bytesWritten += rc;
		    bytesToWrite -= rc;
		} else {
		    // $$jb:  This happens when the device buffers cannot
		    // be written to.  Make sure we're not shutting down and 
		    // sleep a bit so that we don't completely hog the CPU
		    if( g_shutDownDoubleBuffer == FALSE ) {
			HAE_SleepFrameThread(context, HAE_LINUX_SOUND_PERIOD);
		    } else {
			break;
		    }
		} 
	    }


	    // O.k. We're done for now.
	    // Let the rest of the system know we're done ....
	    
	    ioctl(g_waveDevice, SNDCTL_DSP_GETOPTR, &audio_info);
	    currentPos = audio_info.bytes;
	    // $$jb: We have successfully written all our bytes.  
	    // If we encountered a problem while writing, play.error will be 1.
	    // This should be reset.

#ifdef TODO
	    if ( sunAudioHeader.play.error != 0 ) {
		AUDIO_INITINFO(&sunAudioHeader);
		sunAudioHeader.play.error = 0;
		ioctl(g_waveDevice, AUDIO_SETINFO, &sunAudioHeader);
	    }
#endif
	    
	    while ((currentPos < lastPos) && (g_shutDownDoubleBuffer == FALSE))	{
		HAE_SleepFrameThread(context, HAE_LINUX_SOUND_PERIOD);		// in ms
		
		ioctl(g_waveDevice, SNDCTL_DSP_GETOPTR, &audio_info);
		currentPos = audio_info.bytes;
		
		// $$jb: Removing the bit of code that breaks out
		// of this timing loop on sunAudioHeader.play.error != 0.
	    }
	    
	    lastPos += (g_audioByteBufferSize * HAE_LINUX_FRAMES_PER_BLOCK);
	    // ... and reschedule ourselves.	
	}
	TRACE0("g_activeDoubleBuffer = FALSE;\n");
	g_activeDoubleBuffer = FALSE;
    }
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

#define NO_FULL_DUPLEX

int HAE_OpenSoundCard(int forCapture)
{
    int test;
    int openMode;
    //printf("Trying to open card\n");
#ifdef FULL_DUPLEX
    if (g_waveDevice == 0) {
	//printf("Opening sound card for %d\n", forCapture);
	g_waveDevice = open("/dev/dsp", O_RDWR);
	//printf("Opening FullDuplex\n");
	// Set to Full Duplex mode
	ioctl(g_waveDevice, SNDCTL_DSP_SETDUPLEX, 0);
    }
#else
    if (g_waveDevice == 0) {
	//printf("Opening sound card for %d\n", forCapture);
	//first open the device in non-blocking mode to test...\n");
	openMode = forCapture? O_RDONLY : O_WRONLY;
	test = open("/dev/dsp", openMode | O_NONBLOCK);
	if (test < 0) {
		return 0;
	}
	close(test);
	// then re-open when we're sure that we can aquire the card
	g_waveDevice = open("/dev/dsp", openMode);
	//printf("Opening HalfDuplex\n");
    } else
	return 0;
#endif
    
    if (forCapture)
	g_openForCapture = 1;
    else
	g_openForPlayback = 1;
    return g_waveDevice;
}

void HAE_CloseSoundCard(int forCapture)
{
    // If the device is open
    if (g_waveDevice) {
	// Reset the open flags appropriately
	//printf("Closing card for %s\n", forCapture? "capture":"playback");
        ioctl(g_waveDevice, SOUND_PCM_RESET, 0); 
	if (forCapture)
	    g_openForCapture = 0;
	else
	    g_openForPlayback = 0;
	// If neither capturing nor playing back, close the device
	if (!(g_openForPlayback | g_openForCapture)) {
	    close(g_waveDevice);
	    g_waveDevice = 0;
	}
    }
}

// **** Audio card support
// Aquire and enabled audio card
// return 0 if ok, -1 if failed
// $$kk: 08.12.98 merge: modified this function
int HAE_AquireAudioCard(void *context, UINT32 sampleRate, UINT32 channels, UINT32 bits)
{
    int	flag;
    long error;

#ifdef DEBUG_AUDIO
    jio_fprintf(stderr, "Acquire audio card(sampleRate=%d, channels=%d, bits=%d\n", sampleRate, channels, bits);
#endif


    flag = 0;
    g_activeDoubleBuffer = FALSE;
    g_shutDownDoubleBuffer = TRUE;
    

    g_audioFramesToGenerate = HAE_GetMaxSamplePerSlice();

#ifdef TODO // ask Kara
    switch (sampleRate) {
    case 44100:
	g_audioFramesToGenerate = HAE_GetMaxSamplePerSlice();
	break;
    case 11025:
	/* [sbb fix] why is this case thrown away? */
	sampleRate = 22050;
    case 22050:
	g_audioFramesToGenerate = HAE_GetMaxSamplePerSlice()/2;
	break;
    }
#endif

    /* we're going to build this many buffers at a time */
    g_synthFramesPerBlock = HAE_LINUX_FRAMES_PER_BLOCK;
    g_audioPeriodSleepTime = HAE_LINUX_SOUND_PERIOD;
    g_bitSize = bits;
    g_channels = channels;
    if (bits == 8) {
	g_audioByteBufferSize = (sizeof(char) * g_audioFramesToGenerate);
    } else {
	g_audioByteBufferSize = (sizeof(short int) * g_audioFramesToGenerate);
    }
    g_audioByteBufferSize *= channels;

    flag = 1;
    /* allocate buffer blocks */
    g_audioBufferBlock = HAE_Allocate(g_audioByteBufferSize * HAE_LINUX_FRAMES_PER_BLOCK);

    if (g_audioBufferBlock) {

	g_waveDevice = HAE_OpenSoundCard(0); // for playback

	if (g_waveDevice > 0) {
	    /* for linux it's
	     *    set sample format,
	     *    set channels (mono or stereo)
	     *    set sample rate
	     */

	    int format = AFMT_MU_LAW;
	    int stereo = (channels == 2);
	    int speed = sampleRate;

	    switch (bits) {
	    case 8:
		format = AFMT_MU_LAW;	/* [sbb fix] don't know if this is right or not -- maybe should be s8? */
		break;

	    case 16:
		format = LINUX_FORMAT16;
		break;

	    //default:
		//fprintf(stderr, "Warning: unhandled number of data bits %d\n", (int) bits);
	    }

	    error = ioctl(g_waveDevice, SNDCTL_DSP_SETFMT, &format);

	    if (error < 0) {
		//perror("SNDCTL_DSP_SETFMT");
		//exit(1);
	    }

	    error = ioctl(g_waveDevice, SNDCTL_DSP_STEREO, &stereo);

	    if (error < 0) {
		/* [sbb fix] issue some kind of error message */

		//perror("SNDCTL_DSP_STEREO");
		//exit(1);
	    }

 

	    if (ioctl(g_waveDevice, SNDCTL_DSP_SPEED, &speed) < 0) { /* Fatal error */  
		/* [sbb fix] handle this better */
		//perror("SNDCTL_DSP_SPEED");
		// $$ay: dont exit !
		// exit(1);  
	    }       

	    if (speed != (INT32) sampleRate) {
		/* [sbb fix] need to issue a message */
	    }


	    if (error == 0) {
		g_shutDownDoubleBuffer = FALSE;
		g_activeDoubleBuffer = TRUE;	/* must enable process, before thread begins */


		/* $$kk: 05.06.98: added this whole block.
		 * we need to reset the lastPos each time the device gets acquired.
		 * otherwise we may get stuck in the wait loop because we never count
		 * up to the right sample position. */
		{
#ifdef NOT_HERE_BUT_PRESERVED
		    audio_info_t	sunAudioHeader;
		    long sampleFrameSize;
	  
		    ioctl(g_waveDevice, AUDIO_GETINFO, &sunAudioHeader);

		    /* [sbb] I don't think this should be any value other than zero,
		     * since we just opened the device... */
		    /* lastPos = sunAudioHeader.play.samples - ((g_audioByteBufferSize * HAE_LINUX_FRAMES_PER_BLOCK * 2) / sampleFrameSize); */
		    lastPos = 0;
#endif 
		}

				
		/* Spin threads for device service and possibly
		 * stream service.
		 */
		/* create thread to manage and render audio frames */
		error = HAE_CreateFrameThread(context, PV_AudioWaveOutFrameThread);

		if (error == 0) {	/* ok */
		    flag = 0;
		} else {
		    flag = 1;
		    g_activeDoubleBuffer = FALSE;
		}
	    }
	}
    }

    if (flag) {	/* something failed */
	HAE_CloseSoundCard(0); // Close for playback
    }
    return flag;
}

// Release and free audio card.
// return 0 if ok, -1 if failed.
int HAE_ReleaseAudioCard(void *context)
{
    int ctr;
    g_shutDownDoubleBuffer = TRUE;	/* signal thread to stop */
    HAE_DestroyFrameThread(context);
    // $$fb 2002-04-17: wait until PV_AudioWaveOutFrameThread exits
    // fix for 4498848 Sound causes crashes on Linux
    ctr=50;
    while (g_activeDoubleBuffer && --ctr) {
	TRACE1("Waiting %d...\r", ctr);
	// the following call MUST allow the FrameThread to continue 
	// (i.e. to exit the PV_AudioWaveOutFrameThread function)
	HAE_SleepFrameThread(context, HAE_LINUX_SOUND_PERIOD);
    }
    if (!ctr) {
    	ERROR0("Timed out waiting for frame thread to die!\n");
    }

    HAE_CloseSoundCard(0); // Close for playback
    
    if (g_audioBufferBlock) {
	HAE_Deallocate(g_audioBufferBlock);
	g_audioBufferBlock = NULL;
    }

    return 0;
}

// return device position in samples
UINT32 HAE_GetDeviceSamplesPlayedPosition(void)
{
    int	pos;

    pos = 0;
    if (g_waveDevice && g_openForPlayback) {
	count_info audio_info;
	ioctl(g_waveDevice, SNDCTL_DSP_GETOPTR, &audio_info);
	pos = audio_info.bytes;
    }
    return (UINT32) pos;
}

// number of devices. ie different versions of the HAE connection. DirectSound and waveOut
// return number of devices. ie 1 is one device, 2 is two devices.
// NOTE: This function needs to function before any other calls may have happened.
INT32 HAE_MaxDevices(void)
{
    int err;

    err = open("/dev/dsp", O_RDONLY | O_NONBLOCK);
    if (err == -1) {
	// there is no audio hardware available, 
	// if the device file does not exist,
	// or if the driver is not loaded
	err = errno;
	if (err == ENOENT || err == ENODEV) {
	    return 0;
	}
    } else {
	close(err);
    }
    return 1;	// only /dev/dsp
}

// set the current device. device is from 0 to HAE_MaxDevices()
// NOTE:	This function needs to function before any other calls may have happened.
//			Also you will need to call HAE_ReleaseAudioCard then HAE_AquireAudioCard
//			in order for the change to take place. deviceParameter is a device specific
//			pointer. Pass NULL if you don't know what to use.
void HAE_SetDeviceID(INT32 deviceID, void *deviceParameter)
{
    if (deviceID < HAE_MaxDevices())
	{
	    g_currentDeviceID = deviceID;
	}
}

// return current device ID, and fills in the deviceParameter with a device specific
// pointer. It will pass NULL if there is nothing to use.
// NOTE: This function needs to function before any other calls may have happened.
INT32 HAE_GetDeviceID(void *deviceParameter)
{
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
    {	"Linux,/dev/dsp",
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


// EOF of HAE_API_linux.c
