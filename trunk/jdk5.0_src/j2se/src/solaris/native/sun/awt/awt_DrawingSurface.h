/*
 * @(#)awt_DrawingSurface.h	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef _AWT_DRAWING_SURFACE_H_
#define _AWT_DRAWING_SURFACE_H_

#include <jawt.h>
#include <jni.h>
#include <jni_util.h>

_JNI_IMPORT_OR_EXPORT_ JAWT_DrawingSurface* JNICALL
    awt_GetDrawingSurface(JNIEnv* env, jobject target);

_JNI_IMPORT_OR_EXPORT_ void JNICALL
    awt_FreeDrawingSurface(JAWT_DrawingSurface* ds);

_JNI_IMPORT_OR_EXPORT_ void JNICALL
    awt_Lock(JNIEnv* env);

_JNI_IMPORT_OR_EXPORT_ void JNICALL
    awt_Unlock(JNIEnv* env);

_JNI_IMPORT_OR_EXPORT_ jobject JNICALL
    awt_GetComponent(JNIEnv* env, void* platformInfo);

#endif /* !_AWT_DRAWING_SURFACE_H_ */

