/*
 * @(#)awt_KeyboardFocusManager.cpp	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt.h"
#include "awt_KeyboardFocusManager.h"
#include "awt_Component.h"
#include "awt_Toolkit.h"
#include <java_awt_KeyboardFocusManager.h>

jclass AwtKeyboardFocusManager::keyboardFocusManagerCls;
jmethodID AwtKeyboardFocusManager::shouldNativelyFocusHeavyweightMID;
jmethodID AwtKeyboardFocusManager::heavyweightButtonDownMID;
jmethodID AwtKeyboardFocusManager::markClearGlobalFocusOwnerMID;
jmethodID AwtKeyboardFocusManager::removeLastFocusRequestMID;
jfieldID  AwtKeyboardFocusManager::isProxyActive;
jmethodID AwtKeyboardFocusManager::processSynchronousTransfer;

static jobject getNativeFocusState(JNIEnv *env, void*(*ftn)()) {
    jobject lFocusState = NULL;

    jobject gFocusState = reinterpret_cast<jobject>(AwtToolkit::GetInstance().
        InvokeFunction(ftn));
    if (gFocusState != NULL) {
        lFocusState = env->NewLocalRef(gFocusState);
	env->DeleteGlobalRef(gFocusState);
    }

    return lFocusState;
}

extern "C" {

/*
 * Class:     java_awt_KeyboardFocusManager
 * Method:    initIDs
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_java_awt_KeyboardFocusManager_initIDs
    (JNIEnv *env, jclass cls)
{
    TRY;

    AwtKeyboardFocusManager::keyboardFocusManagerCls = (jclass)
        env->NewGlobalRef(cls);
    AwtKeyboardFocusManager::shouldNativelyFocusHeavyweightMID =
        env->GetStaticMethodID(cls, "shouldNativelyFocusHeavyweight",
            "(Ljava/awt/Component;Ljava/awt/Component;ZZJ)I");
    AwtKeyboardFocusManager::heavyweightButtonDownMID =
        env->GetStaticMethodID(cls, "heavyweightButtonDown",
            "(Ljava/awt/Component;J)V");
    AwtKeyboardFocusManager::markClearGlobalFocusOwnerMID =
        env->GetStaticMethodID(cls, "markClearGlobalFocusOwner",
                               "()Ljava/awt/Window;");
    AwtKeyboardFocusManager::removeLastFocusRequestMID = 
        env->GetStaticMethodID(cls, "removeLastFocusRequest",
                               "(Ljava/awt/Component;)V");

    AwtKeyboardFocusManager::processSynchronousTransfer =
        env->GetStaticMethodID(cls, "processSynchronousLightweightTransfer",
                               "(Ljava/awt/Component;Ljava/awt/Component;ZZJ)Z");

    jclass keyclass = env->FindClass("java/awt/event/KeyEvent");
    DASSERT (keyclass != NULL);

    AwtKeyboardFocusManager::isProxyActive = 
        env->GetFieldID(keyclass, "isProxyActive", "Z");

    env->DeleteLocalRef(keyclass);

    DASSERT(AwtKeyboardFocusManager::keyboardFocusManagerCls != NULL);
    DASSERT(AwtKeyboardFocusManager::shouldNativelyFocusHeavyweightMID !=
            NULL);
    DASSERT(AwtKeyboardFocusManager::heavyweightButtonDownMID != NULL);
    DASSERT(AwtKeyboardFocusManager::markClearGlobalFocusOwnerMID != NULL);
    DASSERT(AwtKeyboardFocusManager::removeLastFocusRequestMID != NULL);
    DASSERT(AwtKeyboardFocusManager::processSynchronousTransfer != NULL);
    CATCH_BAD_ALLOC;
}


/*
 * Class:     sun_awt_KeyboardFocusManagerPeerImpl
 * Method:    getNativeFocusOwner
 * Signature: ()Ljava/awt/Component;
 */
JNIEXPORT jobject JNICALL
Java_sun_awt_KeyboardFocusManagerPeerImpl_getNativeFocusOwner
    (JNIEnv *env, jclass cls)
{
    TRY;

    return getNativeFocusState(env, AwtComponent::GetNativeFocusOwner);

    CATCH_BAD_ALLOC_RET(NULL);
}

/*
 * Class:     sun_awt_KeyboardFocusManagerPeerImpl
 * Method:    getNativeFocusedWindow
 * Signature: ()Ljava/awt/Window;
 */
JNIEXPORT jobject JNICALL
Java_sun_awt_KeyboardFocusManagerPeerImpl_getNativeFocusedWindow
    (JNIEnv *env, jclass cls)
{
    TRY;

    return getNativeFocusState(env, AwtComponent::GetNativeFocusedWindow);

    CATCH_BAD_ALLOC_RET(NULL);
}

/*
 * Class:     sun_awt_KeyboardFocusManagerPeerImpl
 * Method:    clearNativeGlobalFocusOwner
 * Signature: (Ljava/awt/Window;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_KeyboardFocusManagerPeerImpl_clearNativeGlobalFocusOwner
    (JNIEnv *env, jobject self, jobject activeWindow)
{
    TRY;

    AwtToolkit::GetInstance().InvokeFunction
        ((void*(*)(void*))AwtComponent::ClearGlobalFocusOwner,activeWindow);

    CATCH_BAD_ALLOC;
}
}
