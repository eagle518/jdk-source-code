/*
 * @(#)AbstractPlayer.c	1.16 03/12/19
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
#include "engine/HAE_API.h"
#include "engine/GenSnd.h"
#include "engine/X_Formats.h" // for midi structures


// UTILITY includes
#include "Utilities.h"

// AbstractPlayer includes
#include "com_sun_media_sound_AbstractPlayer.h"


// MIDI SYNTHESIZER MANIPULATIONS


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_AbstractPlayer_nClose(JNIEnv* e, jobject thisObj, jlong id) {

    GM_Song *pSong = (GM_Song *) (INT_PTR) id;
    OPErr err;
    int waitTime;

    TRACE3("Java_com_sun_media_sound_AbstractPlayer_nClose: e: %lu, thisObj: %lu, pSong: %lu.\n", 
           e, thisObj, (UINT32) id);

    if (pSong) {
	    GM_KillSongNotes(pSong);
	    pSong->disposeSongDataWhenDone = TRUE;   // free our midi pointer
	    GM_PauseSong(pSong);
	    /* remove song from list of playing songs */
	    GM_RemoveFromSongsToPlay(pSong);
	    /* set to impossible scan mode to disable processing */
	    pSong->AnalyzeMode = (ScanMode) -1;
	    /* fix for 4795377: closing sequencer sometimes crashes the VM */
	    QGM_ClearSongFromQueue(pSong);
	    
	    /* wait until there was at least one mixer slice 
	     * processed so that the song notes are really 
	     * not used anymore
	     */
	    waitTime = HAE_GetSliceTimeInMicroseconds()/1000 + 5;
	    TRACE1("nClose: waiting %d millis to process GM_KillSongNotes...\n", waitTime);
	    SleepMillisInJava(e, waitTime);
	    
	    do {
		/* this may fail - wait until it was able to kill the song */
		err = GM_FreeSong((void *)e, pSong);
		if (err == STILL_PLAYING) {
		    ERROR0("nClose: STILL_PLAYING! wait 5 millis...\n");
		    SleepMillisInJava(e, 5);
		} else if (err != NO_ERR) {
		    ERROR1("nClose: GM_FreeSong returned error code: %d\n", err);
		}
	    } while (err == STILL_PLAYING);

	} else {
		ERROR0("Java_com_sun_media_sound_AbstractPlayer_nClose: pSong is NULL\n");
	    }

    TRACE0("Java_com_sun_media_sound_AbstractPlayer_nClose completed.\n");
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_AbstractPlayer_nAddReceiver(JNIEnv* e, jobject thisObj, jlong id, jlong receiverId)
{
    GM_Song			*pSong = (GM_Song *) (INT_PTR) id;
    GM_Synth		*pSynth = NULL;

    TRACE0("Java_com_sun_media_sound_AbstractPlayer_nAddReceiver.\n");

    if (pSong) {
	    // only add it if it's not already in the list
	    while ( (pSynth = GM_GetSongSynth(pSong, pSynth)) != NULL ) {
		    // if it's already in the list, return
		    if (pSynth->deviceHandle == (void *) (INT_PTR) receiverId) {
			    return;
			}
		}


	    // $$kk: 07.12.99: change this!
#if USE_EXTERNAL_SYNTH == TRUE
	    if (receiverId) {
		    pSynth = PV_CreateExternalSynthForDevice(pSong, (void *)receiverId);
		}
#endif

	    // $$kk: 07.12.99: add it even if it's null 'cause that'll give us the
	    // software synth
	    //if (pSynth)
	    {
		GM_AddSongSynth(pSong, pSynth);
	    }
	} else {

		ERROR0("Java_com_sun_media_sound_AbstractPlayer_nAddReceiver: pSong is NULL\n");
	    }

    TRACE0("Java_com_sun_media_sound_AbstractPlayer_nAddReceiver completed.\n");
}


