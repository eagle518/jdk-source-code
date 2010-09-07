/*
 * @(#)awt_Mlib.h	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef _AWT_MLIB_H_
#define _AWT_MLIB_H_

typedef void (*mlib_start_timer)(int);
typedef void (*mlib_stop_timer)(int, int);

void awt_getImagingLib();
mlib_start_timer awt_setMlibStartTimer();
mlib_stop_timer awt_setMlibStopTimer();
void awt_getBIColorOrder(int type, int *colorOrder);


#endif /* _AWT_MLIB_H */


