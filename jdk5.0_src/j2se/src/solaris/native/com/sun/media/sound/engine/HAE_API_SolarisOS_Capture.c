/*
 * @(#)HAE_API_SolarisOS_Capture.c	1.35 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*****************************************************************************/
/*
**	HAE_API_SolarisOS_Capture.c
**
**	This provides platform specfic functions for Windows 95/NT. This interface
**	for HAE is for Windows 95/NT and uses the waveIn API to capture audio
**	buffer slices through the multimedia system.
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
**  2001-07-13: $$fb: changes related to using ctl interface for AUDIODEV devices
**                    added SBUltra (Creative labs) and audiots (SunBlade) audio drivers, 
**                    and these pseudo drivers: none, unknown, sun_generic.
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
typedef unsigned int uint_t;
#else
#include <sys/audio.h> 
#include <sys/mixer.h> 
#include <sys/audioio.h>
#endif

// for I_FLUSH
#include <stropts.h>

// $$kk: 08.12.98 merge 

// $$fb: TODO: support multiple devices !

// $$kk: 04.28.98: we *need* to use string.h, NOT strings.h; 
// the latter is not available on earlier versions of solaris
#include <string.h>

#include "HAE_API.h"
#include "Configure.h"
#include "Utilities.h"

#ifndef TRUE
#define TRUE		1
#endif
#ifndef FALSE
#define FALSE		0
#endif


#define HAE_SOLARIS_NUM_CAPTURE_BUFFERS		1			// number of capture buffers

// $$jb: 03.22.99 Changing buffer size from 16384 to 1024
// #define HAE_SOLARIS_FRAMES_PER_BUFFER		1024;		// how many frames do we read (capture) at a time?
// $$kk: 03.31.99: increasing a bit on solaris.
// need to revisit this!
#define HAE_SOLARIS_FRAMES_PER_BUFFER		4096		// how many frames do we read (capture) at a time?
// $$kk: 10.13.98: need to decide whether / how to let user configure this;
// what is a good default??

#define HAE_SOLARIS_SOUND_CAPTURE_PERIOD	10			// sleep time in milliseconds, between position checks
// $$kk: 10.13.98: need to implement position checks!

#define HAE_SOLARIS_DEFAULT_BUFFERSIZE_IN_MS	150		// default size of buffer in ms
#define HAE_SOLARIS_MIN_BUFFERSIZE_IN_MS		50		// minimum size of buffer in ms (can't go beneath
// hardware buffersize, however)


static void				*g_captureBufferBlock = NULL;	// data buffer for capture
// $$kk: 10.13.98: do we need to double buffer?

static UINT32	g_captureByteBufferSize = 0;	// data buffer size in bytes


static INT32				g_captureSound = NULL;			// the audio device
static int				g_captureShutdown;				// false if capture active, otherwise true

static short int		g_bitSize;						// bit size of current capture format
static short int		g_channels;						// number of channels in current capture format
static short int		g_sampleRate;					// sample rate of current capture format

// $$fb: TODO: support multiple devices ??
static short int		g_soundDeviceIndex = 0;			// if non zero then use this device to open

static UINT32	g_encoding;						// audio encoding

// number of frames to read; right now this is fixed as HAE_SOLARIS_FRAMES_PER_BUFFER 
// $$kk: 10.13.98: need to figure out how to configure this
static INT32				g_audioFramesToRead = HAE_SOLARIS_FRAMES_PER_BUFFER;

// how long to sleep at a time
static unsigned int		g_audioCaptureSleepTime;		// setup upon runtime

static HAE_CaptureDone	g_captureDoneProc;				// capture callback

// magically reduces latency 
// $$kk: 08.06.99: revisit!!!
static int				g_audioCaptureBufferSizeDivisor = 2; 

static int g_activeWaveInThread = FALSE;


// $$kk: 04.23.99: added this variable
// 1 if we are paused, otherwise 0.  using this to knock us out of the sleep loop
// when the device is paused.  otherwise, erratically, we may get stuck there forever
// when the device is resumed, so that the device is not paused but never becomes
// active.
int						g_paused = 0;			


// This proc drives audio capture.  This proc both feeds buffers to the device, into 
// which data is captured, and makes callbacks to deliver the captured data.  
// It may be run in a separate thread.  In the case of Solaris, a separate thread is
// required  because read() is (supposed to be) a blocking call.  
// When used with a Java thread, the context needs to be the  JNI environment pointer 
// valid for the thread running the proc.
void PV_AudioWaveInFrameThread(void* context) {
    audio_info_t	sunAudioHeader;
    uint_t			deviceByteBufferSize;
    char			*pFillBuffer;
    int				currentBytesRead;
    int				totalBytesRead;
    INT32			currentPos, lastPos;
    INT32			count;

    INT32			framesToRead;			// number of sample frames to read per callback
    INT32			bytesToRead;			// number of bytes to read per callback
    INT32			buffersToRead;			// number of buffers to read from the device per callback
    INT32			bytesToReadPerBuffer;	// number of bytes to read from the device per buffer


    // $$kk: 10.14.98: need to make sure our capture buffer isn't larger than the system
    // one; otherwise we will overflow and miss data

    //fprintf(stderr, "> PV_AudioWaveInFrameThread, context: %d\n", context);

    g_activeWaveInThread = TRUE;

    ioctl(g_captureSound, AUDIO_GETINFO, &sunAudioHeader);
    deviceByteBufferSize = sunAudioHeader.record.buffer_size;	
    buffersToRead = g_captureByteBufferSize / deviceByteBufferSize;

    if (buffersToRead == 0) {
	buffersToRead = 1;		
	bytesToReadPerBuffer = g_captureByteBufferSize;
    } else {
	bytesToReadPerBuffer = deviceByteBufferSize;
    }

    bytesToRead = buffersToRead * bytesToReadPerBuffer; 
    framesToRead = bytesToRead / (g_channels * g_bitSize / 8);


    // flush anything we might have accumulated in the capture queue before starting the capture thread
    ioctl(g_captureSound, I_FLUSH, FLUSHR);

    currentPos = sunAudioHeader.record.samples;

    // $$kk: 11.03.98: this is a bit of a hack. 
    // we're not keeping track of our position very well.
    // here, if the device is not newly opened, we make sure to back off
    // our currentPos enough that we can reach our lastPos.  otherwise, 
    // when the device is paused and resumed (when the capture stream is
    // stopped and restarted), we get stuck in the while (currentPos < lastPos)
    // loop below....
    if ( (currentPos - framesToRead) >= 0 ) {
	currentPos -= framesToRead;
    }

    lastPos = currentPos + framesToRead;

    while (g_captureShutdown == FALSE) {	
	pFillBuffer = (char *)g_captureBufferBlock;	
	totalBytesRead = 0;
	currentBytesRead = 0;


	// $$kk: 08.12.99: i'm taking this code out for now.  it's deadlock
	// prone and increases latency.  without it, we make a lot more 
	// callbacks.  is the performance hit worth it?  perhaps!
	/*
	  // wait for the device to record enough data to fill our capture buffer
	  while (currentPos < lastPos) {
	  if (g_captureShutdown == TRUE) {
				// do this here, when we can't call it anymore.
				g_captureDoneProc = NULL;

				// release read buffer
				HAE_Deallocate(g_captureBufferBlock);
				return;
				}

				// $$kk: 04.23.99: if we are paused, update the pos, drop out of this loop, 
				// and block down below on the read instead.  otherwise we might get stuck
				// here forever even once the device is resumed.

				if (g_paused)  {
				lastPos = currentPos;
				break;
				}

				HAE_SleepFrameThread(context, HAE_SOLARIS_SOUND_CAPTURE_PERIOD);		// in ms
				
				ioctl(g_captureSound, AUDIO_GETINFO, &sunAudioHeader);
				currentPos = sunAudioHeader.record.samples;
				}

				lastPos += framesToRead;
	*/

	// now read the data from the device record buffer
		
	for (count = 0; count < buffersToRead; count++) {
	    // This is a blocking call on Solaris unless the device is set to a non-blocking mode.  
	    // It returns the number of bytes read or an error code.
	    // $$kk: 10.13.98: this is not blocking for me, even when i don't open the device
	    // with O_NONBLOCK....  what is the difference between O_NONBLOCK and O_NDELAY?
	    currentBytesRead = read(g_captureSound, pFillBuffer, bytesToReadPerBuffer);
	    pFillBuffer += currentBytesRead;
	    totalBytesRead += currentBytesRead;
	}
		
	// callback to deliver the captured data
		
	if (totalBytesRead > 0) {
	    // now callback with captured data
	    (*g_captureDoneProc)(context, DATA_READY_CAPTURE, &g_captureBufferBlock, &totalBytesRead);		
	} else {
	    HAE_SleepFrameThread(context, HAE_SOLARIS_SOUND_CAPTURE_PERIOD);		// in ms
	}
    }
    g_activeWaveInThread = FALSE;
}