JNIEXPORT void JNICALL
    Java_com_sun_media_sound_AbstractPlayer_nRemoveReceiver(JNIEnv* e, jobject thisObj, jlong id, jlong receiverId)
{
    GM_Song			*pSong = (GM_Song *) (INT_PTR) id;
    GM_Synth		*pSynth = NULL;

    TRACE0("Java_com_sun_media_sound_AbstractPlayer_nRemoveReceiver.\n");

    if (pSong) {
	    // go through list.
	    while ( (pSynth = GM_GetSongSynth(pSong, pSynth)) != NULL ) {
		    // remove it if it's this one
		    if (pSynth->deviceHandle == (void *) (INT_PTR) receiverId) {
			    GM_RemoveSongSynth(pSong, pSynth);
			}
		}
	} else {

		ERROR0("Java_com_sun_media_sound_AbstractPlayer_nRemoveReceiver: pSong is NULL\n");
	    }

    TRACE0("Java_com_sun_media_sound_AbstractPlayer_nRemoveReceiver completed.\n");
}


JNIEXPORT jboolean JNICALL
    Java_com_sun_media_sound_AbstractPlayer_nLoadInstrument(JNIEnv* e, jobject thisObj, jlong id, jint instrumentId)
{
    GM_Song			*pSong = (GM_Song *) (INT_PTR) id;
    OPErr opErr = NOT_SETUP;

    TRACE0("Java_com_sun_media_sound_AbstractPlayer_nLoadInstrument\n");

    if (pSong) {
	    opErr = GM_LoadInstrument(pSong, (XLongResourceID)instrumentId);

	    if (opErr != NO_ERR) {
		    ERROR1("Java_com_sun_media_sound_AbstractPlayer_nLoadInstrument: GM_LoadInstrument returned an error: %d\n", opErr);
		}
	} else {
	    ERROR0("Java_com_sun_media_sound_AbstractPlayer_nLoadInstrument: pSong is NULL\n");
	}

    TRACE0("Java_com_sun_media_sound_AbstractPlayer_nLoadInstrument completed\n");
    return ( (opErr == NO_ERR) ? TRUE : FALSE );
}


JNIEXPORT jboolean JNICALL
    Java_com_sun_media_sound_AbstractPlayer_nUnloadInstrument(JNIEnv* e, jobject thisObj, jlong id, jint instrumentId)
{
    GM_Song			*pSong = (GM_Song *) (INT_PTR) id;
    OPErr opErr = NOT_SETUP;

    TRACE0("Java_com_sun_media_sound_AbstractPlayer_nUnloadInstrument\n");

    if (pSong) {
	    opErr = GM_UnloadInstrument(pSong, (XLongResourceID)instrumentId);
	    if (opErr != NO_ERR) {
		    ERROR1("Java_com_sun_media_sound_AbstractPlayer_nUnloadInstrument: GM_UnloadInstrument returned an error: %d\n", opErr);
		}
	} else {
	    ERROR0("Java_com_sun_media_sound_AbstractPlayer_nUnloadInstrument: pSong is NULL\n");
	}

    TRACE0("Java_com_sun_media_sound_AbstractPlayer_nUnloadInstrument completed\n");
    return ( (opErr == NO_ERR) ? TRUE : FALSE );
}


JNIEXPORT jboolean JNICALL
    Java_com_sun_media_sound_AbstractPlayer_nRemapInstrument(JNIEnv* e, jobject thisObj, jlong id, jint from, jint to)
{
    GM_Song			*pSong = (GM_Song *) (INT_PTR) id;
    OPErr opErr = NOT_SETUP;

    TRACE0("Java_com_sun_media_sound_AbstractPlayer_nRemapInstrument\n");

    if (pSong) {
	    opErr = GM_RemapInstrument(pSong, (XLongResourceID)from, (XLongResourceID)to);

	    if (opErr != NO_ERR) {
		    ERROR1("Java_com_sun_media_sound_AbstractPlayer_nRemapInstrument: GM_RemapInstrument returned an error: %d\n", opErr);
		}
	} else {
	    ERROR0("Java_com_sun_media_sound_AbstractPlayer_nRemapInstrument: pSong is NULL\n");
	}

    TRACE0("Java_com_sun_media_sound_AbstractPlayer_nRemapInstrument completed\n");
    return ( (opErr == NO_ERR) ? TRUE : FALSE );
}
