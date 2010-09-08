/*
 * @(#)Win32ErrorMode.c	1.3 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <windows.h>
#include "sun_io_Win32ErrorMode.h"

/*
 * Class:     sun/io/Win32ErrorMode
 * Method:    setErrorMode
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_sun_io_Win32ErrorMode_setErrorMode
  (JNIEnv *env, jclass thisClass, jlong mode)
{
    return (jlong)SetErrorMode((UINT)mode);
}

