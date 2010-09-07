/*
 * @(#)awt_motif.c	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifdef HEADLESS
    #error This file should not be included in headless library
#endif

#include "awt_motif.h"

#include <jvm.h>

/* Common routines required for both Motif 2.1 and Motif 1.2 */
#include <Xm/ScrollBarP.h>

/* Remove the ScrollBar widget's continuous scrolling timeout handler
   on a ButtonRelease to prevent the continuous scrolling that would
   occur if a timeout expired after the ButtonRelease.
*/
/*  
 * Note: RFE:4263104 is filed when the API is available these needs to removed 
 */
void
awt_motif_Scrollbar_ButtonReleaseHandler(Widget w,
                                         XtPointer data,
                                         XEvent *event,
                                         Boolean *cont)
{
  /* Remove the timeout handler. */
#define END_TIMER         (1<<2)
  XmScrollBarWidget sbw = (XmScrollBarWidget) w;
  if (sbw->scrollBar.timer != NULL) {
    XtRemoveTimeOut( sbw->scrollBar.timer );
    sbw->scrollBar.timer = (XtIntervalId)NULL;
    sbw->scrollBar.flags |= END_TIMER;
  }
}
