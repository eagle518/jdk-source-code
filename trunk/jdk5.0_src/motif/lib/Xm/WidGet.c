/* $XConsortium: WidGet.c /main/6 1995/10/25 20:27:58 cde-sun $ */
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
#include <Xm/PrimitiveP.h>
#include <Xm/ManagerP.h>
#include <Xm/GadgetP.h>



/************************************************************************
 *
 *   XmWidgetGetBaselines
 *
 ************************************************************************/

Boolean
XmWidgetGetBaselines(
        Widget wid,
        Dimension **baselines,
        int *line_count)
{
  _XmWidgetToAppContext(wid);
  _XmAppLock(app);

  if (XmIsPrimitive(wid))
      {
	  XmPrimitiveClassExt              *wcePtr;
	  WidgetClass   wc = XtClass(wid);
	  
	  wcePtr = _XmGetPrimitiveClassExtPtr(wc, NULLQUARK);
	  
	  if (*wcePtr && (*wcePtr)->widget_baseline)
	      {
		_XmAppUnlock(app);
		return( (*((*wcePtr)->widget_baseline)) 
			 (wid, baselines, line_count)) ;
	      } 
      }
  else if (XmIsGadget(wid))
      {
	  XmGadgetClassExt              *wcePtr;
	  WidgetClass   wc = XtClass(wid);
	  
	  wcePtr = _XmGetGadgetClassExtPtr(wc, NULLQUARK);
	  
	  if (*wcePtr && (*wcePtr)->widget_baseline)
	      {
		_XmAppUnlock(app);
		return( (*((*wcePtr)->widget_baseline)) 
			 (wid, baselines, line_count)) ;
	      }
      }
  _XmAppUnlock(app);
  return (False);
}


/************************************************************************
 *
 *   XmWidgetDisplayRect
 *
 ************************************************************************/

Boolean
XmWidgetGetDisplayRect(
        Widget wid,
        XRectangle *displayrect)
{
    _XmWidgetToAppContext(wid);
    _XmAppLock(app);

    if (XmIsPrimitive(wid))
	{
	    XmPrimitiveClassExt              *wcePtr;
	    WidgetClass   wc = XtClass(wid);
	    
	    wcePtr = _XmGetPrimitiveClassExtPtr(wc, NULLQUARK);
	    
	    if (*wcePtr && (*wcePtr)->widget_display_rect)
		(*((*wcePtr)->widget_display_rect)) (wid, displayrect);
	    _XmAppUnlock(app);
	    return (True);
	}
    else if (XmIsGadget(wid))
	{
	    XmGadgetClassExt              *wcePtr;
	    WidgetClass   wc = XtClass(wid);
	    
	    wcePtr = _XmGetGadgetClassExtPtr(wc, NULLQUARK);
	    
	    if (*wcePtr && (*wcePtr)->widget_display_rect)
		(*((*wcePtr)->widget_display_rect)) (wid, displayrect);
	    _XmAppUnlock(app);
	    return (True);
	}
    else {
	_XmAppUnlock(app);
	return (False);
    }
}
