/*
 * @(#)HAE_API_LinuxOS_Capture.c	1.16 04/03/15
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*****************************************************************************/
/*
**	This provides platform specfic functions for Linux. This interface
**	for HAE is for Linux and uses the OSS API to capture audio
**	buffer slices.
**
**	Overview:
**
**	History	-
**
**	JAVASOFT
**	10.12.98	$$kk: Created
**
**	05.12.99:	$$kk: added very simple AUDIODEV environment variable support.
**  06.28.99:   $$jb: merged in changes to provide dynamic capture buffer size,
**              changes API for HAE_AquireAudioCapture
**	2002-03-14	$$fb clean up
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

// for I_FLUSH
#include <stropts.h>

#include <string.h>

#include "HAE_API.h"

#ifndef TRUE
#define TRUE		1
#endif
#ifndef FALSE
#define FALSE		0
#endif


#define HAE_LINUX_NUM_CAPTURE_BUFFERS	1	// number of capture buffers

#define HAE_LINUX_FRAMES_PER_BUFFER	4096	// how many frames do we read (capture) at a time?

#define HAE_LINUX_SOUND_CAPTURE_PERIOD	10	// sleep time in milliseconds, between position checks
// $$kk: 10.13.98: need to implement position checks!

#define HAE_LINUX_DEFAULT_BUFFERSIZE_IN_MS	150		// default size of buffer in ms
#define HAE_LINUX_MIN_BUFFERSIZE_IN_MS		50		// minimum size of buffer in ms (can't go beneath
// hardware buffersize, however)


static void  *g_captureBufferBlock = NULL; // data buffer for capture
// $$kk: 10.13.98: do we need to double buffer?

static INT32 g_captureByteBufferSize = 0; // data buffer size in bytes


static int g_captureShutdown; // false if capture active, otherwise true

static short int g_bitSize;    // bit size of current capture format
static short int g_channels;   // number of channels in current capture format
static short int g_sampleRate; // sample rate of current capture format

static short int g_soundDeviceIndex = 0; // if non zero then use this device to open

static UINT32	g_encoding; // audio encoding

// number of frames to read; right now this is fixed as HAE_LINUX_FRAMES_PER_BUFFER
// $$kk: 10.13.98: need to figure out how to configure this
static INT32 g_audioFramesToRead = HAE_LINUX_FRAMES_PER_BUFFER;

static HAE_CaptureDone	g_captureDoneProc; // capture callback

// magically reduces latency
// $$kk: 08.06.99: revisit!!!
static int g_audioCaptureBufferSizeDivisor = 2;

static int g_activeWaveInThread = FALSE;

#define FLUSHMODE_NONE 0
/* will cause the wave in frame thread to wait until flush is done */
#define FLUSHMODE_FLUSHING 1
/* will cause the wave in frame thread to discard all current data */
#define FLUSHMODE_FLUSHED 2
static int g_flushMode = FLUSHMODE_NONE;



// $$kk: 04.23.99: added this variable
// 1 if we are paused, otherwise 0.  using this to knock us out of the sleep loop
// when the device is paused.  otherwise, erratically, we may get stuck there forever
// when the device is resumed, so that the device is not paused but never becomes
// active.
int g_paused = 0;

// $$ay: Linux
typedef int uint_t;

#define MAXFORMATS (6*2*3*2)  // 6 sample rates, 2 channels, 3 encodings, 2 bit sizes

extern int    g_waveDevice; // $$ay: Shared for playback and capture
extern int    g_openForPlayback;
extern int    g_openForCapture;
extern int    g_queriedFormats;
extern int    g_supportsDuplex;
extern int    g_maxFormats;
extern int    g_supEncodings[MAXFORMATS];
extern int    g_supSampleRates[MAXFORMATS];
extern int    g_supChannels[MAXFORMATS];
extern int    g_supBits[MAXFORMATS];
int    HAE_OpenSoundCard(int forCapture);
void   HAE_CloseSoundCard(int forCapture);
int    HAE_Setup(void);

