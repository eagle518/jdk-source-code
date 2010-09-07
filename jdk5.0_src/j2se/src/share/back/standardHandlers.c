/*
 * @(#)standardHandlers.c	1.12 04/01/12
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * handlers
 *
 * The default event request handler functions
 */

#include "util.h"
#include "eventHandler.h"
#include "threadControl.h"
#include "eventHelper.h"
#include "classTrack.h"

#include "standardHandlers.h"

static void 
handleClassPrepare(JNIEnv *env, EventInfo *evinfo,
                   HandlerNode *node,
                   struct bag *eventBag)
{
    jthread thread = evinfo->thread;

    /* We try hard to avoid class loads/prepares in debugger
     * threads, but it is still possible for them to happen (most
     * likely for exceptions that are thrown within JNI
     * methods). If such an event occurs, we must report it, but
     * we cannot suspend the debugger thread.
     * 
     * 1) We report the thread as NULL because we don't want the
     *    application to get hold of a debugger thread object. 
     * 2) We try to do the right thing wrt to suspending
     *    threads without suspending debugger threads. If the
     *    requested suspend policy is NONE, there's no problem. If
     *    the requested policy is ALL, we can just suspend all
     *    application threads without producing any surprising
     *    results by leaving the debugger thread running. However,
     *    if the requested policy is EVENT_THREAD, we are forced
     *    to do something different than requested. The most
     *    useful behavior is to suspend all application threads
     *    (just as if the policy was ALL). This allows the
     *    application to operate on the class before it gets into
     *    circulation and so it is preferable to the other
     *    alternative of suspending no threads.  
     */
    if (threadControl_isDebugThread(thread)) { 
        evinfo->thread = NULL;
        if (node->suspendPolicy == JDWP_SUSPEND_POLICY(EVENT_THREAD)) {
            node->suspendPolicy = JDWP_SUSPEND_POLICY(ALL);
        }
    }
    eventHelper_recordEvent(evinfo, node->handlerID, 
                            node->suspendPolicy, eventBag);
}

static void 
handleGarbageCollectionFinish(JNIEnv *env, EventInfo *evinfo,
                  HandlerNode *node,
                  struct bag *eventBag)
{
    JDI_ASSERT_MSG(JNI_FALSE, "Should never call handleGarbageCollectionFinish");
}

static void 
handleFrameEvent(JNIEnv *env, EventInfo *evinfo,
                 HandlerNode *node,
                 struct bag *eventBag)
{
    /*
     * The frame id that comes with this event is very transient.
     * We can't send the frame to the helper thread because it
     * might be useless by the time the helper thread can use it 
     * (if suspend policy is NONE). So, get the needed info from 
     * the frame and then use a special command to the helper
     * thread.
     */
    
    jmethodID method;
    jlocation location;
    jvmtiError error;
    FrameNumber fnum = 0;
    
    error = JVMTI_FUNC_PTR(gdata->jvmti,GetFrameLocation)
	    (gdata->jvmti, evinfo->thread, fnum, &method, &location);
    if (error != JVMTI_ERROR_NONE) {
        location = -1;
    }

    eventHelper_recordFrameEvent(node->handlerID, 
                                 node->suspendPolicy,
                                 evinfo->ei, 
                                 evinfo->thread, 
                                 evinfo->clazz, 
                                 evinfo->method, 
                                 location, eventBag);
}

static void  
genericHandler(JNIEnv *env, EventInfo *evinfo,
               HandlerNode *node,  
               struct bag *eventBag)
{ 
    eventHelper_recordEvent(evinfo, node->handlerID, node->suspendPolicy, 
                            eventBag); 
}

HandlerFunction
standardHandlers_defaultHandler(EventIndex ei) 
{
    switch (ei) {
        case EI_BREAKPOINT:
        case EI_EXCEPTION:
        case EI_FIELD_ACCESS:
        case EI_FIELD_MODIFICATION:
        case EI_SINGLE_STEP:
        case EI_THREAD_START:
        case EI_THREAD_END:
        case EI_VM_DEATH: 
            return &genericHandler;

        case EI_CLASS_PREPARE: 
            return &handleClassPrepare;

        case EI_GC_FINISH: 
            return &handleGarbageCollectionFinish;

        case EI_METHOD_ENTRY:
        case EI_METHOD_EXIT:
            return &handleFrameEvent;

        default:
	    /* This NULL will trigger a JVMTI_ERROR_INVALID_EVENT_TYPE */
            return NULL;
    }
}

void 
standardHandlers_onConnect(void)
{
    jboolean installed;

    /* always report VMDeath to a connected debugger */
    installed = (eventHandler_createPermanentInternal(
			EI_VM_DEATH, genericHandler) != NULL);
    if (!installed) {
	EXIT_ERROR(JVMTI_ERROR_INVALID_EVENT_TYPE,"Unable to install VM Death event handler");
    }
}

void 
standardHandlers_onDisconnect(void)
{
}
