/*
 * @(#)awt_Object.h	1.26 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_OBJECT_H
#define AWT_OBJECT_H

#include "awt.h"
#include "awt_Toolkit.h"

#include "java_awt_Event.h"
#include "java_awt_AWTEvent.h"
#include "sun_awt_windows_WObjectPeer.h"

/************************************************************************
 * AwtObject class
 */

class AwtObject {
public:
    class ExecuteArgs {
	public:
	    UINT	cmdId;
	    LPARAM	param1;
	    LPARAM	param2;
	    LPARAM	param3;
	    LPARAM	param4;
    };


    /* sun.awt.windows.WObjectPeer field and method ids */
    static jfieldID pDataID;
    static jfieldID targetID;

    static jmethodID getPeerForTargetMID;
    static jclass wObjectPeerClass;

    AwtObject();
    virtual ~AwtObject();

    // Return the associated AWT peer or target object.
    INLINE jobject GetPeer(JNIEnv *env) { 
        return m_peerObject;
    }

    INLINE jobject GetTarget(JNIEnv *env) { 
        jobject peer = GetPeer(env);
	if (peer != NULL) {
	    return env->GetObjectField(peer, AwtObject::targetID);
	} else {
	    return NULL;
	}
    }

    INLINE jobject GetTargetAsGlobalRef(JNIEnv *env) {
        jobject localRef = GetTarget(env);
        if (localRef == NULL) {
            return NULL;
        }

        jobject globalRef = env->NewGlobalRef(localRef);
        env->DeleteLocalRef(localRef);
        return globalRef;
    }

    // Return the peer associated with some target
    static jobject GetPeerForTarget(JNIEnv *env, jobject target);

    // Java callback routines
    // Invoke a callback on the java peer object asynchronously
    void DoCallback(const char* methodName, const char* methodSig, ...);

    // Allocate and initialize a new event, and post it to the peer's
    // target object.  No response is expected from the target.
    void SendEvent(jobject event);

    INLINE void EnableCallbacks(BOOL e) { m_callbacksEnabled = e; }

    // Execute any code associated with a command ID -- only classes with
    // DoCommand() defined should associate their instances with cmdIDs.
    virtual void DoCommand(void) {
        DASSERT(FALSE);
    }

    // execute given code on Windows message-pump thread
    static LRESULT WinThreadExec(jobject peerObject, UINT cmdId, LPARAM param1 = 0L, LPARAM param2 = 0L, LPARAM param3 = 0L, LPARAM param4 = 0L);
    // callback function to execute code on Windows message-pump thread
    virtual LRESULT WinThreadExecProc(AwtObject::ExecuteArgs * args);
    
    // call this method to scedule deletion of this object
    void scheduleDelete();

protected:
    jobject                       m_peerObject;
    BOOL                          m_callbacksEnabled;
private:
    BOOL m_bDeleteScheduled;
    CriticalSection m_lock;
};

#endif // AWT_OBJECT_H


