/*
 * @(#)MixerSequencer.c	1.26 03/12/19
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
#include "engine/X_Formats.h" // for midi structures


// UTILITY includes
#include "Utilities.h"

// MixerSequencer includes
#include "com_sun_media_sound_MixerSequencer.h"


// GLOBALS

jclass g_mixerSequencerClass = NULL;
jmethodID g_songEndMethodID = 0;
jmethodID g_metaEventMethodID = 0;
jmethodID g_controllerEventMethodID = 0;


// HELPER METHODS

static XBOOL initializeJavaSequencerCallbackVars(JNIEnv* e, jobject thisObj) {	
    jclass objClass = (*e)->GetObjectClass(e, thisObj);
    if (objClass == NULL)
	{
	    ERROR0("initializeJavaSequencerCallbackVars: (objClass == NULL)\n");
	    return (XBOOL)FALSE;
	}

    g_mixerSequencerClass = (*e)->NewGlobalRef(e, objClass);
    if (g_mixerSequencerClass == NULL)
	{
	    ERROR0("initializeJavaSequencerCallbackVars: (g_mixerSequencerClass == NULL)\n");
	    return (XBOOL)FALSE;
	}

    g_songEndMethodID =		(*e)->GetMethodID(e, g_mixerSequencerClass, "callbackSongEnd", "()V");

    if (g_songEndMethodID == 0)
	{
	    ERROR0("initializeJavaSequencerCallbackVars: (failed to get method ID)\n");
	    return (XBOOL)FALSE;
	}

    g_metaEventMethodID = (*e)->GetMethodID(e, g_mixerSequencerClass, "callbackMetaEvent", "(III[B)V");

    if (g_metaEventMethodID == 0)
	{
	    ERROR0("initializeJavaSequencerCallbackVars: (failed to get method ID)\n");
	    return (XBOOL)FALSE;
	}

    g_controllerEventMethodID = (*e)->GetMethodID(e, g_mixerSequencerClass, "callbackControllerEvent", "(IIII)V");

    if (g_controllerEventMethodID == 0)
	{
	    ERROR0("initializeJavaSequencerCallbackVars: (failed to get method ID)\n");
	    return (XBOOL)FALSE;
	}

    return (XBOOL)TRUE;
}


// $$kk: 03.22.99: need to call this on shutdown!!
/* $$fb 2002-03-07: never called... */
/*
static void releaseJavaSequencerCallbackVars(JNIEnv* e) 
{
	jclass localClass = g_mixerSequencerClass;

	g_mixerSequencerClass = NULL;
	g_songEndMethodID = 0;
	g_metaEventMethodID = 0;
	g_controllerEventMethodID = 0;

	(*e)->DeleteGlobalRef(e, localClass);
}
*/


// CALLBACKS

static void PV_SongEndCallback(void *threadContext, GM_Song *pSong)
{
    JNIEnv* e;
    jobject sequencerObj;

    TRACE1("PV_SongEndCallback. "
           "pSong=%lu id=%d\n", (UINT32) (UINT_PTR) pSong);
	
    e = (JNIEnv*)threadContext;
    sequencerObj = (jobject)pSong->context; 

    if ((e == NULL) || (sequencerObj == NULL) || (g_songEndMethodID == 0)) {
	ERROR3("PV_SongEndCallback: received a NULL variable: e: %p; sequencerObj: %p; g_songEndMethodID: %p\n", e, sequencerObj, g_songEndMethodID);
	return;
    }


    (*e)->CallVoidMethod(e, sequencerObj, g_songEndMethodID);
}

