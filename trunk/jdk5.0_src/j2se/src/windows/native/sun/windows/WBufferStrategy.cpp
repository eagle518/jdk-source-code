/*
 * @(#)WBufferStrategy.cpp	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "sun_awt_windows_WBufferStrategy.h"
#include "jni_util.h"


static jmethodID getBackBufferID;

/*
 * Class:     sun_awt_windows_WBufferStrategy
 * Method:    initIDs
 * Signature: (Ljava/lang/Class;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WBufferStrategy_initIDs(JNIEnv *env, jclass wbs,
					     jclass componentClass)
{
    getBackBufferID = env->GetMethodID(componentClass, "getBackBuffer",
				       "()Ljava/awt/Image;");
}

/**
 * Native method of WBufferStrategy.java.  Given a Component
 * object, this method will find the back buffer associated
 * with the Component's BufferStrategy and return a handle
 * to it.
 */
extern "C" JNIEXPORT jobject JNICALL
Java_sun_awt_windows_WBufferStrategy_getDrawBuffer(JNIEnv *env, jclass wbs,
					   	   jobject component)
{
    if (!JNU_IsNull(env, getBackBufferID)) {
	return env->CallObjectMethod(component, getBackBufferID);
    } else {
	return NULL;
    }
}
