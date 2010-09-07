/*
 * @(#)HeadspaceMixer.c	1.42 03/12/19
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
#include "engine/X_API.h"
#include "engine/GenSnd.h"
#include "engine/GenPriv.h"		// $$kk: 10.02.98: need for references to MusicGlobals
#include "engine/HAE_API.h"		// $$kk: 10.02.98: need for HAE_GetMaxSamplePerSlice(); should get rid of this??


// UTILITY includes
#include "Utilities.h"

// JAVA includes
#include "com_sun_media_sound_HeadspaceMixer.h"

// Audio mode variables.  There's no "get" method for these, so I'm caching them
// for use in GM_ChangeAudioModes
Quality mixerQuality;
TerpMode mixerTerp;
AudioModifiers mixerModifiers;


// Variables for the output proc (used for output taps)
jobject globalSystemDeviceObj        = NULL;
jclass globalSystemDeviceClass       = NULL;
jmethodID callbackOutputProcMethodID = NULL;
jbyteArray outputProcByteArray       = NULL;



// ENGINE PROCS

// runs audio stream service
static void AudioTaskCallbackProc(void* context, UINT32 ticks)
{
    GM_AudioStreamService(context);
}


// Sends data up to Java before for output taps before writing it to 
// the device.
/*
static void AudioOutputCallbackProc(void* context, void *samples, INT32 sampleSize, INT32 channels, UINT32 lengthInFrames)
{

   	JNIEnv* e = (JNIEnv*)context;
	int arrayLength = lengthInFrames * channels * sampleSize;

    TRACE3("AudioOutputCallbackProc, outputProcByteArray: %lu, arrayLength: %d, lengthInFrames: %lu;\n", outputProcByteArray, arrayLength, lengthInFrames);

	// $$kk: 10.02.98: i think that it should not be possible to overstep the array here with the engine operating as it
	// does currently... right??	 	 
	(*e)->SetByteArrayRegion(e, outputProcByteArray, (jsize)0, (jsize)arrayLength, (jbyte *)samples); 
	(*e)->CallVoidMethod(e, globalSystemDeviceObj, callbackOutputProcMethodID, outputProcByteArray, (jint)lengthInFrames);

    TRACE0("AudioOutputCallbackProc completed.\n");
}
*/

// NATIVE METHODS