static void PV_MetaEventCallback(void *threadContext, GM_Song *pSong, char markerType, void *pText, INT32 textLength, short currentTrack)
{
    JNIEnv* e;
    jobject sequencerObj;
    jbyteArray localArray;
    char *pTemp;
    char buffer[1024];
    int i;

    TRACE0(" in PV_MetaEventCallback");

    pTemp = pText;
    for(i=0;i<textLength;i++) {
	buffer[i] = *pTemp++;
    }
    buffer[textLength]=0;

    e = (JNIEnv*)threadContext;

    localArray = (*e)->NewByteArray(e,(jsize)textLength);
    (*e)->SetByteArrayRegion(e,localArray,(jsize)0,(jsize)textLength,(jbyte *)pText);

    sequencerObj = (jobject)pSong->context; 

    if ((e == NULL) || (sequencerObj == NULL) || (g_metaEventMethodID == 0)) {
	ERROR3("PV_MetaEventCallback: received a NULL variable: e: %p; sequencerObj: %p; g_metaEventMethodID: %p\n", e, sequencerObj, g_metaEventMethodID);
	return;
    }

    (*e)->CallVoidMethod(e, sequencerObj, g_metaEventMethodID, (jint) markerType, (jint) textLength, (jint)currentTrack, (jbyteArray)localArray );
}

static void PV_ControllerEventCallback(void *threadContext, GM_Song *pSong, void * reference, short int channel, short int track, short int controller, short int value )
{
    JNIEnv* e;
    jobject sequencerObj;

    TRACE0(" in PV_ControllerEventCallback");

    e = (JNIEnv*)threadContext;

    sequencerObj = (jobject)pSong->context; 

    if ((e == NULL) || (sequencerObj == NULL) || (g_controllerEventMethodID == 0)) {
	ERROR3("PV_ControllerEventCallback: received a NULL variable: e: %p; sequencerObj: %p; g_controllerEventMethodID: %p\n", e, sequencerObj, g_controllerEventMethodID);
	return;
    }

    (*e)->CallVoidMethod(e, sequencerObj, g_controllerEventMethodID, (jint) channel, (jint) track, (jint) controller, (jint) value);
	
}


// MIDI SONG MANIPULATIONS

JNIEXPORT jlong JNICALL
Java_com_sun_media_sound_MixerSequencer_nOpenMidiSequencer
(JNIEnv* e, jobject thisObj, jbyteArray midiData, jint length) {

    GM_Song           *pSong = NULL;
    void              *pData = NULL; 

    SongResource      *xSong;
    OPErr             opErr;
    XShortResourceID  id;
	
    INT16             midiVoices;
    INT16             channelVoices;
    INT16             volumeDivisor;
    ReverbMode        reverbMode;

    // $$kk: 04.22.99: do we need this???
    jobject           globalSequencerObj = NULL;


    TRACE0("Java_com_sun_media_sound_MixerSequencer_nOpenMidiSequencer.\n");

    // get all the class, method, and field id's ONCE.
    // store a global reference to the java channel class to make sure the id's remain valid.
    // $$kk: 03.22.99: need to make sure we release this reference eventually!

    if (g_mixerSequencerClass == NULL) {
	if (!initializeJavaSequencerCallbackVars(e, thisObj)) {
	    ERROR0("Failed to initalized Java sequencer callback vars!\n");
	    return 0;
	}
    }

    // $$kk: 03.15.98: GLOBAL REF!  this has definite memory leak potential; need to make sure
    // to release all global refs....
    globalSequencerObj = (*e)->NewGlobalRef(e, thisObj);


    id = getMidiSongCount();
	
    // copy the data out of the java object, then give it to the engine and tell the
    // engine not to copy it again.

    pData = XNewPtr((INT32)length);

    if (!pData) {
	ERROR0("Java_com_sun_media_sound_MixerSequencer_nOpenMidiSequencer failed: failed to allocate memory for midi song\n");
	return 0;
    }

    (*e)->GetByteArrayRegion(e, midiData, (jint)0, (jint)length, (jbyte*)pData);

    // get the mixer info

    GM_GetSystemVoices(&midiVoices, &volumeDivisor, &channelVoices);
    reverbMode = GM_GetReverbType();

    // create the song
    xSong = XNewSongPtr(SONG_TYPE_SMS, id,
			(short)midiVoices,
			(short)volumeDivisor,
			(short)channelVoices,
			reverbMode);

    if (!xSong) {
	ERROR0("Failed to create song from data \n");
	XDisposePtr(pData);
	return 0;
    }

    pSong = GM_LoadSong((void *)e, (void *)globalSequencerObj, id, (void*)xSong,
			pData, 
			(INT32)length,
			NULL,       // no callback
			TRUE,       // load instruments
			//                                FALSE,       // load instruments
			TRUE,		// ignore bad instruments $$kk: 02.17.98: changed this to TRUE.
			// this is a hack to get around the JMF problem where erratically a MIDI
			// song tries and fails to load an invalid instrument.
			//                                FALSE,      // ignore bad instruments
			&opErr);

    if ( (!pSong) || (opErr != NO_ERR) ) {
	ERROR0("Failed to load midi song \n");
	XDisposePtr(pData);
	return 0;
    }

    //$$jb: link meta event callbacks here, now that we have a pSong
    //      we'll have to link controller callbacks elsewhere when we know
    //      what controllers to listen for

    GM_SetSongMetaEventCallback(pSong, *(PV_MetaEventCallback), id);


    GM_SetSongLoopFlag(pSong, FALSE);        // don't loop song
    pSong->disposeSongDataWhenDone = TRUE;   // dispose of midi data

    pSong->userReference = (void *) ((UINT_PTR) id);
    TRACE2("Java_com_sun_media_sound_MixerSequencer_nOpenMidiSequencer completed. "
           "pSong=%lu id=%d\n", (UINT32) (UINT_PTR) pSong, (INT32) id);
    return (jlong) (INT_PTR) pSong;
}


