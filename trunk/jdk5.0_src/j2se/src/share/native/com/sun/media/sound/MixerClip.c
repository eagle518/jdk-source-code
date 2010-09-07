/*
 * @(#)MixerClip.c	1.27 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#define USE_ERROR
#define USE_TRACE

// STANDARD includes


// JNI includes
#include <jni.h>

// ENGINE includes
#include "engine/GenSnd.h"
#include "engine/X_API.h"
#include "engine/HAE_API.h"


// UTILITY includes
#include "Utilities.h"

// MixerClip includes
#include "com_sun_media_sound_MixerClip.h"



// GLOBALS

jclass g_mixerClipClass = NULL;

jmethodID g_sampleLoopMethodID = 0;
jmethodID g_sampleEndMethodID = 0;
//jmethodID g_sampleFrameCallbackMethodId = 0;


// HELPER METHODS

static XBOOL initializeJavaClipCallbackVars(JNIEnv* e, jobject thisObj) 
{	
    jclass objClass = (*e)->GetObjectClass(e, thisObj);
    if (objClass == NULL)
	{
	    ERROR0("initializeJavaClipCallbackVars: (objClass == NULL)\n");
	    return (XBOOL)FALSE;
	}

    g_mixerClipClass = (*e)->NewGlobalRef(e, objClass);
    if (g_mixerClipClass == NULL)
	{
	    ERROR0("initializeJavaClipCallbackVars: (g_mixerClipClass == NULL)\n");
	    return (XBOOL)FALSE;
	}

    g_sampleLoopMethodID			=	(*e)->GetMethodID(e, g_mixerClipClass, "callbackSampleLoop", "()Z");
    g_sampleEndMethodID				=	(*e)->GetMethodID(e, g_mixerClipClass, "callbackSampleEnd", "()V");
    //g_sampleFrameCallbackMethodId	= 	(*e)->GetMethodID(e, g_mixerClipClass, "callbackSampleFramePosition", "(I)V");

    //if ( (g_sampleLoopMethodID == 0) || (g_sampleEndMethodID == 0) || (g_sampleFrameCallbackMethodId == 0) )
    if ( (g_sampleLoopMethodID == 0) || (g_sampleEndMethodID == 0) )
	{
	    ERROR0("initializeJavaClipCallbackVars: (failed to get method IDs)\n");
	    return (XBOOL)FALSE;
	}

    return (XBOOL)TRUE;
}


// $$kk: 03.22.99: need to call this on shutdown!!
/* $$fb 2002-03-07: never called ! */
/*
static void releaseJavaCallbackVars(JNIEnv* e) 
{
	jclass localClass = g_mixerClipClass;

	g_mixerClipClass = NULL;
	g_sampleLoopMethodID = 0;
	g_sampleEndMethodID = 0;

	(*e)->DeleteGlobalRef(e, localClass);
}
*/


// CALLBACKS FROM THE ENGINE
// $$kk: 04.16.99: need to work on these!!!


// This is the default loop callback, which tells the mixer to never loop samples

// $$kk: 04.19.99
static XBOOL PV_SampleLoopDoneCallback(void *context, void *threadContext)
{
    JNIEnv *e = (JNIEnv*)threadContext;

    TRACE2("PV_SampleLoopDoneCallback, context: %p, threadContext: %p\n", context, threadContext);

    if (((threadContext) == (void *)0) || ((threadContext) == (void *)-1))
	{
	    ERROR1("Bad threadContext: %p\n", threadContext);
	    return FALSE;
	}

    return (XBOOL)(*e)->CallBooleanMethod(e, (jobject)context, g_sampleLoopMethodID);

}


// Called when a sample finishes

// $$fb2003-03-14: add sender (fix for 4828556)
static void PV_SampleDoneCallback(VOICE_REFERENCE sender, void *context, void *threadContext)
{
    JNIEnv *e = (JNIEnv*)threadContext;

    TRACE3("PV_SampleDoneCallback, voice:%d, context: %p, threadContext: %p\n", sender, context, threadContext);

    if (((threadContext) == (void *)0) || ((threadContext) == (void *)-1))
	{
	    ERROR1("Bad threadContext: %p\n", threadContext);
	    return;
	}

    TRACE3(">> PV_SampleDoneCallback, CallVoidMethod: e: %p, context: %p, threadContext: %p\n", e, context, threadContext);	
    (*e)->CallVoidMethod(e, (jobject)context, g_sampleEndMethodID);
    TRACE0("<< PV_SampleDoneCallback, CallVoidMethod returned\n");	

    // $$kk: 06.28.99: now delete the global reference.  we shouldn't see
    // any native methods involving this object invoked anymore.
    // (moved this from nClose, where it is hazardous!!)
    (*e)->DeleteGlobalRef(e, (jobject)context);
}