JNIEXPORT void JNICALL
Java_com_sun_media_sound_HeadspaceMixer_nOpenMixer(JNIEnv* e, jobject thisObj, jint sampleSizeInBits, jint channels, 
												   jint sampleRate, jint terpMode, jint midiVoices, jint sampledVoices, 
												   jint mixLevel)
{

	OPErr           opErr = NO_ERR;

	// possible return values: NO_ERR, PARAM_ERR, MEMORY_ERR, 

	// quality is sample rate
	// theMods: 16bit/stereo rendering + reverb?
	// midi voices
	// normVoices: volume divisor.
	// maxEffects: sampled audio voices

	// reverb is just going to be on by default right now; however, i am not implementing anything for it.

	Quality theQuality;
	TerpMode theTerp;
	AudioModifiers theMods;
	INT16 maxVoices;
	INT16 normVoices; 
	INT16 maxEffects;

    TRACE0("Java_com_sun_media_sound_HeadspaceMixer_nOpenMixer.\n");

    VTRACE1("sampleRate: %d\n", (int)sampleRate);
    VTRACE1("terpMode: %d\n", (int)terpMode);
    VTRACE1("sampleSizeInBits: %d\n", (int)sampleSizeInBits);
    VTRACE1("channels: %d\n", (int)channels);
    VTRACE1("midiVoices: %d\n", (int)midiVoices);
    VTRACE1("sampledVoices: %d\n", (int)sampledVoices);
    VTRACE1("mixLevel: %d\n", (int)mixLevel);

	switch ((int)sampleRate)
	{
	case 8000:
		theQuality = Q_8K;
		break;
	case 11025:
		theQuality = Q_11K;
		break;
	case 22050:
		theQuality = Q_22K;
		break;
	case 44100:
		theQuality = Q_44K;
		break;
	case 48000:
		theQuality = Q_48K;
		break;
	default:
	    ERROR1("bad sampleRate: %d\n", (int)sampleRate);
		opErr = PARAM_ERR;
		break;
	} 

	if (opErr == NO_ERR)
	{

		switch((int)terpMode)
		{
		case (int)E_AMP_SCALED_DROP_SAMPLE:
		case (int)E_2_POINT_INTERPOLATION:
		case (int)E_LINEAR_INTERPOLATION:
			theTerp = (int)terpMode;
			break;
		default:
		    ERROR1("bad terpMode: %d\n", (int)terpMode);
			opErr = PARAM_ERR;
			break;
		}

		if (opErr == NO_ERR)
		{

			theMods = 0;
			if (sampleSizeInBits == 16) 
			{
				theMods |= M_USE_16;
			}
			if (channels == 2) 
			{
				theMods |= M_USE_STEREO;
			}

			// $$kk: 03.06.98: reverb is enabled by default.  to disable, set flag here in theMods.
			// $$kk: 03.11.98: we are not allowing the opportunity to set M_STEREO_FILTER.

			maxVoices = (INT16)midiVoices;
			maxEffects = (INT16)sampledVoices;
			normVoices = (INT16)mixLevel;

			// possible return values: NO_ERR, PARAM_ERR, MEMORY_ERR, 
			opErr = GM_InitGeneralSound((void*)e, theQuality, theTerp, theMods, maxVoices, normVoices, maxEffects);

			// $$kk: 06.21.99: actually, we'll set this when they actually "resume" audio
			// and the java thread is created....
			// now we have to start the callback proc to run the stream service
			//GM_SetAudioTask(AudioTaskCallbackProc);
		}
	}

	// throw an exception if it didn't work
	if (opErr != NO_ERR)
	{
		if (opErr == MEMORY_ERR)
		{
			ERROR0("GM_InitGeneralSound failed; throwing OutOfMemoryError\n");
			ThrowJavaOpErrException(e, "java/lang/OutOfMemoryError", opErr);
		}
		// $$jb: changing this to throw LineUnavailableException instead of
		// IllegalArgumentException
		ERROR0("GM_InitGeneralSound failed; throwing LineUnavailableException\n");
		ThrowJavaOpErrException(e, JAVA_SAMPLED_PACKAGE_NAME"/LineUnavailableException", opErr);
	}
	else 
	{
		// save the audio mode variables
		mixerQuality = theQuality;
		mixerTerp = theTerp;
		mixerModifiers = theMods;

		TRACE0("Java_com_sun_media_sound_HeadspaceMixer_nOpenMixer succeeded\n");
	}
}


JNIEXPORT void JNICALL
Java_com_sun_media_sound_HeadspaceMixer_nCloseMixer(JNIEnv* e, jobject thisObj)
{

    TRACE0("Java_com_sun_media_sound_HeadspaceMixer_nCloseMixer.\n");

	// $$kk: 06.21.99: after this, the music globals variable will be NULL.
	// we need to make sure *nothing* can access it after we close the mixer!

	GM_SetAudioTask(NULL);
	GM_StopHardwareSoundManager((void*)e);
	GM_FinisGeneralSound((void*)e);
    
	TRACE0("Java_com_sun_media_sound_HeadspaceMixer_nCloseMixer completed.\n");
}

// added 2nd meaning:
// if sampleSizeInBits = 0 then it returns if there is an audio device accessible at all