// This proc drives audio capture.  This proc both feeds buffers to the device, into
// which data is captured, and makes callbacks to deliver the captured data.
// It may be run in a separate thread.  In the case of Solaris, a separate thread is
// required  because read() is (supposed to be) a blocking call.
// When used with a Java thread, the context needs to be the  JNI environment pointer
// valid for the thread running the proc.
void PV_AudioWaveInFrameThread(void* context) {

    uint_t			deviceByteBufferSize;
    char			*pFillBuffer;
    audio_buf_info info;
    int				currentBytesRead;
    int				totalBytesRead;
    long			buffersToRead;			// number of buffers to read from the device per callback
    long			bytesToReadPerBuffer;	// number of bytes to read from the device per buffer
    int firstTime = TRUE;

    //fprintf(stderr, "> PV_AudioWaveInFrameThread\n");

    g_activeWaveInThread = TRUE;

    ioctl(g_waveDevice, SNDCTL_DSP_GETBLKSIZE, &deviceByteBufferSize);

    buffersToRead = g_captureByteBufferSize / deviceByteBufferSize;

    if (buffersToRead == 0) {
	buffersToRead = 1;
	bytesToReadPerBuffer = g_captureByteBufferSize / 2;
    } else {
	bytesToReadPerBuffer = deviceByteBufferSize;
    }

    // flush anything we might have accumulated in the capture queue before starting the capture thread
    // $$ay:
    HAE_FlushAudioCapture();
    g_flushMode = FLUSHMODE_NONE;

    while (!g_captureShutdown) {
	pFillBuffer = (char *)g_captureBufferBlock;
	totalBytesRead = 0;
	currentBytesRead = 0;

	// now read the data from the device record buffer
	while (!g_captureShutdown && totalBytesRead < buffersToRead * bytesToReadPerBuffer) {
	    info.bytes = 0;
	    if (ioctl(g_waveDevice, SNDCTL_DSP_GETISPACE, &info) >= 0) {
		//printf("  dev/dsp has %d bytes available\n", (int) info.bytes);
	    	if (g_captureShutdown) {
		    /* read the remaining data */
		    bytesToReadPerBuffer = info.bytes;
		    if (bytesToReadPerBuffer > g_captureByteBufferSize - totalBytesRead) {
		    	bytesToReadPerBuffer = g_captureByteBufferSize - totalBytesRead;
		    }
		}
		if (firstTime) {
		    /* need to trigger start of device with first read? */
		    info.bytes = bytesToReadPerBuffer;
		    firstTime = FALSE;
		}
		if (info.bytes >= bytesToReadPerBuffer
		    && g_flushMode == FLUSHMODE_NONE) {
		    // It returns the number of bytes read or an error code.
		    currentBytesRead = read(g_waveDevice, pFillBuffer, bytesToReadPerBuffer);
		    //printf("  read %d bytes\n", currentBytesRead);
		    if (currentBytesRead > 0) {
			pFillBuffer += currentBytesRead;
			totalBytesRead += currentBytesRead;
		    }
		} else {
		    if (g_flushMode == FLUSHMODE_FLUSHED) {
			break;
		    }
		    HAE_SleepFrameThread(context, HAE_LINUX_SOUND_CAPTURE_PERIOD);
		}
	    } else if (!g_captureShutdown) {
		/* what to do here ? */
		HAE_SleepFrameThread(context, HAE_LINUX_SOUND_CAPTURE_PERIOD);
	    }
	}
	if (g_flushMode == FLUSHMODE_FLUSHED) {
	    /* prevent callback */
	    g_flushMode = FLUSHMODE_NONE;
	    //printf("capture frame thread: discarding %d bytes in response to flush\n", totalBytesRead);
	} else if (totalBytesRead > 0) {
	    // callback to deliver the captured data
	    (*g_captureDoneProc)(context, DATA_READY_CAPTURE, &g_captureBufferBlock, &totalBytesRead);
	}
    } // while
    //fprintf(stderr, "< PV_AudioWaveInFrameThread\n");
    g_activeWaveInThread = FALSE;
}


