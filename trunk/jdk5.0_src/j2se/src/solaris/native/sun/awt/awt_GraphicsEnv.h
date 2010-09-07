/*
 * @(#)awt_GraphicsEnv.h	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef _AWT_GRAPHICSENV_H_
#define _AWT_GRAPHICSENV_H_

#include <jni_util.h>

#ifndef HEADLESS
#define MITSHM
#endif /* !HEADLESS */

#define UNSET_MITSHM (-2)
#define NOEXT_MITSHM (-1)
#define CANT_USE_MITSHM (0)
#define CAN_USE_MITSHM (1)

#ifdef MITSHM

#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>

extern int XShmQueryExtension();

void TryInitMITShm(JNIEnv *env, jint *shmExt, jint *shmPixmaps);
void resetXShmAttachFailed();
jboolean isXShmAttachFailed(); 
  
#endif /* MITSHM */

/* fieldIDs for X11GraphicsConfig fields that may be accessed from C */
struct X11GraphicsConfigIDs {
    jfieldID aData;
    jfieldID bitsPerPixel;
    jfieldID screen;
};

/* fieldIDs for X11GraphicsDevice fields that may be accessed from C */
struct X11GraphicsDeviceIDs {
    jfieldID screen;
};

#endif /* _AWT_GRAPHICSENV_H_ */
