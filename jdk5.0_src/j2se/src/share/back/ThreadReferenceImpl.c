/*
 * @(#)ThreadReferenceImpl.c	1.64 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "util.h"
#include "ThreadReferenceImpl.h"
#include "eventHandler.h"
#include "threadControl.h"
#include "inStream.h"
#include "outStream.h"
#include "FrameID.h"

static jboolean 
name(PacketInputStream *in, PacketOutputStream *out) 
{
    JNIEnv *env;
    jthread thread;

    thread = inStream_readThreadRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    if (threadControl_isDebugThread(thread)) {
        outStream_setError(out, JDWP_ERROR(INVALID_THREAD));
        return JNI_TRUE;
    }

    env = getEnv();
    
    WITH_LOCAL_REFS(env, 1) {
	
	jvmtiThreadInfo info;
	jvmtiError error;
        
	(void)memset(&info, 0, sizeof(info));

	error = JVMTI_FUNC_PTR(gdata->jvmti,GetThreadInfo)
				(gdata->jvmti, thread, &info);
	
	if (error != JVMTI_ERROR_NONE) {
	    outStream_setError(out, map2jdwpError(error));
	} else {
	    (void)outStream_writeString(out, info.name);
	}
	
	if ( info.name != NULL )
	    jvmtiDeallocate(info.name);
    
    } END_WITH_LOCAL_REFS(env);
    
    return JNI_TRUE;
}

static jboolean 
suspend(PacketInputStream *in, PacketOutputStream *out) 
{
    jvmtiError error;
    jthread thread;
    
    thread = inStream_readThreadRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    if (threadControl_isDebugThread(thread)) {
        outStream_setError(out, JDWP_ERROR(INVALID_THREAD));
        return JNI_TRUE;
    }
    error = threadControl_suspendThread(thread, JNI_FALSE);
    if (error != JVMTI_ERROR_NONE) {
        outStream_setError(out, map2jdwpError(error));
    }
    return JNI_TRUE;
}

static jboolean 
resume(PacketInputStream *in, PacketOutputStream *out) 
{
    jvmtiError error;
    jthread thread;
    
    thread = inStream_readThreadRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    if (threadControl_isDebugThread(thread)) {
        outStream_setError(out, JDWP_ERROR(INVALID_THREAD));
        return JNI_TRUE;
    }

    /* true means it is okay to unblock the commandLoop thread */
    error = threadControl_resumeThread(thread, JNI_TRUE);
    if (error != JVMTI_ERROR_NONE) {
        outStream_setError(out, map2jdwpError(error));
    }
    return JNI_TRUE;
}

static jboolean 
status(PacketInputStream *in, PacketOutputStream *out) 
{
    jdwpThreadStatus threadStatus;
    jint statusFlags;
    jvmtiError error;
    jthread thread;
    
    thread = inStream_readThreadRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    if (threadControl_isDebugThread(thread)) {
        outStream_setError(out, JDWP_ERROR(INVALID_THREAD));
        return JNI_TRUE;
    }

    error = threadControl_applicationThreadStatus(thread, &threadStatus, 
                                                          &statusFlags);
    if (error != JVMTI_ERROR_NONE) {
        outStream_setError(out, map2jdwpError(error));
        return JNI_TRUE;
    }
    (void)outStream_writeInt(out, threadStatus);
    (void)outStream_writeInt(out, statusFlags);
    return JNI_TRUE;
}

static jboolean 
threadGroup(PacketInputStream *in, PacketOutputStream *out) 
{
    JNIEnv *env;
    jthread thread;
    
    thread = inStream_readThreadRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    if (threadControl_isDebugThread(thread)) {
        outStream_setError(out, JDWP_ERROR(INVALID_THREAD));
        return JNI_TRUE;
    }

    env = getEnv();
    
    WITH_LOCAL_REFS(env, 1) {

	jvmtiThreadInfo info;
	jvmtiError error;
    
	(void)memset(&info, 0, sizeof(info));
	
	error = JVMTI_FUNC_PTR(gdata->jvmti,GetThreadInfo)
				(gdata->jvmti, thread, &info);
	
	if (error != JVMTI_ERROR_NONE) {
	    outStream_setError(out, map2jdwpError(error));
	} else {
	    (void)outStream_writeObjectRef(out, info.thread_group);
        } 
	
	if ( info.name!=NULL )
	    jvmtiDeallocate(info.name);
    
    } END_WITH_LOCAL_REFS(env);

    return JNI_TRUE;
}

