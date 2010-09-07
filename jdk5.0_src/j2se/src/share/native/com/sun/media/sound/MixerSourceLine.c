/*
 * @(#)MixerSourceLine.c	1.32 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//#define USE_TRACE

// STANDARD includes

// JNI includes
#include <jni.h>

// ENGINE includes
#include "engine/GenSnd.h"
#include "engine/X_API.h"


// UTILITY includes
#include "Utilities.h"																											  

// MixerSourceLine includes
#include "com_sun_media_sound_MixerSourceLine.h"


// $$kk: 03.21.99: TEMP: need this for old un-merged code
//typedef long STREAM_REFERENCE;


// FORWARDS

// callbacks for stream create, get data, and destroy
static OPErr    MixerSourceLineCallbackProc(void* context, GM_StreamMessage message, GM_StreamData* pAS);


// GLOBALS

jclass g_mixerSourceLineClass = NULL;

jmethodID g_destroyMethodID = 0;
jmethodID g_getDataMethodID = 0;
jmethodID g_startMethodID = 0;
jmethodID g_stopMethodID = 0;
jmethodID g_eomMethodID = 0;
jmethodID g_activeMethodID = 0;
jmethodID g_inactiveMethodID = 0;

jfieldID g_dataArrayFieldID = 0; 


// HELPER METHODS

static XBOOL initializeJavaCallbackVars(JNIEnv* e, jobject thisObj) 
{	
    jclass objClass = (*e)->GetObjectClass(e, thisObj);
    if (objClass == NULL)
	{
	    ERROR0("initializeJavaCallbackVars: (objClass == NULL)\n");
	    return (XBOOL)FALSE;
	}

    g_mixerSourceLineClass = (*e)->NewGlobalRef(e, objClass);
    if (g_mixerSourceLineClass == NULL)
	{
	    ERROR0("initializeJavaCallbackVars: (g_mixerSourceLineClass == NULL)\n");
	    return (XBOOL)FALSE;
	}

    g_destroyMethodID =  (*e)->GetMethodID(e, g_mixerSourceLineClass, "callbackStreamDestroy", "()V");
    g_getDataMethodID =  (*e)->GetMethodID(e, g_mixerSourceLineClass, "callbackStreamGetData", "([BI)I");
    g_startMethodID =    (*e)->GetMethodID(e, g_mixerSourceLineClass, "callbackStreamStart", "()V");
    g_stopMethodID =     (*e)->GetMethodID(e, g_mixerSourceLineClass, "callbackStreamStop", "()V");
    g_eomMethodID =      (*e)->GetMethodID(e, g_mixerSourceLineClass, "callbackStreamEOM", "()V");
    g_activeMethodID =   (*e)->GetMethodID(e, g_mixerSourceLineClass, "callbackStreamActive", "()V");
    g_inactiveMethodID = (*e)->GetMethodID(e, g_mixerSourceLineClass, "callbackStreamInactive", "()V");

    if ( (g_destroyMethodID == 0) || (g_getDataMethodID == 0) || (g_startMethodID == 0) ||
	 (g_stopMethodID == 0) || (g_eomMethodID == 0) )
	{
	    ERROR0("initializeJavaCallbackVars: (failed to get method IDs)\n");
	    return (XBOOL)FALSE;
	}

    g_dataArrayFieldID =	(*e)->GetFieldID(e, g_mixerSourceLineClass, "dataBuffer", "[B");
    if (g_dataArrayFieldID == 0)
	{
	    ERROR0("initializeJavaCallbackVars: (g_dataArrayFieldID == 0)\n");
	    return (XBOOL)FALSE;
	}

    return (XBOOL)TRUE;
}


// $$kk: 03.22.99: need to call this on shutdown!!
static void releaseJavaCallbackVars(JNIEnv* e) 
{
    jclass localClass = g_mixerSourceLineClass;

    g_mixerSourceLineClass = NULL;
    g_destroyMethodID = 0;
    g_getDataMethodID = 0;
    g_startMethodID = 0;
    g_stopMethodID = 0;
    g_eomMethodID = 0;

    (*e)->DeleteGlobalRef(e, localClass);
}


// STREAMING CHANNEL MANIPULATIONS


JNIEXPORT jlong JNICALL
    Java_com_sun_media_sound_MixerSourceLine_nOpen(JNIEnv* e, jobject thisObj, jint sampleSizeInBits, jint channels, jfloat sampleRate, jint bufferSize)
{

    STREAM_REFERENCE id = NULL;
    jobject			globalSourceLineObj = NULL;

    // get all the class, method, and field id's ONCE.
    // store a global reference to the java source line class to make sure the id's remain valid.
    // $$kk: 03.22.99: need to make sure we release this reference eventually!

    if (g_mixerSourceLineClass == NULL) 
	{
	    if (!initializeJavaCallbackVars(e, thisObj))
		{
		    ERROR0("Failed to initalized Java callback vars!\n");
		    return GENERAL_BAD;
		}
	}

	
    // $$kk: 03.15.98: GLOBAL REF!  this has definite memory leak potential; need to make sure
    // to release all global refs....
    globalSourceLineObj = (*e)->NewGlobalRef(e, thisObj);

	
    // possible error values: NO_ERR, STREAM_STOP_PLAY, PARAM_ERR, NO_FREE_VOICES, possibly others??, 

    TRACE0("Java_com_sun_media_sound_MixerSourceLine_nOpen.\n");
    TRACE2("e %lx, thisObj %lx\n", e, thisObj);
    TRACE2("bufferSize %lu, sampleRate %d\n", (UINT32)bufferSize, FLOAT_TO_FIXED(sampleRate));
    TRACE2("sampleSizeInBits %d, channels %d\n", (int)sampleSizeInBits, (int)channels);

    TRACE0("Calling GM_AudioStreamSetup\n");

    // we are not handling errors properly here
    id = GM_AudioStreamSetup((void*)e, (void *)globalSourceLineObj, MixerSourceLineCallbackProc,
			     (UINT32)bufferSize,
			     FLOAT_TO_FIXED(sampleRate),
			     (char)sampleSizeInBits, (char)channels);

    TRACE0("Returned from GM_AudioStreamSetup\n");


    if (id != NULL)
	{
	    if (GM_AudioStreamError(id) != NO_ERR)
		{
		    id = NULL;
		}
	}

    TRACE1("Java_com_sun_media_sound_MixerSourceLine_nOpen completed -> new stream id: %lx\n", id);
	
    return (jlong) (INT_PTR) id;
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_MixerSourceLine_nStart(JNIEnv* e, jobject thisObj, jlong id)
{
    OPErr           opErr = NO_ERR;

    TRACE1("Java_com_sun_media_sound_MixerSourceLine_nStart %lx.\n", id);
    VTRACE1("-> stream id: %lx\n", (STREAM_REFERENCE) (INT_PTR) id);

    // $$kk: 03.21.99: TEMP: get rid of this line for old un-merged code
    opErr = GM_AudioStreamPreroll((STREAM_REFERENCE) (INT_PTR) id);
    if (opErr != NO_ERR)
	{
	    ERROR1("Returned an error from GM_AudioStreamPreroll: %d\n", opErr);
	}
    else 
	{
	    opErr = GM_AudioStreamStart((STREAM_REFERENCE) (INT_PTR) id);

	    if (opErr != NO_ERR) 
		{
		    ERROR1("Returned an error from GM_AudioStreamStart: %d\n", opErr);
		}
	}

    TRACE1("Java_com_sun_media_sound_MixerSourceLine_nStart %lx completed.\n", id);
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_MixerSourceLine_nClose(JNIEnv* e, jobject thisObj, jlong id)
{

    TRACE1("Java_com_sun_media_sound_MixerSourceLine_nClose %lx.\n", id);

    GM_AudioStreamStop((void *)e, (STREAM_REFERENCE) (INT_PTR) id);

    TRACE1("Java_com_sun_media_sound_MixerSourceLine_nClose %lx completed.\n", id);
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_MixerSourceLine_nPause(JNIEnv* e, jobject thisObj, jlong id)
{

    TRACE1("Java_com_sun_media_sound_MixerSourceLine_nPause %lx.\n", id);

    GM_AudioStreamPause((STREAM_REFERENCE) (INT_PTR) id);

    TRACE1("Java_com_sun_media_sound_MixerSourceLine_nPause %lx completed\n", id);
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_MixerSourceLine_nResume(JNIEnv* e, jobject thisObj, jlong id)
{

    TRACE1("Java_com_sun_media_sound_MixerSourceLine_nResume( %lx.\n", id);

    GM_AudioStreamResume((STREAM_REFERENCE) (INT_PTR) id);

    TRACE1("Java_com_sun_media_sound_MixerSourceLine_nResume( %lx completed\n", id);
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_MixerSourceLine_nDrain(JNIEnv* e, jobject thisObj, jlong id)
{

    TRACE1("Java_com_sun_media_sound_MixerSourceLine_nDrain %lx.\n", id);

    GM_AudioStreamDrain((void *)e, (STREAM_REFERENCE) (INT_PTR) id);
    
    TRACE1("Java_com_sun_media_sound_MixerSourceLine_nDrain %lx completed\n", id);
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_MixerSourceLine_nFlush(JNIEnv* e, jobject thisObj, jlong id)
{

    TRACE1("Java_com_sun_media_sound_MixerSourceLine_nFlush %lx.\n", id);

    GM_AudioStreamFlush((STREAM_REFERENCE) (INT_PTR) id);

    TRACE1("Java_com_sun_media_sound_MixerSourceLine_nFlush %lx completed\n", id);
}


JNIEXPORT jlong JNICALL
    Java_com_sun_media_sound_MixerSourceLine_nGetPosition(JNIEnv* e, jobject thisObj, jlong id)
{
    jlong samples;

    VTRACE1("Java_com_sun_media_sound_MixerSourceLine_nGetPosition %lx.\n", id);
    VTRACE1("-> stream id: %lx\n", (STREAM_REFERENCE) (INT_PTR) id);

    samples = (jlong)GM_AudioStreamGetSamplesPlayed((STREAM_REFERENCE) (INT_PTR) id);

    TRACE1("Java_com_sun_media_sound_MixerSourceLine_nGetPosition %lu completed.\n", id);
    return samples;
}


JNIEXPORT jfloat JNICALL
    Java_com_sun_media_sound_MixerSourceLine_nGetLevel(JNIEnv* e, jobject thisObj, jlong id)
{
    ERROR0("Java_com_sun_media_sound_MixerSourceLine_nGetLevel: NOT IMPLEMENTED! \n");
    return (jfloat)0;
}


// CONTROLS

JNIEXPORT jfloat JNICALL
    Java_com_sun_media_sound_MixerSourceLine_nSetLinearGain(JNIEnv* e, jobject thisObj, jlong id, jfloat linearGain)
{

    TRACE1("Java_com_sun_media_sound_MixerSourceLine_nSetLinearGain %lx.\n", id);

    GM_AudioStreamSetVolume((STREAM_REFERENCE) (INT_PTR) id, FLOAT_TO_VOLUME(linearGain), FALSE);

    TRACE1("Java_com_sun_media_sound_MixerSourceLine_nSetLinearGain %lu completed.\n", id);

    return VOLUME_TO_FLOAT(GM_AudioStreamGetVolume((STREAM_REFERENCE) (INT_PTR) id));
}


JNIEXPORT jfloat JNICALL
    Java_com_sun_media_sound_MixerSourceLine_nSetPan(JNIEnv* e, jobject thisObj, jlong id, jfloat pan)
{

    TRACE1("Java_com_sun_media_sound_MixerSourceLine_nSetPan %lx.\n", id);
    VTRACE1("-> stream id: %lx\n", (STREAM_REFERENCE) (INT_PTR) id);

    GM_AudioStreamSetStereoPosition((STREAM_REFERENCE) (INT_PTR) id, (short int)(FLOAT_TO_PAN(pan)));
    TRACE1("Java_com_sun_media_sound_MixerSourceLine_nSetPan %lx completed.\n", id);

    return (jfloat)(PAN_TO_FLOAT(GM_AudioStreamGetStereoPosition((STREAM_REFERENCE) (INT_PTR) id)));
}


JNIEXPORT jint JNICALL
    Java_com_sun_media_sound_MixerSourceLine_nSetSampleRate(JNIEnv* e, jobject thisObj, jlong id, jint rate)
{

    TRACE1("Java_com_sun_media_sound_MixerSourceLine_nSetSampleRate %lx.\n", id);
    VTRACE1("-> stream id: %lx\n", (STREAM_REFERENCE) (INT_PTR) id);
	
    // $$kk: 04.06.99: steve hales says i should be using the UNSIGNED_LONG macros here.  
    GM_AudioStreamSetRate((STREAM_REFERENCE) (INT_PTR) id, UNSIGNED_LONG_TO_XFIXED(rate));
    TRACE1("Java_com_sun_media_sound_MixerSourceLine_nSetSampleRate %lx completed.\n", id);

    // $$kk: 04.06.99: steve hales says i should be using the UNSIGNED_LONG macros here.  
    return (jint)XFIXED_TO_UNSIGNED_LONG(GM_AudioStreamGetRate((STREAM_REFERENCE) (INT_PTR) id));
}


// CALLBACKS INTO JAVA

static OPErr CallToJavaStreamDestroy(JNIEnv* e, jobject sourceLine)
{
    TRACE1("CallToJavaStreamDestroy, sourceLine: %lx.\n", sourceLine);
    (*e)->CallVoidMethod(e, sourceLine, g_destroyMethodID);

    // $$kk: 04.13.98: moving this here from above because we can still get this callback *after* nDisposeSourceLine,
    // giving us a vm crash....
	
    (*e)->DeleteGlobalRef(e, sourceLine);

    TRACE1("CallToJavaStreamDestroy, sourceLine: %lx completed.\n", sourceLine);
    return NO_ERR;
}


static jint CallToJavaStreamGetData(JNIEnv* e, jobject sourceLine, jbyteArray array, jint dataLength) 
{
    jint length;    
    VTRACE2("CallToJavaStreamGetData, sourceLine: %lx, dataLength: %d\n", sourceLine, dataLength);
    length = (*e)->CallIntMethod(e, sourceLine, g_getDataMethodID, array, dataLength);
    VTRACE1("CallToJavaStreamGetData, sourceLine: %lx completed.\n", sourceLine);
    return length; 
}

static OPErr CallToJavaStreamStart(JNIEnv* e, jobject sourceLine)
{
    TRACE1("CallToJavaStreamStart, sourceLine: %lx.\n", sourceLine);
    (*e)->CallVoidMethod(e, sourceLine, g_startMethodID);
    TRACE1("CallToJavaStreamStart, sourceLine: %lx completed.\n", sourceLine);
    return NO_ERR;
}

static OPErr CallToJavaStreamStop(JNIEnv* e, jobject sourceLine)
{
    TRACE1("CallToJavaStreamStop, sourceLine: %lx.\n", sourceLine);
    (*e)->CallVoidMethod(e, sourceLine, g_stopMethodID);
    TRACE1("CallToJavaStreamStop, sourceLine: %lx completed.\n", sourceLine);
    return NO_ERR;
}

static OPErr CallToJavaStreamEOM(JNIEnv* e, jobject sourceLine)
{
    TRACE1("CallToJavaStreamEOM, sourceLine: %lx.\n", sourceLine);
    (*e)->CallVoidMethod(e, sourceLine, g_eomMethodID);
    TRACE1("CallToJavaStreamEOM, sourceLine: %lx completed.\n", sourceLine);
    return NO_ERR;
}

static OPErr CallToJavaStreamActive(JNIEnv* e, jobject sourceLine)
{
    TRACE1("CallToJavaStreamActive, sourceLine: %lx.\n", sourceLine);
    (*e)->CallVoidMethod(e, sourceLine, g_activeMethodID);
    TRACE1("CallToJavaStreamActive, sourceLine: %lx completed.\n", sourceLine);
    return NO_ERR;
}

static OPErr CallToJavaStreamInactive(JNIEnv* e, jobject sourceLine)
{
    TRACE1("CallToJavaStreamInactive, sourceLine: %lx.\n", sourceLine);
    (*e)->CallVoidMethod(e, sourceLine, g_inactiveMethodID);
    TRACE1("CallToJavaStreamInactive, sourceLine: %lx completed.\n", sourceLine);
    return NO_ERR;
}


// CALLBACK PROC

static OPErr MixerSourceLineCallbackProc(void* context, GM_StreamMessage message, GM_StreamData* pAS)
{
    JNIEnv* e = (JNIEnv*)context;														    
    VTRACE2("MixerSourceLineCallbackProc, context: %lx, message: %d\n", context, message);

    // for STREAM_CREATE, we just need to allocate the buffer.
    // we need to convert from gmData->dataLength (number of frames) to number of bytes.
    // $$kk: 03.15.98: this is the same calculation done by PV_GetSampleSizeInBytes.  however, this
    // method is not available outside GenAudioStreams.c right now.

    if (message == STREAM_CREATE)
	{
	    jint    const byteCount = pAS->dataLength * pAS->channelSize * (pAS->dataBitSize / 8);

	    VTRACE1("STREAM_CREATE: %lx\n", pAS->userReference);
	    VTRACE3("STREAM_CREATE: byteCount = %d, dataLength = %lu, frameSize = %d.\n", byteCount, pAS->dataLength, (pAS->channelSize * (pAS->dataBitSize / 8)) );

	    pAS->pData = XNewPtr(byteCount);
	    if (!pAS->pData)
		{
		    ERROR1("MixerSourceLineCallbackProc: STREAM_CREATE: Failed to allocate %d bytes\n", byteCount);
		    return MEMORY_ERR;
		}

	    VTRACE1("STREAM_CREATE: %lx completed\n", pAS->userReference);
	}
    else if (message == STREAM_DESTROY)
	{
	    VTRACE1("STREAM_DESTROY: %lx\n", pAS->userReference);

	    CallToJavaStreamDestroy(e, (jobject)pAS->userReference);

	    // $$kk: 12.21.98: for some reason, this causes the vm to crash some
	    // random time later.  so we have a memory leak here.  i don't know
	    // why we can't release the memory here....
		
	    // $$kk: 03.22.00: i am putting this code back in.  the memory leak is
	    // causing problems, and so far i have not been able to reproduce the
	    // crash.  the memory leak is reported in bug #4319431: "Memory leaks
	    // on opening and closing source DataLines."

	    if (pAS->pData)
		{
		    XDisposePtr(pAS->pData);
		}

	    VTRACE1("STREAM_DESTROY: %lx completed\n", pAS->userReference);
	}
    else if (message == STREAM_GET_DATA)
	{		
	    int length;
	    int frameLengthInBytes = pAS->channelSize * (pAS->dataBitSize / 8);
	    jbyteArray array = (jbyteArray)(*e)->GetObjectField(e, (jobject)pAS->userReference, g_dataArrayFieldID);

	    if (array == NULL) 
		{
		    ERROR0("STREAM_GET_DATA: failed to get array\n");
		    return GENERAL_BAD;
		}

	    // make the callback.
	    // pAS->dataLength is the length in FRAMES.
	    // length returned is length in FRAMES.
	    length = (int)CallToJavaStreamGetData(e, (jobject)pAS->userReference, array, (jint)pAS->dataLength);

	    // length will be -1 when the stream finishes
		
	    if (length < 0)
		{
		    VTRACE0("STREAM_GET_DATA: got negative length; stopping stream\n");
		    return STREAM_STOP_PLAY;
		}
													   
	    // get the relevant array elements

	    // $$kk: 03.15.98: can we avoid this copy??
	    (*e)->GetByteArrayRegion(e, array, (jint)0, (jint)(length * frameLengthInBytes), (jbyte*)(pAS->pData));
		
	    // $$kk: 04.21.00: fix for bug #4332327: "JNI leaks local references on 
	    // keeping a SourceLine open for long."
	    (*e)->DeleteLocalRef(e, array);		
		
	    // set dataLength to the number of frames read
	    pAS->dataLength = length;

	    VTRACE2("STREAM_GET_DATA: length: %d, pAS->dataLength: %lu\n", length, pAS->dataLength);
	    VTRACE1("STREAM_GET_DATA: %lx completed\n", pAS->userReference);
	}
    else if (message == STREAM_START)
	{
	    TRACE2("STREAM_START: stream: %lx,  framePosition: %lu\n", pAS->userReference, pAS->startSample);
	    CallToJavaStreamStart(e, (jobject)pAS->userReference);
	    TRACE1("STREAM_START: %lx completed\n", pAS->userReference);
	}
    else if (message == STREAM_STOP)
	{
	    TRACE2("STREAM_STOP: %lx,  framePosition: %lu\n", pAS->userReference, pAS->startSample);
	    CallToJavaStreamStop(e, (jobject)pAS->userReference);
	    TRACE1("STREAM_STOP: %lx completed\n", pAS->userReference);
	}
    else if (message == STREAM_EOM)
	{
	    TRACE2("STREAM_EOM: %lx,  framePosition: %lu\n", pAS->userReference, pAS->startSample);
	    CallToJavaStreamEOM(e, (jobject)pAS->userReference);
	    TRACE1("STREAM_EOM: %lx completed\n", pAS->userReference);
	}
    else if (message == STREAM_ACTIVE)
	{
	    TRACE2("STREAM_ACTIVE: %lx,  framePosition: %lu\n", pAS->userReference, pAS->startSample);
	    CallToJavaStreamActive(e, (jobject)pAS->userReference);
	    TRACE1("STREAM_ACTIVE: %lx completed\n", pAS->userReference);
	}
    else if (message == STREAM_INACTIVE)
	{
	    TRACE2("STREAM_INACTIVE: %lx,  framePosition: %lu\n", pAS->userReference, pAS->startSample);
	    CallToJavaStreamInactive(e, (jobject)pAS->userReference);
	    TRACE1("STREAM_INACTIVE: %lx completed\n", pAS->userReference);
	}

    else
	{
	    ERROR1("Bad message code in MixerSourceLineCallbackProc() : %d\n", message);
	    return GENERAL_BAD;
	}

    return NO_ERR;
}
