/* $XConsortium: ChColor.c /main/6 1995/10/25 19:55:45 cde-sun $ */
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
#include <Xm/ManagerP.h>
#include <Xm/PrimitiveP.h>
#include <Xm/GadgetP.h>
#include "XmI.h"

/*********************************************************************
 *
 *  XmChangeColor - change set of colors for existing widget, given 
 *                  background color
 *
 *********************************************************************/
void 
XmChangeColor(
	      Widget w,
	      Pixel background )

{
  Widget windowed_ancestor = w;
  Pixel foreground_ret;
  Pixel topshadow_ret;
  Pixel bottomshadow_ret;
  Pixel select_ret; 
  Pixel use_background = background;
  Arg args[5];
  Pixel unselect_ret; /* 4231112 */
  Pixel background_ret; /* 4231112 */
  int n = 0; /* 4231112 */

  /*
   * If the gadget is in BC mode (for colors), it must follow its parent's
   * color scheme.  We therefore ignore the background pixel that was
   * passed in.
   */

  _XmWidgetToAppContext(w);
  _XmAppLock(app);

/* 4231112 start */
  if (XmIsPrimitive(w) || XmIsGadget(w))
  {
      XtSetArg(args[n], XmNbackground, &background_ret); n++;
      if (XmIsToggleButton(w) || XmIsToggleButtonGadget(w))
      {
         XtSetArg(args[n], XmNunselectColor, &unselect_ret); n++;
      }
      XtGetValues(w,args,n);
  }
/* 4231112 end */

  if (XmIsGadget(w))
    {
      Widget parent = XtParent(w);

      windowed_ancestor = parent;
      if (background_ret == parent->core.background_pixel) /* 4231112 */
	use_background = parent->core.background_pixel;
    }

  XmGetColors(windowed_ancestor->core.screen, windowed_ancestor->core.colormap,
	      use_background, &foreground_ret, &topshadow_ret,
	      &bottomshadow_ret, NULL );

  if ( (XmIsManager(w)) ||  (XmIsPrimitive(w)) || (XmIsGadget(w)) )
    { 
      n = 0;
      XtSetArg (args[n], XmNbackground, (XtArgVal) use_background); n++;
      XtSetArg (args[n], XmNforeground, (XtArgVal) foreground_ret); n++;
      XtSetArg (args[n], XmNtopShadowColor, (XtArgVal) topshadow_ret); n++;
      XtSetArg (args[n], XmNbottomShadowColor, (XtArgVal) bottomshadow_ret); n++;
      XtSetArg (args[n], XmNhighlightColor, (XtArgVal) foreground_ret); n++;
      
      XtSetValues (w, args, n);
      
      if (XmIsPrimitive(w) || XmIsGadget(w))
	{
	  if ( (XmIsScrollBar(w)) ||
	       (XmIsPushButton(w)) || (XmIsPushButtonGadget(w)) ||
	       (XmIsToggleButton(w)) || (XmIsToggleButtonGadget(w)) )
	    { 
	      XmGetColors( windowed_ancestor->core.screen,
			   windowed_ancestor->core.colormap,
			   background, NULL, NULL, NULL, &select_ret);

              n = 0;
	      if (XmIsScrollBar(w))
	      {
		XtSetArg (args[n], XmNtroughColor, (XtArgVal) select_ret); n++;
	      }
	      else if (XmIsPushButton(w) || XmIsPushButtonGadget(w))
	      {
		XtSetArg (args[n], XmNarmColor, (XtArgVal) select_ret); n++;
	      }
	      else if (XmIsToggleButton(w) || XmIsToggleButtonGadget(w))
	      {
		XtSetArg (args[n], XmNselectColor, (XtArgVal) select_ret); n++;

		if (background_ret == unselect_ret) /* 4231112 */
   		   XtSetArg(args[n], XmNunselectColor, (XtArgVal) use_background); n++;
	      }
	      XtSetValues (w, args, n);
	    }
	}
    }
  _XmAppUnlock(app);
}



