/*
 * @(#)jawt.cpp	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#define _JNI_IMPLEMENTATION_
#include <jawt.h>

#include "awt_DrawingSurface.h"

/*
 * Get the AWT native structure.  This function returns JNI_FALSE if
 * an error occurs.
 */
extern "C" JNIEXPORT jboolean JNICALL JAWT_GetAWT(JNIEnv* env, JAWT* awt)
{
    if (awt == NULL) {
        return JNI_FALSE;
    }

    if (awt->version != JAWT_VERSION_1_3
        && awt->version != JAWT_VERSION_1_4) {
        return JNI_FALSE;
    }

    awt->GetDrawingSurface = DSGetDrawingSurface;
    awt->FreeDrawingSurface = DSFreeDrawingSurface;
    if (awt->version >= JAWT_VERSION_1_4) {
        awt->Lock = DSLockAWT;
        awt->Unlock = DSUnlockAWT;
        awt->GetComponent = DSGetComponent;
    }

    return JNI_TRUE;
}
