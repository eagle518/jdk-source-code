/*
 * @(#)HAE_API_WinOS_Capture.c	1.26 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*****************************************************************************/
/*
**	HAE_API_WinOS_Capture.c
**
**	This provides platform specfic functions for Windows 95/NT. This interface
**	for HAE is for Windows 95/NT and uses the waveIn API to capture audio
**	buffer slices through the multimedia system.
**
**	Overview:
**
**	History	-
**	6/19/98		Created
**	7/23/98		Added a new parameter to HAE_AquireAudioCapture
**				Added HAE_PauseAudioCapture & HAE_ResumeAudioCapture
**	8/3/98		Added support for multi devices and control which device is active
**				Fixed some type casting
**
**	JAVASOFT
**	10.14.98	$$kk: messed with this file utterly.  changed callback mechanism to
**				run a separate thread for callbacks (analogous with output) and not
**				use windows callbacks: 1) dangerous from java because you can't pass
**				JNIEnv pointers between threads and 2) on WinNT, waveInAddBuffer called
**				on windows callback thread locks forever.
**	03.31.99:	$$kk: fixed bug in HAE_GetCaptureDeviceName that causes a crash.
**  06.28.99:   $$jb: merged in changes to provide dynamic capture buffer size,
**              changes API for HAE_AquireAudioCapture
**
**	10.27.99:	$$kk: added HAE_MaxCaptureFormats and HAE_GetSupportedCaptureFormats.
**				added encoding parameter to HAE_AquireAudioCapture.
**
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

/* for waveformat extensible */
#include <mmreg.h>
#include <ks.h>

#include <stdio.h>
#include <fcntl.h>
#include <io.h>

#include "Utilities.h"
#include "HAE_API.h"

#define HAE_WAVEIN_NUM_BUFFERS			3	// number of capture buffers
#define HAE_WAVEIN_SOUND_PERIOD			11	// sleep period between position checks (in ms)

#define HAE_WAVEIN_DEFAULT_BUFFERSIZE_IN_MS	150	// default buffer size in ms
#define HAE_WAVEIN_MIN_BUFFERSIZE_IN_MS		50	// minimum buffer size in ms


static void	*g_audioBufferBlock[HAE_WAVEIN_NUM_BUFFERS];	// actual data buffers
static INT32	g_audioBytesPerBuffer;	// size of each audio buffer in bytes

static HWAVEIN	g_captureSound = NULL;	// the capture device
static BOOL	g_captureShutdown;	// false if capture active, otherwise true

static INT32	g_audioFramesPerBuffer;	// size in sample frames of each capture buffer

static short int g_bitSize;		// bit size of current capture format
static short int g_channels;		// number of channels in current capture format
static short int g_sampleRate;		// sample rate of current capture format

static UINT32	g_encoding;		// audio encoding

static short int g_soundDeviceIndex = 0;// if non zero then use this device to open

static HAE_CaptureDone	g_captureDoneProc;
static void *g_captureDoneContext;
static BOOL g_activeWaveInThread = FALSE;
static BOOL g_waveInStarted = FALSE;

#define FLUSHMODE_NONE 0
/* will cause the wave in frame thread to wait until flush is done */
#define FLUSHMODE_FLUSHING 1
/* will cause the wave in frame thread to discard all current data */
#define FLUSHMODE_FLUSHED 2
static int g_flushMode = FLUSHMODE_NONE;

