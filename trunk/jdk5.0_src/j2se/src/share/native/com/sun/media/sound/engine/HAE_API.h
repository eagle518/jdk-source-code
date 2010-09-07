/*
 * @(#)HAE_API.h	1.25 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**	HAE_API.h
**
**	This provides platform specfic functions for HAE
**
**	History	-
**	7/14/97		Created
**	7/22/97		Added HAE_GetAudioByteBufferSize
**	11/11/97	Added HAE_MaxDevices & HAE_SetDeviceID & HAE_GetDeviceID & HAE_GetDeviceName
**	12/16/97	Modified HAE_GetDeviceID and HAE_SetDeviceID to pass a device parameter pointer
**				that is specific for that device.
**	1/9/98		Added HAE_FileDelete
**	2/13/98		Changed HAE_AquireAudioCard to use unsigned longs
**	3/17/98		Added HAE_Is8BitSupported
**	6/28/98		Added Capture API
**	7/23/98		Added a new parameter to HAE_AquireAudioCapture
**				Added HAE_PauseAudioCapture & HAE_ResumeAudioCapture
**	7/31/98		Added HAE_GetCaptureDeviceName
**	8/13/98		Changed HAE_CaptureDone proc pointer to be void * parameters rather than
**				the direct objects.
**
** JAVASOFT
** 10.09.98		Changed HAE_StartAudioCapture parameters
** 10.13.98		added HAE_GetCaptureBufferSizeInFrames
** 10.14.98		added HAE_GetDeviceSamplesCapturedPosition (need to implement!)
** 10.15.98		added HAE_GetCaptureBufferCount
**
** 12.09.98		kk: added HAE_StartAudioOutput and HAE_StopAudioOutput to explicitly start
**				and stop the audio output thread.  changed HAE_AquireAudioCard to not
**				automatically stop the thread.
**
** 03.17.99		kk: removed HAE_StartAudioOutput and HAE_StopAudioOutput 'cause Steve
**				doesn't like 'em!
**
**	12/17/98	Added HAE_GetHardwareBalance & HAE_SetHardwareBalance
**	3/5/99		Changed context to threadContext to reflect what is really is used
**				in the system.
** 05.19.99     jb: added audioFramesPerBuffer parameter to HAE_AquireAudioCapture
*/
/*****************************************************************************/

#ifndef HAE_API
#define HAE_API

#include "X_API.h"

