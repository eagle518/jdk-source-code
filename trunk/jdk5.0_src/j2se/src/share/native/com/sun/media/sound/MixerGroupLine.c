/*
 * @(#)MixerGroupLine.c	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
// STANDARD includes


// JNI includes
#include <jni.h>

// ENGINE includes
#include "engine/GenSnd.h"
#include "engine/X_API.h"


// UTILITY includes
#include "Utilities.h"																											  

// MixerGroupLine includes
#include "com_sun_media_sound_MixerGroupLine.h"


// $$kk: 06.30.99: we can't preroll here.  since we
// probably got no data on the two GET_DATA callbacks
// during open(), we are STREAM_MODE_DEAD and the 
// stream will try to start....  this is a bug!
// as a work-around, i'm going to move all this code
// to the start method.  this is not a real solution 
// because we lose the sync guarantee

JNIEXPORT jint JNICALL
Java_com_sun_media_sound_MixerGroupLine_nOpen(JNIEnv* e, jobject thisObj, jintArray idArray)
{
	OPErr theErr;
	jsize len;
	jint *body;
	
	LINKED_STREAM_REFERENCE top = NULL;
	LINKED_STREAM_REFERENCE link;

	int i;

    TRACE0("Java_com_sun_media_sound_MixerGroupLine_nOpen.\n");

	len = (*e)->GetArrayLength(e, idArray);
	body = (*e)->GetIntArrayElements(e, idArray, 0);

	for (i = 0; i < len; i++) {
		
		// preroll the stream

		// $$kk: 06.30.99: we can't preroll here.  since we
		// probably got no data on the two GET_DATA callbacks
		// during open(), we are STREAM_MODE_DEAD and the 
		// stream will try to start....  this is a bug!
		// as a work-around, i'm going to move all this code
		// to the start method.  this is not a real solution 
		// because we lose the sync guarantee
		theErr = GM_AudioStreamPreroll((STREAM_REFERENCE)body[i]);

		// if we fail to preroll any one, break and return failure
		if (theErr != NO_ERR)
		{
			top = NULL;
			break;
		}

		// add the stream to the list
		link = GM_NewLinkedStreamList((STREAM_REFERENCE)body[i], (void *)e);
		top = GM_AddLinkedStream(top, link);

		// if we ever get NULL, break and return failure
		if (top == NULL) 
		{
			break;
		}
	}

	(*e)->ReleaseIntArrayElements(e, idArray, body, 0);

    TRACE1("Java_com_sun_media_sound_MixerGroupLine_nOpen returning %d.\n", top);

	return (jint)top;
}



JNIEXPORT jboolean JNICALL
Java_com_sun_media_sound_MixerGroupLine_nClose(JNIEnv* e, jobject thisObj, jint linkRef)
{
    TRACE0("Java_com_sun_media_sound_MixerGroupLine_nClose.\n");

	GM_EndLinkedStreams((LINKED_STREAM_REFERENCE)linkRef);
	GM_FreeLinkedStreamList((LINKED_STREAM_REFERENCE)linkRef);

	return (jboolean)TRUE;
}


//JNIEXPORT jboolean JNICALL
//Java_com_sun_media_sound_MixerGroupLine_nStart(JNIEnv* e, jobject thisObj, jint linkRef)

JNIEXPORT jint JNICALL
Java_com_sun_media_sound_MixerGroupLine_nStart(JNIEnv* e, jobject thisObj, jintArray idArray)
{
	OPErr theErr;
	jsize len;
	jint *body;
	
	LINKED_STREAM_REFERENCE top = NULL;
	LINKED_STREAM_REFERENCE link;

	int i;

    TRACE0("Java_com_sun_media_sound_MixerGroupLine_nStart.\n");

	len = (*e)->GetArrayLength(e, idArray);
	body = (*e)->GetIntArrayElements(e, idArray, 0);

	TRACE1("Java_com_sun_media_sound_MixerGroupLine_nStart: %d streams\n", len);
		
	for (i = 0; i < len; i++) {
		
		// prebuffer the stream
		theErr = GM_AudioStreamPrebuffer((STREAM_REFERENCE)body[i], (void *)e);

		if (theErr != NO_ERR)
		{
			ERROR3("Java_com_sun_media_sound_MixerGroupLine_nStart: GM_AudioStreamPrebuffer returned %d for stream %d, id %d\n", theErr, i, body[i]);
		}
		else
		{
			// preroll the stream
			theErr = GM_AudioStreamPreroll((STREAM_REFERENCE)body[i]);
		}

		// if we fail to preroll any one, break and return failure
		if (theErr != NO_ERR)
		{
			ERROR3("Java_com_sun_media_sound_MixerGroupLine_nStart: GM_AudioStreamPreroll returned %d for stream %d, id %d\n", theErr, i, body[i]);
			top = NULL;
			break;
		}

		// add the stream to the list.  
		// $$kk: 07.01.99: i think the only ways these two methods can fail
		// are if the id is not a valid stream or there's not enough memory
		// to allocate the structures....
		link = GM_NewLinkedStreamList((STREAM_REFERENCE)body[i], (void *)e);
		top = GM_AddLinkedStream(top, link);

		// if we ever get NULL, break and return failure
		if (top == NULL) 
		{
			ERROR2("Java_com_sun_media_sound_MixerGroupLine_nStart: (top == NULL) for stream %d, id %d\n", i, body[i]);
			break;
		}
	}


	(*e)->ReleaseIntArrayElements(e, idArray, body, 0);

	if ( (theErr == NO_ERR) && (top) )
	{
		TRACE1("Java_com_sun_media_sound_MixerGroupLine_nStart: calling GM_StartLinkedStreams with %d\n", top);
		theErr = GM_StartLinkedStreams((LINKED_STREAM_REFERENCE)top);
	}

	if (theErr != NO_ERR) 
	{
		ERROR1("Java_com_sun_media_sound_MixerGroupLine_nStart: GM_StartLinkedStreams returned %d\n", theErr);
		TRACE1("Java_com_sun_media_sound_MixerGroupLine_nStart: GM_StartLinkedStreams returned %d\n", theErr);
		if (top)
		{
			TRACE0("Java_com_sun_media_sound_MixerGroupLine_nStart: calling GM_FreeLinkedStreamList\n");
			GM_FreeLinkedStreamList(top);
		}
		TRACE0("Java_com_sun_media_sound_MixerGroupLine_nStart: (setting top = NULL)\n");
		top = NULL;
	}

    TRACE1("Java_com_sun_media_sound_MixerGroupLine_nStart returning %d.\n", top);
	return (jint)top;
}


JNIEXPORT jboolean JNICALL
Java_com_sun_media_sound_MixerGroupLine_nResume(JNIEnv* e, jobject thisObj, jintArray idArray)
{
	int i;
	jsize len;
	jint *body;
	//LINKED_STREAM_REFERENCE link;

    TRACE0("Java_com_sun_media_sound_MixerGroupLine_nResume.\n");

	len = (*e)->GetArrayLength(e, idArray);
	body = (*e)->GetIntArrayElements(e, idArray, 0);

	for (i = 0; i < len; i++) {

		// $$kk: 07.01.99: we are not using the linked stream methods here!
		//link = (LINKED_STREAM_REFERENCE)body[i];
		//GM_AudioStreamResume(GM_GetLinkedStreamPlaybackReference(link));
		GM_AudioStreamResume((STREAM_REFERENCE)body[i]);
	}

	(*e)->ReleaseIntArrayElements(e, idArray, body, 0);

	return (jboolean)TRUE;
}


JNIEXPORT jboolean JNICALL
Java_com_sun_media_sound_MixerGroupLine_nPause(JNIEnv* e, jobject thisObj, jintArray idArray)
{
	int i;
	jsize len;
	jint *body;
	//LINKED_STREAM_REFERENCE link;

    TRACE0("Java_com_sun_media_sound_MixerGroupLine_nPause.\n");

	len = (*e)->GetArrayLength(e, idArray);
	body = (*e)->GetIntArrayElements(e, idArray, 0);

	for (i = 0; i < len; i++) {

		// $$kk: 07.01.99: we are not using the linked stream methods here!
		//link = (LINKED_STREAM_REFERENCE)body[i];
		//GM_AudioStreamPause(GM_GetLinkedStreamPlaybackReference(link));
		GM_AudioStreamPause((STREAM_REFERENCE)body[i]);
	}

	(*e)->ReleaseIntArrayElements(e, idArray, body, 0);

    TRACE0("Java_com_sun_media_sound_MixerGroupLine_nPause completed.\n");
	return (jboolean)TRUE;
}



// CONTROLS

JNIEXPORT jfloat JNICALL
Java_com_sun_media_sound_MixerGroupLine_nSetLinearGain(JNIEnv* e, jobject thisObj, jint linkRef, jfloat linearGain)
{

    TRACE1("Java_com_sun_media_sound_MixerGroupLine_nSetLinearGain %d.\n", linkRef);

	GM_SetLinkedStreamVolume((LINKED_STREAM_REFERENCE)linkRef, FLOAT_TO_VOLUME(linearGain), FALSE);

    TRACE1("Java_com_sun_media_sound_MixerGroupLine_nSetLinearGain %d completed.\n", linkRef);

	// $$kk: 06.29.99: there's no "get" method.  maybe should check the value on a one stream?
	// ugh... not synchronized, not reliable....
	return (linearGain);
}


JNIEXPORT jfloat JNICALL
Java_com_sun_media_sound_MixerGroupLine_nSetPan(JNIEnv* e, jobject thisObj, jint linkRef, jfloat pan)
{

    TRACE1("Java_com_sun_media_sound_MixerGroupLine_nSetPan %d.\n", linkRef);
 	VTRACE1("-> stream linkRef: %d\n", (LINKED_STREAM_REFERENCE)linkRef);

	GM_SetLinkedStreamPosition((LINKED_STREAM_REFERENCE)linkRef, (short int)(FLOAT_TO_PAN(pan)));
    TRACE1("Java_com_sun_media_sound_MixerGroupLine_nSetPan %d completed.\n", linkRef);

	// $$kk: 06.29.99: there's no "get" method.  maybe should check the value on a one stream?
	// ugh... not synchronized, not reliable....
    return (pan);
}


JNIEXPORT jint JNICALL
Java_com_sun_media_sound_MixerGroupLine_nSetSampleRate(JNIEnv* e, jobject thisObj, jint linkRef, jint rate)
{

TRACE1("Java_com_sun_media_sound_MixerGroupLine_nSetSampleRate %d.\n", linkRef);
VTRACE1("-> stream linkRef: %d\n", (LINKED_STREAM_REFERENCE)linkRef);
	
// $$kk: 04.06.99: steve hales says i should be using the UNSIGNED_LONG macros here.  
GM_SetLinkedStreamRate((LINKED_STREAM_REFERENCE)linkRef, UNSIGNED_LONG_TO_XFIXED(rate));
TRACE1("Java_com_sun_media_sound_MixerGroupLine_nSetSampleRate %d completed.\n", linkRef);

// $$kk: 06.29.99: there's no "get" method.  maybe should check the value on a one stream?
// ugh... not synchronized, not reliable....
return (rate);
}
*/