// this is run by a java thread; the context needs to be the JNI environment
// pointer valid for the thread.
void PV_AudioWaveInFrameThread(void* context) {
    WAVEHDR	waveHeader[HAE_WAVEIN_NUM_BUFFERS];
    long	count, framesToRead, bytesToRead, error;
    long	waveHeaderCount;	// current index in the array of waveheaders
    LPWAVEHDR	pCurrentWaveHdr;
    DWORD dwBytesRecorded;
    LPSTR lpData;

    TRACE0("> PV_AudioWaveInFrameThread\n");

    g_activeWaveInThread = TRUE;
    
    bytesToRead = g_audioBytesPerBuffer;
    framesToRead = g_audioFramesPerBuffer;

    memset(&waveHeader, 0, sizeof(WAVEHDR) * HAE_WAVEIN_NUM_BUFFERS);
    
    // set up all the capture buffers
    for (count = 0; count < HAE_WAVEIN_NUM_BUFFERS; count++) {
	    waveHeader[count].lpData = (char *)g_audioBufferBlock[count];
	    waveHeader[count].dwBufferLength = g_audioBytesPerBuffer;
	    waveHeader[count].dwFlags 		= 0;
	    waveHeader[count].dwLoops 		= 0;
	    error = waveInPrepareHeader(g_captureSound, &waveHeader[count], (INT32)sizeof(WAVEHDR));
    }
    
    /* loop for flushes */
    while (g_captureShutdown == FALSE) {
	
	// add all the capture buffers
	for (count = 0; count < HAE_WAVEIN_NUM_BUFFERS; count++) {
	    waveInAddBuffer(g_captureSound, &waveHeader[count], sizeof(WAVEHDR));
	}
	if (g_flushMode == FLUSHMODE_FLUSHED) {
	    if (g_waveInStarted) {
		waveInStart(g_captureSound);
	    }
	    g_flushMode = FLUSHMODE_NONE;
	}


	// now run this loop to do the capture.
	// we wait for enough samples to be captured to fill one capture buffer,
	// callback with the captured data, and put the buffer back in the queue.

	waveHeaderCount = 0; // which buffer we're processing
	while (g_captureShutdown == FALSE) {
	    TRACE0("  PV_AudioWaveInFrameThread: in loop\n");
	    // wait for the device to record enough data to fill our capture buffer

	    // this is the data buffer for the current capture buffer
	    pCurrentWaveHdr = &waveHeader[waveHeaderCount];

	    while ((g_flushMode == FLUSHMODE_FLUSHING)
	           || ((!(pCurrentWaveHdr->dwFlags & WHDR_DONE)) 
	                 && (g_captureShutdown == FALSE)) ) {
		//printf("  PV_AudioWaveInFrameThread: sleep\n");
		HAE_SleepFrameThread(context, HAE_WAVEIN_SOUND_PERIOD);		// in ms
	    }
	    if (g_flushMode == FLUSHMODE_FLUSHED) {
		/* discard all buffers by bailing out to
		 * the outer loop in order to 
		 * - re-add all buffers 
		 * - and restart the device
		 */
		break;
	    }

	    // then process the captured data
	    if (g_captureShutdown == FALSE
	        && pCurrentWaveHdr->dwFlags & WHDR_DONE) {
		dwBytesRecorded = pCurrentWaveHdr->dwBytesRecorded;
		lpData = pCurrentWaveHdr->lpData;

		// callback with the captured data
		//printf("  PV_AudioWaveInFrameThread: callback\n");
		(*g_captureDoneProc)(context, DATA_READY_CAPTURE, &lpData, (void *)&dwBytesRecorded);

		// add the buffer back into the queue
		//printf("  PV_AudioWaveInFrameThread: in addBuffer\n");
		error = waveInAddBuffer(g_captureSound, pCurrentWaveHdr, sizeof(WAVEHDR));
		// increment to the next wavehdr
		waveHeaderCount++;
		if (waveHeaderCount == HAE_WAVEIN_NUM_BUFFERS) {
		    waveHeaderCount = 0;
		}
	    }
	} // while (inner loop)
    } // while (outer loop to support flush())
	

    //printf("  PV_AudioWaveInFrameThread: reset\n");
    waveInReset(g_captureSound); // stop all audio before unpreparing headers
    
    /* send all pending captured buffers to the app */
    count = 0;
    for (count = 0; count < HAE_WAVEIN_NUM_BUFFERS; count++) {
	pCurrentWaveHdr = &waveHeader[waveHeaderCount];
	if (pCurrentWaveHdr->dwFlags & WHDR_DONE) {
	    dwBytesRecorded = pCurrentWaveHdr->dwBytesRecorded;
	    lpData = pCurrentWaveHdr->lpData;
	    //printf("  PV_AudioWaveInFrameThread: callback\n");
	    (*g_captureDoneProc)(context, DATA_READY_CAPTURE, &lpData, (void *)&dwBytesRecorded);
	} else {
	    break;
	}
	waveHeaderCount++;
	if (waveHeaderCount == HAE_WAVEIN_NUM_BUFFERS) {
	    waveHeaderCount = 0;
	}
    }

    // unprepare headers
    for (count = 0; count < HAE_WAVEIN_NUM_BUFFERS; count++) {
	waveInUnprepareHeader(g_captureSound, &waveHeader[count], (INT32)sizeof(WAVEHDR));
    }
    // do this here, when we can't call it anymore.
    g_captureDoneProc = NULL;
    TRACE0("< PV_AudioWaveInFrameThread\n");
    g_activeWaveInThread = FALSE;
}


