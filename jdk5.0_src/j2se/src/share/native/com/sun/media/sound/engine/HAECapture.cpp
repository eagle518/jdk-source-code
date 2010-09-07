/*
 * @(#)HAECapture.cpp	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*****************************************************************************/
/*
** "HAECapture.cpp"
**
**	Generalized Audio Synthesis package presented in an oop fashion
**
** Modification History:
**
**	6/4/98		Created
**	7/31/98		Changed the class structure to match the mixer class. Finished
**				implementing.
**	8/11/98		Renamed HAECaptureAudio to HAECapture. Added HAECaptureStream.
*/
/*****************************************************************************/

#include "HAE.h"
#include "HAECapture.h"
#include "HAEPrivate.h"

#include "HAE_API.h"

#include "X_API.h"
#include "X_Formats.h"
#include "GenSnd.h"
#include "GenPriv.h"

#if USE_CAPTURE_API

// Define XCaptureDevice
typedef struct 
{
    HAECapture			*pCapture;
    HAEStreamData		streamData;
    void				*pCaptureBuffer;
    unsigned long		captureBufferSize;
} XCaptureDevice;

static char			captureAudioSetup = 0;			// audio will only be setup once


// Class implemention for HAECapture

//#pragma mark (HAECapture class)

HAECapture::HAECapture()
{
    mCallbackProc = NULL;
    mCaptureAudioEngaged = FALSE;
    mPlaybackStream = NULL;
    mReference = 0;

    // set default settings
    mStreamSampleInfo.baseMidiPitch = 60;
    mStreamSampleInfo.waveSize = 0;
    mStreamSampleInfo.waveFrames = 0;
    mStreamSampleInfo.startLoop = 0;
    mStreamSampleInfo.endLoop = 0;
    mStreamSampleInfo.sampledRate = LONG_TO_FIXED(22050);
    mStreamSampleInfo.bitSize = 16;
    mStreamSampleInfo.channels = 1;

    mData = XNewPtr((long)sizeof(XCaptureDevice));
    if (mData)
	{
	    ((XCaptureDevice *)mData)->pCapture = this;
	}
}

HAECapture::~HAECapture()
{
    Close();
    XDisposePtr(mData);
    mData = NULL;
}

HAE_BOOL HAECapture::IsOpen(void) const
{
    return captureAudioSetup ? (HAE_BOOL)TRUE : (HAE_BOOL)FALSE;
}

// open the capture.
HAEErr HAECapture::Open(HAE_UNSIGNED_FIXED captureSampleRate, char dataBitSize, char channelSize)
{
    HAEErr	theErr;

    theErr = HAE_NO_ERROR;
    mStreamSampleInfo.sampledRate = captureSampleRate;
    mStreamSampleInfo.bitSize = dataBitSize;
    mStreamSampleInfo.channels = channelSize;

    if (HAECapture::IsOpen() == FALSE)	// don't allow more than one device open at a time. This may change
	{		
	    // try to get the capture hardware with current configuration
	    if (HAE_AquireAudioCapture(	NULL, 
					UNSIGNED_FIXED_TO_LONG(captureSampleRate), 
					(unsigned long)channelSize, 
					(unsigned long)dataBitSize, NULL) == 0)
		{
		    captureAudioSetup++;
		    mCaptureAudioEngaged = TRUE;
		}
	    else
		{
		    theErr = HAE_DEVICE_UNAVAILABLE;
		}
	}
    else
	{
	    theErr = HAE_NOT_REENTERANT;		// can't be reentrant
	}
    return theErr;
}

// close the capture
HAEErr HAECapture::Close(void)
{
    HAEErr	theErr;

    theErr = HAE_NO_ERROR;
    if (mCaptureAudioEngaged)
	{
	    if (HAE_ReleaseAudioCapture(NULL) == 0)
		{
		    mCaptureAudioEngaged = FALSE;
		    captureAudioSetup--;
		}
	    else
		{
		    theErr = HAE_DEVICE_UNAVAILABLE;
		}
	}
    else
	{
	    theErr = HAE_NOT_SETUP;
	}
    return theErr;
}


// is the mixer connected to the audio hardware
HAE_BOOL HAECapture::IsCaptureEngaged(void)
{
    return mCaptureAudioEngaged;
}

// disengage from audio hardware
HAEErr HAECapture::DisengageAudio(void)
{
    HAEErr	theErr;

    theErr = HAE_ALREADY_PAUSED;
    if (mCaptureAudioEngaged)
	{
	    theErr = HAECapture::Close();
	}
    return theErr;
}

