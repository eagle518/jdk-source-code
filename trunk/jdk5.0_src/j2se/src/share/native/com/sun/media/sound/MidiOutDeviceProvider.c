/*
 * @(#)MidiOutDeviceProvider.c	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


//#define USE_ERROR
//#define USE_TRACE

// STANDARD includes

// JNI includes
#include <jni.h>

// for types
#include "engine/X_API.h"

// Platform MIDI includes
#include "PlatformMidi.h"

// UTILITY includes
#include "Utilities.h"

// for strcpy
#include <string.h>

// SimpleInputDeviceProvider includes
#include "com_sun_media_sound_MidiOutDeviceProvider.h"

#define MAX_STRING_LENGTH 128


JNIEXPORT jint JNICALL
Java_com_sun_media_sound_MidiOutDeviceProvider_nGetNumDevices(JNIEnv* e, jobject thisObj) {

    INT32 numDevices = 0;
	
    TRACE0("Java_com_sun_media_sound_MidiOutDeviceProvider_nGetNumDevices.\n");

#if USE_PLATFORM_MIDI_OUT == TRUE
    numDevices = MIDI_OUT_GetNumDevices();
#endif

    TRACE1("Java_com_sun_media_sound_MidiOutDeviceProvider_nGetNumDevices returning %d.\n", (int) numDevices);
    return (jint) numDevices;
}


JNIEXPORT jstring JNICALL
Java_com_sun_media_sound_MidiOutDeviceProvider_nGetName(JNIEnv* e, jobject thisObj, jint index) {

    char name[MAX_STRING_LENGTH + 1];
    jstring jString = NULL;

    TRACE0("Java_com_sun_media_sound_MidiOutDeviceProvider_nGetName.\n");
    name[0] = 0;

#if USE_PLATFORM_MIDI_OUT == TRUE
    MIDI_OUT_GetDeviceName((INT32)index, name, (UINT32)MAX_STRING_LENGTH);
#endif

    if (name[0] == 0) {
	strcpy(name, "Unknown name");
    }
    jString = (*e)->NewStringUTF(e, name);
    TRACE0("Java_com_sun_media_sound_MidiOutDeviceProvider_nGetName completed.\n");
    return jString;
}


JNIEXPORT jstring JNICALL
Java_com_sun_media_sound_MidiOutDeviceProvider_nGetVendor(JNIEnv* e, jobject thisObj, jint index) {

    char name[MAX_STRING_LENGTH + 1];
    jstring jString = NULL;

    TRACE0("Java_com_sun_media_sound_MidiOutDeviceProvider_nGetVendor.\n");
    name[0] = 0;

#if USE_PLATFORM_MIDI_OUT == TRUE
    MIDI_OUT_GetDeviceVendor((INT32)index, name, (UINT32)MAX_STRING_LENGTH);
#endif

    if (name[0] == 0) {
	strcpy(name, "Unknown vendor");
    }
    jString = (*e)->NewStringUTF(e, name);
    TRACE0("Java_com_sun_media_sound_MidiOutDeviceProvider_nGetVendor completed.\n");
    return jString;
}


JNIEXPORT jstring JNICALL
Java_com_sun_media_sound_MidiOutDeviceProvider_nGetDescription(JNIEnv* e, jobject thisObj, jint index) {

    char name[MAX_STRING_LENGTH + 1];
    jstring jString = NULL;

    TRACE0("Java_com_sun_media_sound_MidiOutDeviceProvider_nGetDescription.\n");
    name[0] = 0;

#if USE_PLATFORM_MIDI_OUT == TRUE
    MIDI_OUT_GetDeviceDescription((INT32)index, name, (UINT32)MAX_STRING_LENGTH);
#endif

    if (name[0] == 0) {
	strcpy(name, "No details available");
    }
    jString = (*e)->NewStringUTF(e, name);
    TRACE0("Java_com_sun_media_sound_MidiOutDeviceProvider_nGetDescription completed.\n");
    return jString;
}


JNIEXPORT jstring JNICALL
Java_com_sun_media_sound_MidiOutDeviceProvider_nGetVersion(JNIEnv* e, jobject thisObj, jint index) {

    char name[MAX_STRING_LENGTH + 1];
    jstring jString = NULL;

    TRACE0("Java_com_sun_media_sound_MidiOutDeviceProvider_nGetVersion.\n");
    name[0] = 0;

#if USE_PLATFORM_MIDI_OUT == TRUE
    MIDI_OUT_GetDeviceVersion((INT32)index, name, (UINT32)MAX_STRING_LENGTH);
#endif
    if (name[0] == 0) {
	strcpy(name, "Unknown version");
    }
    jString = (*e)->NewStringUTF(e, name);

    TRACE0("Java_com_sun_media_sound_MidiOutDeviceProvider_nGetVersion completed.\n");

    return jString;
}