// $$kk: 10.27.99: determine what formats are supported by the device

static UINT32 sampleRateArray[] = { 8000, 11025, 16000, 22050, 32000, 44100, 48000, 56000, 88000, 96000, 172000, 192000 };
static UINT32 channelsArray[] = { 1, 2};
static UINT32 bitsArray[] = { 8, 16};

#define SAMPLERATE_COUNT sizeof(sampleRateArray)/sizeof(UINT32)
#define CHANNELS_COUNT sizeof(channelsArray)/sizeof(UINT32)
#define BITS_COUNT sizeof(bitsArray)/sizeof(UINT32)

int HAE_MaxCaptureFormats(INT32 deviceID) {
    return (SAMPLERATE_COUNT * CHANNELS_COUNT * BITS_COUNT);
}

#ifndef _WAVEFORMATEXTENSIBLE_
#define _WAVEFORMATEXTENSIBLE_
typedef struct {
    WAVEFORMATEX    Format;
    union {
        WORD wValidBitsPerSample;       /* bits of precision  */
        WORD wSamplesPerBlock;          /* valid if wBitsPerSample==0 */
        WORD wReserved;                 /* If neither applies, set to zero. */
    } Samples;
    DWORD           dwChannelMask;      /* which channels are */
                                        /* present in stream  */
    GUID            SubFormat;
} WAVEFORMATEXTENSIBLE, *PWAVEFORMATEXTENSIBLE;
#endif // !_WAVEFORMATEXTENSIBLE_

#if !defined(WAVE_FORMAT_EXTENSIBLE)
#define  WAVE_FORMAT_EXTENSIBLE                 0xFFFE
#endif // !defined(WAVE_FORMAT_EXTENSIBLE)

#if !defined(DEFINE_WAVEFORMATEX_GUID)
#define DEFINE_WAVEFORMATEX_GUID(x) (USHORT)(x), 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71
#endif
#ifndef STATIC_KSDATAFORMAT_SUBTYPE_PCM
#define STATIC_KSDATAFORMAT_SUBTYPE_PCM\
    DEFINE_WAVEFORMATEX_GUID(WAVE_FORMAT_PCM)
#endif


void createWaveFormat(WAVEFORMATEXTENSIBLE* format, 
                      int sampleRate, 
                      int channels, 
                      int bits) {
    GUID subtypePCM = {STATIC_KSDATAFORMAT_SUBTYPE_PCM};
    format->Format.nSamplesPerSec = (DWORD)sampleRate;
    format->Format.nChannels = (WORD) channels;
    /* do not support useless padding, like 24-bit samples stored in 32-bit containers */
    format->Format.wBitsPerSample = (WORD) ((bits + 7) & 0xFFF8);

    if (channels <= 2 && bits <= 16) {
	format->Format.wFormatTag = WAVE_FORMAT_PCM;
	format->Format.cbSize = 0;
    } else {
	format->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	format->Format.cbSize = 22;
	format->Samples.wValidBitsPerSample = bits;
	/* no way to specify speaker locations */
	format->dwChannelMask = 0xFFFFFFFF;
	format->SubFormat = subtypePCM;
    }
    format->Format.nBlockAlign = (WORD)((format->Format.wBitsPerSample * format->Format.nChannels) / 8);
    format->Format.nAvgBytesPerSec = format->Format.nSamplesPerSec * format->Format.nBlockAlign;
}