// $$fb: 2001-07-13: add ctl support for AUDIODEV environment variable
// isCtl:
//   0 -> the audio device is returned (e.g. /dev/audio)
//   1 -> the control device is returned (e.g. /dev/audioctl)
char* HAE_GetAudioDevRec(long deviceID, int isCtl) {
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

// $$kk: 10.27.99: determine what formats are supported by the device
// $$fb: 2001-07-13: Bug 4426708: add unknown, none, SunBlade, Creative Labs, SunRay, and generic Sun devices. 
//                   better names for constants

#define AUDIO_DRIVER_none          -2
#define AUDIO_DRIVER_unknown       -1
#define AUDIO_DRIVER_sbpro          0
#define AUDIO_DRIVER_sb16           1
#define AUDIO_DRIVER_audiocs        2
#define AUDIO_DRIVER_dbri           3
#define AUDIO_DRIVER_audioamd       4
#define AUDIO_DRIVER_audiots        5
#define AUDIO_DRIVER_SBUltra        6
#define AUDIO_DRIVER_sungeneric   100

static char sbproStr[] = "SUNW,sbpro";
static char sb16Str[] = "SUNW,sb16";
static char audiocsStr[] = "SUNW,CS4231";
static char dbriStr[] = "SUNW,dbri";
static char audioamdStr[] = "SUNW,am79c30";	
static char audiotsStr[] = "SUNW,audiots";
static char SBUltraStr[] = "CREAF,SBUltra";

static char sungenericStrStart[] = "SUNW";


static int HAE_GetDriver(INT32 deviceID) {
    //$$fb do not cache values. The user may wish to change audio devices at runtime 
    int driver = AUDIO_DRIVER_unknown;

    int pseudoDevice = 0;
    INT32				error;

    audio_device_t		deviceInfo;

    // currently, ignore deviceID
    deviceID=deviceID;

    // $$kk: 05.12.99: need to handle AUDIODEV environment variable here!!
    // $$fb 2001-07-13: fixed, bug #4478861
    //pseudoDevice = open("/dev/audioctl", O_RDONLY);
    pseudoDevice = open(HAE_GetAudioDevRec(deviceID, 1), O_RDONLY); // get ctl device name
	
    if (pseudoDevice == -1) {
	//fprintf(stderr, "could not get pseudoDevice\n");
	driver = AUDIO_DRIVER_none;
    } else {
	error = ioctl(pseudoDevice, AUDIO_GETDEV, &deviceInfo);
	close(pseudoDevice);

	if (error < 0) {
	    driver = AUDIO_DRIVER_none;
	}
	else if (strcmp(sbproStr, deviceInfo.name) == 0) {
	    driver = AUDIO_DRIVER_sbpro;
	}
	else if (strcmp(sb16Str, deviceInfo.name) == 0) {
	    driver = AUDIO_DRIVER_sb16;
	}
	else if (strcmp(audiocsStr, deviceInfo.name) == 0) {
	    driver = AUDIO_DRIVER_audiocs;
	}
	else if (strcmp(dbriStr, deviceInfo.name) == 0) {
	    driver = AUDIO_DRIVER_dbri;
	}
	else if (strcmp(audioamdStr, deviceInfo.name) == 0) {
	    driver = AUDIO_DRIVER_audioamd;
	}
	else if (strcmp(audiotsStr, deviceInfo.name) == 0) {
	    driver = AUDIO_DRIVER_audiots;
	}
	else if (strcmp(SBUltraStr, deviceInfo.name) == 0) {
	    driver = AUDIO_DRIVER_SBUltra;
	}
	// if it is a SUNW* driver, assume a minimum set of capability
	else if (strncmp(sungenericStrStart, deviceInfo.name, strlen(sungenericStrStart))==0) {
	    driver = AUDIO_DRIVER_sungeneric;
	}
    }
    //fprintf(stderr, "Found driver %d\n", driver);
    return driver;
}


int HAE_MaxCaptureFormats(INT32 deviceID) {
    int theDriver = HAE_GetDriver(deviceID);
    int maxFormats = 0;

    switch(theDriver) {
    case AUDIO_DRIVER_sbpro:
	maxFormats = 16;
	break;
    case AUDIO_DRIVER_sb16:
	maxFormats = 6 * 2 * 2 * 2;
	break;
    case AUDIO_DRIVER_audiocs:
	maxFormats = 3 * 10 * 2 * 2;
	break;
    case AUDIO_DRIVER_dbri:
	maxFormats = 3 * 10 * 2 * 2;		
	break;
    case AUDIO_DRIVER_audioamd:
	maxFormats = 2;
	break;
    case AUDIO_DRIVER_audiots:
	maxFormats = 11 * 2 * 3;
	break;
    case AUDIO_DRIVER_SBUltra:
	maxFormats = 11 * 2 * 3;
	break;
    case AUDIO_DRIVER_sungeneric:
	maxFormats = 6 * 2 * 2 + 1;
	break;
    case AUDIO_DRIVER_unknown:
	maxFormats = 4 * 2 + 1;
	break;
    default:
	// unknown driver;
	break;
    }
    //fprintf(stderr, "MaxFormats=%d\n", maxFormats);
    return (maxFormats);
}


int HAE_GetSupportedCaptureFormats(INT32 deviceID, UINT32 *encodings, UINT32 *sampleRates, UINT32 *channels, UINT32 *bits, int maxFormats) {

    int encodingCount; int rateCount; int channelCount; int bitCount;
    int encodingIndex, rateIndex, channelIndex, bitIndex;
    int numSupportedFormats = 0;

    /* $$fb 2001-07-13: use HAE_GetDriver !

       char sbproStr[] = "SUNW,sbpro";
       char sb16Str[] = "SUNW,sb16";
       char audiocsStr[] = "SUNW,CS4231";
       char dbriStr[] = "SUNW,dbri";
       char audioamdStr[] = "SUNW,am79c30";	

       int					pseudoDevice = 0;
       INT32				error;
       audio_device_t		deviceInfo;


       // $$kk: 05.12.99: need to handle AUDIODEV environment variable here!!
       pseudoDevice = open("/dev/audioctl", O_RDONLY);

       if (pseudoDevice == -1)
       {
       //fprintf(stderr, "could not get pseudoDevice\n");
       return numSupportedFormats;
       }

       error = ioctl(pseudoDevice, AUDIO_GETDEV, &deviceInfo);
       close(pseudoDevice);
    */

    //$$fb 2001-07-13: use HAE_GetDriver
    int driver=HAE_GetDriver(deviceID);
	
    // max formats: 16
    //if (strcmp(sbproStr, deviceInfo.name) == 0) {
    if (driver==AUDIO_DRIVER_sbpro) {
	// 8 bit linear unsigned data
	// mono: 4000Hz - 44100Hz (ugh; should just choose some here or what??
	// stereo: 11025Hz and 22050Hz only
	// AUDIO_ENCODING_LINEAR or AUDIO_ENCODING_ULAW; some also AUDIO_ENCODING_ALAW

	UINT32 sampleRateArray[] = { 8000, 11025, 16000, 22050, 32000, 44100 };
	rateCount = 6;

	// do 8-bit mono PCM case
	for (rateIndex = 0; rateIndex < rateCount; rateIndex++) {
	    encodings[numSupportedFormats] = PCM;
	    channels[numSupportedFormats] = 1;
	    bits[numSupportedFormats] = 8;
	    sampleRates[numSupportedFormats] = sampleRateArray[rateIndex];
	    numSupportedFormats++;
	    if (numSupportedFormats >= maxFormats) return numSupportedFormats;
	}

	// do 8-bit mono ULAW case
	for (rateIndex = 0; rateIndex < rateCount; rateIndex++) {
	    encodings[numSupportedFormats] = ULAW;
	    channels[numSupportedFormats] = 1;
	    bits[numSupportedFormats] = 8;
	    sampleRates[numSupportedFormats] = sampleRateArray[rateIndex];
	    numSupportedFormats++;
	    if (numSupportedFormats >= maxFormats) return numSupportedFormats;
	}

	// do 8-bit stereo PCM case: 11025Hz and 22050Hz

	// 11025Hz
	encodings[numSupportedFormats] = PCM;
	channels[numSupportedFormats] = 2;
	bits[numSupportedFormats] = 8;
	sampleRates[numSupportedFormats] = 11025;
	numSupportedFormats++;
	if (numSupportedFormats >= maxFormats) return numSupportedFormats;

	// 22050Hz
	encodings[numSupportedFormats] = PCM;
	channels[numSupportedFormats] = 2;
	bits[numSupportedFormats] = 8;
	sampleRates[numSupportedFormats] = 22050;
	numSupportedFormats++;
	if (numSupportedFormats >= maxFormats) return numSupportedFormats;

	// do 8-bit stereo ULAW case: 11025Hz and 22050Hz

	// 11025Hz
	encodings[numSupportedFormats] = ULAW;
	channels[numSupportedFormats] = 2;
	bits[numSupportedFormats] = 8;
	sampleRates[numSupportedFormats] = 11025;
	numSupportedFormats++;
	if (numSupportedFormats >= maxFormats) return numSupportedFormats;

	// 22050Hz
	encodings[numSupportedFormats] = ULAW;
	channels[numSupportedFormats] = 2;
	bits[numSupportedFormats] = 8;
	sampleRates[numSupportedFormats] = 22050;
	numSupportedFormats++;
	if (numSupportedFormats >= maxFormats) return numSupportedFormats;

	return numSupportedFormats;
    }
	

    // max formats: 48
    // if (strcmp(sb16Str, deviceInfo.name) == 0) {
    else if (driver==AUDIO_DRIVER_sb16) {
	// 8 bit linear unsigned data or 16 bit linear signed data
	// mono or stereo, 5000Hz - 44100Hz; some up to 48000Hz
	// just PCM or ULAW too?  presumable ULAW as well...?

	UINT32 encodingsArray[] = { PCM, ULAW };
	UINT32 sampleRateArray[] = { 8000, 11025, 16000, 22050, 32000, 44100 };
	UINT32 channelsArray[] = { 1, 2 };
	UINT32 bitsArray[] = { 8, 16 };

	encodingCount = 2;
	rateCount = 6;
	channelCount = 2;
	bitCount = 2;

	for (encodingIndex = 0; encodingIndex < encodingCount; encodingIndex++) {
	    for (rateIndex = 0; rateIndex < rateCount; rateIndex++) {
		for (channelIndex = 0; channelIndex < channelCount; channelIndex++) {
		    for (bitIndex = 0; bitIndex < bitCount; bitIndex++) {
			encodings[numSupportedFormats] = encodingsArray[encodingIndex];
			sampleRates[numSupportedFormats] = sampleRateArray[rateIndex];
			channels[numSupportedFormats] = channelsArray[channelIndex];
			bits[numSupportedFormats] = bitsArray[bitIndex];
			numSupportedFormats++;					
			if (numSupportedFormats >= maxFormats) return numSupportedFormats;
		    }
		}
	    }		
	} // for

	return numSupportedFormats;
    }

    // max formats: 40 (PCM: 10 * 2; ALAW: 10; ULAW: 10)
    //if (strcmp(audiocsStr, deviceInfo.name) == 0) {
    else if (driver==AUDIO_DRIVER_audiocs) {
	// same as AUDIO_DRIVER_dbri...

	// not entirely sure about the signed / unsigned stuff here....
	// all sample rates for PCM, ULAW, and ALAW.  
	// only 8-bit mono for ULAW and ALAW

	UINT32 encodingsArray[] = { PCM, ULAW, ALAW };
	UINT32 sampleRateArray[] = { 8000, 9600, 11025, 16000, 18900, 22050, 32000, 37800, 44100, 48000 };
	UINT32 channelsArray[] = { 1, 2 };
	UINT32 bitsArray[] = { 8, 16 };

	encodingCount = 3;
	rateCount = 10;
	channelCount = 2;
	bitCount = 2;

	for (encodingIndex = 0; encodingIndex < encodingCount; encodingIndex++) {
	    for (rateIndex = 0; rateIndex < rateCount; rateIndex++) {
		for (channelIndex = 0; channelIndex < channelCount; channelIndex++) {
		    for (bitIndex = 0; bitIndex < bitCount; bitIndex++) {
			if ( ((encodingsArray[encodingIndex] == PCM) 
			      && (bitsArray[bitIndex] == 16)) 
			     || (((encodingsArray[encodingIndex] == ALAW) 
				  || (encodingsArray[encodingIndex] == ULAW)) 
				 && (channelsArray[channelIndex] == 1) 
				 && (bitsArray[bitIndex] == 8))) {
			    encodings[numSupportedFormats] = encodingsArray[encodingIndex];
			    sampleRates[numSupportedFormats] = sampleRateArray[rateIndex];
			    channels[numSupportedFormats] = channelsArray[channelIndex];
			    bits[numSupportedFormats] = bitsArray[bitIndex];
			    numSupportedFormats++;					
			    if (numSupportedFormats >= maxFormats) return numSupportedFormats;
			}
		    }
		}
	    }		
	} // for

	return numSupportedFormats;
    }

    // max formats: 40 (PCM: 10 * 2; ALAW: 10; ULAW: 10)
    //if (strcmp(dbriStr, deviceInfo.name) == 0) {
    else if (driver==AUDIO_DRIVER_dbri) {
	// same as AUDIO_DRIVER_audiocs...

	// not entirely sure about the signed / unsigned stuff here....
	// all sample rates for PCM, ULAW, and ALAW.  
	// only 8-bit mono for ULAW and ALAW

	UINT32 encodingsArray[] = { PCM, ULAW, ALAW };
	UINT32 sampleRateArray[] = { 8000, 9600, 11025, 16000, 18900, 22050, 32000, 37800, 44100, 48000 };
	UINT32 channelsArray[] = { 1, 2 };
	UINT32 bitsArray[] = { 8, 16 };

	encodingCount = 3;
	rateCount = 10;
	channelCount = 2;
	bitCount = 2;

	for (encodingIndex = 0; encodingIndex < encodingCount; encodingIndex++) {
	    for (rateIndex = 0; rateIndex < rateCount; rateIndex++) {
		for (channelIndex = 0; channelIndex < channelCount; channelIndex++) {
		    for (bitIndex = 0; bitIndex < bitCount; bitIndex++) {
			if ( ((encodingsArray[encodingIndex] == PCM) 
			      && (bitsArray[bitIndex] == 16)) 
			     || (((encodingsArray[encodingIndex] == ALAW) 
				  || (encodingsArray[encodingIndex] == ULAW)) 
				 && (channelsArray[channelIndex] == 1) 
				 && (bitsArray[bitIndex] == 8))) {
			    encodings[numSupportedFormats] = encodingsArray[encodingIndex];
			    sampleRates[numSupportedFormats] = sampleRateArray[rateIndex];
			    channels[numSupportedFormats] = channelsArray[channelIndex];
			    bits[numSupportedFormats] = bitsArray[bitIndex];
			    numSupportedFormats++;					
			    if (numSupportedFormats >= maxFormats) return numSupportedFormats;
			}
		    }
		}
	    }		
	} // for

	return numSupportedFormats;
    }

    // max formats: 2
    //if (strcmp(audioamdStr, deviceInfo.name) == 0) {
    else if (driver==AUDIO_DRIVER_audioamd) {
	// 8000Hz 8-bit mono ULAW and ALAW only

	// ULAW case
	encodings[numSupportedFormats] = ULAW;
	channels[numSupportedFormats] = 1;
	bits[numSupportedFormats] = 8;
	sampleRates[numSupportedFormats] = 8000;
	numSupportedFormats++;
	if (numSupportedFormats >= maxFormats) return numSupportedFormats;

	// ALAW case
	encodings[numSupportedFormats] = ALAW;
	channels[numSupportedFormats] = 1;
	bits[numSupportedFormats] = 8;
	sampleRates[numSupportedFormats] = 8000;
	numSupportedFormats++;
	if (numSupportedFormats >= maxFormats) return numSupportedFormats;

	return numSupportedFormats;
    }
	
    //$$fb 2001-07-13: added audiots driver
    // maxformats=11*2*3=66
    else if (driver==AUDIO_DRIVER_audiots) {
	// ALI M5451 audio processor, AC-97 codec
	// supports full duplex with different sample rates if mixer is disabled
	// the following formats are all available when mixer is enabled
	// (it supports more sample rates when mixer is disabled)
	// 11 sample rates, 1 or 2 channels, ulaw/alaw, and pcm 16bit
	unsigned long encodingsArray[] = { PCM, ULAW, ALAW };
	unsigned long sampleRateArray[] = { 8000, 9600, 11025, 16000, 18900, 22050, 32000, 33075, 37800, 44100, 48000 };
	unsigned long channelsArray[] = { 1, 2 };

	encodingCount = 3;
	rateCount = 11;
	channelCount = 2;

	for (encodingIndex = 0; encodingIndex < encodingCount; encodingIndex++) {
	    for (rateIndex = 0; rateIndex < rateCount; rateIndex++) {
		for (channelIndex = 0; channelIndex < channelCount; channelIndex++) {
		    encodings[numSupportedFormats] = encodingsArray[encodingIndex];
		    sampleRates[numSupportedFormats] = sampleRateArray[rateIndex];
		    channels[numSupportedFormats] = channelsArray[channelIndex];
				// PCM only at 16 bit, ULAW and ALAW have 8bit
		    bits[numSupportedFormats] = (encodingsArray[encodingIndex]==PCM)?16:8;
		    numSupportedFormats++;					
		    if (numSupportedFormats >= maxFormats) return numSupportedFormats;
		}
	    }		
	}
	//fprintf(stderr, "audiots: returning %d formats.\n", 		numSupportedFormats);
	return numSupportedFormats;
    }
	
    //$$fb 2001-07-19: added Creative Labs SBUltra driver
    // maxformats=11*2*3=66
    else if (driver==AUDIO_DRIVER_SBUltra) {
	// audiosbu for Creative Labs Sound Blaster Ultra.
	// supports full duplex with different sample rates if mixer is disabled
	// the following formats are always available.
	// (it supports more sample rates when mixer is disabled)
	// 11 sample rates, 1 or 2 channels, ulaw/alaw, and pcm 16bit
	unsigned long encodingsArray[] = { PCM, ULAW, ALAW };
	unsigned long sampleRateArray[] = { 8000, 9600, 11025, 16000, 18900, 22050, 27420, 32000, 33075, 44100, 48000 };
	unsigned long channelsArray[] = { 1, 2 };

	encodingCount = 3;
	rateCount = 11;
	channelCount = 2;

	for (encodingIndex = 0; encodingIndex < encodingCount; encodingIndex++) {
	    for (rateIndex = 0; rateIndex < rateCount; rateIndex++) {
		for (channelIndex = 0; channelIndex < channelCount; channelIndex++) {
		    encodings[numSupportedFormats] = encodingsArray[encodingIndex];
		    sampleRates[numSupportedFormats] = sampleRateArray[rateIndex];
		    channels[numSupportedFormats] = channelsArray[channelIndex];
				// PCM only at 16 bit, ULAW and ALAW have 8bit
		    bits[numSupportedFormats] = (encodingsArray[encodingIndex]==PCM)?16:8;
		    numSupportedFormats++;					
		    if (numSupportedFormats >= maxFormats) return numSupportedFormats;
		}
	    }		
	}
	//fprintf(stderr, "audiots: returning %d formats.\n", 		numSupportedFormats);
	return numSupportedFormats;
    }

    //$$fb 2001-07-13: added sungeneric "driver"
    // maxformats: 6*2*2+1=25
    else if (driver==AUDIO_DRIVER_sungeneric) {
	// same as sb16, but ulaw only for 8KHz mono
	// 8 bit linear unsigned data or 16 bit linear signed data
	// mono or stereo, 8, 11.025, 16, 22.05, 32, 44.1 KHz
	// PCM and Ulaw at 8KHz mono

	unsigned long sampleRateArray[] = { 8000, 11025, 16000, 22050, 32000, 44100 };
	unsigned long channelsArray[] = { 1, 2 };
	unsigned long bitsArray[] = { 8, 16 };

	rateCount = 6;
	channelCount = 2;
	bitCount = 2;

	// ULAW case
	encodings[numSupportedFormats] = ULAW;
	channels[numSupportedFormats] = 1;
	bits[numSupportedFormats] = 8;
	sampleRates[numSupportedFormats] = 8000;
	numSupportedFormats++;
	if (numSupportedFormats >= maxFormats) return numSupportedFormats;

	for (rateIndex = 0; rateIndex < rateCount; rateIndex++) {
	    for (channelIndex = 0; channelIndex < channelCount; channelIndex++) {
		for (bitIndex = 0; bitIndex < bitCount; bitIndex++) {
		    encodings[numSupportedFormats] = PCM;
		    sampleRates[numSupportedFormats] = sampleRateArray[rateIndex];
		    channels[numSupportedFormats] = channelsArray[channelIndex];
		    bits[numSupportedFormats] = bitsArray[bitIndex];
		    numSupportedFormats++;					
		    if (numSupportedFormats >= maxFormats) return numSupportedFormats;
		}
	    }
	}		
	return numSupportedFormats;
    }
	
    //$$fb 2001-07-13: added unknown driver
    // max formats: 2*4+1
    else if (driver==AUDIO_DRIVER_unknown) {
	// when a soundcard is there, but we have no clue what it is.
	// currently: 8000Hz 8-bit mono ULAW and 16 bit 8000, 11025, 22050, 44100 mono/stereo PCM

	unsigned long sampleRateArray[] = { 8000, 11025, 22050, 44100 };
	unsigned long channelsArray[] = { 1, 2 };

	// ULAW 8000Hz mono
	encodings[numSupportedFormats] = ULAW;
	channels[numSupportedFormats] = 1;
	bits[numSupportedFormats] = 8;
	sampleRates[numSupportedFormats] = 8000;
	numSupportedFormats++;
	if (numSupportedFormats >= maxFormats) return numSupportedFormats;

	rateCount = 4;
	channelCount = 2;

	for (rateIndex = 0; rateIndex < rateCount; rateIndex++) {
	    for (channelIndex = 0; channelIndex < channelCount; channelIndex++) {
		encodings[numSupportedFormats] = PCM;
		sampleRates[numSupportedFormats] = sampleRateArray[rateIndex];
		channels[numSupportedFormats] = channelsArray[channelIndex];
		bits[numSupportedFormats] = 16;
		numSupportedFormats++;					
		if (numSupportedFormats >= maxFormats) return numSupportedFormats;
	    }
	}		

	return numSupportedFormats;
    }

    //fprintf(stderr, "unrecognized audio driver; format support unknown\n");
    return numSupportedFormats;
}


// Aquire and enabled audio card
// return 0 if ok, -1 if failed
int HAE_AquireAudioCapture(void *context, UINT32 encoding, UINT32 sampleRate, UINT32 channels, UINT32 bits,
			   UINT32 audioFramesPerBuffer, UINT_PTR *pCaptureHandle) {
    audio_info_t sunAudioHeader;
    INT32 error = -1;
	
    char* pAudioDev = HAE_GetAudioDevRec(g_soundDeviceIndex, 0);
    INT32 minFramesPerBuffer;

    //fprintf(stderr, "Entering HAE_AquireAudioCapture(encoding=%d, samplerate=%d, channels=%d, bits=%d, framesPerBuffer=%d)\n", 
    //	encoding, sampleRate, channels, bits, audioFramesPerBuffer);

    g_encoding = encoding;
    g_bitSize = bits;
    g_channels = channels;
    g_sampleRate = sampleRate;	

    if( audioFramesPerBuffer == 0 ) {
	audioFramesPerBuffer = sampleRate * HAE_SOLARIS_DEFAULT_BUFFERSIZE_IN_MS / 1000;
    }

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
	
    g_captureSound = open(pAudioDev,O_RDONLY|O_NONBLOCK);

    if (g_captureSound > 0) {

	/* set to multiple open */
	if (ioctl(g_captureSound, AUDIO_MIXER_MULTIPLE_OPEN, NULL) >= 0) {
	    TRACE1("HAE_AquireAudioCapture: %s set to multiple open\n", pAudioDev);
	} else {
	    ERROR1("HAE_AquireAudioCapture: ioctl AUDIO_MIXER_MULTIPLE_OPEN failed on %s!\n", pAudioDev);
	}

	AUDIO_INITINFO(&sunAudioHeader);
		
	// Set capture format of the sun device.
	sunAudioHeader.record.sample_rate = sampleRate;
	sunAudioHeader.record.precision = bits;
	sunAudioHeader.record.channels = channels;
	sunAudioHeader.record.buffer_size = g_audioFramesToRead * channels * bits / 8;
		
	sunAudioHeader.record.encoding = AUDIO_ENCODING_LINEAR;
	if (g_encoding == ULAW) {
	    sunAudioHeader.record.encoding = AUDIO_ENCODING_ULAW;			
	} 
	else if (g_encoding == ALAW) {
	    sunAudioHeader.record.encoding = AUDIO_ENCODING_ALAW;			
	} 



	// start out paused so we don't overflow the device driver buffers
	sunAudioHeader.record.pause = 1;
	error = ioctl(g_captureSound, AUDIO_SETINFO, &sunAudioHeader);

	if (error != -1) {
	    // flush anything we might have accumulated in the capture queue before pausing
	    error = ioctl(g_captureSound, I_FLUSH, FLUSHR);

	    error = ioctl(g_captureSound, AUDIO_GETINFO, &sunAudioHeader);
	    g_audioFramesToRead = sunAudioHeader.record.buffer_size / (channels * bits / 8);


	    if (error != -1) {
		if (pCaptureHandle) {
		    *pCaptureHandle = (UINT_PTR)g_captureSound;
		}
	    } 
	}
    }

    if (error == -1) {	// something failed
	HAE_ReleaseAudioCapture(context);
    }

    //fprintf(stderr, "<< HAE_API_SolarisOS_Capture: HAE_AquireAudioCapture() returning %d\n", error);
    return error;
}


// Given the capture hardware is working, fill a buffer with some data. This call is
// asynchronous. When this buffer is filled, the function done will be called.
// returns 0 for success, -1 for failure
int HAE_StartAudioCapture(HAE_CaptureDone done, void *callbackContext) {	
    audio_info_t sunAudioHeader;
    INT32 error = -1;

    //fprintf(stderr, ">> HAE_API_SolarisOS_Capture: HAE_StartAudioCapture()\n");
	
    if (g_captureSound) {	
	// start capture
	g_captureDoneProc = done;
		
	// allocate the capture buffer
	// $$jb: 05.19.99:  This is set in HAE_AquireAudioCapture
	//g_audioFramesToRead = HAE_SOLARIS_FRAMES_PER_BUFFER;	// our read buffer will hold this many frames of sampled audio data

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

	if (error != 0) {
	    g_captureShutdown = TRUE;
	}
    }

    //fprintf(stderr, "<< HAE_API_SolarisOS_Capture: HAE_StartAudioCapture() returning %d\n", error);
    return (error == 0) ? 0 : -1;
}


// stop the capture hardware
// returns 0 for success, -1 for failure
int HAE_StopAudioCapture(void* context) {
    audio_info_t sunAudioHeader;
    INT32 error = -1;

    //fprintf(stderr, ">> HAE_API_SolarisOS_Capture: HAE_StopAudioCapture()\n");
	
    if (g_captureSound) {
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

    //fprintf(stderr, "<< HAE_API_SolarisOS_Capture: HAE_StopAudioCapture() returning %d\n", error);
    return (error == 0) ? 0 : -1;
}


// returns 0 for success, -1 for failure
int HAE_PauseAudioCapture(void) {
    audio_info_t sunAudioHeader;
    INT32 error = -1;

    //fprintf(stderr, ">> HAE_API_SolarisOS_Capture: HAE_PauseAudioCapture()\n");
	
    if (g_captureSound) {
	AUDIO_INITINFO(&sunAudioHeader);
	error = ioctl(g_captureSound, AUDIO_GETINFO, &sunAudioHeader);
		
	if (error != -1) {
	    // pause capture
	    sunAudioHeader.record.pause = 1;
	    error = ioctl(g_captureSound, AUDIO_SETINFO, &sunAudioHeader);
	}

	// $$kk: 04.23.99: added this
	if (error == 0) {
	    g_paused = 1;
	}
    }

    //fprintf(stderr, "<< HAE_API_SolarisOS_Capture: HAE_PauseAudioCapture() returning %d\n", error);
    return (error != -1) ? 0 : -1;
}


// returns 0 for success, -1 for failure
int HAE_ResumeAudioCapture(void) {
    audio_info_t sunAudioHeader;
    INT32 error = -1;

    //fprintf(stderr, ">> HAE_API_SolarisOS_Capture: HAE_ResumeAudioCapture()\n");
	
    if (g_captureSound) {	
	AUDIO_INITINFO(&sunAudioHeader);
	error = ioctl(g_captureSound, AUDIO_GETINFO, &sunAudioHeader);
		
	if (error != -1) {
	    // unpause capture
	    sunAudioHeader.record.pause = 0;
	    error = ioctl(g_captureSound, AUDIO_SETINFO, &sunAudioHeader);
	}

	// $$kk: 04.23.99: added this
	if (error != -1) {
	    g_paused = 0;
	}
    }

    //fprintf(stderr, "<< HAE_API_SolarisOS_Capture: HAE_ResumeAudioCapture() returning %d\n", error);	
    return (error != -1) ? 0 : -1;
}



// Release and free audio card.
// returns 0 for success, -1 for failure
int HAE_ReleaseAudioCapture(void *context) {
    //fprintf(stderr, ">> HAE_API_SolarisOS_Capture: HAE_ReleaseAudioCapture()\n");
	
    if (!g_captureShutdown) {
	HAE_StopAudioCapture(context);
    }

    if (g_captureSound) {
	close(g_captureSound);
	g_captureSound = 0;
    }

    //fprintf(stderr, "<< HAE_API_SolarisOS_Capture: HAE_ResumeAudioCapture() returning 0\n");	
    return 0;
}


// number of devices. ie different versions of the HAE connection. DirectSound and waveOut
// return number of devices. ie 1 is one device, 2 is two devices.
// NOTE: This function needs to function before any other calls may have happened.
INT32 HAE_MaxCaptureDevices(void) {
    /* OK to always return 1; if no device is installed, no formats will be returned */
    return 1;	// only AUDIODEV (defaulting to /dev/audio)
}


// set the current device. device is from 0 to HAE_MaxDevices()
// NOTE:	This function needs to function before any other calls may have happened.
//			Also you will need to call HAE_ReleaseAudioCard then HAE_AquireAudioCard
//			in order for the change to take place.
void HAE_SetCaptureDeviceID(INT32 deviceID, void *deviceParameter) {
    if (deviceID < HAE_MaxCaptureDevices()) {
	g_soundDeviceIndex = deviceID;
    }
    deviceParameter;
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
    char		*data;
    static char *names[] = {
#ifdef __linux__
	"LinuxOS,dev/audio,multi threaded",
#else
	"SolarisOS,dev/audio,multi threaded",
#endif
    };
    if (cName && cNameLength) {
	if (deviceID < HAE_MaxCaptureDevices()) {
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


// return the number of frames in the capture buffer
// (should make this settable?)
UINT32 HAE_GetCaptureBufferSizeInFrames() {
    return g_audioFramesToRead * g_audioCaptureBufferSizeDivisor;
}


// return the number of buffers used.
int HAE_GetCaptureBufferCount() {
    return HAE_SOLARIS_NUM_CAPTURE_BUFFERS;
}

// return the number of samples captured at the device
// $$kk: 10.15.98: need to implement!
UINT32 HAE_GetDeviceSamplesCapturedPosition() {
    return 0L;
}

void HAE_FlushAudioCapture() {
    ioctl(g_captureSound, I_FLUSH, FLUSHR);
}

// EOF of HAE_API_SolarisOS_Capture.c