// reengage to audio hardware
HAEErr HAECapture::ReengageAudio(void)
{
    HAEErr	theErr;

    theErr = HAE_ALREADY_RESUMED;
    if (mCaptureAudioEngaged)
	{
	    theErr = HAECapture::Open(mStreamSampleInfo.sampledRate,
				      (char)mStreamSampleInfo.bitSize,
				      (char)mStreamSampleInfo.channels);
	}
    return theErr;
}


// return number of devices available to this mixer hardware
// will return a number from 1 to max number of devices.
// ie. a value of 2 means two devices
long HAECapture::GetMaxDeviceCount(void)
{
#if USE_DEVICE_ENUM_SUPPORT == TRUE
    return HAE_MaxCaptureDevices();
#else
    return 0;
#endif
}

// set current device. should be a number from 0 to HAECapture::GetDeviceCount()
void HAECapture::SetCurrentDevice(long deviceID, void *deviceParameter)
{
#if USE_DEVICE_ENUM_SUPPORT == TRUE
    if (deviceID < GetMaxDeviceCount())
	{
	    if (IsOpen())
		{
		    DisengageAudio();		// shutdown from hardware
		}
	    HAE_SetCaptureDeviceID(deviceID, deviceParameter);	// change to new device
	    if (IsOpen())
		{
		    ReengageAudio();		// connect back to audio with new device
		}
	}
#else
    deviceID = deviceID;
    deviceParameter = deviceParameter;
#endif
}

// get current device.
long HAECapture::GetCurrentDevice(void *deviceParameter)
{
    return HAE_GetCaptureDeviceID(deviceParameter);
}

// get device name
// NOTE:	This function needs to function before any other calls may have happened.
//			Format of string is a zero terminated comma delinated C string.
//			"platform,method,misc"
//	example	"MacOS,Sound Manager 3.0,SndPlayDoubleBuffer"
//			"WinOS,DirectSound,multi threaded"
//			"WinOS,waveOut,multi threaded"
//			"WinOS,VxD,low level hardware"
//			"WinOS,plugin,Director"
void HAECapture::GetDeviceName(long deviceID, char *cName, unsigned long cNameLength)
{
#if USE_DEVICE_ENUM_SUPPORT == TRUE
    HAE_GetCaptureDeviceName(deviceID, cName, cNameLength);
#else
    deviceID = deviceID;
    cName = cName;
    cNameLength = cNameLength;
#endif
}


//***************************

// call callback of HAECapture object
static void PV_CallCaptureCallback(HAECaptureStream *pCaptureStream, HAEStreamMessage message, HAEStreamData *pData)
{
    HAEStreamObjectProc callback;
    XCaptureDevice		*pCapture;
    HAECapture			*pCaptureObject;

    if (pCaptureStream && pData)
	{
	    pCaptureObject = pCaptureStream->GetCapture();
	    if (pCaptureObject)
		{
		    pCapture = (XCaptureDevice *)pCaptureObject->GetPrivateData();

		    callback = pCaptureStream->GetCallbackProc();
		    if (callback)
			{
			    (*callback)(message, pData);
			}
		}
	}
}

// streaming capture callback. Used to translate between GenAudio API functions and the HAE C++ function error codes.
static OPErr PV_CustomInputStreamCallback(void *context, GM_StreamMessage message, GM_StreamData *pAS)
{
    HAECaptureStream		*pCaptureStream;
    HAEStreamObjectProc		callback;
    HAEStreamData			iData;
    OPErr					theErr;
    HAEErr					igorErr;
    HAEStreamMessage		igorMessage;

    context = context;
    theErr = NO_ERR;
    igorErr = HAE_NO_ERROR;
    pCaptureStream = (HAECaptureStream *)pAS->userReference;
    if (pCaptureStream)
	{
	    iData.userReference = (long)pCaptureStream->GetReference();
	    iData.pData = pAS->pData;
	    iData.dataLength = pAS->dataLength;
	    iData.sampleRate = pAS->sampleRate;
	    iData.dataBitSize = pAS->dataBitSize;
	    iData.channelSize = pAS->channelSize;
	    iData.startSample = pAS->startSample;
	    iData.endSample = pAS->endSample;
	    callback = (HAEStreamObjectProc)pCaptureStream->GetCallbackProc();
	    if (callback)
		{
		    switch (message)
			{
			default:
			    igorMessage = HAE_STREAM_NULL;
			    break;
			case STREAM_CREATE:
			    igorMessage = HAE_STREAM_CREATE;
			    break;
			case STREAM_DESTROY:
			    igorMessage = HAE_STREAM_DESTROY;
			    break;
			case STREAM_GET_DATA:
			    igorMessage = HAE_STREAM_GET_DATA;
			    break;
			case STREAM_GET_SPECIFIC_DATA:
			    igorMessage = HAE_STREAM_GET_SPECIFIC_DATA;
			    break;
			case STREAM_HAVE_DATA:
			    igorMessage = HAE_STREAM_HAVE_DATA;
			    break;
			}
		    if (igorMessage != HAE_STREAM_NULL)
			{
			    igorErr = (*callback)(igorMessage, &iData);
			}
		    else
			{
			    igorErr = HAE_NOT_SETUP;
			}
		    pAS->pData = iData.pData;
		    pAS->dataLength = iData.dataLength;
		    pAS->sampleRate = iData.sampleRate;
		    pAS->dataBitSize = iData.dataBitSize;
		    pAS->channelSize = iData.channelSize;
		    pAS->startSample = iData.startSample;
		    pAS->endSample = iData.endSample;
		}
	}
    switch (igorErr)
	{
	default:
	    theErr = GENERAL_BAD;
	    break;
	case HAE_NO_ERROR:
	    theErr = NO_ERR;
	    break;
	case HAE_BUFFER_TO_SMALL:
	    theErr = BUFFER_TO_SMALL;
	    break;
	case HAE_NOT_SETUP:
	    theErr = NOT_SETUP;
	    break;
	case HAE_PARAM_ERR:
	    theErr = PARAM_ERR;
	    break;
	case HAE_MEMORY_ERR:
	    theErr = MEMORY_ERR;
	    break;
	case HAE_STREAM_STOP_PLAY:
	    theErr = STREAM_STOP_PLAY;
	    break;
	}
    return theErr;
}


