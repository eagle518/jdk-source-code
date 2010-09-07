/*
 * @(#)Platform.c	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

// Utilities includes
#include "Utilities.h"

// Platform.java includes
#include "com_sun_media_sound_Platform.h"

/*
 * Class:     com_sun_media_sound_Platform
 * Method:    nIsBigEndian
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_sun_media_sound_Platform_nIsBigEndian(JNIEnv *env, jclass clss) {
    return UTIL_IsBigEndianPlatform();
}

/*
 * Class:     com_sun_media_sound_Platform
 * Method:    nIsSigned8
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_sun_media_sound_Platform_nIsSigned8(JNIEnv *env, jclass clss) {
#if ((CPU_TYPE == kSPARC) || (CPU_TYPE == kSPARCV9))
    return 1;
#else
    return 0;
#endif
}

/*
 * Class:     com_sun_media_sound_Platform
 * Method:    nGetExtraLibraries
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_sun_media_sound_Platform_nGetExtraLibraries(JNIEnv *env, jclass clss) {
    return (*env)->NewStringUTF(env, EXTRA_SOUND_JNI_LIBS);
}

/*
 * Class:     com_sun_media_sound_Platform
 * Method:    nGetLibraryForFeature
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_sun_media_sound_Platform_nGetLibraryForFeature
  (JNIEnv *env, jclass clazz, jint feature) {
  	
// for every OS
#if X_PLATFORM == X_WINDOWS
    switch (feature) {
    case com_sun_media_sound_Platform_FEATURE_MIDIIO:       
	return com_sun_media_sound_Platform_LIB_MAIN;
    case com_sun_media_sound_Platform_FEATURE_PORTS:
	return com_sun_media_sound_Platform_LIB_MAIN;
    case com_sun_media_sound_Platform_FEATURE_DIRECT_AUDIO:
	return com_sun_media_sound_Platform_LIB_DSOUND;
    }
#endif
#if (X_PLATFORM == X_SOLARIS)
    switch (feature) {
    case com_sun_media_sound_Platform_FEATURE_MIDIIO:
	return com_sun_media_sound_Platform_LIB_SOLMIDI;
    case com_sun_media_sound_Platform_FEATURE_PORTS:
	return com_sun_media_sound_Platform_LIB_MAIN;
    case com_sun_media_sound_Platform_FEATURE_DIRECT_AUDIO:
	return com_sun_media_sound_Platform_LIB_MAIN;
    }
#endif
#if (X_PLATFORM == X_LINUX)
    switch (feature) {
    case com_sun_media_sound_Platform_FEATURE_MIDIIO:
	return com_sun_media_sound_Platform_LIB_ALSA;
    case com_sun_media_sound_Platform_FEATURE_PORTS:
	return com_sun_media_sound_Platform_LIB_ALSA;
    case com_sun_media_sound_Platform_FEATURE_DIRECT_AUDIO:
	return com_sun_media_sound_Platform_LIB_ALSA;
    }
#endif
    return 0;
}