JNIEXPORT jboolean JNICALL
Java_com_sun_media_sound_HeadspaceMixer_nSetMixerFormat(JNIEnv* e, jobject thisObj, jint sampleSizeInBits, 
							jint channels, jint sampleRate)
{
	OPErr           opErr = NO_ERR;

	Quality theQuality;
	AudioModifiers theMods;

    TRACE0("Java_com_sun_media_sound_HeadspaceMixer_nSetMixerFormat.\n");
    
    if (sampleSizeInBits == 0) {
    	return (jboolean) (HAE_MaxDevices()>0);
    }

	switch ((int)sampleRate)
	{
	case 11025:
		theQuality = Q_11K;
		break;
	case 22050:
		theQuality = Q_22K;
		break;
	case 44100:
		theQuality = Q_44K;
		break;
	case 48000:
		theQuality = Q_48K;
		break;
	default:
		opErr = PARAM_ERR;
		break;
	} 

	if (opErr == NO_ERR)
	{
		theMods = 0;
		if (sampleSizeInBits == 16) 
		{
			theMods |= M_USE_16;
		}
		if (channels == 2) 
		{
			theMods |= M_USE_STEREO;
		}
		// $$kk: 03.06.98: reverb is enabled by default.  to disable, set flag here in theMods.

		// possible return values: NO_ERR, PARAM_ERR, MEMORY_ERR, 
		opErr = GM_ChangeAudioModes((void*)e, theQuality, mixerTerp, theMods);
	}

	// throw an exception if it didn't work
	if (opErr != NO_ERR)
	{
		ERROR0("GM_ChangeAudioModes failed\n");
		return (jboolean)FALSE;
	}
	else
	{
		// store the new values
		mixerQuality = theQuality;
		mixerModifiers = theMods;
		
		TRACE0("Java_com_sun_media_sound_HeadspaceMixer_nSetMixerFormat succeeded\n");
		return (jboolean)TRUE;
	}
}

/*
JNIEXPORT void JNICALL
Java_com_sun_media_sound_HeadspaceMixer_nOpenDevice(JNIEnv* e, jobject thisObj)
{
	OPErr           opErr;

    TRACE0("Java_com_sun_media_sound_HeadspaceMixer_nOpenDevice.\n");

    // possible return values: NO_ERR, ALREADY_RESUMED, DEVICE_UNAVAILABLE
	opErr = GM_ResumeGeneralSound((void*)e);

	// we don't treat this as an error 
	if (opErr == ALREADY_RESUMED)
    {
        TRACE0("Java_com_sun_media_sound_HeadspaceMixer_nOpenDevice: GM_ResumeGeneralSound returned ALREADY_RESUMED.\n");
		opErr = NO_ERR;
    }

	// if we failed, throw a LineUnavailableException
	if (opErr != NO_ERR)
	{
		TRACE0("GM_ResumeGeneralSound failed; throwing LineUnavailableException\n");
		ThrowJavaOpErrException(e, JAVA_SAMPLED_PACKAGE_NAME"/LineUnavailableException", opErr);
	}
	else 
	{
		TRACE0("Java_com_sun_media_sound_HeadspaceMixer_nOpenDevice succeeded\n");
	}
}


JNIEXPORT void JNICALL
Java_com_sun_media_sound_HeadspaceMixer_nCloseDevice(JNIEnv* e, jobject thisObj)
{
	XBOOL			ok;
	OPErr           opErr;

    TRACE0("Java_com_sun_media_sound_HeadspaceMixer_nCloseDevice.\n");

	GM_PauseGeneralSound((void*)e);	// disconnect from hardware
	//GM_StopHardwareSoundManager((void*)e);	// disconnect from hardware
}
*/


JNIEXPORT void JNICALL
Java_com_sun_media_sound_HeadspaceMixer_nPause(JNIEnv* e, jobject thisObj)
{
	OPErr           opErr;

    TRACE0("Java_com_sun_media_sound_HeadspaceMixer_nPause.\n");

	// $$kk: 06.21.99: set the audio task to NULL now, before pausing the java thread
	GM_SetAudioTask(NULL);
	opErr = GM_PauseGeneralSound((void*)e);

	// we do not treat ALREADY_PAUSED as an error 
    if (opErr == ALREADY_PAUSED)
    {
        TRACE0("Java_com_sun_media_sound_HeadspaceMixer_nPause: GM_PauseGeneralSound returned ALREADY_PAUSED.\n");
    }

    TRACE0("Java_com_sun_media_sound_HeadspaceMixer_nPause returning.\n");	
	return;
}


	
JNIEXPORT void JNICALL
Java_com_sun_media_sound_HeadspaceMixer_nResume(JNIEnv* e, jobject thisObj)
{
	OPErr           opErr;

    TRACE0("Java_com_sun_media_sound_HeadspaceMixer_nResume.\n");

	opErr = GM_ResumeGeneralSound((void*)e);

	// we don't treat this as an error 
	if (opErr == ALREADY_RESUMED)
    {
        TRACE0("Java_com_sun_media_sound_HeadspaceMixer_nResume: GM_ResumeGeneralSound returned ALREADY_RESUMED.\n");
		opErr = NO_ERR;
    }

	// if we failed, it's an error.
	// $$kk: 12.09.98: how to handle this??
	if (opErr != NO_ERR)
	{
		ERROR0("GM_ResumeGeneralSound failed; throwing LineUnavailableException\n");
		ThrowJavaOpErrException(e, JAVA_SAMPLED_PACKAGE_NAME"/LineUnavailableException", opErr);
	}
	else 
	{
		// $$kk: 06.21.99: set the audio task now, after resuming the java thread
		GM_SetAudioTask(AudioTaskCallbackProc);
		TRACE0("Java_com_sun_media_sound_HeadspaceMixer_nResume succeeded\n");
	}


    TRACE0("Java_com_sun_media_sound_HeadspaceMixer_nResume returning.\n");
	
	return;
}