int HAE_MaxCaptureFormats(INT32 deviceID)
{
    if (!g_queriedFormats) {
	HAE_Setup();
    }
    return (g_maxFormats);
}


int HAE_GetSupportedCaptureFormats(INT32 deviceID, UINT32 *encodings,
				   UINT32 *sampleRates, UINT32 *channels,
				   UINT32 *bits, int maxFormats)
{
    // $$ay: Get the global values from HAE_Setup
    int formats;
    if (!g_queriedFormats) {
	HAE_Setup();
    }
    for (formats = 0; formats < maxFormats; formats++) {
	encodings[formats] = g_supEncodings[formats];
	sampleRates[formats] = g_supSampleRates[formats];
	channels[formats] = g_supChannels[formats];
	bits[formats] = g_supBits[formats];
    }
    return maxFormats;

}


// Aquire and enabled audio card
// return 0 if ok, -1 if failed
// $$fb 2002-02-01: itanium port
int HAE_AquireAudioCapture(void *context, UINT32 encoding, UINT32 sampleRate,
			   UINT32 channels, UINT32 bits,
			   UINT32 audioFramesPerBuffer, UINT_PTR *pCaptureHandle)
{
    long			error = -1;
    //long			minFramesPerBuffer;

    g_encoding = encoding;
    g_bitSize = bits;
    g_channels = channels;
    g_sampleRate = sampleRate;

    // $$jb: 05.19.99: Setting the buffer size
    // $$kk: 08.06.99: i got rid of the fixed minimum value for buffersize
    // minFramesPerBuffer = sampleRate * HAE_LINUX_MIN_BUFFERSIZE_IN_MS / 1000;
    if ( audioFramesPerBuffer == 0 ) {
	audioFramesPerBuffer = sampleRate * HAE_LINUX_DEFAULT_BUFFERSIZE_IN_MS / 1000;
    }


    // $$kk: 08.06.99: got rid of fixed minimum buffer size
    /*
      if ( audioFramesPerBuffer >= minFramesPerBuffer ) {
      g_audioFramesToRead = audioFramesPerBuffer;
      } else {
      g_audioFramesToRead = minFramesPerBuffer;
      }
    */
    g_audioFramesToRead = audioFramesPerBuffer / g_audioCaptureBufferSizeDivisor;
    if (pCaptureHandle) {
	*pCaptureHandle = 0L;
    }

    // try to open wave device for recording
    // $$kk: 12.17.97: need O_NONBLOCK flag to be compatible with windows

    // $$kk: 10.13.98: we want O_NONBLOCK so that we return failure immediately if the
    // device is busy (or absent or whatever) rather than blocking.  however, i think that
    // this same O_NONBLOCK flag dictates whether the read() calls should block.  even
    // without the O_NONBLOCK flag set, read() does *not* block for me, so i'm keeping
    // the flag for now....
    HAE_OpenSoundCard(1); // Open for capture

    if (g_waveDevice > 0) {

	int format = AFMT_MU_LAW;
	int stereo = (channels == 2);
	int speed = sampleRate;

	switch (bits) {
	case 8:
	    format = AFMT_MU_LAW;	/* [sbb fix] don't know if this is right or not -- maybe should be s8? */
	    break;

	case 16:
	    format = AFMT_S16_LE;	/* [sbb fix] needs to be conditional? */
	    break;

	//default:
	//    fprintf(stderr, "Warning: unhandled number of data bits %d\n", (int) bits);
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
	    // flush anything we might have accumulated in the capture queue before pausing
	    HAE_FlushAudioCapture();

	    error = ioctl(g_waveDevice, SNDCTL_DSP_GETBLKSIZE, &g_audioFramesToRead);
	    g_audioFramesToRead = g_audioFramesToRead / (channels * bits / 8);


	    if (error == 0) {
		if (pCaptureHandle) {
		    *pCaptureHandle = (UINT_PTR)g_waveDevice;
		}
	    }
	}
    }

    if (error != 0) {	// something failed
	HAE_ReleaseAudioCapture(context);
    }

    return (error == 0 ? 0 : -1);
}


