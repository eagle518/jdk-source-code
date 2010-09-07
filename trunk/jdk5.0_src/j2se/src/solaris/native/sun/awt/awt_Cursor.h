/*
 * @(#)awt_Cursor.h	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni_util.h"
#include "awt_p.h"

#define CACHE_UPDATE 0 		/* cache the component and update cursor */
#define UPDATE_ONLY  1 		/* update cursor, but not cache component */
#define CACHE_ONLY   2 		/* cache the component, no cursor update */

/* fieldIDs for Cursor fields that may be accessed from C */
struct CursorIDs {
  jfieldID type;
  jfieldID pData;
};

#ifndef HEADLESS
Cursor getCursor(JNIEnv *env, jobject jCur);
void updateCursor(XPointer client_data, int32_t replace);
#endif /* !HEADLESS */
