/*
 * @(#)HAECapture.h	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*****************************************************************************/
/*
** "HAECapture.h"
**
**	Generalized Audio Synthesis package presented in an oop fashion
**		Capature Audio API in a platform independent fashion
**
** Modification History:
**
**	6/4/98		Created
**	8/11/98		Renamed HAECaptureAudio to HAECapture. Added HAECaptureStream.
*/
/*****************************************************************************/

#ifndef HAE_CAPTURE
#define HAE_CAPTURE

#ifndef HAE_AUDIO
#include "HAE.h"
#endif

// HAE Capture Audio device
class HAECapture
{
    public:
    friend class HAECaptureStream;

    HAECapture();
    virtual				~HAECapture();

    // open the capture.
    HAEErr				Open(	HAE_UNSIGNED_FIXED captureSampleRate = LONG_TO_UNSIGNED_FIXED(22050),	// Fixed 16.16
						char dataBitSize = 16,													// 8 or 16 bit data
						char channelSize = 1);												// 1 or 2 channels of date

    // close the capture
    HAEErr				Close(void);

    void				*GetPrivateData(void) const	{return mData;}

    // number of devices. ie different versions of the HAE connection. DirectSound and waveOut
    // return number of devices. ie 1 is one device, 2 is two devices.
    // NOTE: This function can be called before Open is called
    long 			GetMaxDeviceCount(void);

    // set current device. should be a number from 0 to HAEAudioMixer::GetMaxDeviceCount()
    // deviceParameter is a pointer to device specific info. It will
    // be NULL if there's nothing interesting. For Windows Device 1 (DirectSound) it
    // is the window handle that DirectSound will attach to. Pass NULL, if you don't know
    // what is correct.
    void 			SetCurrentDevice(long deviceID, void *deviceParameter = NULL);

    // get current device. deviceParameter is a pointer to device specific info. It will
    // be NULL if there's nothing interesting. For Windows Device 1 (DirectSound) it
    // is the window handle that DirectSound will attach to.
    long 			GetCurrentDevice(void *deviceParameter = NULL);

    // get device name
    // NOTE:	This function can be called before Open()
    //			Format of string is a zero terminated comma delinated C string.
    //			"platform,method,misc"
    //	example	"MacOS,Sound Manager 3.0,SndPlayDoubleBuffer"
    //			"WinOS,DirectSound,multi threaded"
    //			"WinOS,waveOut,multi threaded"
    //			"WinOS,VxD,low level hardware"
    //			"WinOS,plugin,Director"
    void			GetDeviceName(long deviceID, char *cName, unsigned long cNameLength);
	
    // Will return TRUE if the HAECapture is already open
    HAE_BOOL		IsOpen(void) const;

    // get information about the current stream
    HAEErr			GetInfo(HAESampleInfo *pInfo);

    // Returns TRUE if valid
    HAE_BOOL		IsValid(void);

    // Get the sample rate of a capture stream
    HAE_FIXED		GetRate(void);

    // is the mixer connected to the audio hardware
    HAE_BOOL		IsCaptureEngaged(void);

    // disengage from audio hardware
    HAEErr			DisengageAudio(void);

    // reengage to audio hardware
    HAEErr			ReengageAudio(void);


    private:
	HAESampleInfo			mStreamSampleInfo;
    HAE_BOOL				mCaptureAudioEngaged;
    long					mReference;
    HAEStreamObjectProc		mCallbackProc;
    void					*mData;
    HAESoundStream			*mPlaybackStream;
};



// HAECaptureStream class
class HAECaptureStream
{
    friend class HAECapture;
    public:
	HAECaptureStream(HAECapture *pCaptureDevice, char *cName = 0L, void * userReference = 0L);
    virtual	~HAECaptureStream();

    void				*GetReference(void)	const	{return mUserReference;}
    HAECapture			*GetCapture(void) 			{return mCapture;}

    // This will start a streaming capture audio object.
    // INPUT:
    //	pProc			is a HAEStreamObjectProc proc pointer. At startup of the streaming the proc will be called
    //					with HAE_STREAM_CREATE, then followed by a HAE_STREAM_HAVE_DATA calls when data the capture
    //					buffer is full of data and finally HAE_STREAM_DESTROY when finished.
    //	bufferSize		total size of buffer to work with. This will not allocate memory, instead it will call
    //					your control callback with a HAE_STREAM_CREATE with a size
    HAEErr				SetupCustomStream(	HAEStreamObjectProc pProc, 	// control callback
								unsigned long bufferSize);		// buffer size 

    // This will start capturing audio
    HAEErr				Start(void);

    // This will stop a streaming capture audio object and free any memory.
    void				Stop(void);

    // if you want to send the captured audio to a stream for processing and playback
    // use this method with a callback function. Create a new HAESoundStream with new,
    // pass it into this method with a callback function. You're call back will be called
    // when you use Start to start capture.
    //
    HAEErr				CreatePlaybackStream(HAESoundStream *pPlaybackStream, HAEStreamObjectProc pProc);

    void				SetCallbackProc(HAEStreamObjectProc callbackProc);
    HAEStreamObjectProc GetCallbackProc(void) const {return mCallbackProc;}

	
    private:
	long					mReference;
    char					mName[64];
    void					*mUserReference;
    HAECapture				*mCapture;
    HAEStreamObjectProc		mCallbackProc;
};








#endif	// HAE_CAPTURE

