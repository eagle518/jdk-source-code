/*
 * @(#)awt_Container.cpp	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt_Container.h"
#include "awt.h"

/************************************************************************
 * AwtContainer fields
 */

jfieldID AwtContainer::ncomponentsID;
jfieldID AwtContainer::componentID;
jfieldID AwtContainer::layoutMgrID;
jmethodID AwtContainer::findComponentAtMID;

/************************************************************************
 * AwtContainer native methods
 */

extern "C" {

JNIEXPORT void JNICALL 
Java_java_awt_Container_initIDs(JNIEnv *env, jclass cls) {
    TRY;

    AwtContainer::ncomponentsID = env->GetFieldID(cls, "ncomponents", "I");
    AwtContainer::componentID =
        env->GetFieldID(cls, "component", "[Ljava/awt/Component;");

    AwtContainer::layoutMgrID = 
        env->GetFieldID(cls, "layoutMgr", "Ljava/awt/LayoutManager;");

    AwtContainer::findComponentAtMID =
        env->GetMethodID(cls, "findComponentAt", "(IIZ)Ljava/awt/Component;");

    DASSERT(AwtContainer::ncomponentsID != NULL);
    DASSERT(AwtContainer::componentID != NULL);
    DASSERT(AwtContainer::layoutMgrID != NULL);
    DASSERT(AwtContainer::findComponentAtMID);

    CATCH_BAD_ALLOC;
}

} /* extern "C" */
