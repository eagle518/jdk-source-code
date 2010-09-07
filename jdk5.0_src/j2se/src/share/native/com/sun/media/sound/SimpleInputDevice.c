/*
 * @(#)SimpleInputDevice.c	1.35 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


// STANDARD includes


// JNI includes
#include <jni.h>

// ENGINE includes
#include "engine/X_API.h"
#include "engine/GenSnd.h"
#include "engine/GenPriv.h"		// $$kk: 10.02.98: need for references to MusicGlobals
#include "engine/HAE_API.h"		// $$kk: 10.02.98: need for HAE_GetMaxSamplePerSlice(); should get rid of this??


// UTILITY includes
#include "Utilities.h"

// SimpleInputDevice includes
#include "com_sun_media_sound_SimpleInputDevice.h"

#define MAX_BUFFERS                 2  // double buffer
#define NO_DEVICE					-1

// this our one and only GM_AudioCaptureStream for now;
// obviously this is *bad* and needs to change....
void *reference = NULL;

// $$kk: 12.11.98: true if device acquired.  probably should put a method 
// in the capture api for this.
//XBOOL acquired = FALSE;

// $$kk: 06.03.99: this is the index of the currently open device.
// if -1, no device is acquired.  because of how the headspace code
// handles capture, we can only capture from one device at a time.
// if a device is open for capture, this variable reflects the index
// of the device
int currentInputDeviceIndex = NO_DEVICE;

// format
UINT32 g_sampleRate;
UINT32 g_channels;
UINT32 g_sampleSizeInBits;
UINT32 g_audioFramesPerBuffer;

UINT32 g_encoding;
	
UINT32	bufferSizeInBytes;

// $$kk: 10.13.98: this stuff needs to change!
jclass globalCaptureDeviceClass				= NULL;
jmethodID callbackPutDataMethodID			= NULL;
jmethodID callbackDestroyMethodID			= NULL;

jbyteArray globalArray						= NULL;	

XBOOL callbacksOk							= FALSE;
int currentCallbacks						= 0;


// UTILITY FUNCTIONS

// create and set up global java array for calling back with captured data.
// this needs to change!  we are not supporting multiple native capture
// streams here....
// returns 0 for success, -1 for failure
int CreateGlobalArray(void *context, GM_StreamData *pStreamData) 
{
    JNIEnv* e = (JNIEnv*)context;
    jobject globalCaptureDeviceObj = (jobject)pStreamData->userReference;

    jbyteArray localArray;
    jclass localClass;

    INT32 arrayLength = pStreamData->dataLength * pStreamData->channelSize * (pStreamData->dataBitSize / 8);

    TRACE2("CreateGlobalArray, pStreamData->dataLength: %d, arrayLength: %d.\n", pStreamData->dataLength, arrayLength);

    // save a global ref to the java capture device object *class*.
    // need this to keep the class from getting unloaded and invalidating the method id
    localClass = (*e)->GetObjectClass(e, globalCaptureDeviceObj);
    if (localClass == NULL)
	{
	    ERROR0("SimpleInputDevice: CreateGlobalArray: localClass is NULL\n");
	    return -1;
	}

    globalCaptureDeviceClass = (*e)->NewGlobalRef(e, localClass);	
    if (globalCaptureDeviceClass == NULL)
	{
	    ERROR0("SimpleInputDevice: CreateGlobalArray: globalCaptureDeviceClass is NULL\n");
	    return -1;
	}

    // save the put-data callback method id.  this is valid as long as the class is not unloaded.
    callbackPutDataMethodID = (*e)->GetMethodID(e, globalCaptureDeviceClass, "callbackStreamPutData", "([BI)V");
    if (callbackPutDataMethodID == 0)
	{
	    ERROR0("SimpleInputDevice: CreateGlobalArray: callbackPutDataMethodID is 0\n");
	    return -1;
	}

    // save the stream destroy callback method id.  this is valid as long as the class is not unloaded.
    callbackDestroyMethodID = (*e)->GetMethodID(e, globalCaptureDeviceClass, "callbackCaptureStreamDestroy", "()V");
    if (callbackDestroyMethodID == 0)
	{
	    ERROR0("SimpleInputDevice: CreateGlobalArray: callbackDestroyMethodID is 0\n");
	    return -1;
	}

    // create the global array
    localArray = (*e)->NewByteArray(e, (jsize)arrayLength);
    if (localArray == NULL)
	{
	    ERROR0("SimpleInputDevice: CreateGlobalArray: localArray is NULL\n");
	    return -1;
	}

    globalArray = (*e)->NewGlobalRef(e, localArray);
    if (globalArray == NULL)
	{
	    ERROR0("SimpleInputDevice: CreateGlobalArray: globalArray is NULL\n");
	    return -1;
	}

    return 0;
}


// returns 0 for success, -1 for failure
int DestroyGlobalArray(void *context)
{
    JNIEnv* e = (JNIEnv*)context;

    // delete the global refs and the output proc array
    (*e)->DeleteGlobalRef(e, globalCaptureDeviceClass);
    (*e)->DeleteGlobalRef(e, globalArray);
    globalCaptureDeviceClass = NULL;
    callbackPutDataMethodID = NULL; 
    callbackDestroyMethodID = NULL;

    // $$kk: 10.02.98: how to delete this??
    globalArray = NULL;

    return 0;
}

// CALLBACKS INTO JAVA

// these should be changed to operate per-stream rather than per-device

static OPErr CallToJavaCaptureStreamDestroy(JNIEnv* e, jobject captureDevice)
{
    OPErr opErr = NO_ERR;

    TRACE1("CallToJavaCaptureStreamDestroy, captureDevice: %lx.\n", captureDevice);
	
    currentCallbacks++;
    if (!callbacksOk)
	{
	    ERROR0("CallToJavaCaptureStreamDestroy: CALLBACK DENIED\n");
	    opErr = GENERAL_BAD;
	}
    else
	{
	    // $$kk: 08.05.99: need to change this so we save a *global* ref to the
	    // java object in the engine struct, *not* a local one!!!!  then this
	    // will be ok....
	    (*e)->CallVoidMethod(e, captureDevice, callbackDestroyMethodID);
	}
    currentCallbacks--;

    TRACE1("CallToJavaCaptureStreamDestroy, captureDevice: %lx completed.\n", captureDevice);
    return opErr;
}


static OPErr CallToJavaStreamHaveData(JNIEnv* e, GM_StreamData *pStreamData) 
{
    OPErr opErr = NO_ERR;
    jobject globalCaptureDeviceObj = (jobject)pStreamData->userReference;


    VTRACE0(">> SimpleInputDevice: CallToJavaStreamHaveData.\n");

    currentCallbacks++;

    if (!callbacksOk)
	{
	    ERROR0("CallToJavaStreamHaveData: CALLBACK DENIED\n");
	    opErr = GENERAL_BAD;
	}
    else
	{
	
	    if (pStreamData != NULL)
		{
		    void *pData = pStreamData->pData;
		    int lengthInFrames = (int)pStreamData->dataLength;
		    int channels = (int)pStreamData->channelSize;
		    int sampleSize = (int)pStreamData->dataBitSize / 8;
		    int arrayLength = lengthInFrames * channels * sampleSize;

			
		    // copy data from stream data buffer into java array
		    // $$kk: 10.13.98: java array is currently global; should be per-stream!
			
		    // $$kk: 10.02.98: i think that it should not be possible to overstep the array here with the engine operating as it
		    // does currently... right??	 	 

		    VTRACE3("calling SetByteArrayRegion with globalArray: %d, arrayLength: %d, pData: %d\n", globalArray, arrayLength, pData);
		    (*e)->SetByteArrayRegion(e, globalArray, (jsize)0, (jsize)arrayLength, (jbyte *)pData); 		

		    // callback to java with captured data		
		    (*e)->CallVoidMethod(e, globalCaptureDeviceObj, callbackPutDataMethodID, globalArray, (jint)lengthInFrames);
		} 
	    else
		{
		    opErr = GENERAL_BAD;
		}
	}

    currentCallbacks--;

    VTRACE0("<< SimpleInputDevice: CallToJavaStreamHaveData completed.\n");
    return opErr;
}


// Pushes captured data up to Java.
static OPErr AudioInputCallbackProc(void* context, GM_StreamMessage message, GM_StreamData *pAS)
{
    JNIEnv* e = (JNIEnv*)context;
    OPErr theErr = NO_ERR;	
    /* jobject globalCaptureDeviceObj = (jobject)pAS->userReference; */

    TRACE1("SimpleInputDevice: AudioInputCallbackProc, context: %lx.\n", context);

    // for STREAM_CREATE, we just need to allocate the buffer.
    // we need to convert from gmData->dataLength (number of frames) to number of bytes.
    // $$kk: 03.15.98: this is the same calculation done by PV_GetSampleSizeInBytes.  however, this
    // method is not available outside GenAudioStreams.c right now.

    if (message == STREAM_CREATE)
	{	
	    // allocating memory for both java and c here; this is bad!

	    TRACE1("STREAM_CREATE: %lx\n", pAS->userReference);

	    // allocate java byte array
	    if ( CreateGlobalArray(context, pAS) != 0 )
		{
		    ERROR0("AudioInputCallbackProc: STREAM_CREATE: Failed to create global array\n");
		    return MEMORY_ERR;
		}

	    /*
	      // allocate c array
	      pAS->pData = XNewPtr(arrayLength);
	      if (!pAS->pData)
	      {
	      ERROR1("AudioInputCallbackProc: STREAM_CREATE: Failed to allocate %d bytes\n", arrayLength);
	      return MEMORY_ERR;
	      }
	    */	
	    TRACE1("STREAM_CREATE: %lx completed\n", pAS->userReference);
	}

    else if (message == STREAM_DESTROY)
	{
	    // store the global reference to the capture device
	    // $$kk: 08.25.99: (note: this should happen either in nOpen and 
	    // nClose or int STREAM_CREATE and STREAM_DESTROY, but should not 
	    // be asymmetric like this!
	    jobject globalCaptureDeviceObj = (jobject)pAS->userReference;
	
	    TRACE1("STREAM_DESTROY: %lx\n", pAS->userReference);

	    if (pAS->pData)
		{
		    XDisposePtr(pAS->pData);
		}

	    CallToJavaCaptureStreamDestroy(e, (jobject)pAS->userReference);

	    // release java byte array
	    DestroyGlobalArray(context);

	    // delete the global reference to the capture device.
	    // $$kk: 08.25.99: (note: this should happen either in nOpen and 
	    // nClose or int STREAM_CREATE and STREAM_DESTROY, but should not 
	    // be asymmetric like this!
	    if (globalCaptureDeviceObj != NULL)
		{
		    (*e)->DeleteGlobalRef(e, globalCaptureDeviceObj);
		    globalCaptureDeviceObj = NULL;
		}
		
	    TRACE1("STREAM_DESTROY: %lx completed\n", pAS->userReference);
	}

    else if (message == STREAM_HAVE_DATA)
	{
	    VTRACE1("STREAM_HAVE_DATA: %lx\n", pAS->userReference);
									
	    CallToJavaStreamHaveData(e, pAS);

	    VTRACE1("STREAM_HAVE_DATA: %lx completed\n", pAS->userReference);
	}

    else 
	{
	    ERROR1("AudioInputCallbackProc: unknown message: %d.\n", message);
	}

    VTRACE0("AudioInputCallbackProc completed.\n");
    return theErr;
}
								 