int HAE_GetSupportedCaptureFormats(INT32 deviceID, 
                                   UINT32 *encodings, 
                                   UINT32 *sampleRates, 
                                   UINT32 *channels, 
                                   UINT32 *bits, 
                                   int maxFormats) {
    MMRESULT		theErr;
    WAVEFORMATEXTENSIBLE format;

    int rateIndex, channelIndex, bitIndex;
    int numSupportedFormats = 0;

    /* $$fb 2002-04-10: fix for 4514334: JavaSoundDemo Capture not works on Windows2000 with USB Port */
    if (deviceID == 0) {
	deviceID = WAVE_MAPPER;
    } else {
	deviceID--;
    }

    for (rateIndex = 0; rateIndex < SAMPLERATE_COUNT; rateIndex++) {
	for (channelIndex = 0; channelIndex < CHANNELS_COUNT; channelIndex++) {
	    for (bitIndex = 0; bitIndex < BITS_COUNT; bitIndex++) {
		createWaveFormat(&format, sampleRateArray[rateIndex], channelsArray[channelIndex], bitsArray[bitIndex]);
		theErr = waveInOpen(0, deviceID, (WAVEFORMATEX*) &format, 
			            0, 0, WAVE_FORMAT_QUERY | WAVE_FORMAT_DIRECT);

		if (theErr == MMSYSERR_NOERROR) {
		    // this format is supported!!
		    encodings[numSupportedFormats] = PCM;
		    sampleRates[numSupportedFormats] = sampleRateArray[rateIndex];
		    channels[numSupportedFormats] = channelsArray[channelIndex];
		    bits[numSupportedFormats] = bitsArray[bitIndex];

		    numSupportedFormats++;
		    if (numSupportedFormats >= maxFormats) return numSupportedFormats;
		}
	    }
	}
    }
    return numSupportedFormats;
}



// Aquire and enabled audio card
// return 0 if ok, -1 if failed
int HAE_AquireAudioCapture(void *context, UINT32 encoding, UINT32 sampleRate, UINT32 channels, UINT32 bits,
			   UINT32 audioFramesPerBuffer, UINT_PTR *pCaptureHandle)
{
    MMRESULT		theErr;
    WAVEINCAPS		caps;
    WAVEFORMATEX	format;
    INT32			deviceID;
    ULONG			minFramesPerBuffer;

    if (encoding != PCM) {
	// fprintf(stderr, "HAE_AquireAudioCapture: unsupported encoding: %d\n", g_encoding);
	return -1;
    }

    g_encoding = encoding;
    g_bitSize = bits;
    g_channels = channels;
    g_sampleRate = sampleRate;

    minFramesPerBuffer = sampleRate * HAE_WAVEIN_MIN_BUFFERSIZE_IN_MS / 1000;

    if( audioFramesPerBuffer == 0 ) {
	audioFramesPerBuffer = sampleRate * HAE_WAVEIN_DEFAULT_BUFFERSIZE_IN_MS / 1000;
    }

    g_audioFramesPerBuffer = audioFramesPerBuffer;

    if (pCaptureHandle) {
	/* what about multi-thread ? */
	*pCaptureHandle = 0L;
    }

    /* $$fb 2002-04-10: fix for 4514334: JavaSoundDemo Capture not works on Windows2000 with USB Port */
    if (g_soundDeviceIndex == 0) {
	deviceID = WAVE_MAPPER;
    } else {
	deviceID = g_soundDeviceIndex - 1;
    }

    theErr = waveInGetDevCaps(deviceID, &caps, sizeof(WAVEINCAPS));

    if (theErr == MMSYSERR_NOERROR) {
	    format.wFormatTag = WAVE_FORMAT_PCM;
	    format.nSamplesPerSec = sampleRate;
	    format.wBitsPerSample = (WORD)bits;
	    format.nChannels = (WORD)channels;

	    format.nBlockAlign = (WORD)((format.wBitsPerSample * format.nChannels) / 8);
	    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
	    format.cbSize = 0;

	    // $$fb 2002-02-01: itanium port: use UINT_PTR for casting a pointer to UINT
	    theErr = waveInOpen(&g_captureSound, deviceID, &format,
				0L, (UINT_PTR)context, CALLBACK_NULL);

	    if (theErr == MMSYSERR_NOERROR) {
		    g_captureShutdown = FALSE;
		    g_waveInStarted = FALSE;

		    if (pCaptureHandle) {
			    *pCaptureHandle = (UINT_PTR)g_captureSound;
			}
		} else {
		    HAE_ReleaseAudioCapture(context);
		}
	}

    return (theErr == MMSYSERR_NOERROR) ? 0 : -1;
}

