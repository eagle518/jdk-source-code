/* $XConsortium: ValTime.c /main/5 1995/07/15 20:57:05 drk $ */
/*
 * COPYRIGHT NOTICE
 * Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 * ALL RIGHTS RESERVED (MOTIF).  See the file named COPYRIGHT.MOTIF
 * for the full copyright text.
 * 
 */
/*
 * HISTORY
 */

#include "XmI.h"

/**********************************************************************
 *
 *  _XmValidTimestamp
 *      Returns a valid timestamp by generating an event.  The time
 *      that is returned is not 0 or CurrentTime.
 *      Arguments: w - any widget on the display.
 *
 **********************************************************************/

Time 
_XmValidTimestamp(
     Widget w )
{
  Window win;
  Display *dsp = XtDisplay(w);
  XEvent event;
  EventMask shellMask;
  Atom timeProp = XInternAtom(dsp, "_MOTIF_CURRENT_TIME", False);

  while (!XtIsShell(w)) w = XtParent(w);
  win = XtWindow(w);
 
  if (! ((shellMask = XtBuildEventMask(w)) & PropertyChangeMask) )
    XSelectInput(dsp, win, shellMask | PropertyChangeMask);

  XChangeProperty(dsp, win, timeProp, timeProp,
		  8, PropModeAppend, NULL, 0);

  XWindowEvent(dsp, win, PropertyChangeMask, &event);
  if (!(shellMask & PropertyChangeMask))
    XSelectInput(dsp, win, shellMask);

  return (event.xproperty.time);
}