JNIEXPORT void JNICALL
Java_com_sun_media_sound_HeadspaceMixer_nDrain(JNIEnv* e, jobject thisObj)
{
	ERROR0("Java_com_sun_media_sound_HeadspaceMixer_nDrain: NOT IMPLEMENTED! \n");
}


JNIEXPORT void JNICALL
Java_com_sun_media_sound_HeadspaceMixer_nFlush(JNIEnv* e, jobject thisObj)
{
	ERROR0("Java_com_sun_media_sound_HeadspaceMixer_nFlush: NOT IMPLEMENTED! \n");
}


JNIEXPORT jlong JNICALL
Java_com_sun_media_sound_HeadspaceMixer_nGetPosition(JNIEnv* e, jobject thisObj)
{
	ERROR0("Java_com_sun_media_sound_HeadspaceMixer_nGetPosition: NOT IMPLEMENTED! \n");
	return (jlong)0;
}


JNIEXPORT jfloat JNICALL
Java_com_sun_media_sound_HeadspaceMixer_nGetLevel(JNIEnv* e, jobject thisObj)
{
	ERROR0("Java_com_sun_media_sound_HeadspaceMixer_nGetLevel: NOT IMPLEMENTED! \n");
	return (jfloat)0;
}


JNIEXPORT jboolean JNICALL
Java_com_sun_media_sound_HeadspaceMixer_nAllocateVoices(JNIEnv* e, jobject thisObj, jint midiVoices, jint sampledVoices)
{
	OPErr opErr;

	INT16 maxSongVoices; 
	INT16 mixLevel;
	INT16 maxEffectVoices;

    TRACE0("Java_com_sun_media_sound_HeadspaceMixer_nAllocateVoices.\n");

	// void return
	GM_GetSystemVoices(&maxSongVoices, &mixLevel, &maxEffectVoices);

	// possible return values: NO_ERR, PARAM_ERR
	opErr = GM_ChangeSystemVoices((INT16)midiVoices, (INT16)mixLevel, (INT16)sampledVoices);

	return ((opErr != NO_ERR) ? (jboolean)TRUE : (jboolean)FALSE);
}


JNIEXPORT jboolean JNICALL
Java_com_sun_media_sound_HeadspaceMixer_nSetMixLevel(JNIEnv* e, jobject thisObj, jint newMixLevel)
{
	OPErr opErr;

	INT16 maxSongVoices; 
	INT16 mixLevel;
	INT16 maxEffectVoices;

    TRACE0("Java_com_sun_media_sound_HeadspaceMixer_nSetMixLevel.\n");

	// void return
	GM_GetSystemVoices(&maxSongVoices, &mixLevel, &maxEffectVoices);

	// possible return values: NO_ERR, PARAM_ERR
	opErr = GM_ChangeSystemVoices(maxSongVoices, (INT16)newMixLevel, maxEffectVoices);

	return ((opErr != NO_ERR) ? (jboolean)TRUE : (jboolean)FALSE);
}


