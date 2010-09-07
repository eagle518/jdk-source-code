/*
 * @(#)awt_Object.cpp	1.39 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt_Object.h"
#include "ObjectList.h"
#ifdef DEBUG
#include "awt_Unicode.h"
#endif

#ifdef DEBUG
static BOOL reportEvents = FALSE;
#endif


/************************************************************************
 * AwtObject fields
 */

jfieldID AwtObject::pDataID;
jfieldID AwtObject::targetID;
jclass AwtObject::wObjectPeerClass;
jmethodID AwtObject::getPeerForTargetMID;


/************************************************************************
 * AwtObject methods
 */

AwtObject::AwtObject()
{
    theAwtObjectList.Add(this);
    m_peerObject = NULL;
    m_callbacksEnabled = TRUE;
    m_bDeleteScheduled = FALSE;
}

AwtObject::~AwtObject()
{
    DASSERT(m_bDeleteScheduled);
    theAwtObjectList.Remove(this);
}

/*
 * Return the peer associated with some target.  This information is
 * maintained in a hashtable at the java level.  
 */
jobject AwtObject::GetPeerForTarget(JNIEnv *env, jobject target)
{
    jobject result = 
	env->CallStaticObjectMethod(AwtObject::wObjectPeerClass, 
                                    AwtObject::getPeerForTargetMID, 
                                    target);

    DASSERT(!safe_ExceptionOccurred(env));
    return result;
}

/* Execute a callback to the associated Java peer. */
void 
AwtObject::DoCallback(const char* methodName, const char* methodSig, ...)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    
    /* don't callback during the create & initialization process */
    if (m_peerObject != NULL && m_callbacksEnabled) {
        va_list args;
        va_start(args, methodSig);
#ifdef DEBUG
        if (reportEvents) {
	    jstring targetStr = 
		(jstring)JNU_CallMethodByName(env, NULL, GetTarget(env), 
					      "getName",
					      "()Ljava/lang/String;").l;
            DASSERT(!safe_ExceptionOccurred(env));
            printf("Posting %s%s method to %S\n", methodName, methodSig,
		   TO_WSTRING(targetStr));
        }
#endif
	/* caching would do much good here */
	JNU_CallMethodByNameV(env, NULL, GetPeer(env), 
			      methodName, methodSig, args);
	{
	    jthrowable exc = safe_ExceptionOccurred(env);
	    if (exc) {
	        env->DeleteLocalRef(exc);
	        env->ExceptionDescribe();
		env->ExceptionClear();
	    }
        }
	DASSERT(!safe_ExceptionOccurred(env));
        va_end(args);
    }
}

void AwtObject::SendEvent(jobject event)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);

#ifdef DEBUG
    if (reportEvents) {
	jstring eventStr = JNU_ToString(env, event);
        DASSERT(!safe_ExceptionOccurred(env));
	jstring targetStr = 
	    (jstring)JNU_CallMethodByName(env, NULL, GetTarget(env),"getName",
					  "()Ljava/lang/String;").l;
        DASSERT(!safe_ExceptionOccurred(env));
        printf("Posting %S to %S\n", TO_WSTRING(eventStr),
	       TO_WSTRING(targetStr));
    }
#endif
    /* Post event to the system EventQueue. */
    JNU_CallMethodByName(env, NULL, GetPeer(env), "postEvent", 
			 "(Ljava/awt/AWTEvent;)V", event);
    {
        jthrowable exc = safe_ExceptionOccurred(env);
	if (exc) {
	    env->DeleteLocalRef(exc);
	    env->ExceptionDescribe();
	}
    }
    DASSERT(!safe_ExceptionOccurred(env));
}

//
// (static)
// Switches to Windows thread via SendMessage and synchronously
// calls AwtObject::WinThreadExecProc with the given command id
// and parameters.
//
// Useful for writing code that needs to be synchronized with
// what's happening on the Windows thread.
//
LRESULT AwtObject::WinThreadExec(
    jobject			 	peerObject,
    UINT				cmdId,
    LPARAM 				param1,
    LPARAM 				param2,
    LPARAM 				param3,
    LPARAM 				param4 )
{
    DASSERT( peerObject != NULL);

    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    AwtObject* object = (AwtObject *)JNI_GET_PDATA(peerObject);
    DASSERT( !IsBadReadPtr(object, sizeof(AwtObject)) );

    ExecuteArgs		args;
    LRESULT         retVal;

    // setup arguments
    args.cmdId = cmdId;
    args.param1 = param1;
    args.param2 = param2;
    args.param3 = param3;
    args.param4 = param4;

    // call WinThreadExecProc on the toolkit thread
    retVal = AwtToolkit::GetInstance().SendMessage(WM_AWT_EXECUTE_SYNC,
						   (WPARAM)object,
						   (LPARAM)&args);
    return retVal;
}

LRESULT AwtObject::WinThreadExecProc(ExecuteArgs * args)
{
    DASSERT(FALSE); // no default handler
    return 0L;
}

void AwtObject::scheduleDelete() {
    CriticalSection::Lock l(m_lock);
    
    if (!m_bDeleteScheduled) {
        m_bDeleteScheduled = TRUE;
        AwtToolkit::GetInstance().PostMessage(WM_AWT_DISPOSE, (WPARAM)this, 0);
    }
}

/************************************************************************
 * WObjectPeer native methods
 */

extern "C" {

JNIEXPORT void JNICALL 
Java_sun_awt_windows_WObjectPeer_initIDs(JNIEnv *env, jclass cls) {
    TRY;

    AwtObject::wObjectPeerClass = cls;
    AwtObject::pDataID = env->GetFieldID(cls, "pData", "J");
    AwtObject::targetID = env->GetFieldID(cls, "target", 
					      "Ljava/lang/Object;");

    AwtObject::getPeerForTargetMID = 
	env->GetStaticMethodID(cls, "getPeerForTarget", 
			 "(Ljava/lang/Object;)Lsun/awt/windows/WObjectPeer;");

    DASSERT(AwtObject::pDataID != NULL);
    DASSERT(AwtObject::targetID != NULL);
    DASSERT(AwtObject::getPeerForTargetMID != NULL);

    CATCH_BAD_ALLOC;
}

} /* extern "C" */