JNIEXPORT void JNICALL
Java_com_sun_media_sound_MixerSequencer_nAddControllerEventCallback(JNIEnv* e, jobject thisObj,jlong id, jint controller)
{
    GM_Song			*pSong = (GM_Song *) (INT_PTR) id;

    TRACE0("Java_com_sun_media_sound_MixerSequencer_nAddControllerEventCallback.\n");

    GM_SetControllerCallback(pSong, (void *)pSong->userReference, *(PV_ControllerEventCallback), (short int)controller);
}



JNIEXPORT jlong JNICALL
Java_com_sun_media_sound_MixerSequencer_nOpenRmfSequencer(JNIEnv* e, jobject thisObj, jbyteArray rmfData, jint length)
{

    GM_Song				*pSong = NULL;
    void				*pData = NULL; 

    SongResource		*xSong;
    OPErr               opErr;
    jint				id;	
	
    XFILE				fileRef;

    // $$kk: 04.22.99: do we need this???
    jobject			globalSequencerObj = NULL;



    TRACE0("Java_com_sun_media_sound_MixerSequencer_nOpenRmfSequencer.\n");


    // get all the class, method, and field id's ONCE.
    // store a global reference to the java channel class to make sure the id's remain valid.
    // $$kk: 03.22.99: need to make sure we release this reference eventually!

    if (g_mixerSequencerClass == NULL) 
	{
	    if (!initializeJavaSequencerCallbackVars(e, thisObj))
		{
		    ERROR0("Failed to initalized Java sequencer callback vars!\n");
		    return 0;
		}
	}

    // $$kk: 03.15.98: GLOBAL REF!  this has definite memory leak potential; need to make sure
    // to release all global refs....
    globalSequencerObj = (*e)->NewGlobalRef(e, thisObj);


    id = getMidiSongCount();
	
    // copy the data out of the java object, then give it to the engine and tell the
    // engine not to copy it again.

    pData = XNewPtr(length);	// can have memory error

    if (!pData) 
	{
	    ERROR0("Java_com_sun_media_sound_MixerSequencer_nOpenRmfSequencer failed: failed to allocate memory for midi song\n");
	    return 0;
	}

    (*e)->GetByteArrayRegion(e, rmfData, (jint)0, (jint)length, (jbyte*)pData);

    // $$kk: 04.10.98: what does TRUE do?  what can cause failure here?
    fileRef = XFileOpenResourceFromMemory(pData, length, TRUE);

    if (!fileRef)
	{
	    ERROR0("Failed to create resource file from data \n");
	    XDisposePtr(pData);
	    return 0;
	}

    // look for first song. RMF files only contain one SONG resource
    // bad file if this failed and no xSong
    xSong = (SongResource*)XGetIndexedResource(ID_SONG, (INT32*)(&id), 0, NULL, (INT32*)(&length)); // fails for bad file

    if (!xSong)
	{
	    ERROR0("Failed to create song resource: bad RMF file \n");
	    XDisposePtr(pData);
	    XFileClose(fileRef);
	    return 0;
	}

    pSong = GM_LoadSong((void *)e, (void *)globalSequencerObj, (short)id, xSong, NULL, 0,
			NULL,// no callback
			TRUE,// load instruments
			TRUE,// ignore bad instruments
			&opErr);	  

    if (!pSong)
	{
	    ERROR0("Failed to load rmf song \n");
	    XDisposePtr(pData);
	    XFileClose(fileRef);
	    return 0;
	}

    XFileClose(fileRef);

    //$$jb: link meta event callbacks here, now that we have a pSong
    //      we'll have to link controller callbacks elsewhere when we know
    //      what controllers to listen for

    GM_SetSongMetaEventCallback(pSong, *(PV_MetaEventCallback), id);

    pSong->ignoreBadInstruments = FALSE;
    pSong->disposeSongDataWhenDone = TRUE;   // dispose of midi data
    GM_SetSongLoopFlag(pSong, FALSE);      // don't loop song

    pSong->userReference = (void *) ((UINT_PTR) id);
    TRACE0("Java_com_sun_media_sound_MixerSequencer_nOpenRmfSequencer completed.\n");
    return (jlong) (INT_PTR) pSong;
}