#ifdef __cplusplus
extern "C" {
#endif

    // This file contains API's that need to be defined in order to get HAE (IgorAudio)
    // to link and compile.


    // **** System setup and cleanup functions
    // Setup function. Called before memory allocation, or anything serious. Can be used to
    // load a DLL, library, etc.
    // return 0 for ok, or -1 for failure
    int HAE_Setup(void);

    // Cleanup function. Called after all memory and the last buffers are deallocated, and
    // audio hardware is released. Can be used to unload a DLL, library, etc.
    // return 0 for ok, or -1 for failure
    int HAE_Cleanup(void);

    // **** Memory management
    // allocate a block of locked, zeroed memory. Return a pointer
    void *HAE_Allocate(UINT32 size);

    // dispose of memory allocated with HAE_Allocate
    void HAE_Deallocate(void * memoryBlock);

    // Given a memory pointer and a size, validate of memory pointer is a valid memory address
    // with at least size bytes of data avaiable from the pointer.
    // This is used to determine if a memory pointer and size can be accessed without
    // causing a memory protection fault.
    // return 0 for valid, or 1 for bad pointer, or 2 for not supported.
    int HAE_IsBadReadPointer(void *memoryBlock, UINT32 size);

    // this will return the size of the memory pointer allocated with HAE_Allocate. Return
    // 0 if you don't support this feature
    UINT32 HAE_SizeOfPointer(void * memoryBlock);

    // block move memory. This is basicly memcpy, but its exposed to take advantage of
    // special block move speed ups various hardware might have available.
    void HAE_BlockMove(void * source, void * dest, UINT32 size);

    // **** Audio Card modifiers
    // Return 1 if stereo hardware is supported, otherwise 0.
    int HAE_IsStereoSupported(void);

    // Return 1, if sound hardware support 8 bit output, otherwise 0.
    int HAE_Is8BitSupported(void);

    // Return 1, if sound hardware support 16 bit output, otherwise 0.
    int HAE_Is16BitSupported(void);

    // returned volume is in the range of 0 to 256. If you're hardware doesn't support this
    // range, just scale it.
    short int HAE_GetHardwareVolume(void);

    // theVolume is in the range of 0 to 256. If you're hardware doesn't support this
    // range, just scale it.
    void HAE_SetHardwareVolume(short int theVolume);

    // returned balance is in the range of -256 to 256. Left to right. If you're hardware doesn't support this
    // range, just scale it.
    short int HAE_GetHardwareBalance(void);

    // 'balance' is in the range of -256 to 256. Left to right. If you're hardware doesn't support this
    // range, just scale it.
    void HAE_SetHardwareBalance(short int balance);

    // **** Timing services
    // return microseconds, preferably quantized better than 1000 microseconds, but can live
    // with it being as bad as 11000 microseconds.
    UINT32 HAE_Microseconds(void);

    // wait or sleep this thread for this many microseconds
    void HAE_WaitMicroseocnds(UINT32 wait);

    // **** File support
    // NOTE: about the fileName parameters. For "C" string path name based OS's the name
    // will be passed in from the C++ API. For MacOS, or structure based file systems, the
    // native structure will be passed in. For example: on a MacOS a FSSpec pointer is passed
    // from the C++ API to these various functions.

    // Given the fileNameSource that comes from the call HAE_FileXXXX, copy the name
    // in the native format to the pointer passed fileNameDest.
    void HAE_CopyFileNameNative(void *fileNameSource, void *fileNameDest);

    // Create a file, delete orignal if duplicate file name.
    // Return -1 if error
    INT32 HAE_FileCreate(void *fileName);

    // Delete a file. Returns -1 if there's an error, or 0 if ok.
    INT32 HAE_FileDelete(void *fileName);

    // Open a file
    // Return -1 if error, otherwise file handle
    XFILE_HANDLE HAE_FileOpenForRead(void *fileName);
    XFILE_HANDLE HAE_FileOpenForWrite(void *fileName);
    XFILE_HANDLE HAE_FileOpenForReadWrite(void *fileName);

    // Close a file
    void HAE_FileClose(XFILE_HANDLE fileReference);

    // Read a block of memory from a file
    // Return -1 if error, otherwise length of data read.
    INT32 HAE_ReadFile(XFILE_HANDLE fileReference, void *pBuffer, INT32 bufferLength);

    // Write a block of memory from a file
    // Return -1 if error, otherwise length of data written.
    INT32 HAE_WriteFile(XFILE_HANDLE fileReference, void *pBuffer, INT32 bufferLength);

    // set file position in absolute file byte position
    INT32 HAE_SetFilePosition(XFILE_HANDLE fileReference, UINT32 filePosition);

    // get file position in absolute file bytes
    UINT32 HAE_GetFilePosition(XFILE_HANDLE fileReference);

    // get length of file
    UINT32 HAE_GetFileLength(XFILE_HANDLE fileReference);

    // set the length of a file. Return 0, if ok, or -1 for error
    int HAE_SetFileLength(XFILE_HANDLE fileReference, UINT32 newSize);

    // **** Audio card support
    // Aquire and enabled audio card. sampleRate is 44100, 22050, or 11025; channels is 1 or 2;
    // bits is 8 or 16.
    // return 0 if ok, -1 if failed
    int HAE_AquireAudioCard(void *threadContext, UINT32 sampleRate, UINT32 channels, UINT32 bits);

    // Release and free audio card.
    // return 0 if ok, -1 if failed.
    int HAE_ReleaseAudioCard(void *threadContext);

    UINT32 HAE_GetDeviceSamplesPlayedPosition(void);

    // number of devices. ie different versions of the HAE connection. DirectSound and waveOut
    // return number of devices. ie 1 is one device, 2 is two devices.
    // NOTE: This function needs to function before any other calls may have happened.
    INT32 HAE_MaxDevices(void);

    // set the current device. device is from 0 to HAE_MaxDevices()
    // NOTE:	This function needs to function before any other calls may have happened.
    //			Also you will need to call HAE_ReleaseAudioCard then HAE_AquireAudioCard
    //			in order for the change to take place. deviceParameter is a device specific
    //			pointer. Pass NULL if you don't know what to use.
    void HAE_SetDeviceID(INT32 deviceID, void *deviceParameter);

    // return current device ID, and fills in the deviceParameter with a device specific
    // pointer. It will pass NULL if there is nothing to use.
    // NOTE: This function needs to function before any other calls may have happened.
    INT32 HAE_GetDeviceID(void *deviceParameter);

    // get deviceID name
    // NOTE:	This function needs to function before any other calls may have happened.
    //			Format of string is a zero terminated comma delinated C string.
    //			"platform,method,misc"
    //	example	"MacOS,Sound Manager 3.0,SndPlayDoubleBuffer"
    //			"WinOS,DirectSound,multi threaded"
    //			"WinOS,waveOut,multi threaded"
    //			"WinOS,VxD,low level hardware"
    //			"WinOS,plugin,Director"
    void HAE_GetDeviceName(INT32 deviceID, char *cName, UINT32 cNameLength);

    // Return the number of microseconds of real time that will be generated when calling
    // HAE_BuildMixerSlice.
    UINT32 HAE_GetSliceTimeInMicroseconds(void);

    // Return the number of buffer blocks that are built at one time. The value returned
    // from this function and HAE_GetSliceTimeInMicroseconds will give you the amount
    // of lantancy
    int HAE_GetAudioBufferCount(void);

    // Return the number of bytes used for audio buffer for output to card
    INT32 HAE_GetAudioByteBufferSize(void);

    // **** Audio Engine feedback functions. These functions are used to direct or get
    //		information about the engine.
    //
    // NOTE:	These are external functions that your audio card code calls to process
    //			a mixer buffer. You call this, don't define it.

    // Based upon sample rate, channels, and bit size, build 11 milliseconds worth of mixer
    // output, and store it in pAudioBuffer.
    // bufferByteLength is assumed to be 256, 512, 1024, or 2048 only. The value is ignored
    // at the moment. sampleFrames is how many sample frames. These two values should match
    // ie. sampleFrames = bufferByteLength / channels / bitsize / 8
    extern void HAE_BuildMixerSlice(void *threadContext, void *pAudioBuffer,
				    INT32 bufferByteLength,
				    INT32 sampleFrames);

    // Return the maximumn number of samples for 11 milliseconds worth of 44 khz data.
    // Typically this is 512. Use this in your calculation of audio buffers
    extern short HAE_GetMaxSamplePerSlice(void);


    //CLS:  THREADING API:

    // the type of function called by the frame thread
    // "threadContext" is a platform-specific value or (#if JAVA_SOUND) a JNIEnv* pointer
    typedef void (*HAE_FrameThreadProc)(void* threadContext);

    // Create and start the frame thread
    extern int	HAE_CreateFrameThread(void* threadContext, HAE_FrameThreadProc proc);

    // Stop and destroy the frame thread
    extern int	HAE_DestroyFrameThread(void* threadContext);

    // Make the frame thread sleep for the given number of milliseconds
    extern int	HAE_SleepFrameThread(void* threadContext, INT32 msec);

    // $$kk: 12.09.98: added this method
    //extern int HAE_StartAudioOutput(void *context);

    // $$kk: 12.09.98: added this method
    //extern void HAE_StopAudioOutput(void *context);

    // CAPTURE API

    // $$kk: 10.27.99
    // encoding defines (should use the defines from X_Formats.h ??)

#define		PCM		0
#define		ULAW	1
#define		ALAW	2

    // returns the maximum number of capture formats supported.
    int HAE_MaxCaptureFormats(INT32 deviceID);

    // returns the number of capture formats filled in.  the values go into the parameters passed.
    int HAE_GetSupportedCaptureFormats(INT32 deviceID, UINT32 *encodings, UINT32 *sampleRates, UINT32 *channels, UINT32 *bits, int maxFormats);


    // Aquire and capture audio. sampleRate is 48000 to 2000. Will fail if device doesn't support it
    // bits is 8 or 16; channels is 1 or 2;
    // *pCaptureHandle will contain a device dependent ID for the capture device.
    // pCaptureHandle can be NULL.
    // return 0 if ok, -1 if failed
    // $$fb 2002-02-01: itanium port
    int HAE_AquireAudioCapture(void *threadContext, UINT32 encoding, UINT32 sampleRate, UINT32 channels, UINT32 bits,
			       UINT32 audioFramesPerBuffer, UINT_PTR *pCaptureHandle);

    // Release and free audio capture device
    // return 0 if ok, -1 if failed.
    int HAE_ReleaseAudioCapture(void *threadContext);

    typedef enum
    {
	OPEN_CAPTURE = 0,
	CLOSE_CAPTURE,
	DATA_READY_CAPTURE
    } HAECaptureMessage;

    // Called when HAE_StartAudioCapture is called.
    // 	OPEN_CAPTURE
    //		parmeter1 and parmeter2 are NULL.

    // Called when HAE_StopAudioCapture is called.
    //	CLOSE_CAPTURE
    //		parmeter1 and parmeter2 are NULL.

    // Called when the audio buffer has been filled by the capture audio subsystem.
    //	DATA_READY_CAPTURE
    //		parmeter1 is a pointer to an char * which is the pointer to the audio block
    //		parmeter2 is a pointer to an unsigned long which is the length of buffer

    typedef void (*HAE_CaptureDone)(void *callbackContext, HAECaptureMessage message, void *parmeter1, void *parameter2);

    // Given the capture hardware is working, fill a buffer with some data. This call is
    // asynchronous. When this buffer is filled, the function done will be called.
    // $$kk: 10.09.98: changed this
    // $$kk: 10.15.98: should go back and make buffer size configurable later!
    //int HAE_StartAudioCapture(void *buffer, unsigned long bufferSize, HAE_CaptureDone done, void *callbackContext);
    int HAE_StartAudioCapture(HAE_CaptureDone done, void *callbackContext);

    // stop the capture hardware
    int HAE_StopAudioCapture(void* threadContext);

    // Pause and resume the audio capture. You might lose a buffer doing this.
    int HAE_PauseAudioCapture(void);
    int HAE_ResumeAudioCapture(void);

    // number of capture devices
    // return number of devices. ie 1 is one device, 2 is two devices.
    // NOTE: This function needs to function before any other calls may have happened.
    INT32 HAE_MaxCaptureDevices(void);

    // set the current device. device is from 0 to HAE_MaxCaptureDevices()
    // NOTE:	This function needs to function before any other calls may have happened.
    //			Also you will need to call HAE_ReleaseAudioCapture then HAE_AquireAudioCapture
    //			in order for the change to take place. deviceParameter is a device specific
    //			pointer. Pass NULL if you don't know what to use.
    void HAE_SetCaptureDeviceID(INT32 deviceID, void *deviceParameter);

    // return current device ID, and fills in the deviceParameter with a device specific
    // pointer. It will pass NULL if there is nothing to use.
    // NOTE: This function needs to function before any other calls may have happened.
    INT32 HAE_GetCaptureDeviceID(void *deviceParameter);

    // get deviceID name
    // NOTE:	This function needs to function before any other calls may have happened.
    //			Format of string is a zero terminated comma delinated C string.
    //			"platform,method,misc"
    //	example	"MacOS,Sound Manager 3.0,SndPlayDoubleBuffer"
    //			"WinOS,DirectSound,multi threaded"
    //			"WinOS,waveIn,multi threaded"
    //			"WinOS,VxD,low level hardware"
    //			"WinOS,plugin,Director"
    void HAE_GetCaptureDeviceName(INT32 deviceID, char *cName, UINT32 cNameLength);

    // return the number of frames in the capture buffer
    // $$kk: 10.13.98: added this
    UINT32 HAE_GetCaptureBufferSizeInFrames();

    // return the number of buffers used in capture.  the capture latency can be
    // determined from this and HAE_GetCaptureBufferSizeInFrames.
    // $$kk: 10.15.98: added this
    int HAE_GetCaptureBufferCount();

    // Get the count of samples captured at the capture device
    // $$kk: 10.14.98: added this; need to implement!
    UINT32 HAE_GetDeviceSamplesCapturedPosition();

    void HAE_FlushAudioCapture();


#ifdef __cplusplus
}
#endif

#endif	// HAE_API

