/*
 * @(#)awt_Cursor.h	1.14 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni_util.h"
#include "awt_p.h"

#define CACHE_UPDATE 0 		/* cache the component and update cursor */
#define UPDATE_ONLY  1 		/* update cursor, but not cache component */
#define CACHE_ONLY   2 		/* cache the component, no cursor update */

/* fieldIDs for Cursor fields that may be accessed from C */
struct CursorIDs {
  jfieldID type;
  jmethodID mSetPData;
  jfieldID pData;
};

#ifndef HEADLESS
Cursor getCursor(JNIEnv *env, jobject jCur);
void updateCursor(XPointer client_data, int32_t replace);
#endif /* !HEADLESS */