JNIEXPORT void JNICALL
Java_com_sun_media_sound_MixerSequencer_nStartSequencer(JNIEnv* e, jobject thisObj, jlong id)
{

    OPErr opErr = NO_ERR;
	
    TRACE1("Java_com_sun_media_sound_MixerSequencer_nStartSequencer. "
           "pSong=%lu\n", (UINT32) (UINT_PTR) id);

    opErr = GM_BeginSong((GM_Song *) (INT_PTR) id, (GM_SongCallbackProcPtr)PV_SongEndCallback, FALSE, FALSE);

    TRACE0("Java_com_sun_media_sound_MixerSequencer_nStartSequencer completed.\n");
}


JNIEXPORT void JNICALL
Java_com_sun_media_sound_MixerSequencer_nPauseSequencer
(JNIEnv* e, jobject thisObj, jlong id) {

    GM_Song			*pSong = (GM_Song *) (INT_PTR) id;

    TRACE1("Java_com_sun_media_sound_MixerSequencer_nPauseSequencer."
           "pSong=%lu\n", (UINT32) (UINT_PTR) id);

    GM_PauseSong(pSong);

    TRACE0("Java_com_sun_media_sound_MixerSequencer_nPauseSequencer completed.\n");
}


JNIEXPORT void JNICALL
Java_com_sun_media_sound_MixerSequencer_nResumeSequencer(JNIEnv* e, jobject thisObj, jlong id) 
{

    GM_Song			*pSong = (GM_Song *) (INT_PTR) id;

    TRACE1("Java_com_sun_media_sound_MixerSequencer_nResumeSequencer. "
           "pSong=%lu\n", (UINT32) (UINT_PTR) id);

    // $$kk: 07.19.99: the callback gets cleared each time the song ends and the callback
    // gets called.  i want to make sure it gets reset here so we don't have to call
    // GM_BeginSong again.
    GM_SetSongCallback(pSong, (GM_SongCallbackProcPtr)PV_SongEndCallback);	

    // $$kk: 07.19.99: we won't get the callback unless this is set to FALSE.
    // this code should be in the engine, not here!!  it's used in GenSong.c 
    // and GenSeq.c
    pSong->songFinished = FALSE;
	
    GM_ResumeSong(pSong);


    TRACE0("Java_com_sun_media_sound_MixerSequencer_nResumeSequencer completed.\n");
}


