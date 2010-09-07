/*
 * @(#)Utilities.c	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


// Utilities includes
#include "Utilities.h"


/* $$fb note that HAE.cpp keeps its own midiSongCount counter. 
 * I don't know why.
 */
static XShortResourceID midiSongCount = 0; // everytime a new song is loaded, this increments

XShortResourceID getMidiSongCount() {
    return ++midiSongCount;
}


void ThrowJavaMessageException(JNIEnv* e, char const* exception, char const* message) {
    jclass newExcCls;

    ERROR1("throw exception: %s\n", message);
    newExcCls = (*e)->FindClass(e, exception);													  
    if (newExcCls == 0) {
	/* Unable to find the new exception class, give up. */
	ERROR0("ThrowJavaMessageException unable to find class!\n");
	return;
    }
    (*e)->ThrowNew(e, newExcCls, message);
}


void ThrowJavaOpErrException(JNIEnv* e, char const* exception, OPErr opErr) {
    ThrowJavaMessageException(e, exception, TranslateOPErr(opErr));	
}


void SleepMillisInJava(JNIEnv* e, INT32 millis) {
    jclass threadClass;
    jmethodID sleepMethodID;

    threadClass = (*e)->FindClass(e, "java/lang/Thread");
    sleepMethodID = (*e)->GetStaticMethodID(e, threadClass, "sleep", "(J)V");

    if (sleepMethodID == 0) {
	ERROR0("SleepMillisInJava: Failed to get thread sleep method ID\n");
	return;
    }
    (*e)->CallStaticVoidMethod(e, threadClass, sleepMethodID, (jlong)millis);
}

char const* TranslateOPErr(OPErr opErr) {
    switch(opErr) {

    case NO_ERR:
	return ("NO_ERR is not an error.");
    case STREAM_STOP_PLAY:
	return ("STREAM_STOP_PLAY is not an error.");
    case PARAM_ERR:
	return ("Bad Parameters");
    case MEMORY_ERR:
	return ("Out of Memory");
    case BAD_SAMPLE:
	return ("Bad Sample Data");
    case BAD_INSTRUMENT:
	return ("Bad Instrument");
    case BAD_MIDI_DATA:
	return ("Bad Midi Data");
    case ALREADY_PAUSED:
	return ("Already Paused");
    case ALREADY_RESUMED:
	return ("Already Resumed");
    case DEVICE_UNAVAILABLE:
	return ("Audio Device Unavailable");
    case NO_SONG_PLAYING:
	return ("No Song Playing");
    case STILL_PLAYING:
	return ("Still Playing");
    case NO_VOLUME:
	return ("No Volume");
    case TOO_MANY_SONGS_PLAYING:
	return ("Too Many Songs Playing");
    case BAD_FILE:
	return ("Bad File");
    case NOT_REENTERANT:
	return ("Not Re-entrant");
    case NOT_SETUP:
	return ("Not Set Up");
    case BUFFER_TO_SMALL:
	return ("Buffer Too Small");
    case NO_FREE_VOICES:
	return ("No Free Voices");
    case BAD_FILE_TYPE:
	return ("Bad File Type");
    case GENERAL_BAD:
	return ("General Failure");
    default :
	return ("Unexpected Error");
    }
}

int UTIL_IsBigEndianPlatform() {
#if X_WORD_ORDER == TRUE
    return 0;
#else
    return 1;
#endif
}