// Given the capture hardware is working, fill a buffer with some data. This call is
// asynchronous. When this buffer is filled, the function done will be called.
// returns 0 for success, -1 for failure
int HAE_StartAudioCapture(HAE_CaptureDone done, void *callbackContext)
{
    long			error = -1;

    //fprintf(stderr, ">> HAE_API_LinuxOS_Capture: HAE_StartAudioCapture()\n");

    if (g_waveDevice && g_openForCapture) {
	// start capture
	g_captureDoneProc = done;

	// allocate the capture buffer
	// $$jb: 05.19.99:  This is set in HAE_AquireAudioCapture
	//g_audioFramesToRead = HAE_LINUX_FRAMES_PER_BUFFER;	// our read buffer will hold this many frames of sampled audio data

	// we're going to build this many buffers at a time
	if (g_bitSize == 8) {
	    g_captureByteBufferSize = (sizeof(char) * g_audioFramesToRead);
	} else {
	    g_captureByteBufferSize = (sizeof(short int) * g_audioFramesToRead);
	}
	g_captureByteBufferSize *= g_channels;

	// allocate read buffer
	g_captureBufferBlock = HAE_Allocate(g_captureByteBufferSize);

	if (g_captureBufferBlock) {
	    // $$kk: 10.12.98: added this so we can restart capture
	    // $$kk: 11.03.98: mark ourselves active *before* creating frame thread
	    g_captureShutdown = FALSE;

	    // create thread to manage audio capture

	    error = HAE_CreateFrameThread(callbackContext, PV_AudioWaveInFrameThread);


	    if (error == 0) {
		error = HAE_ResumeAudioCapture();
	    }
	}

	if (error != 0)  {
	    g_captureShutdown = TRUE;
	}
    }
    //fprintf(stderr, "<< HAE_API_LinuxOS_Capture: HAE_StartAudioCapture() returning %d\n", error);
    return (error == 0) ? 0 : -1;
}


// stop the capture hardware
// returns 0 for success, -1 for failure
int HAE_StopAudioCapture(void* context)
{
    long			error = -1;

    //fprintf(stderr, ">> HAE_API_LinuxOS_Capture: HAE_StopAudioCapture()\n");

    if (g_waveDevice && g_openForCapture) {
	g_captureShutdown = TRUE;

	// stop streaming data
	error = HAE_PauseAudioCapture();

	// wait for thread to complete
	while (g_activeWaveInThread) {
	    HAE_SleepFrameThread(context, 10);
	}
    }

    // $$kk: 04.13.99: should do this regardless of error value??
    if (error == 0) {
	// destroy the audio capture thread
	error = HAE_DestroyFrameThread(NULL);
    }
    //fprintf(stderr, "<< HAE_API_LinuxOS_Capture: HAE_StopAudioCapture() returning %d\n", error);
    return (error == 0) ? 0 : -1;
}


// returns 0 for success, -1 for failure
int HAE_PauseAudioCapture(void)
{
    //fprintf(stderr, ">> HAE_API_LinuxOS_Capture: HAE_PauseAudioCapture()\n");
    if (g_waveDevice && g_openForCapture) {
	g_paused = 1;
    }

    return 0;
}

static char dummyBuffer[4096];

// returns 0 for success, -1 for failure
int HAE_ResumeAudioCapture(void)
{
    //fprintf(stderr, "<< HAE_API_LinuxOS_Capture: HAE_ResumeAudioCapture() returning %d\n", error);
    HAE_FlushAudioCapture();
    g_paused = 0;
    return 0;
}



// Release and free audio card.
// returns 0 for success, -1 for failure
int HAE_ReleaseAudioCapture(void *context)
{
    //fprintf(stderr, ">> HAE_API_LinuxOS_Capture: HAE_ReleaseAudioCapture()\n");

    // play it safe: destroy thread if not already done
    if (!g_captureShutdown) {
	HAE_StopAudioCapture(context);
    }

    HAE_CloseSoundCard(1); // CLose for capture

    //fprintf(stderr, "<< HAE_API_LinuxOS_Capture: HAE_ResumeAudioCapture() returning 0\n");
    return 0;
}