JNIEXPORT jlong JNICALL
Java_com_sun_media_sound_MixerSequencer_nGetSequencerTickPosition(JNIEnv* e, jobject thisObj, jlong id) 
{	
    TRACE1("Java_com_sun_media_sound_MixerSequencer_nGetSequencerTickPosition "
           "pSong=%lu\n", (UINT32) (UINT_PTR) id);

    return (jlong)GM_SongTicks((GM_Song *) (INT_PTR) id);
}


JNIEXPORT jlong JNICALL
Java_com_sun_media_sound_MixerSequencer_nSetSequencerTickPosition(JNIEnv* e, jobject thisObj, jlong id, jlong tick)
{	
    TRACE2("Java_com_sun_media_sound_MixerSequencer_nSetSequencerTickPosition: %d. "
           "pSong=%lu\n", (UINT32) (UINT_PTR) id, tick);

    GM_SetSongTickPosition((GM_Song *) (INT_PTR) id, (UINT32)tick);
    return (jlong)GM_SongTicks((GM_Song *) (INT_PTR) id);
}


JNIEXPORT jlong JNICALL
Java_com_sun_media_sound_MixerSequencer_nGetSequencerMicrosecondPosition(JNIEnv* e, jobject thisObj, jlong id)
{	
    TRACE1("Java_com_sun_media_sound_MixerSequencer_nGetSequencerMicrosecondPosition "
           "pSong=%lu\n", (UINT32) (UINT_PTR) id);

    return (jlong)GM_SongMicroseconds((GM_Song *) (INT_PTR) id);
}


JNIEXPORT jlong JNICALL
Java_com_sun_media_sound_MixerSequencer_nSetSequencerMicrosecondPosition(JNIEnv* e, jobject thisObj, jlong id, jlong microseconds)
{	
    TRACE2("Java_com_sun_media_sound_MixerSequencer_nSetSequencerMicrosecondPosition: %d. "
           "pSong=%lu\n", (UINT32) (UINT_PTR) id, microseconds);

    GM_SetSongMicrosecondPosition((GM_Song *) (INT_PTR) id, (UINT32)microseconds);
    return (jlong)GM_SongMicroseconds((GM_Song *) (INT_PTR) id);
}

 
JNIEXPORT jint JNICALL
Java_com_sun_media_sound_MixerSequencer_nGetTempoInBPM(JNIEnv* e, jobject thisObj, jlong id)
{	
    TRACE0("Java_com_sun_media_sound_MixerSequencer_nGetTempoInBPM\n");
    VTRACE1("-> pSong: %lu\n", id);

    return (jint)GM_GetSongTempoInBeatsPerMinute((GM_Song *) (INT_PTR) id);
}


JNIEXPORT jint JNICALL
Java_com_sun_media_sound_MixerSequencer_nSetTempoInBPM(JNIEnv* e, jobject thisObj, jlong id, jint bpm)
{	
    TRACE1("Java_com_sun_media_sound_MixerSequencer_nSetTempoInBPM: %d.\n", bpm);
    VTRACE1("-> pSong: %lu\n", id);

    GM_SetSongTempInBeatsPerMinute((GM_Song *) (INT_PTR) id, (UINT32)bpm);
    return (jint)GM_GetSongTempoInBeatsPerMinute((GM_Song *) (INT_PTR) id);
}

 
JNIEXPORT jint JNICALL
Java_com_sun_media_sound_MixerSequencer_nGetTempoInMPQ(JNIEnv* e, jobject thisObj, jlong id)
{	
    TRACE0("Java_com_sun_media_sound_MixerSequencer_nGetTempoInMPQ\n");
    VTRACE1("-> pSong: %lu\n", id);

    return (jint)GM_GetSongTempo((GM_Song *) (INT_PTR) id);
}