JNIEXPORT jboolean JNICALL
Java_com_sun_media_sound_HeadspaceMixer_nSetInterpolation(JNIEnv* e, jobject thisObj, jint terpMode)
{
	OPErr opErr = NO_ERR;
	TerpMode theTerp;

    TRACE0("Java_com_sun_media_sound_HeadspaceMixer_nSetInterpolation.\n");

	switch((int)terpMode)
	{
	case (int)E_AMP_SCALED_DROP_SAMPLE:
	case (int)E_2_POINT_INTERPOLATION:
	case (int)E_LINEAR_INTERPOLATION:
		theTerp = (int)terpMode;
		break;
	default:
		opErr = PARAM_ERR;
		break;
	}

	if (opErr == NO_ERR)
	{
		// possible return values: NO_ERR, PARAM_ERR, MEMORY_ERR, 
		opErr = GM_ChangeAudioModes((void*)e, mixerQuality, theTerp, mixerModifiers);
	}

	return ((opErr != NO_ERR) ? (jboolean)TRUE : (jboolean)FALSE);
}


JNIEXPORT jfloat JNICALL
Java_com_sun_media_sound_HeadspaceMixer_nGetCpuLoad(JNIEnv* e, jobject thisObj)
{
	ERROR0("Java_com_sun_media_sound_HeadspaceMixer_nGetCpuLoad: NOT IMPLEMENTED! \n");
	return (jfloat)0;
}


JNIEXPORT jint JNICALL Java_com_sun_media_sound_HeadspaceMixer_nGetTotalVoices(JNIEnv *e, jclass thisClass) 
{
	return (jint)MAX_VOICES;
}


JNIEXPORT jint JNICALL
Java_com_sun_media_sound_HeadspaceMixer_nGetDefaultBufferSize(JNIEnv* e, jclass thisClass)
{
	return (jint)(HAE_GetMaxSamplePerSlice() * HAE_GetAudioBufferCount());
}


JNIEXPORT jint JNICALL
Java_com_sun_media_sound_HeadspaceMixer_nSetReverb(JNIEnv* e, jclass thisClass, jint reverbMode)
{
	// ReverbMode is a typedef char
    ReverbMode mode;

	TRACE1("Java_com_sun_media_sound_HeadspaceMixer_nSetReverb: %d\n", reverbMode);

	mode = (ReverbMode)reverbMode;
	
	GM_SetReverbType(mode);

	return (jint)GM_GetReverbType();
}



JNIEXPORT jlong JNICALL
Java_com_sun_media_sound_HeadspaceMixer_nCreateLinkedStreams(JNIEnv* e, jobject thisObj, jlongArray idArray)
{
	OPErr theErr;
	jsize len;
	jlong *body;
	
	LINKED_STREAM_REFERENCE top = NULL;
	LINKED_STREAM_REFERENCE link;

	int i;

    TRACE0("Java_com_sun_media_sound_HeadspaceMixer_nCreateLinkedStreams.\n");

	len = (*e)->GetArrayLength(e, idArray);
	body = (*e)->GetLongArrayElements(e, idArray, 0);

	for (i = 0; i < len; i++) {
		
		// preroll the stream
		theErr = GM_AudioStreamPreroll((STREAM_REFERENCE) (INT_PTR) (body[i]));

		// if we fail to preroll any one, break and return failure
		if (theErr != NO_ERR)
		{
			top = NULL;
			break;
		}

		// add the stream to the list
		link = GM_NewLinkedStreamList((STREAM_REFERENCE) (INT_PTR) (body[i]), (void *)e);
		top = GM_AddLinkedStream(top, link);

		// if we ever get NULL, break and return failure
		if (top == NULL) 
		{
			break;
		}
	}

	(*e)->ReleaseLongArrayElements(e, idArray, body, 0);

	return (jlong) (INT_PTR) top;
}


JNIEXPORT jboolean JNICALL
Java_com_sun_media_sound_HeadspaceMixer_nStartLinkedStreams(JNIEnv* e, jobject thisObj, jlong linkRef)
{
	OPErr opErr;

    TRACE0("Java_com_sun_media_sound_HeadspaceMixer_nStartLinkedStreams.\n");

	opErr = GM_StartLinkedStreams((LINKED_STREAM_REFERENCE) (INT_PTR) linkRef);

	return (opErr == NO_ERR ? (jboolean)TRUE : (jboolean)FALSE);
}


