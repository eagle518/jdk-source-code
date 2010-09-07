/*
 * @(#)awt_AWTEvent.cpp	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt_AWTEvent.h"
#include "awt_Component.h"
#include <java_awt_AWTEvent.h>

/************************************************************************
 * AwtAWTEvent fields
 */

jfieldID AwtAWTEvent::bdataID;
jfieldID AwtAWTEvent::idID;
jfieldID AwtAWTEvent::consumedID;

/************************************************************************
 * AwtAWTEvent static methods
 */

void AwtAWTEvent::saveMSG(JNIEnv *env, MSG *pMsg, jobject jevent)
{
    if (env->EnsureLocalCapacity(1) < 0) {
	return;
    }
    jbyteArray bdata = env->NewByteArray(sizeof(MSG));
    if(bdata == 0) {
        throw std::bad_alloc();
    }
    env->SetByteArrayRegion(bdata, 0, sizeof(MSG), (jbyte *)pMsg);
    DASSERT(AwtAWTEvent::bdataID);
    env->SetObjectField(jevent, AwtAWTEvent::bdataID,  bdata);
    env->DeleteLocalRef(bdata);
}									 

/************************************************************************
 * AwtEvent native methods
 */

extern "C" {

/*
 * Class:     java_awt_AWTEvent
 * Method:    initIDs
 * Signature: ()V
 */
JNIEXPORT void JNICALL 
Java_java_awt_AWTEvent_initIDs(JNIEnv *env, jclass cls)
{
    TRY;

    AwtAWTEvent::bdataID = env->GetFieldID(cls, "bdata", "[B");
    AwtAWTEvent::idID = env->GetFieldID(cls, "id", "I");
    AwtAWTEvent::consumedID = env->GetFieldID(cls, "consumed", "Z");

    DASSERT(AwtAWTEvent::bdataID != NULL);
    DASSERT(AwtAWTEvent::idID != NULL);
    DASSERT(AwtAWTEvent::consumedID != NULL);

    CATCH_BAD_ALLOC;
}
  
/*
 * Class:     java_awt_AWTEvent
 * Method:    nativeSetSource
 * Signature: (Ljava/awt/peer/ComponentPeer;)V
 */
JNIEXPORT void JNICALL Java_java_awt_AWTEvent_nativeSetSource
    (JNIEnv *env, jobject self, jobject newSource)
{
    TRY;
 
    JNI_CHECK_NULL_RETURN(self, "null AWTEvent");
 
    MSG *pMsg;

    jbyteArray bdata = (jbyteArray)
	env->GetObjectField(self, AwtAWTEvent::bdataID);
    if (bdata != NULL) {
	jboolean dummy;
	PDATA pData;
	JNI_CHECK_PEER_RETURN(newSource);
	AwtComponent *p = (AwtComponent *)pData;
	HWND hwnd = p->GetHWnd();

	pMsg = (MSG *)env->GetPrimitiveArrayCritical(bdata, &dummy);
	if (pMsg == NULL) {
	    throw std::bad_alloc();
	}
	pMsg->hwnd = hwnd;
	env->ReleasePrimitiveArrayCritical(bdata, (void *)pMsg, 0);
    }
    
    CATCH_BAD_ALLOC;
}

} /* extern "C" */