// Given the capture hardware is working, fill a buffer with some data. This call is
// asynchronous. When this buffer is filled, the function done will be called.
// returns 0 for success, -1 for failure
int HAE_StartAudioCapture(HAE_CaptureDone done, void *callbackContext) {
    long error = 0;
    int i;

    // start capture
    g_captureDoneProc = done;
    g_captureDoneContext = callbackContext;

    // calculate the number of bytes per capture buffer
    if (g_bitSize == 8) {
	    g_audioBytesPerBuffer = (sizeof(char) * g_audioFramesPerBuffer);
	} else {
	    g_audioBytesPerBuffer = (sizeof(short int) * g_audioFramesPerBuffer);
	}
    g_audioBytesPerBuffer *= g_channels;

    // allocate the capture data buffers
    // this could really be done in initialiation...?
    for (i = 0; i < HAE_WAVEIN_NUM_BUFFERS; i++) {
	    g_audioBufferBlock[i] = HAE_Allocate(g_audioBytesPerBuffer);
	    if (g_audioBufferBlock[i] == NULL) {
		    error = -1;	// something is wrong
		    break;
		}
	} // for

    if (error == 0) {
	    // create thread to manage audio capture
	    error = HAE_CreateFrameThread(callbackContext, PV_AudioWaveInFrameThread);
	    
	    if (error == 0) {
		    error = HAE_ResumeAudioCapture();
		}
	}

    if (error == 0) {
	    // $$kk: 10.12.98: added this so we can restart capture
	    g_captureShutdown = FALSE;
	}

    return (error == 0) ? 0 : -1;
}


// stop the capture hardware
int HAE_StopAudioCapture(void* context) {
    MMRESULT	theErr;
    int i;

    TRACE0("> HAE_StopAudioCapture\n");
    if (g_captureSound) {
	    // tell the thread to die
	    // the thread will also reset the device
	    g_captureShutdown = TRUE;

	    // stop streaming data
	    theErr = HAE_PauseAudioCapture();

	    // destroy the audio capture thread. 
    
	    /* $$fb:
	     * this is a dummy operation! It wouldn't even
	     * distinguish between playback thread and capture thread...
	     */
	    HAE_DestroyFrameThread(NULL);

	    //printf("  waiting for thread to complete\n");
	    // wait for thread to complete
	    while (g_activeWaveInThread) {
		HAE_SleepFrameThread(context, 10);
	    }

	    // deallocate the capture data buffers
	    for (i = 0; i < HAE_WAVEIN_NUM_BUFFERS; i++) {
		HAE_Deallocate(g_audioBufferBlock[i]);
	    } // for
	}
    TRACE0("< HAE_StopAudioCapture\n");
    return 0;
}

int HAE_PauseAudioCapture(void) {
    if (g_captureSound) {
	    // stop streaming data
	    g_waveInStarted = FALSE;
	    waveInStop(g_captureSound);
	}
    return 0;
}

int HAE_ResumeAudioCapture(void) {
    if (g_captureSound) {
	    // start streaming data
	    waveInStart(g_captureSound);
	    g_waveInStarted = TRUE;
	}
    return 0;
}