// number of devices. ie different versions of the HAE connection. DirectSound and waveOut
// return number of devices. ie 1 is one device, 2 is two devices.
// NOTE: This function needs to function before any other calls may have happened.
INT32 HAE_MaxCaptureDevices(void)
{
    int err;

    err = open("/dev/dsp", O_WRONLY | O_NONBLOCK);
    if (err == -1) {
	// there is no audio hardware available,
	// if the device file does not exist,
	// or if the driver is not loaded
	err = errno;

	if (err == ENOENT || err == ENODEV) {
	    return 0;
	}

	// $$fb fix for 4964288: Unexpected IAE raised while getting TargetDataLine
	// add a return 0 as fix
	// this will have as implication that when there is no access
	// to the device, it will not even appear at all. This is wrong, because
	// we know that the device is there, it's just not accessible for
	// some (maybe temporary) reasons. But the problem is that
	// this line will then not provide any formats, and therefore will
	// be unusable anyway, even if the permissions are corrected meanwhile.
	// so there is no way around getting a new Mixer.
	return 0;
    } else {
	close(err);
    }
    return 1;	// only /dev/dsp
}


// set the current device. device is from 0 to HAE_MaxCaptureDevices()
// NOTE:	This function needs to function before any other calls may have happened.
//			Also you will need to call HAE_ReleaseAudioCard then HAE_AquireAudioCard
//			in order for the change to take place.
void HAE_SetCaptureDeviceID(INT32 deviceID, void *deviceParameter)
{
    if (deviceID < HAE_MaxCaptureDevices())
	{
	    g_soundDeviceIndex = deviceID;
	}
}


// return current device ID
// NOTE: This function needs to function before any other calls may have happened.
INT32 HAE_GetCaptureDeviceID(void *deviceParameter)
{
    return g_soundDeviceIndex;
}


// get deviceID name
// NOTE:	This function needs to function before any other calls may have happened.
//			The names returned in this function are platform specific.
void HAE_GetCaptureDeviceName(INT32 deviceID, char *cName, UINT32 cNameLength)
{
    char		*data;
    static char *names[] =
    {	"Linux,dev/dsp,multi threaded",
    };
    if (cName && cNameLength)
	{
	    if (deviceID < HAE_MaxCaptureDevices())
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


// return the number of frames in the capture buffer
// (should make this settable?)
UINT32 HAE_GetCaptureBufferSizeInFrames()
{
    return g_audioFramesToRead * g_audioCaptureBufferSizeDivisor;
}


// return the number of buffers used.
int HAE_GetCaptureBufferCount()
{
    return HAE_LINUX_NUM_CAPTURE_BUFFERS;
}

// return the number of samples captured at the device
// $$kk: 10.15.98: need to implement!
UINT32 HAE_GetDeviceSamplesCapturedPosition()
{
    return 0L;
}

void HAE_FlushAudioCapture() {
    int error;
    audio_buf_info abinfo;
    int avail;

    //fprintf(stderr, ">> HAE_API_LinuxOS_Capture: HAE_ResumeAudioCapture()\n");
    if (g_waveDevice && g_openForCapture) {
	g_flushMode = FLUSHMODE_FLUSHING;
	abinfo.bytes = 0;
	error = ioctl(g_waveDevice, SNDCTL_DSP_GETISPACE, &abinfo);
	if (error >= 0) {
		avail = abinfo.bytes;
		//printf("flush(): Dummy-reading %d bytes\n", avail);
		while ((int) sizeof(dummyBuffer) < avail) {
		    int bytesRead = read(g_waveDevice, dummyBuffer, sizeof(dummyBuffer));
		    if (bytesRead <= 0) break;
		    avail -= bytesRead;
		}
		if (avail > 0) {
		    if (avail > (int) sizeof(dummyBuffer)) {
		    	avail = sizeof(dummyBuffer);
		    }
		    read(g_waveDevice, dummyBuffer, avail);
		}
	}
	g_flushMode = FLUSHMODE_FLUSHED;
    }
}

// EOF of HAE_API_LinuxOS_Capture.c
