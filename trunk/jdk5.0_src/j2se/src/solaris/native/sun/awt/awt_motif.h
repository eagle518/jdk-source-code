/*
 * @(#)awt_motif.h	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef _SWITCHXM_P_H_
#define _SWITCHXM_P_H_

#include <sun_awt_motif_MComponentPeer.h>

#include "gdefs.h"
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>

#define MOTIF_NA  sun_awt_motif_MComponentPeer_MOTIF_NA
#define MOTIF_V1  sun_awt_motif_MComponentPeer_MOTIF_V1
#define MOTIF_V2  sun_awt_motif_MComponentPeer_MOTIF_V2


extern int32_t awt_motif_getIMStatusHeight(Widget w, jobject tc);
extern XVaNestedList awt_motif_getXICStatusAreaList(Widget w, jobject tc);
extern void awt_motif_Scrollbar_ButtonReleaseHandler (Widget, 
                                                      XtPointer, 
                                                      XEvent *, 
                                                      Boolean *) ;

    /* This function causes an UnsatisfiedLinkError on Linux.
     * It's a no-op for Motif 2.1.
     * Since Linux only links against Motif 2.1, we can safely remove
     * this function altogether from the Linux build.
     * bchristi 1/22/2001
     */
#ifdef __solaris__
extern void awt_motif_adjustDragTriggerEvent(XEvent* xevent);
#endif

void awt_motif_enableSingleDragInitiator(Widget w);

#endif /* _SWITCHXM_P_H_ */