// Class implemention for HAECaptureStream

//#pragma mark (HAECaptureStream class)

HAECaptureStream::HAECaptureStream(HAECapture *pCaptureDevice, char *cName, void * userReference)
{
    mCapture = pCaptureDevice;
    mCallbackProc = NULL;
    mUserReference = userReference;
    mReference = 0;

    mName[0] = 0;
    if (cName)
	{
	    char	tempName[2048];
		
	    XStrCpy(tempName, (char *)cName);
	    if (XStrLen(tempName) > 63)
		{
		    tempName[63] = 0;	// anything larger than 64 characters is truncated
		}
	    XStrCpy(mName, tempName);
	}
}

HAECaptureStream::~HAECaptureStream()
{
}

// This will start a streaming capture audio object.
HAEErr HAECaptureStream::Start(void)
{
    if (mReference)
	{
	    return HAE_TranslateOPErr(GM_AudioCaptureStreamStart(mReference));
	}
    return HAE_NOT_SETUP;
}

void HAECaptureStream::Stop(void)
{
    if (mReference)
	{
	    GM_AudioCaptureStreamStop(NULL, mReference);
	}
}

void HAECaptureStream::SetCallbackProc(HAEStreamObjectProc callbackProc)
{
    mCallbackProc = callbackProc;
}

HAEErr HAECaptureStream::CreatePlaybackStream(HAESoundStream *pPlaybackStream, HAEStreamObjectProc pProc)
{
    pPlaybackStream;
    pProc;
    return HAE_NO_ERROR;
}

// This will setup a streaming capture audio object.
// INPUT:
//	pProc			is a HAEStreamObjectProc proc pointer. At startup of the streaming the proc will be called
//					with HAE_STREAM_CREATE, then followed by a HAE_STREAM_HAVE_DATA calls when data the capture
//					buffer is full of data and finally HAE_STREAM_DESTROY when finished.
//	bufferSize		total size of buffer to work with. This will not allocate memory, instead it will call
//					your control callback with a HAE_STREAM_CREATE with a size
HAEErr HAECaptureStream::SetupCustomStream(	HAEStreamObjectProc pProc, 	// control callback
						unsigned long bufferSize) 		// buffer size 
{
    HAEErr	theErr;
    OPErr	oErr;

    theErr = HAE_NO_ERROR;
    mCallbackProc = pProc;
    if (mCapture)
	{
	    if (pProc && bufferSize && mCapture->mStreamSampleInfo.sampledRate)
		{
		    mReference = GM_AudioCaptureStreamSetup(NULL,									// platform context
							    (long)this, 							// user reference
							    PV_CustomInputStreamCallback, 			// control callback
							    bufferSize, 							// buffer size 
							    mCapture->mStreamSampleInfo.sampledRate,			// Fixed 16.16
							    (char)mCapture->mStreamSampleInfo.bitSize,		// 8 or 16 bit data
							    (char)mCapture->mStreamSampleInfo.channels,		// 1 or 2 channels of date
							    &oErr);
		    theErr = HAE_TranslateOPErr(oErr);
		}
	    else
		{
		    theErr = HAE_PARAM_ERR;
		}
	}
    else
	{
	    theErr = HAE_NOT_SETUP;
	}
    return theErr;
}

#endif	// USE_CAPTURE_API

// EOF of HAECapture.cpp

