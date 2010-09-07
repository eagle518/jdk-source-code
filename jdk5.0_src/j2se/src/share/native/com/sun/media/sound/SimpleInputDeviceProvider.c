/*
 * @(#)SimpleInputDeviceProvider.c	1.11 03/12/19
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

// SimpleInputDeviceProvider includes
#include "com_sun_media_sound_SimpleInputDeviceProvider.h"

// $$kk: 03.29.99: need to figure out how long the name might be.
// for win32, is is MAXPNAMELEN = 32, as defined in mmsystem.h.
// what about solaris??
#define MAX_STRING_LENGTH 128


JNIEXPORT jint JNICALL
    Java_com_sun_media_sound_SimpleInputDeviceProvider_nGetNumDevices(JNIEnv* e, jobject thisObj)
{
    INT32 numDevices;
	
    TRACE0("Java_com_sun_media_sound_SimpleInputDeviceProvider_nGetNumDevices.\n");

    numDevices = HAE_MaxCaptureDevices();

    TRACE1("Java_com_sun_media_sound_SimpleInputDeviceProvider_nGetNumDevices returning %d.\n", numDevices);

    return numDevices;
}


JNIEXPORT jstring JNICALL
    Java_com_sun_media_sound_SimpleInputDeviceProvider_nGetName(JNIEnv* e, jobject thisObj, jint index)
{
    char devName[MAX_STRING_LENGTH];
    jstring jDevName;

    TRACE0("Java_com_sun_media_sound_SimpleInputDeviceProvider_nGetName.\n");

    HAE_GetCaptureDeviceName((INT32)index, devName, (UINT32)MAX_STRING_LENGTH);

    // $$jb: 11.15.99: HAE_GetCaptureDeviceName() does not return a Pascal string
    //       on either platform, as it is currently implemented
    //XPtoCstr(devName);
    jDevName = (*e)->NewStringUTF(e, devName);

    TRACE0("Java_com_sun_media_sound_SimpleInputDeviceProvider_nGetName completed.\n");

    return jDevName;
}


JNIEXPORT jstring JNICALL
    Java_com_sun_media_sound_SimpleInputDeviceProvider_nGetVendor(JNIEnv* e, jobject thisObj, jint index)
{
    char vendorName[MAX_STRING_LENGTH] = "Unknown Vendor";
    jstring jVendorName;

    TRACE0("Java_com_sun_media_sound_SimpleInputDeviceProvider_nGetVendor.\n");

    /* $$kk: 06.03.99: need to implement */
	
    // $$jb: 11.15.99: vendorName is not a Pascal string
    //XPtoCstr(vendorName);
    jVendorName = (*e)->NewStringUTF(e, vendorName);

    TRACE0("Java_com_sun_media_sound_SimpleInputDeviceProvider_nGetVendor completed.\n");

    return jVendorName;
}


JNIEXPORT jstring JNICALL
    Java_com_sun_media_sound_SimpleInputDeviceProvider_nGetDescription(JNIEnv* e, jobject thisObj, jint index)
{
    char description[MAX_STRING_LENGTH] = "No details available";
    jstring jDescription;

    TRACE0("Java_com_sun_media_sound_SimpleInputDeviceProvider_nGetDescription.\n");

    /* $$kk: 06.03.99: need to implement */
	
    // $$jb: 11.15.99: description is not a Pascal string
    //XPtoCstr(description);
    jDescription = (*e)->NewStringUTF(e, description);

    TRACE0("Java_com_sun_media_sound_SimpleInputDeviceProvider_nGetDescription completed.\n");

    return jDescription;
}


JNIEXPORT jstring JNICALL
    Java_com_sun_media_sound_SimpleInputDeviceProvider_nGetVersion(JNIEnv* e, jobject thisObj, jint index)
{
    char version[MAX_STRING_LENGTH] = "Unknown Version";
    jstring jVersion;

    TRACE0("Java_com_sun_media_sound_SimpleInputDeviceProvider_nGetVersion.\n");

    /* $$kk: 06.03.99: need to implement */
	
    // $$jb: 11.15.99: version is not a Pascal string
    //XPtoCstr(version);
    jVersion = (*e)->NewStringUTF(e, version);

    TRACE0("Java_com_sun_media_sound_SimpleInputDeviceProvider_nGetVersion completed.\n");

    return jVersion;
}
