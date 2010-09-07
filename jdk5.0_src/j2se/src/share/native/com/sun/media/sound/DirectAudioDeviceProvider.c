/*
 * @(#)DirectAudioDeviceProvider.c	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//#define USE_TRACE
//#define USE_ERROR

// STANDARD includes

// JNI includes
#include <jni.h>

// for type definitions
#include "engine/X_API.h"

// DirectAudio includes
#include "DirectAudio.h"

// UTILITY includes
#include "Utilities.h"

// DirectAudioDeviceProvider includes
#include "com_sun_media_sound_DirectAudioDeviceProvider.h"

//////////////////////////////////////////// DirectAudioDeviceProvider ////////////////////////////////////////////

int getDirectAudioDeviceDescription(int mixerIndex, DirectAudioDeviceDescription* desc) {
    desc->deviceID = 0;
    desc->maxSimulLines = 0;
    strcpy(desc->name, "Unknown Name");
    strcpy(desc->vendor, "Unknown Vendor");
    strcpy(desc->description, "Unknown Description");
    strcpy(desc->version, "Unknown Version");
#if USE_DAUDIO == TRUE
    DAUDIO_GetDirectAudioDeviceDescription(mixerIndex, desc);
#endif // USE_DAUDIO
    return TRUE;
}

JNIEXPORT jint JNICALL Java_com_sun_media_sound_DirectAudioDeviceProvider_nGetNumDevices(JNIEnv *env, jclass cls) {
    INT32 numDevices = 0;

    TRACE0("Java_com_sun_media_sound_DirectAudioDeviceProvider_nGetNumDevices.\n");

#if USE_DAUDIO == TRUE
    numDevices = DAUDIO_GetDirectAudioDeviceCount();
#endif // USE_DAUDIO

    TRACE1("Java_com_sun_media_sound_DirectAudioDeviceProvider_nGetNumDevices returning %d.\n", (int) numDevices);

    return (jint)numDevices;
}

JNIEXPORT jobject JNICALL Java_com_sun_media_sound_DirectAudioDeviceProvider_nNewDirectAudioDeviceInfo
    (JNIEnv *env, jclass cls, jint mixerIndex) {

    jclass directAudioDeviceInfoClass;
    jmethodID directAudioDeviceInfoConstructor;
    DirectAudioDeviceDescription desc;
    jobject info = NULL;
    TRACE1("Java_com_sun_media_sound_DirectAudioDeviceProvider_nNewDirectAudioDeviceInfo(%d).\n", mixerIndex);

    // retrieve class and constructor of DirectAudioDeviceProvider.DirectAudioDeviceInfo
    directAudioDeviceInfoClass = (*env)->FindClass(env, IMPLEMENTATION_PACKAGE_NAME"/DirectAudioDeviceProvider$DirectAudioDeviceInfo");
    if (directAudioDeviceInfoClass == NULL) {
	ERROR0("Java_com_sun_media_sound_DirectAudioDeviceProvider_nNewDirectAudioDeviceInfo: directAudioDeviceInfoClass is NULL\n");
	return NULL;
    }
    directAudioDeviceInfoConstructor = (*env)->GetMethodID(env, directAudioDeviceInfoClass, "<init>",
                  "(IIILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
    if (directAudioDeviceInfoConstructor == NULL) {
	ERROR0("Java_com_sun_media_sound_DirectAudioDeviceProvider_nNewDirectAudioDeviceInfo: directAudioDeviceInfoConstructor is NULL\n");
	return NULL;
    }

    TRACE1("Get description for device %d\n", mixerIndex);

    if (getDirectAudioDeviceDescription(mixerIndex, &desc)) {
	// create a new DirectAudioDeviceInfo object and return it
	info = (*env)->NewObject(env, directAudioDeviceInfoClass, directAudioDeviceInfoConstructor, 
				 mixerIndex, 
	                         desc.deviceID,
	                         desc.maxSimulLines,
	                         (*env)->NewStringUTF(env, desc.name),
	                         (*env)->NewStringUTF(env, desc.vendor),
	                         (*env)->NewStringUTF(env, desc.description),
	                         (*env)->NewStringUTF(env, desc.version));
    } else {
	ERROR1("ERROR: getDirectAudioDeviceDescription(%d, desc) returned FALSE!\n", mixerIndex);
    }

    TRACE0("Java_com_sun_media_sound_DirectAudioDeviceProvider_nNewDirectAudioDeviceInfo succeeded.\n");
    return info;
}

