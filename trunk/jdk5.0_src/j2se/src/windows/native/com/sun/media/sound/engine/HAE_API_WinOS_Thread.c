/*
 * @(#)HAE_API_WinOS_Thread.c	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**	HAE_API_WinOS_Thread.c
**
**	This provides platform-specfic frame-thread-managment functions for
**  Windows 95/NT.
**
**	Overview:
**
**	History	-
**	12/1/97		Created
**	8/12/98		Modified PV_NativeFrameThreadProc to eliminate a warning
**	3/5/99		Changed context to threadContext to reflect what is really is used
**				in the system.
*/
/*****************************************************************************/

#if JAVA_THREAD
#if !JAVA_SOUND
#error JAVA_THREAD defined when JAVA_SOUND not defined
#endif
#else

#if JAVA_SOUND
#include <jni.h>
#endif

#ifndef WIN32_EXTRA_LEAN
#define WIN32_EXTRA_LEAN
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "HAE_API.h"


////////////////////////////////////////////////// SWITCHES:

#ifndef _MT
#define USE_WIN32_THREAD_API	TRUE
#else
#define USE_WIN32_THREAD_API	FALSE
#endif
	
#if USE_WIN32_THREAD_API == FALSE
#ifndef _MT
#error if (USE_WIN32_THREAD_API == TRUE) "_MT" must be defined!
#endif
#include <process.h>
#endif


////////////////////////////////////////////////// MACROS:

#if _DEBUG		//CLS?? Why isn't this in some shared header file?

#include <stdio.h>
#define DEBUG_STR(x)	printf(x)
#else
#define DEBUG_STR(x)
#endif


////////////////////////////////////////////////// STATIC VARIABLES:

// Thread used to build audio frames
static HANDLE				theFrameThread = NULL;
// The function to call 
static HAE_FrameThreadProc	theFrameProc = NULL;


////////////////////////////////////////////////// FORWARDS:

// WINAPI makes the function _stdcall
#if USE_WIN32_THREAD_API
#define THREAD_CALL	DWORD WINAPI
#else
#define THREAD_CALL	unsigned int WINAPI
#endif
static THREAD_CALL PV_NativeFrameThreadProc(LPVOID lpv);


static void*		PV_CreateAudioContext(void);
static void			PV_DestroyAudioContext(void);


////////////////////////////////////////////////// THREAD-API FUNCTIONS:

// Create and start the frame thread
int HAE_CreateFrameThread(void* threadContext, HAE_FrameThreadProc proc)
{
    threadContext;
    if (proc)
	{
#if USE_WIN32_THREAD_API
	    DWORD	threadID;
#else
	    unsigned int	threadID;
#endif
	    theFrameProc = proc;
#if USE_WIN32_THREAD_API
	    theFrameThread = CreateThread(NULL, 0L, 
					  PV_NativeFrameThreadProc,
					  0L, 0L, &threadID);
#else
	    theFrameThread = (HANDLE)_beginthreadex(NULL, 0L,
						    PV_NativeFrameThreadProc,
						    NULL, 0, &threadID);
#endif
	    if (theFrameThread)
		{
		    // THREAD_PRIORITY_HIGHEST
		    // THREAD_PRIORITY_NORMAL
		    // THREAD_PRIORITY_TIME_CRITICAL
		    SetThreadPriority(theFrameThread, THREAD_PRIORITY_TIME_CRITICAL);
		    return 0;
		}
	    theFrameProc = NULL;
	}
    return -1;
}
// Stop and destroy the frame thread, 
int HAE_DestroyFrameThread(void* threadContext)
{
    threadContext;
    if (theFrameThread)
	{
	    DWORD		result;

	    result = WaitForSingleObject(theFrameThread, 500);	// 500 milliseconds
	    // If thread takes too long, kill it
	    if (result == WAIT_TIMEOUT)
		{
		    TerminateThread(theFrameThread, 0);
		}
	    CloseHandle(theFrameThread);
	    theFrameThread = NULL;
	    theFrameProc = NULL;
	    return 0;
	}
    DEBUG_STR("PV_NativeFrameThreadProc() called when no thread started\n");
    return -1;
}
// Make the frame thread sleep for the given number of milliseconds
int HAE_SleepFrameThread(void* threadContext, long msec)
{
    threadContext;
    Sleep(msec);
    return  0;
}

static THREAD_CALL PV_NativeFrameThreadProc(LPVOID lpv)
{
    lpv;	// eliminate unreferenced-parameter warning

    if (theFrameProc)
	{
	    (*theFrameProc)(PV_CreateAudioContext());
	    PV_DestroyAudioContext();
	}
    else
	{
	    DEBUG_STR("PV_NativeFrameThreadProc() called when thread supposed to be dead!");
	}
#if USE_WIN32_THREAD_API
    ExitThread(0);
#else
    _endthreadex(0);
#endif
    return 0;
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
// EOF of HAE_API_WinOS_Thread.c