// Release and free audio card.
// return 0 if ok, -1 if failed.
int HAE_ReleaseAudioCapture(void *context) {
    TRACE0("> HAE_ReleaseAudioCapture\n");
    if (g_captureSound) {
	    // play it safe: destroy thread if not already done
	    if (!g_captureShutdown) {
		HAE_StopAudioCapture(context);
	    }

	    //printf("  WaveInClose\n");
	    while (waveInClose(g_captureSound) == WAVERR_STILLPLAYING) {
		    HAE_SleepFrameThread(context, 10); // in millis
		}
	    g_waveInStarted = FALSE;
	    g_captureSound = NULL;
	}
    TRACE0("< HAE_ReleaseAudioCapture\n");
    return 0;
}


// number of devices. ie different versions of the HAE connection. DirectSound and waveOut
// return number of devices. ie 1 is one device, 2 is two devices.
// NOTE: This function needs to function before any other calls may have happened.
INT32 HAE_MaxCaptureDevices(void) {
    /* $$fb 2002-04-10: fix for 4514334: JavaSoundDemo Capture not works on Windows2000 with USB Port
     * return WAVE_MAPPER + available devices
     */
    return waveInGetNumDevs()?waveInGetNumDevs()+1:0;
}


// set the current device. device is from 0 to HAE_MaxDevices() - 1
// NOTE:	This function needs to function before any other calls may have happened.
//			Also you will need to call HAE_ReleaseAudioCard then HAE_AquireAudioCard
//			in order for the change to take place.
void HAE_SetCaptureDeviceID(INT32 deviceID, void *deviceParameter) {
    if (deviceID < HAE_MaxCaptureDevices()) {
	    g_soundDeviceIndex = deviceID;
	}
}


// return current device ID
// NOTE: This function needs to function before any other calls may have happened.
INT32 HAE_GetCaptureDeviceID(void *deviceParameter) {
    return g_soundDeviceIndex;
}


// get deviceID name
// NOTE:	This function needs to function before any other calls may have happened.
//			The names returned in this function are platform specific.
void HAE_GetCaptureDeviceName(INT32 deviceID, char *cName, UINT32 cNameLength) {
    WAVEINCAPS		caps;
    MMRESULT		theErr;

    if (deviceID < HAE_MaxCaptureDevices()) {
	    /* $$fb 2002-04-10: fix for 4514334: JavaSoundDemo Capture not works on Windows2000 with USB Port */
	    if (deviceID == 0) {
		deviceID = (INT32) WAVE_MAPPER;
	    } else {
		deviceID--;
	    }

	    theErr = waveInGetDevCaps((UINT)deviceID, &caps, sizeof(WAVEINCAPS));
	    if ((theErr == MMSYSERR_NOERROR) && cName && cNameLength) {
		    strncpy(cName, caps.szPname, cNameLength-1);
		    cName[cNameLength-1] = 0;
		}
	}
}


// return the number of frames in the capture buffer
// (should make this settable?)
UINT32 HAE_GetCaptureBufferSizeInFrames() {
    return g_audioFramesPerBuffer;
}


// return the number of buffers used.
int HAE_GetCaptureBufferCount() {
    return HAE_WAVEIN_NUM_BUFFERS;
}


// return the number of samples captured at the device
// $$kk: 10.15.98: need to implement!
UINT32 HAE_GetDeviceSamplesCapturedPosition() {
    /* get the current position */
    /* MMTIME audioStatus;
	long currentPos;
	long lastPos;
	
	memset(&audioStatus, 0, (INT32)sizeof(MMTIME));
	audioStatus.wType = TIME_BYTES;	// get byte position
	error = waveInGetPosition(g_captureSound, &audioStatus, sizeof(MMTIME));
	currentPos = audioStatus.u.cb;
	lastPos = currentPos + g_audioBytesPerBuffer;
    */
    return 0L;
}


void HAE_FlushAudioCapture() {
    TRACE0("> HAE_FlushAudioCapture\n");
    if (g_captureSound) {
	g_flushMode = FLUSHMODE_FLUSHING;
	/* flush will cease activity, and mark all buffers as done */
	waveInReset(g_captureSound);
	g_flushMode = FLUSHMODE_FLUSHED;
    }
}

// EOF of HAE_API_WinOS_Capture.c