// Called at sample frame positions marked by GM_AddSampleOffsetCallback
// $$rratta : 01.27.2000  Solaris 64 port
//	      Changed reference from INT32 to long in case this 
//	      ever gets used
/*
static void PV_SampleFrameCallback(void *threadContext, long reference, INT32 sampleFrame)
{	
	JNIEnv *e = (JNIEnv*)threadContext;

	TRACE3("PV_SampleFrameCallback, threadContext: %lu, reference: %lu, sampleFrame: %d\n", threadContext, reference, sampleFrame);

	if ((((long)threadContext) == 0) || (((long)threadContext) == -1))
	{
		ERROR1("Bad threadContext: %lu\n", threadContext);
		return;
	}

	(*e)->CallVoidMethod(e, (jobject)reference, g_sampleFrameCallbackMethodId, sampleFrame);
}
*/

// SAMPLE MANIPULATIONS


JNIEXPORT jlong JNICALL
    Java_com_sun_media_sound_MixerClip_nOpen(JNIEnv* e, jobject thisObj, jint sampleSizeInBits, jint channels, jfloat sampleRate, jbyteArray loadedData, jint offset, jint lengthInFrames)
{

    void				*pData = NULL; 

    GM_Waveform 		*pWave;
    int					lengthInBytes;

    // get all the class, method, and field id's ONCE.
    // store a global reference to the java clip class to make sure the id's remain valid.
    // $$kk: 03.22.99: need to make sure we release this reference eventually!

    if (g_mixerClipClass == NULL) 
	{
	    if (!initializeJavaClipCallbackVars(e, thisObj))
		{
		    ERROR0("Failed to initalize Java callback vars!\n");
		    return GENERAL_BAD;
		}
	}

    // $$kk: 04.15.99: note that the offset is in *bytes* and the length is in *frames*.
    
    TRACE0("Java_com_sun_media_sound_MixerClip_nOpen.\n");

    lengthInBytes = lengthInFrames * (sampleSizeInBits/8) * channels;

    // copy the data out of the java object, then give it to the engine and tell the
    // engine not to copy it again.

    pData = XNewPtr(lengthInBytes);

    if (!pData) 
	{
	    ERROR0("Java_com_sun_media_sound_MixerClip_nOpen failed: failed to allocate memory for sample data\n");
	    return 0;
	}

    (*e)->GetByteArrayRegion(e, loadedData, (jint)offset, (jint)lengthInBytes, (jbyte*)pData);


    // fill in the GM_Waveform structure
    pWave = (GM_Waveform *)XNewPtr((INT32)sizeof(GM_Waveform));
    TRACE1("Created waveform %p\n", pWave);
    if (pWave)
	{
	    pWave->waveSize = lengthInBytes;
	    pWave->waveFrames = lengthInFrames;
	    pWave->startLoop = 0;
	    pWave->endLoop = lengthInFrames;
	    pWave->baseMidiPitch = 60;
	    pWave->bitSize = (unsigned char)sampleSizeInBits;
	    pWave->channels = (unsigned char)channels;
	    pWave->sampledRate = FLOAT_TO_FIXED(sampleRate);	
	    pWave->theWaveform = (SBYTE *)pData;

	}
    else 
	{
	    ERROR0("Java_com_sun_media_sound_MixerClip_nOpen failed: failed to allocate memory for pWave\n");
	    XDisposePtr(pData);
	    return 0;
	}

    TRACE1("Java_com_sun_media_sound_MixerClip_nOpen completed -> new waveform id: %p\n", pWave);
    return (jlong) (INT_PTR) pWave;
}


