/*
 * @(#)PortMixerProvider.c	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//#define USE_TRACE

// STANDARD includes

// JNI includes
#include <jni.h>

// for type definitions
#include "engine/X_API.h"

// Port includes
#include "Ports.h"

// UTILITY includes
#include "Utilities.h"

// PortMixerProvider includes
#include "com_sun_media_sound_PortMixerProvider.h"

//////////////////////////////////////////// PortMixerProvider ////////////////////////////////////////////

int getPortMixerDescription(int mixerIndex, PortMixerDescription* desc) {
    strcpy(desc->name, "Unknown Name");
    strcpy(desc->vendor, "Unknown Vendor");
    strcpy(desc->description, "Port Mixer");
    strcpy(desc->version, "Unknown Version");
#if USE_PORTS == TRUE
    PORT_GetPortMixerDescription(mixerIndex, desc);
#endif // USE_PORTS
    return TRUE;
}

JNIEXPORT jint JNICALL Java_com_sun_media_sound_PortMixerProvider_nGetNumDevices(JNIEnv *env, jclass cls) {
    INT32 numDevices = 0;

    TRACE0("Java_com_sun_media_sound_PortMixerProvider_nGetNumDevices.\n");

#if USE_PORTS == TRUE
    numDevices = PORT_GetPortMixerCount();
#endif // USE_PORTS

    TRACE1("Java_com_sun_media_sound_PortMixerProvider_nGetNumDevices returning %d.\n", (int) numDevices);

    return (jint)numDevices;
}

JNIEXPORT jobject JNICALL Java_com_sun_media_sound_PortMixerProvider_nNewPortMixerInfo(JNIEnv *env, jclass cls, jint mixerIndex) {
    jclass portMixerInfoClass;
    jmethodID portMixerInfoConstructor;
    PortMixerDescription desc;
    jobject info = NULL;
    TRACE1("Java_com_sun_media_sound_PortMixerProvider_nNewPortMixerInfo(%d).\n", mixerIndex);

    // retrieve class and constructor of PortMixerProvider.PortMixerInfo
    portMixerInfoClass = (*env)->FindClass(env, IMPLEMENTATION_PACKAGE_NAME"/PortMixerProvider$PortMixerInfo");
    if (portMixerInfoClass == NULL) {
	ERROR0("Java_com_sun_media_sound_PortMixerProvider_nNewPortMixerInfo: portMixerInfoClass is NULL\n");
	return NULL;
    }
    portMixerInfoConstructor = (*env)->GetMethodID(env, portMixerInfoClass, "<init>",
                  "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
    if (portMixerInfoConstructor == NULL) {
	ERROR0("Java_com_sun_media_sound_PortMixerProvider_nNewPortMixerInfo: portMixerInfoConstructor is NULL\n");
	return NULL;
    }

    if (getPortMixerDescription(mixerIndex, &desc)) {
	// create a new PortMixerInfo object and return it
	info = (*env)->NewObject(env, portMixerInfoClass, portMixerInfoConstructor, mixerIndex,
	                         (*env)->NewStringUTF(env, desc.name),
	                         (*env)->NewStringUTF(env, desc.vendor),
	                         (*env)->NewStringUTF(env, desc.description),
	                         (*env)->NewStringUTF(env, desc.version));
    }

    TRACE0("Java_com_sun_media_sound_PortMixerProvider_nNewPortMixerInfo succeeded.\n");
    return info;
}