// NATIVE METHODS


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_SimpleInputDevice_nGetFormats(JNIEnv* e, jobject thisObj, jint index, jobject formats, jobject pcm_signed, jobject pcm_unsigned, jobject ulaw, jobject alaw)
{
    int i = 0;						// current index as we iterate through the set of formats

    // variables for vector operations
    jclass vectorClass = NULL;
    jmethodID addElementMethodID = NULL;

    // variables for format support queries
    jclass formatClass = NULL;
    jmethodID initMethodID = NULL;
    jobject newFormatObject = NULL;
	
    int maxFormats = 0;
	
    UINT32* encodings = NULL;
    UINT32* sampleRates = NULL;
    UINT32* channels = NULL;
    UINT32* bits = NULL;

    TRACE0("Java_com_sun_media_sound_SimpleInputDevice_nGetFormats\n");

    // this sets maxFormats to the maximum number of formats that may be supported
    maxFormats = HAE_MaxCaptureFormats(index);
    encodings = (UINT32*) malloc(sizeof(UINT32) * maxFormats);
    sampleRates = (UINT32*) malloc(sizeof(UINT32) * maxFormats);
    channels = (UINT32*) malloc(sizeof(UINT32) * maxFormats);
    bits = (UINT32*) malloc(sizeof(UINT32) * maxFormats);
    
    if (encodings!=NULL && sampleRates!=NULL && channels!=NULL && bits!=NULL) {
	// get the set of supported capture formats.
	// the new value of maxFormats will be the number of capture formats actually supported
	maxFormats = HAE_GetSupportedCaptureFormats(index, encodings, sampleRates, channels, bits, maxFormats);
    }
    if (maxFormats > 0) {
	// get the vector stuff set up
	vectorClass = (*e)->GetObjectClass(e, formats);

	if (vectorClass == NULL) {
	    ERROR0("Java_com_sun_media_sound_SimpleInputDevice_nGetFormats: vectorClass is NULL\n");
	} else {
	    addElementMethodID = (*e)->GetMethodID(e, vectorClass, "addElement", "(Ljava/lang/Object;)V");

	    if (addElementMethodID == NULL) {
		ERROR0("Java_com_sun_media_sound_SimpleInputDevice_nGetFormats: addElementMethodID is NULL\n");
	    }
	    // get the AudioFormat class, init method id, etc.
	    formatClass = (*e)->FindClass(e, "javax/sound/sampled/AudioFormat");
	    if (formatClass == NULL) {
		ERROR0("Java_com_sun_media_sound_SimpleInputDevice_nGetFormats: formatClass is NULL\n");
	    } else {
		initMethodID = (*e)->GetMethodID(e, formatClass, "<init>", "(Ljavax/sound/sampled/AudioFormat$Encoding;FIIIFZ)V");
		if (initMethodID == NULL) {
		    ERROR0("Java_com_sun_media_sound_SimpleInputDevice_nGetFormats: initMethodID is NULL\n");
		}
	    }
	}
    }
    if (initMethodID != NULL) {

	// create the AudioFormat objects
	for (i = 0; i < maxFormats; i++) {
	    switch(encodings[i]) {
		case PCM:

		    if (bits[i] == 8) {
			    // do signed and unsigned encodings
			    newFormatObject = (*e)->NewObject(e, formatClass, initMethodID, pcm_signed, (jfloat)sampleRates[i], (jint)bits[i], (jint)channels[i], (jint)(channels[i] * (bits[i] / 8)), (jfloat)sampleRates[i], (jboolean)FALSE);
			    if (newFormatObject != NULL) {
				    // add it to the vector
				    (*e)->CallVoidMethod(e, formats, addElementMethodID, newFormatObject);
				}

			    newFormatObject = (*e)->NewObject(e, formatClass, initMethodID, pcm_unsigned, (jfloat)sampleRates[i], (jint)bits[i], (jint)channels[i], (jint)(channels[i] * (bits[i] / 8)), (jfloat)sampleRates[i], (jboolean)FALSE);
			    if (newFormatObject != NULL) {
				    // add it to the vector
				    (*e)->CallVoidMethod(e, formats, addElementMethodID, newFormatObject);
				}
			}
		    else if (bits[i] > 8) {
				// do big and little endian
			    newFormatObject = (*e)->NewObject(e, formatClass, initMethodID, pcm_signed, (jfloat)sampleRates[i], (jint)bits[i], (jint)channels[i], (jint)(channels[i] * (bits[i] / 8)), (jfloat)sampleRates[i], (jboolean)TRUE);
			    if (newFormatObject != NULL) {
				    // add it to the vector
				    (*e)->CallVoidMethod(e, formats, addElementMethodID, newFormatObject);
				}

			    newFormatObject = (*e)->NewObject(e, formatClass, initMethodID, pcm_signed, (jfloat)sampleRates[i], (jint)bits[i], (jint)channels[i], (jint)(channels[i] * (bits[i] / 8)), (jfloat)sampleRates[i], (jboolean)FALSE);
			    if (newFormatObject != NULL) {
				    // add it to the vector
				    (*e)->CallVoidMethod(e, formats, addElementMethodID, newFormatObject);
				}
			}

		    break;

		case ULAW:

		    if (bits[i] <= 8) {
			    newFormatObject = (*e)->NewObject(e, formatClass, initMethodID, ulaw, (jfloat)sampleRates[i], (jint)bits[i], (jint)channels[i], (jint)(channels[i] * (bits[i] / 8)), (jfloat)sampleRates[i], (jboolean)FALSE);
			    if (newFormatObject != NULL) {
				    // add it to the vector
				    (*e)->CallVoidMethod(e, formats, addElementMethodID, newFormatObject);
				}
			} else {
			    // do big and little endian
			    newFormatObject = (*e)->NewObject(e, formatClass, initMethodID, ulaw, (jfloat)sampleRates[i], (jint)bits[i], (jint)channels[i], (jint)(channels[i] * (bits[i] / 8)), (jfloat)sampleRates[i], (jboolean)TRUE);
			    if (newFormatObject != NULL) {
				    // add it to the vector
				    (*e)->CallVoidMethod(e, formats, addElementMethodID, newFormatObject);
				}

			    newFormatObject = (*e)->NewObject(e, formatClass, initMethodID, ulaw, (jfloat)sampleRates[i], (jint)bits[i], (jint)channels[i], (jint)(channels[i] * (bits[i] / 8)), (jfloat)sampleRates[i], (jboolean)FALSE);
			    if (newFormatObject != NULL) {
				    // add it to the vector
				    (*e)->CallVoidMethod(e, formats, addElementMethodID, newFormatObject);
				}
			}

		    break;

		case ALAW:

		    if (bits[i] <= 8) {
			    newFormatObject = (*e)->NewObject(e, formatClass, initMethodID, alaw, (jfloat)sampleRates[i], (jint)bits[i], (jint)channels[i], (jint)(channels[i] * (bits[i] / 8)), (jfloat)sampleRates[i], (jboolean)FALSE);
			    if (newFormatObject != NULL) {
				    // add it to the vector
				    (*e)->CallVoidMethod(e, formats, addElementMethodID, newFormatObject);
				}
			} else {
			    // do big and little endian
			    newFormatObject = (*e)->NewObject(e, formatClass, initMethodID, alaw, (jfloat)sampleRates[i], (jint)bits[i], (jint)channels[i], (jint)(channels[i] * (bits[i] / 8)), (jfloat)sampleRates[i], (jboolean)TRUE);
			    if (newFormatObject != NULL) {
				    // add it to the vector
				    (*e)->CallVoidMethod(e, formats, addElementMethodID, newFormatObject);
				}

			    newFormatObject = (*e)->NewObject(e, formatClass, initMethodID, alaw, (jfloat)sampleRates[i], (jint)bits[i], (jint)channels[i], (jint)(channels[i] * (bits[i] / 8)), (jfloat)sampleRates[i], (jboolean)FALSE);
			    if (newFormatObject != NULL) {
				    // add it to the vector
				    (*e)->CallVoidMethod(e, formats, addElementMethodID, newFormatObject);
				}
			}

		    break;

		default:

		    ERROR1("Java_com_sun_media_sound_SimpleInputDevice_nGetFormats: Unknown encoding %d.\n", (int) encodings[i]);
		    break;

		} // end switch
	} // end for
    }
    if (encodings != NULL) free(encodings);
    if (sampleRates != NULL) free(sampleRates);
    if (channels != NULL) free(channels);
    if (bits != NULL) free(bits);

    TRACE0("Java_com_sun_media_sound_SimpleInputDevice_nGetFormats completed\n");
    return;
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_SimpleInputDevice_nOpen(JNIEnv* e, jobject thisObj, jint index, jint encoding, jfloat sampleRate, jint sampleSizeInBits, jint channels, jint bufferSize)
{
    OPErr           opErr;
    jobject			globalCaptureDeviceObj = NULL;
		
    TRACE0("Java_com_sun_media_sound_SimpleInputDevice_nOpen.\n");

    // $$kk: 04.13.99: right now this both opens the device and creates the 
    // one and only supported capture stream.

    // if this capture device is already open, just return
    if (index == currentInputDeviceIndex) 
	{
	    TRACE0("Java_com_sun_media_sound_SimpleInputDevice_nOpen: this device already open\n");
	    return;
	}

    // if another capture device is open, fail
    if ( (currentInputDeviceIndex != NO_DEVICE) || (reference != NULL) )
	{
	    char *msg = "Another capture device is already open\0";
	    ERROR0("Java_com_sun_media_sound_SimpleInputDevice_nOpen: another capture device is already open\n");
	    ThrowJavaMessageException(e, JAVA_SAMPLED_PACKAGE_NAME"/LineUnavailableException", msg);
	}

    // ok, try to open this capture device.  
    // first set the requested device to be the current one.  (this is not elegant!)

    HAE_SetCaptureDeviceID(index, NULL);

    g_encoding = (UINT32) encoding;
    g_sampleRate = (UINT32)sampleRate;
    g_channels = (UINT32)channels;
    g_sampleSizeInBits = (UINT32)sampleSizeInBits;
    g_audioFramesPerBuffer = (UINT32)bufferSize;

    // try to get the capture hardware with current configuration
    opErr = HAE_AquireAudioCapture((void *)e, 
				   g_encoding,
				   g_sampleRate, 
				   g_channels, 
				   g_sampleSizeInBits, 
				   g_audioFramesPerBuffer, NULL);

    // if we failed, throw an LineUnavailableException
    if (opErr != NO_ERR)
	{
	    ERROR0("HAE_AquireAudioCapture failed; throwing LineUnavailableException\n");
	    ThrowJavaOpErrException(e, JAVA_SAMPLED_PACKAGE_NAME"/LineUnavailableException", opErr);
	    return;
	}

    bufferSizeInBytes = HAE_GetCaptureBufferSizeInFrames() * g_channels * (g_sampleSizeInBits / 8);
    TRACE1("Java_com_sun_media_sound_SimpleInputDevice_nOpen: bufferSizeInBytes: %d\n", bufferSizeInBytes);

    // $$kk: 03.15.98: GLOBAL REF!  this has definite memory leak potential; need to make sure
    // to release all global refs....
    // $$kk: 08.06.99: need to initialize this here so it goes into the setup call
    globalCaptureDeviceObj = (*e)->NewGlobalRef(e, thisObj);

    reference = GM_AudioCaptureStreamSetup			((void *)e,											// platform context
								 (void *)globalCaptureDeviceObj, 						// user reference
								 (GM_StreamObjectProc)&AudioInputCallbackProc, 		// control callback
								 (UINT32)bufferSizeInBytes, 					// buffer size $$kk: 10.06.98: buffer size in *bytes*
								 (XFIXED)FLOAT_TO_FIXED(g_sampleRate),				// Fixed 16.16
								 (char)g_sampleSizeInBits,							// 8 or 16 bit data
								 (char)g_channels,									// 1 or 2 channels of date
								 &opErr);

    // if we failed, throw an LineUnavailableException
    if ( (opErr != NO_ERR) || (reference == NULL) )
	{
	    ERROR0("GM_AudioCaptureStreamSetup failed; throwing LineUnavailableException\n");
	    ThrowJavaOpErrException(e, JAVA_SAMPLED_PACKAGE_NAME"/LineUnavailableException", opErr);
	    return;
	}
	
    // it worked!  set the current device index to reflect the newly acquired device
    currentInputDeviceIndex = index;
	
    // $$kk: 06.13.99: ok to go!
    callbacksOk = TRUE;

    TRACE0("Java_com_sun_media_sound_SimpleInputDevice_nOpen succeeded\n");
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_SimpleInputDevice_nClose(JNIEnv* e, jobject thisObj)
{
    OPErr           opErr;

    jclass threadClass = NULL;
    jmethodID sleepMethodID = NULL;
    jlong msec = 10;	

    TRACE0("Java_com_sun_media_sound_SimpleInputDevice_nClose.\n");


    // $$kk: 06.13.99: *not* ok to go!
    callbacksOk = FALSE;

    if (currentCallbacks > 0) {

	threadClass = (*e)->FindClass(e, "java/lang/Thread");

	if (threadClass == NULL)
	    {
		ERROR0("Java_com_sun_media_sound_SimpleInputDevice_nClose: Failed to get thread class\n");
		return;
	    }
	
	sleepMethodID = (*e)->GetStaticMethodID(e, threadClass, "sleep", "(J)V");
	if (sleepMethodID == NULL)
	    {
		ERROR0("Java_com_sun_media_sound_SimpleInputDevice_nClose: Failed to get thread sleep method ID\n");
		return;
	    }
    }

    while (currentCallbacks > 0)
	{
	    /*  wait; should sleep or wait for notification! */
	    (*e)->CallStaticVoidMethod(e, threadClass, sleepMethodID, msec);
	}

	
    // $$kk: 04.13.99: right now this both destroys the one and only supported
    // capture stream, and releases the hardware.

    if (reference != NULL)
	{
	    // this calls GM_AudioCaptureStreamStop and then frees the stream
	    GM_AudioCaptureStreamCleanup((void*) e, reference);
	    reference = NULL;
	}
    else
	{
	    ERROR0("Java_com_sun_media_sound_SimpleInputDevice_nClose: reference is NULL\n");
	}

    opErr = HAE_ReleaseAudioCapture((void*) e);
    currentInputDeviceIndex = NO_DEVICE;

    // $$kk: 08.25.99: *need* to release the global ref to the device object here!!!!
    // but first i need to fix how it is stored: right now i don't have access to it....
    // i am deleting it in STREAM_DESTROY right now....

    if (opErr == NO_ERR)
	{
	    TRACE0("Java_com_sun_media_sound_SimpleInputDevice_nClose succeeded\n");
	}
    else
	{
	    ERROR1("Java_com_sun_media_sound_SimpleInputDevice_nClose: HAE_ReleaseAudioCapture returned an error: %d\n", opErr);
	}
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_SimpleInputDevice_nStart(JNIEnv* e, jobject thisObj)
{
    OPErr           opErr;

    TRACE0("Java_com_sun_media_sound_SimpleInputDevice_nStart.\n");

    // this will allocate the buffers and create and start the capture thread
    opErr = GM_AudioCaptureStreamStart((void *)e, reference);

    if (opErr != NO_ERR)
	{
	    TRACE0("Java_com_sun_media_sound_SimpleInputDevice_nStart failed\n");
	}
    else
	{
	    TRACE0("Java_com_sun_media_sound_SimpleInputDevice_nStart succeeded\n");
	}
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_SimpleInputDevice_nStop(JNIEnv* e, jobject thisObj)
{
    OPErr           opErr;

    TRACE0("Java_com_sun_media_sound_SimpleInputDevice_nStop.\n");

    // this will deallocate the buffers and stop the capture thread
    opErr = GM_AudioCaptureStreamStop((void *)e, reference);

    if (opErr == NO_ERR) 
	{
	    TRACE0("Java_com_sun_media_sound_SimpleInputDevice_nStop succeeded\n");
	}
    else
	{
	    ERROR1("Java_com_sun_media_sound_SimpleInputDevice_nStop: GM_AudioCaptureStreamStop returned an error: %d\n", opErr);
	}
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_SimpleInputDevice_nPause(JNIEnv* e, jobject thisObj)
{
    OPErr           opErr;

    TRACE0("Java_com_sun_media_sound_SimpleInputDevice_nPause.\n");

    opErr = HAE_PauseAudioCapture();

    if (opErr != NO_ERR)
	{
	    ERROR0("Java_com_sun_media_sound_SimpleInputDevice_nPause failed\n");
	}
    else
	{
	    TRACE0("Java_com_sun_media_sound_SimpleInputDevice_nPause succeeded\n");
	}
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_SimpleInputDevice_nResume(JNIEnv* e, jobject thisObj)
{
    OPErr           opErr;

    TRACE0("Java_com_sun_media_sound_SimpleInputDevice_nResume.\n");

    opErr = HAE_ResumeAudioCapture();

    if (opErr != NO_ERR)
	{
	    ERROR0("Java_com_sun_media_sound_SimpleInputDevice_nResume failed\n");
	}
    else
	{
	    TRACE0("Java_com_sun_media_sound_SimpleInputDevice_nResume succeeded\n");
	}
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_SimpleInputDevice_nDrain(JNIEnv* e, jobject thisObj)
{

    TRACE0("Java_com_sun_media_sound_SimpleInputDevice_nDrain.\n");

    ERROR0("DRAIN not implemented\n");

    TRACE0("Java_com_sun_media_sound_SimpleInputDevice_nDrain not implemented\n");
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_SimpleInputDevice_nFlush(JNIEnv* e, jobject thisObj)
{

    TRACE0("Java_com_sun_media_sound_SimpleInputDevice_nFlush.\n");

    if (reference == NULL)
	{
	    ERROR0("Java_com_sun_media_sound_SimpleInputDevice_nFlush: reference is NULL\n");
	    return;
	}
    else
	{
	    HAE_FlushAudioCapture();
	}

    TRACE0("Java_com_sun_media_sound_SimpleInputDevice_nFlush completed\n");
}


JNIEXPORT jlong JNICALL
    Java_com_sun_media_sound_SimpleInputDevice_nGetPosition(JNIEnv* e, jobject thisObj)
{
    UINT32 samplesCaptured;

    TRACE0("Java_com_sun_media_sound_SimpleInputDevice_nGetPosition.\n");

    if (reference == NULL)
	{
	    ERROR0("Java_com_sun_media_sound_SimpleInputDevice_nGetPosition: reference is NULL\n");
	    return (jlong)0;
	}
    else
	{
	    samplesCaptured = GM_AudioCaptureStreamGetSamplesCaptured(reference);
	    TRACE1("Java_com_sun_media_sound_SimpleInputDevice_nGetPosition returning %d\n", samplesCaptured);
	    return (jlong)samplesCaptured;
	}
}


JNIEXPORT jint JNICALL
    Java_com_sun_media_sound_SimpleInputDevice_nGetBufferSizeInFrames(JNIEnv* e, jobject thisObj)
{
    UINT32 bufferSize; 
	
    TRACE0("Java_com_sun_media_sound_SimpleInputDevice_nGetBufferSizeInFrames.\n");

    bufferSize = HAE_GetCaptureBufferSizeInFrames();

    TRACE1("Java_com_sun_media_sound_SimpleInputDevice_nGetBufferSizeInFrames returning: %d.\n", bufferSize);

    return (jint)bufferSize;
}


JNIEXPORT jboolean JNICALL
    Java_com_sun_media_sound_SimpleInputDevice_nSupportsSampleRate(JNIEnv* e, jobject thisObj, jfloat sampleRate) 
{
    TRACE1("Java_com_sun_media_sound_SimpleInputDevice_nSupportsSampleRate: %d.\n", sampleRate);

    if ( (sampleRate == 8000) || (sampleRate == 11025) || (sampleRate == 22050) || (sampleRate == 44100) || (sampleRate == 48000) ) 
	{
	    TRACE1("Java_com_sun_media_sound_SimpleInputDevice_nSupportsSampleRate: %d returning TRUE.\n", sampleRate);
	    return (jboolean)TRUE;
	}

    TRACE1("Java_com_sun_media_sound_SimpleInputDevice_nSupportsSampleRate: %d returning FALSE.\n", sampleRate);
    return (jboolean)FALSE;
}


JNIEXPORT jboolean JNICALL
    Java_com_sun_media_sound_SimpleInputDevice_nSupportsSampleSizeInBits(JNIEnv* e, jobject thisObj, jint sampleSizeInBits) 
{
    TRACE1("Java_com_sun_media_sound_SimpleInputDevice_nSupportsSampleSizeInBits: %d.\n", sampleSizeInBits);

    if (sampleSizeInBits == 16)
	{
	    TRACE2("Java_com_sun_media_sound_SimpleInputDevice_nSupportsSampleSizeInBits: %d returning %d.\n", sampleSizeInBits, (XIs16BitSupported()));
	    return (jboolean)XIs16BitSupported();
	}

    if (sampleSizeInBits == 8)
	{
	    TRACE2("Java_com_sun_media_sound_SimpleInputDevice_nSupportsSampleSizeInBits: %d returning %d.\n", sampleSizeInBits, (XIs8BitSupported()));
	    return (jboolean)XIs8BitSupported();
	}

    TRACE1("Java_com_sun_media_sound_SimpleInputDevice_nSupportsSampleSizeInBits: %d returning FALSE.\n", sampleSizeInBits);
    return (jboolean)FALSE;
}


JNIEXPORT jboolean JNICALL
    Java_com_sun_media_sound_SimpleInputDevice_nSupportsChannels(JNIEnv* e, jobject thisObj, jint channels) 
{

    if (channels == 1)
	return (jboolean)TRUE;

    if (channels == 2)
	return (jboolean)XIsStereoSupported();

    return (jboolean)FALSE;
}


JNIEXPORT jint JNICALL
    Java_com_sun_media_sound_SimpleInputDevice_nGetNumPorts(JNIEnv* e, jobject thisObj)
{

    TRACE0("Java_com_sun_media_sound_SimpleInputDevice_nGetNumPorts.\n");

    ERROR0("Java_com_sun_media_sound_SimpleInputDevice_nGetNumPorts not implemented\n");

    TRACE0("Java_com_sun_media_sound_SimpleInputDevice_nGetNumPorts not implemented\n");

    return (jint)0;
}


JNIEXPORT jstring JNICALL
    Java_com_sun_media_sound_SimpleInputDevice_nGetPortName(JNIEnv* e, jobject thisObj, jint index)
{

    TRACE0("Java_com_sun_media_sound_SimpleInputDevice_nGetPortName.\n");

    ERROR0("Java_com_sun_media_sound_SimpleInputDevice_nGetPortName not implemented\n");

    TRACE0("Java_com_sun_media_sound_SimpleInputDevice_nGetPortName not implemented\n");

    return (jstring)NULL;
}


/*
JNIEXPORT jfloat JNICALL
Java_com_sun_media_sound_SimpleInputDevice_nSetLinearGain(JNIEnv* e, jobject thisObj, jfloat linearGain) 
{
    TRACE1("Java_com_sun_media_sound_SimpleInputDevice_nSetLinearGain: %d.\n", linearGain);

    ERROR0("Java_com_sun_media_sound_SimpleInputDevice_nSetLinearGain NOT IMPLEMENTED.\n");

	// need to implement!
	linearGain = 1.0f;
	
	TRACE1("Java_com_sun_media_sound_SimpleInputDevice_nSetLinearGain: %d returning %d.\n", linearGain);
	return linearGain;
}


JNIEXPORT jfloat JNICALL
Java_com_sun_media_sound_SimpleInputDevice_nSetPan(JNIEnv* e, jobject thisObj, jfloat pan) 
{
    TRACE1("Java_com_sun_media_sound_SimpleInputDevice_nSetPan: %d.\n", pan);

    ERROR0("Java_com_sun_media_sound_SimpleInputDevice_nSetPan NOT IMPLEMENTED.\n");

	// need to implement!
	pan = 0.0f;
	
	TRACE1("Java_com_sun_media_sound_SimpleInputDevice_nSetPan: %d returning %d.\n", pan);
	return pan;
}


JNIEXPORT void JNICALL
Java_com_sun_media_sound_SimpleInputDevice_nSetAudioFormat(JNIEnv* e, jobject thisObj, jfloat sampleRate, jint sampleSizeInBits, jint channels)
{
OPErr           opErr;
XBOOL			reacquire;
XBOOL			recreateStream;
unsigned long	bufferSizeInBytes;

TRACE0("Java_com_sun_media_sound_SimpleInputDevice_nSetAudioFormat.\n");

if ( (sampleRate == g_sampleRate) && (channels == g_channels) && (sampleSizeInBits == sampleSizeInBits) )
{
TRACE0("Java_com_sun_media_sound_SimpleInputDevice_nSetAudioFormat: no change.\n");
return;
}

reacquire = acquired;
recreateStream = (reference == NULL) ? FALSE : TRUE;
	
if (recreateStream) 
{
GM_AudioCaptureStreamCleanup((void *)e, reference);
reference = NULL;
}

if (reacquire)
{
opErr = HAE_ReleaseAudioCapture(NULL);
}

g_sampleRate = (unsigned long)sampleRate;
g_channels = (unsigned long)channels;
g_sampleSizeInBits = (unsigned long)sampleSizeInBits;

bufferSizeInBytes = HAE_GetCaptureBufferSizeInFrames() * g_channels * g_sampleSizeInBits / 8;

if (reacquire)
{
opErr = HAE_AquireAudioCapture((void *)e, 
g_sampleRate, 
g_channels, 
g_sampleSizeInBits, NULL);
}

if ((opErr == NO_ERR) && recreateStream)
{
reference = GM_AudioCaptureStreamSetup			((void *)e,									// platform context
(long)thisObj, 							// user reference
(GM_StreamObjectProc)&AudioInputCallbackProc, 				// control callback
(unsigned long)bufferSizeInBytes, 							// buffer size $$kk: 10.06.98: buffer size in *bytes*
(XFIXED)FLOAT_TO_FIXED(g_sampleRate),				// Fixed 16.16
(char)g_sampleSizeInBits,						// 8 or 16 bit data
(char)g_channels,							// 1 or 2 channels of date
&opErr);
}

// if we failed, throw a ResourceUnavailableException
if (opErr != NO_ERR)
{
TRACE0("HAE_AquireAudioCapture failed; throwing ResourceUnavailableException\n");
ThrowJavaOpErrException(e, JAVA_PACKAGE_NAME"/ResourceUnavailableException", opErr);
return;
}


TRACE0("Java_com_sun_media_sound_SimpleInputDevice_nSetAudioFormat returning.\n");
}
*/
