/*
 * @(#)MixerThread.c	1.27 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#if JAVA_THREAD

//#define USE_TRACE

#include "Utilities.h"
#include "engine/X_API.h"
#include "engine/HAE_API.h"
#include "com_sun_media_sound_MixerThread.h"



// STATIC GLOBAL VARIABLES

// the MixerThread java thread class
static jclass globalThreadClass = NULL;

// the java/lang/Thread class
static jclass globalBaseThreadClass = NULL;

// MixerThread sleep method id
static jmethodID globalThreadSleepMethodID  = NULL;

static jmethodID globalGetExistingThreadObjectMethodID = NULL;
static jmethodID globalGetNewThreadObjectMethodID = NULL;

static jmethodID globalThreadUnpauseMethodID = NULL;
static jmethodID globalThreadStartMethodID = NULL;



// NATIVE METHOD IMPLEMENTATION

// called by HaeThread.run()
// calls the proc passed into HAE_CreateFrameThread()
JNIEXPORT void JNICALL Java_com_sun_media_sound_MixerThread_runNative(JNIEnv* e, jobject thisObj, jlong frameProc)
{
    HAE_FrameThreadProc	theFrameProc;

    //TRACE3("Java_com_sun_media_sound_MixerThread_runNative, e: %d, thisObj: %lu, frameProc: %lu\n", e, thisObj, frameProc);	
    TRACE0("Java_com_sun_media_sound_MixerThread_runNative\n");	

    theFrameProc = (HAE_FrameThreadProc) (INT_PTR) frameProc;
	
    if (theFrameProc)
	{
	    (*theFrameProc)((void *)e);
	}
    else
	{
	    ERROR0("Java_com_sun_media_sound_MixerThread_runNative: theFrameProc is NULL");
	}

    TRACE0("Java_com_sun_media_sound_MixerThread_runNative completed\n");
}


// HEADSPACE THREAD API IMPLEMENTATIONS

// The frame thread is created and started only once.
// If it already exists, unPause() is called which causes run() to exit
// its wait() loop.
// returns 0 for success, -1 for failure
int HAE_CreateFrameThread(void* context, HAE_FrameThreadProc proc)
{	 
    jclass threadClass;
    jclass baseThreadClass;
    jobject threadObj;
    jobject globalThreadObj;
    JNIEnv* e = (JNIEnv*)context;
	
    //TRACE2("HAE_CreateFrameThread, context: %lu, proc: %lu\n", context, proc);	
    TRACE0("HAE_CreateFrameThread\n");	

    // get the java thread class and method id's
    if (globalThreadClass == NULL)
	{
	    threadClass = (*e)->FindClass(e, "com/sun/media/sound/MixerThread");

	    if (threadClass == 0)
		{
		    ERROR0("HAE_CreateFrameThread: Failed to get thread class\n");
		    return GENERAL_BAD;
		}

	    globalThreadClass = (*e)->NewGlobalRef(e, threadClass);

	    // It would seem like we can use threadClass to invoke
	    // the static "sleep" method.  But for some reason, MS IE
	    // doesn't like that.  It requires that the static method
	    // "sleep" be invoked from the base "java.lang.Thread" class.
	    // Otherwise, it will give a JNI exception and die.

	    baseThreadClass = (*e)->FindClass(e, "java/lang/Thread");
	    globalBaseThreadClass = (*e)->NewGlobalRef(e, baseThreadClass);
	    globalThreadSleepMethodID = (*e)->GetStaticMethodID(e, globalBaseThreadClass, "sleep", "(J)V");

	    if (globalThreadSleepMethodID == 0)
		{
		    ERROR0("HAE_CreateFrameThread: Failed to get thread sleep method ID\n");
		    return GENERAL_BAD;
		}

	    globalGetExistingThreadObjectMethodID = (*e)->GetStaticMethodID(e, globalThreadClass, "getExistingThreadObject", "(J)Lcom/sun/media/sound/MixerThread;");
	    globalGetNewThreadObjectMethodID = (*e)->GetStaticMethodID(e, globalThreadClass, "getNewThreadObject", "(J)Lcom/sun/media/sound/MixerThread;");

	    if ( (globalGetExistingThreadObjectMethodID == 0) || (globalGetNewThreadObjectMethodID == 0) )
		{
		    ERROR0("HAE_CreateFrameThread: Failed to get globalGetExisting/NewThreadObjectMethodID\n");
		    return GENERAL_BAD;
		}

	    globalThreadUnpauseMethodID = (*e)->GetMethodID(e, globalThreadClass, "unpause", "()V");
	    globalThreadStartMethodID = (*e)->GetMethodID(e, globalThreadClass, "start", "()V");

	    if ( (globalThreadUnpauseMethodID == 0) || (globalThreadStartMethodID == 0) )
		{
		    ERROR0("HAE_CreateFrameThread: Failed to get globalThreadUnpause/StartMethodID\n");
		    return GENERAL_BAD;
		}
	}

    // try to get an existing thread object for this frameProc
    threadObj = (*e)->CallStaticObjectMethod(e, globalThreadClass, globalGetExistingThreadObjectMethodID, (jlong) (INT_PTR) proc);
    if (threadObj != NULL)
	{
	    TRACE0("HAE_CreateFrameThread: Got existing threadObj\n");
	    (*e)->CallVoidMethod(e, threadObj, globalThreadUnpauseMethodID);	  

	    return 0;
	}
    else
	{
	    // create a new object
	    threadObj = (*e)->CallStaticObjectMethod(e, globalThreadClass, globalGetNewThreadObjectMethodID, (jlong) (INT_PTR) proc);

	    if (threadObj == NULL)
		{
		    ERROR0("HAE_CreateFrameThread: Failed to get new threadObj\n");
		    return GENERAL_BAD;
		}

	    // create a global reference to the java thread object
	    // $$kk: 04.20.99: how are we ever going to get rid of this??
	    globalThreadObj = (*e)->NewGlobalRef(e, threadObj);

	    // start the thread
	    (*e)->CallVoidMethod(e, globalThreadObj, globalThreadStartMethodID);
	    return 0;
	}
}

// Called by HAE_ReleaseAudioCard() where global variables are set to cause
// (*theFrameProc)() and runNative() to return.
// run() will then call wait() in a loop, to be interrupted by
// a call to unPause().
// Don't actually destroy the frame thread, just prevent
// runNative() doing anything until HAE_CreateFrameThread() is called.

// $$kk: 03.19.98: okay, when do we actually kill the java thread???

int HAE_DestroyFrameThread(void* context)
{
    TRACE0("HAE_DestroyFrameThread\n");	  

    /*
      // kill the java thread

      // delete the global reference

      // set the vars to NULL
    */

    TRACE0("HAE_DestroyFrameThread completed \n");	  

    return 0;
}


// Make the frame thread sleep for the given number of milliseconds
int HAE_SleepFrameThread(void* context, INT32 msec)
{
    JNIEnv* e = (JNIEnv*)context;

    //VTRACE2("HAE_SleepFrameThread: context %lu, msec %d\n", context, msec);

    (*e)->CallStaticVoidMethod(e, globalBaseThreadClass, globalThreadSleepMethodID, (jlong)msec);

    //VTRACE0("HAE_SleepFrameThread: completed\n");

    return 0;
}

#endif  // JAVA_THREAD