JNIEXPORT jint JNICALL
Java_com_sun_media_sound_MixerSequencer_nSetTempoInMPQ(JNIEnv* e, jobject thisObj, jlong id, jint mpq)
{	
    TRACE1("Java_com_sun_media_sound_MixerSequencer_nSetTempoInMPQ: %d.\n", mpq);
    VTRACE1("-> pSong: %lu\n", id);

    GM_SetSongTempo((GM_Song *) (INT_PTR) id, (UINT32)mpq);
    return (jint)GM_GetSongTempo((GM_Song *) (INT_PTR) id);
}

 
JNIEXPORT jfloat JNICALL
Java_com_sun_media_sound_MixerSequencer_nGetMasterTempo(JNIEnv* e, jobject thisObj, jlong id)
{	
    TRACE0("Java_com_sun_media_sound_MixerSequencer_nGetMasterTempo\n");
    VTRACE1("-> pSong: %lu\n", id);

    return (FIXED_TO_FLOAT(GM_GetMasterSongTempo((GM_Song *) (INT_PTR) id)));
}


JNIEXPORT jfloat JNICALL
Java_com_sun_media_sound_MixerSequencer_nSetMasterTempo(JNIEnv* e, jobject thisObj, jlong id, jfloat factor)
{	
    TRACE1("Java_com_sun_media_sound_MixerSequencer_nSetMasterTempo: %f.\n", factor);
    VTRACE1("-> pSong: %lu\n", id);

    GM_SetMasterSongTempo((GM_Song *) (INT_PTR) id, FLOAT_TO_FIXED(factor));
    return (FIXED_TO_FLOAT(GM_GetMasterSongTempo((GM_Song *) (INT_PTR) id)));
}



JNIEXPORT void JNICALL
Java_com_sun_media_sound_MixerSequencer_nSetTrackMute(JNIEnv* e, jobject thisObj, jlong id, jint track, jboolean mute)
{
    TRACE3("Java_com_sun_media_sound_MixerSequencer_nSetTrackMute: id: %lu track: %d mute: %d.\n", id, track, mute);
	
    if (mute)
	{
	    GM_MuteTrack((GM_Song *) (INT_PTR) id, (short int) track);
	}
    else
	{
	    GM_UnmuteTrack((GM_Song *) (INT_PTR) id, (short int) track);
	}

    TRACE0("Java_com_sun_media_sound_MixerSequencer_nSetTrackMute reutrning\n");
}


JNIEXPORT jboolean JNICALL
Java_com_sun_media_sound_MixerSequencer_nGetTrackMute(JNIEnv* e, jobject thisObj, jlong id, jint track)
{
    if (track >= MAX_TRACKS)
	{
	    return (jboolean)FALSE;
	}
    else
	{
	    char muteStatus[MAX_TRACKS];
	    GM_GetTrackMuteStatus((GM_Song *) (INT_PTR) id, muteStatus);

	    return (jboolean)muteStatus[track];
	}
}


JNIEXPORT void JNICALL
Java_com_sun_media_sound_MixerSequencer_nSetTrackSolo(JNIEnv* e, jobject thisObj, jlong id, jint track, jboolean solo)
{
    TRACE3("Java_com_sun_media_sound_MixerSequencer_nSetTrackSolo: id: %lu track: %d solo: %d.\n", id, track, solo);
	
    if (solo)
	{
	    GM_SoloTrack((GM_Song *) (INT_PTR) id, (short int) track);
	}
    else
	{
	    GM_UnsoloTrack((GM_Song *) (INT_PTR) id, (short int) track);
	}

    TRACE0("Java_com_sun_media_sound_MixerSequencer_nSetTrackSolo reutrning\n");
}


