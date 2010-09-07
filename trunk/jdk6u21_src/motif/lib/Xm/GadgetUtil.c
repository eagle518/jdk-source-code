/* 
 *  @OSF_COPYRIGHT@
 *  COPYRIGHT NOTICE
 *  Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 *  ALL RIGHTS RESERVED (MOTIF). See the file named COPYRIGHT.MOTIF for
 *  the full copyright text.
*/ 
/* 
 * HISTORY
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$XConsortium: GadgetUtil.c /main/16 1996/10/23 15:00:52 cde-osf $"
#endif
#endif
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <Xm/XmP.h>
#include <Xm/GadgetP.h>
#include <X11/Shell.h>
#include <X11/ShellP.h>
#include <Xm/DropSMgr.h>
#include "GadgetUtiI.h"
#include "XmI.h"


/********    Static Function Declarations    ********/


/********    End Static Function Declarations    ********/



/************************************************************************
 *
 *  _XmInputForGadget
 *	This routine is a front-end for XmObjectAtPoint which returns a
 *      gadget or NULL if XmbjectAtPoint is not sensitive.
 *
 ************************************************************************/
XmGadget 
_XmInputForGadget(
        Widget wid,
        int x,
        int y )
{
    Widget widget;

    widget = XmObjectAtPoint (wid, (Position)x, (Position)y); /* Wyoming 64-bit Fix */

    if (!widget  ||  !XtIsSensitive (widget))
	return ((XmGadget) NULL);

   return ((XmGadget) widget);
}




/************************************************************************
 *
 *  XmConfigureObject
 *	Wrapper around Xt equivalent + DropSite update.
 *
 ************************************************************************/
void 
XmeConfigureObject(
        Widget wid,
#if NeedWidePrototypes
        int x,
        int y,
        int width,
        int height,
        int border_width )
#else
        Position x,
        Position y,
        Dimension width,
        Dimension height,
        Dimension border_width )
#endif /* NeedWidePrototypes */
{
    _XmWidgetToAppContext(wid);
    XmDropSiteStartUpdate(wid);

    _XmAppLock(app);

    if (!width && !height) {
	XtWidgetGeometry   desired, preferred ;
	desired.request_mode = 0;
        XtQueryGeometry(wid, &desired, &preferred);
	width = preferred.width;
        height = preferred.height;
    }
    if (!width)  width++;                
    if (!height) height++;
    XtConfigureWidget(wid, x, y, width, height, border_width);

    XmDropSiteEndUpdate(wid);
    _XmAppUnlock(app);
}




/************************************************************************
 *
 *  XmeRedisplayGadgets
 *	Redisplay any gadgets contained within the manager mw which
 *	are intersected by the region.
 *
 ************************************************************************/
void 
XmeRedisplayGadgets(
        Widget w,
        register XEvent *event,
        Region region )
{
   CompositeWidget mw = (CompositeWidget) w ;
   register int i;
   register Widget child;
   XtExposeProc expose;
  
   _XmWidgetToAppContext(w);

   _XmAppLock(app);
   for (i = 0; i < mw->composite.num_children; i++)
   {
      child = mw->composite.children[i];
      if (XmIsGadget(child) && XtIsManaged(child))
      {
         if (region == NULL)
         {
            if (child->core.x < event->xexpose.x + event->xexpose.width      &&
                child->core.x + child->core.width > event->xexpose.x &&
                child->core.y < event->xexpose.y + event->xexpose.height     &&
                child->core.y + child->core.height > event->xexpose.y)
            {
		
	       _XmProcessLock();
	       expose = child->core.widget_class->core_class.expose;
	       _XmProcessUnlock();

               if (expose)
                  (*(expose))
                     (child, event, region);
            }
         }
         else
         {
            if (XRectInRegion (region, child->core.x, child->core.y,
                               child->core.width, child->core.height))
            {
 	      _XmProcessLock();
	      expose = child->core.widget_class->core_class.expose;
	      _XmProcessUnlock();

              if (expose)
                  (*(expose))
                     (child, event, region);
            }
         }
      }
   }
   _XmAppUnlock(app);
}




/************************************************************************
 *
 *  _XmDispatchGadgetInput
 *	Call the gadgets class function and send the desired data to it.
 *
 ************************************************************************/
void 
_XmDispatchGadgetInput(
        Widget wid,
        XEvent *event,
        Mask mask )
{
        XmGadget g = (XmGadget) wid ;
   if (g->gadget.event_mask & mask && 
       XtIsSensitive ((Widget)g) && XtIsManaged ((Widget)g))
      if (event != NULL) {
         XEvent synth_event;

#define CopyEvent(source, dest, type) \
    source.type = dest->type

         switch(mask) {
	   case XmENTER_EVENT:
                   CopyEvent(synth_event, event, xcrossing);
		   if (event->type != EnterNotify) {
		      synth_event.type = EnterNotify;
                   }
                   break;
	   case XmLEAVE_EVENT:
                   CopyEvent(synth_event, event, xcrossing);
		   if (event->type != LeaveNotify) {
		      synth_event.type = LeaveNotify;
                   }
                   break;
	   case XmFOCUS_IN_EVENT:
                   CopyEvent(synth_event, event, xfocus);
		   if (event->type != FocusIn) {
		      synth_event.type = FocusIn;
		   }
		   break;
	   case XmFOCUS_OUT_EVENT:
                   CopyEvent(synth_event, event, xfocus);
		   if (event->type != FocusIn) {
		      synth_event.type = FocusOut;
		   }
		   break;
	   case XmMOTION_EVENT:
                   CopyEvent(synth_event, event, xmotion);
		   if (event->type != MotionNotify) {
		      event->type = MotionNotify;
		   }
		   break;
	   case XmARM_EVENT:
                   CopyEvent(synth_event, event, xkey);
		   if (event->type != ButtonPress &&
		       event->type != KeyPress) {
		      synth_event.type = ButtonPress;
		   }
		   break;
	   case XmACTIVATE_EVENT:
                   CopyEvent(synth_event, event, xkey);
		   if (event->type != ButtonRelease &&
		       event->type != KeyPress) {
		      synth_event.type = ButtonRelease;
		   }
		   break;
	   case XmKEY_EVENT:
                   CopyEvent(synth_event, event, xkey);
		   if (event->type != KeyPress &&
		       event->type != ButtonPress) {
		      synth_event.type = KeyPress;
		   }
		   break;
	   case XmHELP_EVENT:
                   CopyEvent(synth_event, event, xkey);
		   if (event->type != KeyPress) {
		      synth_event.type = KeyPress;
		   }
		   break;
           default:
		   memcpy((char*)&synth_event, (char*)event,
		      (size_t)sizeof(synth_event));
		   break;
         }
   
         (*(((XmGadgetClass) (g->object.widget_class))->
             gadget_class.input_dispatch)) ((Widget) g, 
                                               (XEvent *) &synth_event, mask) ;
      } else
         (*(((XmGadgetClass) (g->object.widget_class))->
             gadget_class.input_dispatch)) ((Widget) g,
                                                      (XEvent *) event, mask) ;
}

