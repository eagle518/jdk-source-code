/* $XConsortium: IsMwmRun.c /main/7 1996/05/21 12:02:11 pascale $ */
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


#include <Xm/XmP.h>
#include <Xm/MwmUtil.h>
#include "XmI.h"

/************************************************************************
 *
 *  XmIsMotifWMRunning
 *
 ************************************************************************/
Boolean 
XmIsMotifWMRunning(
	 Widget shell )
{
    Atom	motif_wm_info_atom;
    Atom	actual_type;
    int		actual_format;
    unsigned long num_items, bytes_after;
    PropMotifWmInfo	*prop = 0;
    Window	root = RootWindowOfScreen(XtScreen(shell));

    _XmWidgetToAppContext(shell);
 
    _XmAppLock(app);

    motif_wm_info_atom = XInternAtom(XtDisplay(shell),
				       _XA_MOTIF_WM_INFO,
				       FALSE);
    _XmProcessLock();

    XGetWindowProperty (XtDisplay(shell), 
			 root,
			 motif_wm_info_atom,
			 0, (long)PROP_MOTIF_WM_INFO_ELEMENTS,
			 FALSE, motif_wm_info_atom,
			 &actual_type, &actual_format,
			 &num_items, &bytes_after,
			 (unsigned char **) &prop);
    _XmProcessUnlock();

    if ((actual_type != motif_wm_info_atom) ||
	 (actual_format != 32) ||
	 (num_items < PROP_MOTIF_WM_INFO_ELEMENTS))
      {
	   if (prop != 0) XFree((char *)prop);
           _XmAppUnlock(app);
	   return (FALSE);
      }
    else
      {
	   Window	wm_window = (Window) prop->wmWindow;
	   Window	top, parent, *children;
	   unsigned int	num_children;
	   Boolean	returnVal;
	   Cardinal	i;

	   if (XQueryTree(XtDisplay(shell),
			  root, &top, &parent,
			  &children, &num_children))
	     {
		 i = 0; 
		 while ((i < num_children) && (children[i] != wm_window))
		   i++;
		 returnVal =  (i == num_children) ? FALSE : TRUE;
	     }
	   else
	     returnVal = FALSE;

	   if (prop) XFree((char *)prop);
	   if (children) XFree((char *)children);
           _XmAppUnlock(app);
	   return (returnVal);
      }
}
