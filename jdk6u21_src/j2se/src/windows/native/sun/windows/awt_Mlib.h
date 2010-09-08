/*
 * @(#)awt_Mlib.h	1.13 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef _AWT_MLIB_H_
#define _AWT_MLIB_H_

#include "awt_ImagingLib.h"


#ifdef __cplusplus
extern "C" {
#endif    
typedef void (*mlib_start_timer)(int);
typedef void (*mlib_stop_timer)(int, int);

JNIEXPORT mlib_status awt_getImagingLib(JNIEnv *env, mlibFnS_t *sMlibFns,
                                        mlibSysFnS_t *sMlibSysFns);
JNIEXPORT mlib_start_timer awt_setMlibStartTimer();
JNIEXPORT mlib_stop_timer awt_setMlibStopTimer();
JNIEXPORT void awt_getBIColorOrder(int type, int *colorOrder);

#ifdef __cplusplus
}; /* end of extern "C" */
#endif    



#endif /* _AWT_MLIB_H */
