/*
 * @(#)awt_Container.cpp	1.16 10/03/23
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt_Container.h"
#include "awt.h"

/************************************************************************
 * AwtContainer fields
 */

jfieldID AwtContainer::layoutMgrID;
jmethodID AwtContainer::findComponentAtMID;

/************************************************************************
 * AwtContainer native methods
 */

extern "C" {

JNIEXPORT void JNICALL 
Java_java_awt_Container_initIDs(JNIEnv *env, jclass cls) {
    TRY;

    AwtContainer::layoutMgrID = 
        env->GetFieldID(cls, "layoutMgr", "Ljava/awt/LayoutManager;");

    AwtContainer::findComponentAtMID =
        env->GetMethodID(cls, "findComponentAt", "(IIZ)Ljava/awt/Component;");

    DASSERT(AwtContainer::layoutMgrID != NULL);
    DASSERT(AwtContainer::findComponentAtMID);

    CATCH_BAD_ALLOC;
}

} /* extern "C" */