JNIEXPORT jboolean JNICALL
Java_com_sun_media_sound_MixerSequencer_nGetTrackSolo(JNIEnv* e, jobject thisObj, jlong id, jint track)
{
    if (track >= MAX_TRACKS)
	{
	    return (jboolean)FALSE;
	}
    else
	{
	    char soloStatus[MAX_TRACKS];
	    GM_GetTrackSoloStatus((GM_Song *) (INT_PTR) id, soloStatus);

	    return (jboolean)soloStatus[track];
	}
}












JNIEXPORT jlong JNICALL
Java_com_sun_media_sound_MixerSequencer_nGetSequenceTickLength(JNIEnv* e, jobject thisObj, jlong id)
{	
    OPErr			theErr;

    TRACE0("Java_com_sun_media_sound_MixerSequencer_nGetSequenceTickLength.\n");
    VTRACE1("-> pSong: %lu\n", id);

    return (jlong)GM_GetSongTickLength((GM_Song *) (INT_PTR) id, &theErr);
}


JNIEXPORT jlong JNICALL
Java_com_sun_media_sound_MixerSequencer_nGetSequenceMicrosecondLength(JNIEnv* e, jobject thisObj, jlong id)
{
    OPErr			theErr;
	
    TRACE0("Java_com_sun_media_sound_MixerSequencer_nGetSequenceMicrosecondLength\n");
    VTRACE1("-> pSong: %lu\n", id);

    return (jlong)GM_GetSongMicrosecondLength((GM_Song *) (INT_PTR) id, &theErr);
}







/*
  JNIEXPORT void JNICALL
  Java_com_sun_media_sound_MixerSequencer_nSetMidiLoop(JNIEnv* e, jobject thisObj, jlong id, jboolean loop) {

	
  TRACE1("Java_com_sun_media_sound_MixerSequencer_nSetMidiLoop: %d.\n", (XBOOL)loop);
  VTRACE1("-> pSong: %lu\n", id);

  GM_SetSongLoopFlag((GM_Song *) (INT_PTR) id, (XBOOL)loop);

  TRACE0("Java_com_sun_media_sound_MixerSequencer_nSetMidiLoop completed.\n");
  }
*/
/*
  JNIEXPORT jfloat JNICALL
  Java_com_sun_media_sound_MixerSequencer_nGetTicksPerSecond(JNIEnv* e, jobject thisObj, jlong id) {

  UINT32 bpm;

  TRACE1("Java_com_sun_media_sound_MixerSequencer_nGetTicksPerSecond: %d.\n", (XBOOL)loop);
  VTRACE1("-> pSong: %lu\n", id);

  bpm = GM_GetSongTempoInBeatsPerMinute((GM_Song *) (INT_PTR) id);

  TRACE0("Java_com_sun_media_sound_MixerSequencer_nGetTicksPerSecond completed.\n");

  return (jfloat)(bpm * 24 * 60);
  }


  JNIEXPORT void JNICALL
  Java_com_sun_media_sound_MixerSequencer_nSetTicksPerSecond(JNIEnv* e, jobject thisObj, jlong id, jfloat ticksPerSecond) {

  TRACE1("Java_com_sun_media_sound_MixerSequencer_nSetTicksPerSecond: %d.\n", (XBOOL)loop);
  VTRACE1("-> pSong: %lu\n", id);

  GM_SetSongTempInBeatsPerMinute((GM_Song *) (INT_PTR) id, ticksPerSecond / (24 * 60));

  TRACE0("Java_com_sun_media_sound_MixerSequencer_nSetTicksPerSecond completed.\n");

  return;
  }
*/
