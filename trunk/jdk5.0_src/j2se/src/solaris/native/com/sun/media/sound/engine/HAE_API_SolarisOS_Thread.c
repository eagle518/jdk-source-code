/*
 * @(#)HAE_API_SolarisOS_Thread.c	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**	HAE_API_SolarisOS_Thread.c
**
**	This provides platform-specfic frame-thread-managment functions for
**  Windows 95/NT.
**
**	Overview:
**
**	Written by Moe
**
**	Confidential-- Internal use only
**
**	History	-
**	12/1/97		Created
**	4/9/98		igor: fixed logic problems with HAE_CreateFrameThread
*/
/*****************************************************************************/

// This thread hack is used only for the Solaris plugin.

#define JAVA_HACK_THREAD	1

#if JAVA_THREAD
#if !JAVA_SOUND
#error JAVA_THREAD defined when JAVA_SOUND not defined
#endif
#else


#include <thread.h>
#if JAVA_SOUND
#include <jni.h>
#endif

#include "HAE_API.h"

#if JAVA_HACK_THREAD
#include <sys/syscall.h>
#include "jri.h"
static JRIEnv *javaEnv  = NULL;
static jref    javaPeer = NULL;
#endif

////////////////////////////////////////////////// MACROS:

#if _DEBUG		//CLS?? Why isn't this in some shared header file?
#define DEBUG_STR(x)	printf(x)
#else
#define DEBUG_STR(x)
#endif

////////////////////////////////////////////////// STATIC VARIABLES:

// Thread used to build audio frames
static thread_t			theFrameThread = NULL;
// The function that the thread calls
static HAE_FrameThreadProc	theFrameProc = NULL;


////////////////////////////////////////////////// FORWARDS:

static void*	PV_NativeFrameThreadProc(void* arg);
static void*	PV_CreateAudioContext(void);
static void		PV_DestroyAudioContext(void);


////////////////////////////////////////////////// THREAD-API FUNCTIONS:

#if JAVA_HACK_THREAD == 0
// Create and start the frame thread
int HAE_CreateFrameThread(void* context, HAE_FrameThreadProc proc)
{
    if (proc)
	{
	    long	error;

	    theFrameProc = proc;
	    error = thr_create(NULL, NULL, PV_NativeFrameThreadProc, NULL,
			       THR_BOUND | THR_NEW_LWP,
			       &theFrameThread);
	    if (error == 0) 
		{
		    error = thr_setprio(theFrameThread, 127);
		    if (error == 0)
			{
			    return 0;
			}
		    else
			{
			    DEBUG_STR("Failure to set priority of Solaris thread\n");
			}
		}
	    else
		{
		    DEBUG_STR("Failure to create Solaris thread\n");
		}
	    theFrameProc = NULL;
	}
    return -1;
}
// Stop and destroy the frame thread, wait up to 1/2 second for audio to finish
int HAE_DestroyFrameThread(void* context)
{
    if (theFrameThread)
	{
	    thr_kill(theFrameThread, 0);
	    theFrameThread = NULL;
	    theFrameProc = NULL;
	    return 0;
	}
    DEBUG_STR("PV_NativeFrameThreadProc() called when no thread started\n");
    return -1;
}
// Make the frame thread sleep for the given number of milliseconds
int HAE_SleepFrameThread(void* context, long msec)
{
    struct timespec request, actual;
    int sleep_ret;

    request.tv_sec = 0;
    request.tv_nsec = msec * 1000;
    sleep_ret = nanosleep(&request, &actual);

    return  0;
}

static void* PV_NativeFrameThreadProc(void* arg)
{
    if (theFrameProc)
	{
	    (*theFrameProc)(PV_CreateAudioContext());
	    PV_DestroyAudioContext();
	}
    else
	{
	    DEBUG_STR("PV_NativeFrameThreadProc() called when thread supposed to be dead!");
	}
    return NULL;
}


////////////////////////////////////////////////// HELPER FUNCTIONS:

#if JAVA_SOUND

static JavaVM* PV_GetCurrentVM(void)
{
    JavaVM*		vmArray[2];
    jsize		count;
    jint		result;

    count = 0;
    result = JNI_GetCreatedJavaVMs(vmArray, 2, &count);	// ask for 2
    if ((result == 0) && (count >= 1))						// error if not just 1
	{
	    if (count > 1)
		{
		    DEBUG_STR("WARNING:  JNI_GetCreatedJavaVMs() returned >1 VMs\n");
		}
	    return vmArray[0];
	}
    DEBUG_STR("JNI_GetCreatedJavaVMs() returned error\n");
    return NULL;
}
static void* PV_CreateAudioContext(void)
{
    JavaVM*		const vm = PV_GetCurrentVM();

    if (vm)
	{
	    JNIEnv*				env;
	    JDK1_1AttachArgs	args;	// whatever
	    jint				result;

	    result = (*vm)->AttachCurrentThread(vm, &env, &args);
	    if (result >= 0)
		{
		    return (void*)env;
		}
	    DEBUG_STR("AttachCurrentThread() failed\n");
	}
    return NULL;
}
static void PV_DestroyAudioContext(void)
{
    JavaVM*		const vm = PV_GetCurrentVM();

    if (vm)
	{
	    jint		result;

	    result = (*vm)->DetachCurrentThread(vm);
	    if (result < 0)
		{
		    DEBUG_STR("DetachCurrentThread() failed\n");
		}
	}
}

#else

static void * PV_CreateAudioContext(void)
{
    return NULL;
}
static void PV_DestroyAudioContext(void)
{
}

#endif


#endif	// !JAVA_THREAD

#endif	// JAVA_HACK_THREAD == 0


#if JAVA_HACK_THREAD

////////////////////////////////////////////////// THREAD-API FUNCTIONS:

extern void HAE_RunThread(void);

// Create and start the frame thread
int HAE_CreateFrameThread(void* context, HAE_FrameThreadProc proc)
{
    if (proc)
	{
	    theFrameProc = proc;
	}
    return 0;
}
// Stop and destroy the frame thread, wait up to 1/2 second for audio to finish
int HAE_DestroyFrameThread(void* context)
{
    return 0;
}

static void * PV_CreateAudioContext(void)
{
    return NULL;
}
static void PV_DestroyAudioContext(void)
{
}

// Make the frame thread sleep for the given number of milliseconds
int HAE_SleepFrameThread(void* context, long msec)
{
#if 0
    if (javaEnv != NULL && javaPeer != NULL)
	{
	    java_lang_Thread_sleep(javaEnv, javaPeer, msec);
	}
#else
    struct timespec request, actual;
    int sleep_ret;

    request.tv_sec = 0;
    request.tv_nsec = msec * 1000;
    sleep_ret = nanosleep(&request, &actual);
#endif

    return  0;
}

extern void HAE_RunThread(void);
void HAE_RunThread(void)
{
    PV_NativeFrameThreadProc(NULL);
}

void setPluginReference(JRIEnv *env, jref peer)
{
    javaEnv = env;
    javaPeer = peer;
}

static void* PV_NativeFrameThreadProc(void* arg)
{
    if (theFrameProc)
	{
	    (*theFrameProc)(PV_CreateAudioContext());
	    PV_DestroyAudioContext();
	}
    else
	{
	    DEBUG_STR("PV_NativeFrameThreadProc() called when thread supposed to be dead!");
	}
    return NULL;
}

#endif	// JAVA_HACK_THREAD

// EOF of HAE_API_WinOS_Thread.c
