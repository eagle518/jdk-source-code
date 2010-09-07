/*
 * @(#)awt_Mlib.h	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef _AWT_MLIB_H_
#define _AWT_MLIB_H_

#ifdef __cplusplus
extern "C" {
#endif    
typedef void (*mlib_start_timer)(int);
typedef void (*mlib_stop_timer)(int, int);

JNIEXPORT void awt_getImagingLib();
JNIEXPORT mlib_start_timer awt_setMlibStartTimer();
JNIEXPORT mlib_stop_timer awt_setMlibStopTimer();
JNIEXPORT void awt_getBIColorOrder(int type, int *colorOrder);

#ifdef __cplusplus
}; /* end of extern "C" */
#endif    



#endif /* _AWT_MLIB_H */