JNIEXPORT void JNICALL
Java_com_sun_media_sound_HeadspaceMixer_nStopLinkedStreams(JNIEnv* e, jobject thisObj, jlong linkRef)
{
    TRACE0("Java_com_sun_media_sound_HeadspaceMixer_nStopLinkedStreams.\n");

	GM_EndLinkedStreams((LINKED_STREAM_REFERENCE) (INT_PTR) linkRef);
	GM_FreeLinkedStreamList((LINKED_STREAM_REFERENCE) (INT_PTR) linkRef);
}




/*

JNIEXPORT jfloat JNICALL
Java_com_sun_media_sound_HeadspaceMixer_nSetLinearGain(JNIEnv* e, jobject thisObj, jfloat linearGain)
{
    TRACE1("Java_com_sun_media_sound_HeadspaceMixer_nSetLinearGain: %f.\n", linearGain);

    GM_SetMasterVolume((unsigned long)(linearGain * MAX_MASTER_VOLUME));

    TRACE1("Java_com_sun_media_sound_HeadspaceMixer_nSetLinearGain returning: %d.\n", (GM_GetMasterVolume() / MAX_MASTER_VOLUME));
	
	return (jfloat)GM_GetMasterVolume() / MAX_MASTER_VOLUME;
}


JNIEXPORT jfloat JNICALL
Java_com_sun_media_sound_HeadspaceMixer_nSetPan(JNIEnv* e, jobject thisObj, jfloat pan) 
{
    TRACE1("Java_com_sun_media_sound_HeadspaceMixer_nSetPan: %f.\n", pan);

    ERROR0("Java_com_sun_media_sound_HeadspaceMixer_nSetPan NOT IMPLEMENTED.\n");

	// need to implement!
	pan = 0.0f;
	
	TRACE1("Java_com_sun_media_sound_HeadspaceMixer_nSetPan: returning %f.\n", pan);
	return pan;
}


JNIEXPORT void JNICALL
Java_com_sun_media_sound_HeadspaceMixer_nSetAudioFormat(JNIEnv* e, jobject thisObj, jfloat sampleRate, jint sampleSizeInBits, jint channels)
{
	OPErr           opErr;

	Quality theQuality;
	TerpMode theTerp;
	AudioModifiers theMods;

    TRACE0("Java_com_sun_media_sound_HeadspaceMixer_nSetAudioFormat.\n");

	switch ((int)sampleRate)
	{
	case 11025:
		theQuality = Q_11K;
		break;
	case 22050:
		theQuality = Q_22K;
		break;
	case 44100:
		theQuality = Q_44K;
		break;
	case 48000:
		theQuality = Q_48K;
		break;
	default:
		opErr = PARAM_ERR;
		break;
	} 

	// throw an exception if sampleRate not valid
	if (opErr == NO_ERR)
	{
		// $$kk: need to either make this accessible (configurable?) from java, or at least make
		// a #define for JAVA_SOUND....
		theTerp = JAVA_SOUND_INTERPOLATION_MODE;

		theMods = 0;
		if (sampleSizeInBits == 16) 
		{
			theMods |= M_USE_16;
		}
		if (channels == 2) 
		{
			theMods |= M_USE_STEREO;
		}
		// $$kk: 03.06.98: reverb is enabled by default.  to disable, set flag here in theMods.

		// possible return values: NO_ERR, PARAM_ERR, MEMORY_ERR, 
		opErr = GM_ChangeAudioModes((void*)e, theQuality, theTerp, theMods);

		// now update the output proc array size, if necessary.
		// $$kk: 10.02.98: definite timing catastrophe waiting for us here....
		if (outputProcByteArray != NULL)
		{
			int newArrayLength = HAE_GetMaxSamplePerSlice() * ((MusicGlobals->generate16output) ? 2 : 1) * ((MusicGlobals->generateStereoOutput) ? 2 : 1);
			int oldArrayLength = (*e)->GetArrayLength(e, outputProcByteArray);

			if (newArrayLength != oldArrayLength) 
			{
				jbyteArray localArray = (*e)->NewByteArray(e, (jsize)newArrayLength);
				(*e)->DeleteGlobalRef(e, outputProcByteArray);
				outputProcByteArray = (*e)->NewGlobalRef(e, localArray);
			}
		}		
	}

	// throw an exception if it didn't work
	if (opErr != NO_ERR)
	{
		ERROR0("GM_ChangeAudioModes failed; throwing EngineException\n");
		ThrowJavaOpErrException(e, IMPLEMENTATION_PACKAGE_NAME"/EngineException", opErr);
	}
	else
	{
		TRACE0("Java_com_sun_media_sound_HeadspaceMixer_nSetAudioFormat succeeded\n");
	}
}


JNIEXPORT jboolean JNICALL
Java_com_sun_media_sound_HeadspaceMixer_nSupportsSampleRate(JNIEnv* e, jobject thisObj, jfloat sampleRate) 
{
    TRACE1("Java_com_sun_media_sound_HeadspaceMixer_nSupportsSampleRate: %f.\n", sampleRate);

	if ( (sampleRate == 8000) || (sampleRate == 11025) || (sampleRate == 22050) || (sampleRate == 44100) || (sampleRate == 48000) ) 
	{
		TRACE1("Java_com_sun_media_sound_HeadspaceMixer_nSupportsSampleRate: %f returning TRUE.\n", sampleRate);
		return (jboolean)TRUE;
	}

	TRACE1("Java_com_sun_media_sound_HeadspaceMixer_nSupportsSampleRate: %f returning FALSE.\n", sampleRate);
	return (jboolean)FALSE;
}


JNIEXPORT jboolean JNICALL
Java_com_sun_media_sound_HeadspaceMixer_nSupportsSampleSizeInBits(JNIEnv* e, jobject thisObj, jint sampleSizeInBits) 
{
    TRACE1("Java_com_sun_media_sound_HeadspaceMixer_nSupportsSampleSizeInBits: %d.\n", sampleSizeInBits);

	if (sampleSizeInBits == 16)
	{
		TRACE2("Java_com_sun_media_sound_HeadspaceMixer_nSupportsSampleSizeInBits: %d returning %d.\n", sampleSizeInBits, (XIs16BitSupported()));
		return (jboolean)XIs16BitSupported();
	}

	if (sampleSizeInBits == 8)
	{
		TRACE2("Java_com_sun_media_sound_HeadspaceMixer_nSupportsSampleSizeInBits: %d returning %d.\n", sampleSizeInBits, (XIs8BitSupported()));
		return (jboolean)XIs8BitSupported();
	}

	TRACE1("Java_com_sun_media_sound_HeadspaceMixer_nSupportsSampleSizeInBits: %d returning FALSE.\n", sampleSizeInBits);
	return (jboolean)FALSE;
}


JNIEXPORT jboolean JNICALL
Java_com_sun_media_sound_HeadspaceMixer_nSupportsChannels(JNIEnv* e, jobject thisObj, jint channels) 
{

	if (channels == 1)
		return (jboolean)TRUE;

	if (channels == 2)
		return (jboolean)XIsStereoSupported();

	return (jboolean)FALSE;
}


// SET THE SOUNDBANK
JNIEXPORT jboolean JNICALL
Java_com_sun_media_sound_HeadspaceMixer_nSetSoundbank(JNIEnv* e, jobject thisObj, jstring soundbankPath)
{
	XFILE file = NULL;
	XFILENAME xfilename;

	const char *str = (*e)->GetStringUTFChars(e, soundbankPath, 0);

	TRACE0("Java_com_sun_media_sound_HeadspaceMixer_nSetSoundbank.\n");

	XConvertNativeFileToXFILENAME((void *)str, &xfilename);
	file = XFileOpenResource(&xfilename, TRUE);
	if (file)
	{
		XFileUseThisResourceFile(file);
	}
	else
	{
		ERROR0("ERROR in Java_com_sun_media_sound_HeadspaceMixer_nSetSoundbank\n");
	}

	(*e)->ReleaseStringUTFChars(e, soundbankPath, str);

	TRACE0("Java_com_sun_media_sound_HeadspaceMixer_nSetSoundbank completed.\n");

	return (file ? (jboolean)TRUE : (jboolean)FALSE);
}


JNIEXPORT void JNICALL
Java_com_sun_media_sound_HeadspaceMixer_nSetOutputCallback(JNIEnv* e, jobject thisObj, jboolean doCallback)
{

	TRACE1("Java_com_sun_media_sound_HeadspaceMixer_nSetOutputCallback, doCallback: %d.\n", doCallback);

	if (doCallback == TRUE)
	{
	
		// set up audio output callback proc stuff.
		// $$kk: 09.28.98: need to find a way to delete global references!
		int arrayLength;
		jbyteArray localArray;

		// save a global ref to the java system device object.
		// need this for calling the output proc callback
		jclass localClass;		
		globalSystemDeviceObj = (*e)->NewGlobalRef(e, thisObj);			
		
		// save a global ref to the java system device object *class*.
		// need this to keep the class from getting unloaded and invalidating the method id
		localClass = (*e)->GetObjectClass(e, thisObj);
		globalSystemDeviceClass = (*e)->NewGlobalRef(e, localClass);	

		// save the output proc method id.  this is valid as long as the class is not unloaded.
		callbackOutputProcMethodID = (*e)->GetMethodID(e, globalSystemDeviceClass, "callbackOutputProc", "([BI)V");

		// calculate the array length we need for the callbacks.
		// $$kk: this can change whenever the output format changes (GM_ChangeAudioModes)!!
		arrayLength = HAE_GetMaxSamplePerSlice() * ((MusicGlobals->generate16output) ? 2 : 1) * ((MusicGlobals->generateStereoOutput) ? 2 : 1);
		localArray = (*e)->NewByteArray(e, (jsize)arrayLength);
		outputProcByteArray = (*e)->NewGlobalRef(e, localArray);
		
		// start the output callback proc
 		TRACE3("Calling GM_SetAudioOutput with globalSystemDeviceObj: %lu, globalSystemDeviceClass: %lu, callbackOutputProcMethodID: %lu\n", globalSystemDeviceObj, globalSystemDeviceClass, callbackOutputProcMethodID);
 		TRACE2("outputProcByteArray: %lu, arrayLength: %d\n", outputProcByteArray, arrayLength);

		GM_SetAudioOutput(AudioOutputCallbackProc);
	}
	else
	{
		// stop the output callback proc
        GM_SetAudioOutput(NULL);

		// delete the global refs and the output proc array
		(*e)->DeleteGlobalRef(e, globalSystemDeviceClass);
		(*e)->DeleteGlobalRef(e, globalSystemDeviceObj);
		(*e)->DeleteGlobalRef(e, outputProcByteArray);
		globalSystemDeviceClass = NULL;
		globalSystemDeviceObj = NULL;
		callbackOutputProcMethodID = NULL;

		// $$kk: 10.02.98: how to delete this??
		outputProcByteArray = NULL;
	}

	TRACE0("Java_com_sun_media_sound_HeadspaceMixer_nSetOutputCallback completed.\n");
}


JNIEXPORT jint JNICALL
Java_com_sun_media_sound_HeadspaceMixer_nGetCurrentData(JNIEnv* e, jobject thisObj, jbyteArray leftArray, jbyteArray rightArray, jint arrayLength)
{
	jbyte *leftBytes;
	jbyte *rightBytes;
	jint frames;
	
	TRACE0("Java_com_sun_media_sound_HeadspaceMixer_nGetCurrentData.\n");

	leftBytes = (*e)->GetByteArrayElements(e, leftArray, 0);
	rightBytes = (*e)->GetByteArrayElements(e, rightArray, 0);

	frames = (jint)GM_GetAudioSampleFrame((INT16 *)leftBytes, (INT16 *)rightBytes);

	(*e)->ReleaseByteArrayElements(e, leftArray, leftBytes, 0);
	(*e)->ReleaseByteArrayElements(e, rightArray, rightBytes, 0);

	TRACE1("Java_com_sun_media_sound_HeadspaceMixer_nGetCurrentData returning %d.\n", frames);

	// $$kk: 12.08.98: returns the number of frames, not bytes.  
	// we need some work to clean up the format confusion.
	return frames;
}


JNIEXPORT jlong JNICALL
Java_com_sun_media_sound_HeadspaceMixer_nGetEngineLatencyInMicroseconds(JNIEnv* e, jobject thisObj)
{
	return (jlong)(GM_GetSyncTimeStampQuantizedAhead() - GM_GetSyncTimeStamp());
}
*/