JNIEXPORT jint JNICALL
    Java_com_sun_media_sound_MixerClip_nSetup(JNIEnv* e, jobject thisObj, jlong waveformId, jint framePosition, jint loopStart, jint loopEnd, jfloat linearGain, jfloat pan, jint sampleRate)
{

    VOICE_REFERENCE id;
    GM_Waveform *pWave;
    /* GM_SampleCallbackEntry	*pLastFrameCallback; */

    jobject globalClipObj = NULL;


    TRACE2("Java_com_sun_media_sound_MixerClip_nSetup, e: %p, thisObj: %p\n", e, thisObj);
    TRACE2("Java_com_sun_media_sound_MixerClip_nSetup, waveformId: %p, framePosition: %d\n", (void*) (UINT_PTR) waveformId, framePosition);

    // this is the GM_Waveform pointer
    pWave = (GM_Waveform *) (INT_PTR) waveformId;

    /*
      // $$kk: 11.15.99: we should never call this with a too-large buffer
      // better to throw the LineUnavailableException; runtime exceptions are
      // a bit harsh.
      // check for excessive size
      if ( (pWave->waveFrames - (UINT32)framePosition) >= MAX_SAMPLE_FRAMES ) 
      {
      ThrowJavaMessageException(e, "java/lang/IllegalArgumentException", "Clip exceeds maximum size");
      }
    */

    // update the GM_Waveform values
    // $$kk: 04.19.99: note that the engine needs these values relative to the sample data
    // once the start offset has been applied!!
    pWave->startLoop = (UINT32)(loopStart - framePosition);
    pWave->endLoop = (UINT32)(loopEnd - framePosition);
    pWave->sampledRate = FLOAT_TO_FIXED(sampleRate);	


    // $$kk: 03.15.98: GLOBAL REF!  this has definite memory leak potential; need to make sure
    // to release all global refs....
    // $$kk: 12.18.98: why is this here?  do we need it?  for callbacks??
    // $$kk: 06.28.99: yes!  need it for callbacks.
    globalClipObj = (*e)->NewGlobalRef(e, thisObj);

    // $$kk: 04.16.99: need to clean this up.  For instance, we need to pass the
    // correct volume and pan values here....
    id = GM_SetupSampleFromInfo((GM_Waveform *) (INT_PTR) waveformId, (void *)globalClipObj, 
				FLOAT_TO_VOLUME(linearGain),
				FLOAT_TO_PAN(pan),
				(GM_LoopDoneCallbackPtr)PV_SampleLoopDoneCallback, 
				PV_SampleDoneCallback,
				(UINT32)framePosition);

    // failed to setup!
    if (id == DEAD_VOICE)
	{
	    // no free voices!  (or sample too big....)
	    ThrowJavaOpErrException(e, JAVA_SAMPLED_PACKAGE_NAME"/LineUnavailableException", NO_FREE_VOICES);
	} else {
	    /* enable high quality interpolation */
	    GM_SetSampleResample(id, TRUE);
	}
	
    // return the voice identifier
    return (jint)id;
}


