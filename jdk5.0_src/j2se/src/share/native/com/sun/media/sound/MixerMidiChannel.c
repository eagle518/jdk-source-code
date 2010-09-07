/*
 * @(#)MixerMidiChannel.c	1.23 03/12/19
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
#include "engine/GenPriv.h"		// for PV_ResetControlers
#include "engine/X_Formats.h"	// for midi structures


// UTILITY includes
#include "Utilities.h"

// MixerMidiChannel includes
#include "com_sun_media_sound_MixerMidiChannel.h"


// MIDI CHANNEL MANIPULATIONS

#include <stdio.h>

JNIEXPORT void JNICALL
    Java_com_sun_media_sound_MixerMidiChannel_nNoteOn(JNIEnv* e, jobject thisObj, jlong id, jint channelNumber, jint noteNumber, jint velocity, jlong tick) {

    TRACE0("Java_com_sun_media_sound_MixerMidiChannel_nNoteOn.\n");
    VTRACE1("-> id: %lu\n", id);
    VTRACE4("-> channelNumber: %d, noteNumber: %d, velocity: %d, tick: %lu\n", channelNumber, noteNumber, velocity, tick);

    /* adjust for real-time */
    if (tick <= 0) {
	tick = XGetRealTimeSyncCount();
    }
    if (tick < 0)
	{
	    /* talk directly to the synthesizer */
	    GM_NoteOn((void *)e, (GM_Song *) (INT_PTR) id, (INT16)channelNumber, (INT16)noteNumber, (INT16)velocity);
	}
    else
	{
	    /* schedule with the sequencer */
	    QGM_NoteOn((void *)e, (GM_Song *) (INT_PTR) id, (UINT32)tick, (INT16)channelNumber, (INT16)noteNumber, (INT16)velocity);
	}

    TRACE0("Java_com_sun_media_sound_MixerMidiChannel_nNoteOn completed.\n");
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_MixerMidiChannel_nNoteOff(JNIEnv* e, jobject thisObj, jlong id, jint channelNumber, jint noteNumber, jint velocity, jlong tick) {

    TRACE0("Java_com_sun_media_sound_MixerMidiChannel_nNoteOff.\n");
    VTRACE1("-> id: %lu\n", id);
    VTRACE4("-> channelNumber: %d, noteNumber: %d, velocity: %d, tick: %lu\n", channelNumber, noteNumber, velocity, tick);

    /* adjust for real-time */
    if (tick <= 0) {
	tick = XGetRealTimeSyncCount();
    }
    tick = XGetRealTimeSyncCount();
    if (tick < 0)
	{
	    /* talk directly to the synthesizer */
	    GM_NoteOff((void *)e, (GM_Song *) (INT_PTR) id, (INT16)channelNumber, (INT16)noteNumber, (INT16)velocity);
	}
    else
	{
	    /* schedule with the sequencer */
	    QGM_NoteOff((void *)e, (GM_Song *) (INT_PTR) id, (UINT32)tick, (INT16)channelNumber, (INT16)noteNumber, (INT16)velocity);
	}

    TRACE0("Java_com_sun_media_sound_MixerMidiChannel_nNoteOff completed.\n");
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_MixerMidiChannel_nControlChange(JNIEnv* e, jobject thisObj, jlong id, jint channelNumber, jint controller, jint value, jlong tick) {

    TRACE0("Java_com_sun_media_sound_MixerMidiChannel_nControlChange.\n");
    VTRACE1("-> id: %lu\n", id);
    VTRACE4("-> channelNumber: %d, controller: %d, value: %d, tick: %lu\n", channelNumber, controller, value, tick);

    /* adjust for real-time */
    if (tick <= 0) {
	tick = XGetRealTimeSyncCount();
    }
    tick = XGetRealTimeSyncCount();
    if (tick < 0)
	{
	    /* talk directly to the synthesizer */
	    GM_Controller((void *)e, (GM_Song *) (INT_PTR) id, (INT16)channelNumber, (INT16)controller, (INT16)value);
	}
    else
	{
	    /* schedule with the sequencer */
	    QGM_Controller((void *)e, (GM_Song *) (INT_PTR) id, (UINT32)tick, (INT16)channelNumber, (INT16)controller, (INT16)value);
	}

    TRACE0("Java_com_sun_media_sound_MixerMidiChannel_nControlChange completed.\n");
}

JNIEXPORT jint JNICALL
    Java_com_sun_media_sound_MixerMidiChannel_nGetController(JNIEnv* e, jobject thisObj, jlong id, jint channelNumber, jint controller) {

    char c;
    int rc;

    TRACE0("Java_com_sun_media_sound_MixerMidiChannel_nGetController.\n");

    c = (GM_GetControllerValue((GM_Song *) (INT_PTR) id, (INT16)channelNumber, (INT16)controller));
    rc = (jint)c;
    return rc;
}

JNIEXPORT void JNICALL
    Java_com_sun_media_sound_MixerMidiChannel_nProgramChange__JIIJ(JNIEnv* e, jobject thisObj, jlong id, jint channelNumber, jint program, jlong tick) {

    TRACE0("Java_com_sun_media_sound_MixerMidiChannel_nProgramChange.\n");
    VTRACE1("-> id: %lu\n", id);
    VTRACE3("-> channelNumber: %d, program: %d, tick: %lu\n", channelNumber, program, tick);

    /* adjust for real-time */
    if (tick <= 0) {
	tick = XGetRealTimeSyncCount();
    }
    if (tick < 0)
	{
	    /* talk directly to the synthesizer */
	    GM_ProgramChange((void *)e, (GM_Song *) (INT_PTR) id, (INT16)channelNumber, (INT16)program);
	}
    else
	{
	    /* schedule with the sequencer */
	    QGM_ProgramChange((void *)e, (GM_Song *) (INT_PTR) id, (UINT32)tick, (INT16)channelNumber, (INT16)program);
	}

    TRACE0("Java_com_sun_media_sound_MixerMidiChannel_nProgramChange completed.\n");
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_MixerMidiChannel_nProgramChange__JIIIJ(JNIEnv* e, jobject thisObj, jlong id, jint channelNumber, jint bank, jint program, jlong tick) {

    TRACE0("Java_com_sun_media_sound_MixerMidiChannel_nProgramChange.\n");
    VTRACE1("-> id: %lu\n", id);
    VTRACE4("-> channelNumber: %d, bank: %d, program: %d, tick: %lu\n", channelNumber, bank, program, tick);

    /* adjust for real-time */
    if (tick <= 0) {
	tick = XGetRealTimeSyncCount();
    }
    if (tick < 0)
	{
	    /* talk directly to the synthesizer */
	    GM_Controller((void *)e, (GM_Song *) (INT_PTR) id, (INT16)channelNumber, (INT16)0, (INT16)bank);
	    GM_ProgramChange((void *)e, (GM_Song *) (INT_PTR) id, (INT16)channelNumber, (INT16)program);
	}
    else
	{
	    /* schedule with the sequencer */
	    QGM_Controller((void *)e, (GM_Song *) (INT_PTR) id, (UINT32)tick, (INT16)channelNumber, (INT16)0, (INT16)bank);
	    QGM_ProgramChange((void *)e, (GM_Song *) (INT_PTR) id, (UINT32)tick, (INT16)channelNumber, (INT16)program);
	}

    TRACE0("Java_com_sun_media_sound_MixerMidiChannel_nProgramChange completed..\n");
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_MixerMidiChannel_nSetPitchBend(JNIEnv* e, jobject thisObj, jlong id, jint channelNumber, jint bendHigh, jint bendLow, jlong tick) {

    TRACE0("Java_com_sun_media_sound_MixerMidiChannel_nSetPitchBend.\n");
    VTRACE1("-> id: %lu\n", id);
    VTRACE4("-> channelNumber: %d, bendLow: %d, bendHigh: %d, tick: %lu\n", channelNumber, bendLow, bendHigh, tick);

    /* adjust for real-time */
    if (tick <= 0) {
	tick = XGetRealTimeSyncCount();
    }
    if (tick < 0)
	{
	    /* talk directly to the synthesizer */
	    GM_PitchBend((void *)e, (GM_Song *) (INT_PTR) id, (INT16)channelNumber, (UBYTE)bendHigh, (UBYTE)bendLow);
	}
    else
	{
	    /* schedule with the sequencer */
	    QGM_PitchBend((void *)e, (GM_Song *) (INT_PTR) id, (UINT32)tick, (INT16)channelNumber, (UBYTE)bendHigh, (UBYTE)bendLow);
	}

    TRACE0("Java_com_sun_media_sound_MixerMidiChannel_nSetPitchBend completed.\n");
}

JNIEXPORT jint JNICALL
    Java_com_sun_media_sound_MixerMidiChannel_nGetPitchBend(JNIEnv* e, jobject thisObj, jlong id, jint channelNumber) {

    unsigned char LSB;
    unsigned char MSB;
    jint rc;

    TRACE0("Java_com_sun_media_sound_MixerMidiChannel_nGetPitchBend.\n");

    GM_GetPitchBend((GM_Song *) (INT_PTR) id, (INT16)channelNumber, &LSB, &MSB);
    rc = (jint)((MSB*128) + LSB);

    return rc;
}

JNIEXPORT void JNICALL
    Java_com_sun_media_sound_MixerMidiChannel_nAllNotesOff(JNIEnv* e, jobject thisObj, jlong engineIdentifier, jint channelNumber, jlong tick)
{

    GM_Song			*pSong = (GM_Song *) (INT_PTR) engineIdentifier;

    TRACE0("Java_com_sun_media_sound_MixerMidiChannel_nAllNotesOff.\n");

    /* adjust for real-time */
    if (tick <= 0) {
	tick = XGetRealTimeSyncCount();
    }
    if (tick < 0)
	{
	    /* talk directly to the synthesizer */
	    GM_Controller((void *)e, pSong, (INT16)channelNumber, 123, 0);		// issue a all notes off the queue
	}
    else
	{
	    /* schedule with the sequencer */
	    QGM_Controller((void *)e, pSong, (UINT32)tick, (INT16)channelNumber, 123, 0);		// issue a all notes off the queue
	}

    TRACE0("Java_com_sun_media_sound_MixerMidiChannel_nAllNotesOff completed.\n");
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_MixerMidiChannel_nResetAllControllers(JNIEnv* e, jobject thisObj, jlong engineIdentifier, jint channelNumber)
{

    GM_Song			*pSong = (GM_Song *) (INT_PTR) engineIdentifier;

    TRACE1("Java_com_sun_media_sound_MixerMidiChannel_nResetAllControllers: %d.\n", channelNumber);

    // $$kk: 11.27.98: TRUE or FALSE?  (complete or semi-complete reset?)
    PV_ResetControlers(pSong, (INT16)channelNumber, TRUE);

    TRACE1("Java_com_sun_media_sound_MixerMidiChannel_nResetAllControllers: %d completed.\n", channelNumber);
}


JNIEXPORT jboolean JNICALL
    Java_com_sun_media_sound_MixerMidiChannel_nSetMute(JNIEnv* e, jobject thisObj, jlong id, jint channelNumber, jboolean muteState) {

    char        channels[16];

    TRACE0("Java_com_sun_media_sound_MixerMidiChannel_nSetMute.\n");
    VTRACE1("-> id: %lu\n", id);
    VTRACE2("-> channelNumber: %d, muteState: %d\n", channelNumber, muteState);

    if (muteState) {

	GM_MuteChannel((GM_Song *) (INT_PTR) id, (INT16)channelNumber);

    } else {

	GM_UnmuteChannel((GM_Song *) (INT_PTR) id, (INT16)channelNumber);
    }

    GM_GetChannelMuteStatus((GM_Song *) (INT_PTR) id, channels);

    TRACE0("Java_com_sun_media_sound_MixerMidiChannel_nSetMute completed.\n");
    return (jboolean)channels[channelNumber];
}


JNIEXPORT jboolean JNICALL
    Java_com_sun_media_sound_MixerMidiChannel_nSetSolo(JNIEnv* e, jobject thisObj, jlong id, jint channelNumber, jboolean soloState) {

    char        channels[16];

    TRACE0("Java_com_sun_media_sound_MixerMidiChannel_nSetSolo.\n");
    VTRACE1("-> id: %lu\n", id);
    VTRACE2("-> channelNumber: %d, soloState: %d\n", channelNumber, soloState);

    if (soloState)
	{
	    GM_SoloChannel((GM_Song *) (INT_PTR) id, (INT16)channelNumber);
	}
    else
	{
	    GM_UnsoloChannel((GM_Song *) (INT_PTR) id, (INT16)channelNumber);
	}

    GM_GetChannelSoloStatus((GM_Song *) (INT_PTR) id, channels);
    TRACE0("Java_com_sun_media_sound_MixerMidiChannel_nSetSolo completed.\n");

    return ((jboolean)(channels[channelNumber]));
}


JNIEXPORT jboolean JNICALL
    Java_com_sun_media_sound_MixerMidiChannel_nGetSolo(JNIEnv* e, jobject thisObj, jlong id, jint channelNumber) {

    char        channels[16];

    TRACE0("Java_com_sun_media_sound_MixerMidiChannel_nGetSolo.\n");
    VTRACE1("-> id: %lu\n", id);
    VTRACE1("-> channelNumber: %d\n", channelNumber);

    GM_GetChannelSoloStatus((GM_Song *) (INT_PTR) id, channels);

    TRACE0("Java_com_sun_media_sound_MixerMidiChannel_nGetSolo completed.\n");

    return ((jboolean)(channels[channelNumber]));
}
