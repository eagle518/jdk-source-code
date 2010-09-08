/*
 * @(#)awt_Mlib.h	1.16 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef _AWT_MLIB_H_
#define _AWT_MLIB_H_

#include "awt_ImagingLib.h"

typedef void (*mlib_start_timer)(int);
typedef void (*mlib_stop_timer)(int, int);

mlib_status awt_getImagingLib(JNIEnv *, mlibFnS_t *, mlibSysFnS_t *);
mlib_start_timer awt_setMlibStartTimer();
mlib_stop_timer awt_setMlibStopTimer();
void awt_getBIColorOrder(int type, int *colorOrder);


#endif /* _AWT_MLIB_H */