JNIEXPORT jboolean JNICALL
    Java_com_sun_media_sound_MixerClip_nStart(JNIEnv* e, jobject thisObj, jint id)
{	
    // $$kk: 04.15.99: can this fail??
    // $$kk: 04.16.99: only if PV_GetVoiceFromSoundReference fails; then we get NOT_SETUP
    if (GM_StartSample((VOICE_REFERENCE)id) != NO_ERR) 
	{
	    ERROR1("Java_com_sun_media_sound_MixerClip_nStart: GM_StartSample return an error; -> new sample id: %d\n", id);

	    // if this failed, end the sample and set the id to 0.
	    // $$fb 2002-04-20: we can safely call GM_EndSample here (rather than 
	    // GM_ReleaseSample), because the voice wasn't started.
	    GM_EndSample((VOICE_REFERENCE)id, (void *)e);
	    return (jboolean)FALSE;
	}

    TRACE1("Java_com_sun_media_sound_MixerClip_nStart completed -> new sample id: %d\n", id);
    return (jboolean)TRUE;
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_MixerClip_nStop(JNIEnv* e, jobject thisObj, jint id)
{

    TRACE0("Java_com_sun_media_sound_MixerClip_nStop.\n");	
	
    // $$fb 2002-04-20: use a safe thread version of GM_EndSample
    // fix for 4498848: Sound causes crashes on Linux
    GM_ReleaseSample((VOICE_REFERENCE)id, (void *)e);

    TRACE0("Java_com_sun_media_sound_MixerClip_nStop completed.\n");
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_MixerClip_nClose(JNIEnv* e, jobject thisObj, jint id, jlong waveformId)
{
    TRACE0("Java_com_sun_media_sound_MixerClip_nClose.\n");

    // $$fb 2002-04-20: use a safe thread version of GM_EndSample
    // fix for 4498848: Sound causes crashes on Linux
    if (id != 0) {
	GM_ReleaseSample((VOICE_REFERENCE)id, (void *)e);
    }

    // $$kk: 04.15.99: need to clean up GM_Waveform memory!!
    if (waveformId != 0)
	{
	    // $$fb 2002-04-20: wait for max 500ms until the voice went from RELEASING to UNUSED state.
	    int ctr=250;
	    while (!GM_IsSoundDone((VOICE_REFERENCE)id) && --ctr) {
		HAE_SleepFrameThread((void*) e, 2);
	    }
	    if (!ctr) {
		ERROR0("MixerClip.c nClose: close timed out at waiting for sound done!\n");
	    }
	    GM_FreeWaveform((GM_Waveform *) (UINT_PTR) waveformId);
	}
	
    // $$kk: 06.28.99: *cannot* call this here or we risk crashing on the
    // PV_SampleDoneCallback callback 'cause that can happen after this method
    // completes!!
    //(*e)->DeleteGlobalRef(e, thisObj);

    TRACE0("Java_com_sun_media_sound_MixerClip_nClose completed.\n");
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_MixerClip_nDrain(JNIEnv* e, jobject thisObj, jint id)
{

    TRACE0("Java_com_sun_media_sound_MixerClip_nDrain.\n");

    /* not implemented */

    TRACE0("Java_com_sun_media_sound_MixerClip_nDrain completed.\n");
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_MixerClip_nFlush(JNIEnv* e, jobject thisObj, jint id)
{

    TRACE0("Java_com_sun_media_sound_MixerClip_nFlush.\n");

    /* not implemented */

    TRACE0("Java_com_sun_media_sound_MixerClip_nFlush completed.\n");
}


JNIEXPORT jlong JNICALL
    Java_com_sun_media_sound_MixerClip_nGetPosition(JNIEnv* e, jobject thisObj, jint id)
{

    TRACE0("Java_com_sun_media_sound_MixerClip_nGetPosition.\n");

    // this returns an unsigned long
    return (jlong)GM_GetSamplePlaybackPosition((VOICE_REFERENCE)id);
}


JNIEXPORT jfloat JNICALL
    Java_com_sun_media_sound_MixerClip_nSetLinearGain(JNIEnv* e, jobject thisObj, jint id, jfloat linearGain)
{

    TRACE0("Java_com_sun_media_sound_MixerClip_nSetLinearGain.\n");

    GM_ChangeSampleVolume((VOICE_REFERENCE)id, FLOAT_TO_VOLUME(linearGain));

    TRACE0("Java_com_sun_media_sound_MixerClip_nSetLinearGain completed.\n");

    return VOLUME_TO_FLOAT(GM_GetSampleVolume((VOICE_REFERENCE)id));
}


JNIEXPORT jfloat JNICALL
    Java_com_sun_media_sound_MixerClip_nSetPan(JNIEnv* e, jobject thisObj, jint id, jfloat pan)
{

    TRACE0("Java_com_sun_media_sound_MixerClip_nSetPan.\n");
    VTRACE1("-> stream id: %lu\n", (INT32)id);

    GM_ChangeSampleStereoPosition((VOICE_REFERENCE)id, (short int)(FLOAT_TO_PAN(pan)));
    TRACE0("Java_com_sun_media_sound_MixerClip_nSetPan completed.\n");

    return (jfloat)(PAN_TO_FLOAT(GM_GetSampleStereoPosition((VOICE_REFERENCE)id)));
}


JNIEXPORT jint JNICALL
    Java_com_sun_media_sound_MixerClip_nSetSampleRate(JNIEnv* e, jobject thisObj, jint id, jint rate)
{

    TRACE0("Java_com_sun_media_sound_MixerClip_nSetSampleRate.\n");
    VTRACE1("-> stream id: %lu\n", (INT32)id);

    GM_ChangeSamplePitch((VOICE_REFERENCE)id, UNSIGNED_LONG_TO_XFIXED(rate));
    TRACE0("Java_com_sun_media_sound_MixerClip_nSetSampleRate completed.\n");

    // $$kk: 10.21.99: note that to fix a bug in set sample rate:
    // 1. must use the XFIXED / UNSIGNED_LONG macros; 
    //    the XFIXED / LONG macros overflow and wrap to a negative number.
    // 2. must make the '+ XFIXED_1 / 2' adjustment to avoid returning a value
    //    that is always one smaller than what was requested; i just copied 
    //    what the XFIXED_TO_LONG_ROUNDED(x) macro does.
    return (jint)(XFIXED_TO_UNSIGNED_LONG(GM_GetSamplePitch((VOICE_REFERENCE)id) + XFIXED_1 / 2));
}