static jboolean 
validateSuspendedThread(PacketOutputStream *out, jthread thread)
{
    jvmtiError error;
    jint count;
    
    error = threadControl_suspendCount(thread, &count);
    if (error != JVMTI_ERROR_NONE) {
        outStream_setError(out, map2jdwpError(error));
        return JNI_FALSE;
    }

    if (count == 0) {
        outStream_setError(out, JDWP_ERROR(THREAD_NOT_SUSPENDED));
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

static jboolean 
frames(PacketInputStream *in, PacketOutputStream *out) 
{
    jvmtiError error;
    FrameNumber fnum;
    jint count;
    JNIEnv *env;
    jthread thread;
    jint startIndex;
    jint length;

    thread = inStream_readThreadRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }
    startIndex = inStream_readInt(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }
    length = inStream_readInt(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    if (threadControl_isDebugThread(thread)) {
        outStream_setError(out, JDWP_ERROR(INVALID_THREAD));
        return JNI_TRUE;
    }

    if (!validateSuspendedThread(out, thread)) {
        return JNI_TRUE;
    }

    error = JVMTI_FUNC_PTR(gdata->jvmti,GetFrameCount)
			(gdata->jvmti, thread, &count);
    if (error != JVMTI_ERROR_NONE) {
        outStream_setError(out, map2jdwpError(error));
        return JNI_TRUE;
    }

    if (length == -1) {
        length = count - startIndex;
    }

    if (length == 0) {
        (void)outStream_writeInt(out, 0);
        return JNI_TRUE;
    }

    if ((startIndex < 0) || (startIndex > count - 1)) {
        outStream_setError(out, JDWP_ERROR(INVALID_INDEX));
        return JNI_TRUE;
    }

    if ((length < 0) || (length + startIndex > count)) {
        outStream_setError(out, JDWP_ERROR(INVALID_LENGTH));
        return JNI_TRUE;
    }

    (void)outStream_writeInt(out, length);
    
    env = getEnv();
    
    for(fnum = startIndex ; fnum < startIndex+length ; fnum++ ) {
    
	WITH_LOCAL_REFS(env, 1) {
	   
	    jclass clazz;
	    jmethodID method;
	    jlocation location;

	    /* Get location info */
	    error = JVMTI_FUNC_PTR(gdata->jvmti,GetFrameLocation)
		(gdata->jvmti, thread, fnum, &method, &location);
	    if (error == JVMTI_ERROR_OPAQUE_FRAME) {
		clazz = NULL;
		location = -1L;
		error = JVMTI_ERROR_NONE;
	    } else if ( error == JVMTI_ERROR_NONE ) {
		error = methodClass(method, &clazz);
		if ( error == JVMTI_ERROR_NONE ) {
		    FrameID frame;
		    frame = createFrameID(thread, fnum);
		    (void)outStream_writeFrameID(out, frame);
		    writeCodeLocation(out, clazz, method, location);
		}
	    }
	
	} END_WITH_LOCAL_REFS(env);
	
	if (error != JVMTI_ERROR_NONE)
	    break;
        
    }

    if (error != JVMTI_ERROR_NONE) {
        outStream_setError(out, map2jdwpError(error));
    }
    return JNI_TRUE;
}

static jboolean 
getFrameCount(PacketInputStream *in, PacketOutputStream *out) 
{
    jvmtiError error;
    jint count;
    jthread thread;

    thread = inStream_readThreadRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    if (threadControl_isDebugThread(thread)) {
        outStream_setError(out, JDWP_ERROR(INVALID_THREAD));
        return JNI_TRUE;
    }

    if (!validateSuspendedThread(out, thread)) {
        return JNI_TRUE;
    }

    error = JVMTI_FUNC_PTR(gdata->jvmti,GetFrameCount)
			(gdata->jvmti, thread, &count);
    if (error != JVMTI_ERROR_NONE) {
        outStream_setError(out, map2jdwpError(error));
        return JNI_TRUE;
    }
    (void)outStream_writeInt(out, count);

    return JNI_TRUE;
}

static jboolean 
ownedMonitors(PacketInputStream *in, PacketOutputStream *out)
{
    JNIEnv *env;
    jthread thread;
    
    thread = inStream_readThreadRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    if (threadControl_isDebugThread(thread)) {
        outStream_setError(out, JDWP_ERROR(INVALID_THREAD));
        return JNI_TRUE;
    }

    if (!validateSuspendedThread(out, thread)) {
        return JNI_TRUE;
    }

    env = getEnv();
    
    WITH_LOCAL_REFS(env, 1) {
	
	jvmtiError error;
	jint count = 0;
	jobject *monitors = NULL;
	
	error = JVMTI_FUNC_PTR(gdata->jvmti,GetOwnedMonitorInfo)
				(gdata->jvmti, thread, &count, &monitors);
	if (error != JVMTI_ERROR_NONE) {
	    outStream_setError(out, map2jdwpError(error));
	} else {
	    int i;
	    (void)outStream_writeInt(out, count);
	    for (i = 0; i < count; i++) {
		jobject monitor = monitors[i];
		(void)outStream_writeByte(out, specificTypeKey(monitor));
		(void)outStream_writeObjectRef(out, monitor);
	    }
	}
	if (monitors != NULL)
	    jvmtiDeallocate(monitors);

    } END_WITH_LOCAL_REFS(env);
    
    return JNI_TRUE;
}

static jboolean 
currentContendedMonitor(PacketInputStream *in, PacketOutputStream *out)
{
    JNIEnv *env;
    jthread thread;
    
    thread = inStream_readThreadRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    if (threadControl_isDebugThread(thread)) {
        outStream_setError(out, JDWP_ERROR(INVALID_THREAD));
        return JNI_TRUE;
    }

    if (!validateSuspendedThread(out, thread)) {
        return JNI_TRUE;
    }

    env = getEnv();
    
    WITH_LOCAL_REFS(env, 1) {
	
	jobject monitor;
	jvmtiError error;
	
	error = JVMTI_FUNC_PTR(gdata->jvmti,GetCurrentContendedMonitor)
				(gdata->jvmti, thread, &monitor);
	
	if (error != JVMTI_ERROR_NONE) {
	    outStream_setError(out, map2jdwpError(error));
	} else {
	    (void)outStream_writeByte(out, specificTypeKey(monitor));
	    (void)outStream_writeObjectRef(out, monitor);
	}
    
    } END_WITH_LOCAL_REFS(env);
    
    return JNI_TRUE;
}

static jboolean 
stop(PacketInputStream *in, PacketOutputStream *out) 
{
    jvmtiError error;
    jthread thread;
    jobject throwable;
    
    thread = inStream_readThreadRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }
    throwable = inStream_readObjectRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    if (threadControl_isDebugThread(thread)) {
        outStream_setError(out, JDWP_ERROR(INVALID_THREAD));
        return JNI_TRUE;
    }

    error = threadControl_stop(thread, throwable);
    if (error != JVMTI_ERROR_NONE) {
        outStream_setError(out, map2jdwpError(error));
    }
    return JNI_TRUE;
}

static jboolean 
interrupt(PacketInputStream *in, PacketOutputStream *out) 
{
    jvmtiError error;
    jthread thread;
    
    thread = inStream_readThreadRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    if (threadControl_isDebugThread(thread)) {
        outStream_setError(out, JDWP_ERROR(INVALID_THREAD));
        return JNI_TRUE;
    }

    error = threadControl_interrupt(thread);
    if (error != JVMTI_ERROR_NONE) {
        outStream_setError(out, map2jdwpError(error));
    }
    return JNI_TRUE;
}

static jboolean 
suspendCount(PacketInputStream *in, PacketOutputStream *out) 
{
    jvmtiError error;
    jint count;
    jthread thread;
    
    thread = inStream_readThreadRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    if (threadControl_isDebugThread(thread)) {
        outStream_setError(out, JDWP_ERROR(INVALID_THREAD));
        return JNI_TRUE;
    }

    error = threadControl_suspendCount(thread, &count);
    if (error != JVMTI_ERROR_NONE) {
        outStream_setError(out, map2jdwpError(error));
        return JNI_TRUE;
    }

    (void)outStream_writeInt(out, count);
    return JNI_TRUE;
}

void *ThreadReference_Cmds[] = { (void *)12,
    (void *)name,
    (void *)suspend,
    (void *)resume,
    (void *)status,
    (void *)threadGroup,
    (void *)frames,
    (void *)getFrameCount,
    (void *)ownedMonitors,
    (void *)currentContendedMonitor,
    (void *)stop,
    (void *)interrupt,
    (void *)suspendCount
    };


