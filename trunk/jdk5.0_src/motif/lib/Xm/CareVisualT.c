/* $XConsortium: CareVisualT.c /main/6 1996/07/24 19:20:24 pascale $ */
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
#include <Xm/TraitP.h>
#include <Xm/CareVisualT.h>
#include "CareVisualTI.h"

/************************************************************************
 *
 *  _XmNotifyChildrenVisual
 *	Loop through the child set of new and for any widget/gadget that has
 *	a CareParentVisual trait, call the redraw proc.
 *      The redraw proc will return True if the widget needs to have the
 *	parent redrawn as well.
 *
 ************************************************************************/
Boolean 
_XmNotifyChildrenVisual(
        Widget cur,
        Widget new_w,
        Mask visual_flag)
{
   register int i;
   Widget child;
   Boolean redisplay = False;
   XmCareVisualTrait care_visual ;
   CompositeWidget cw = (CompositeWidget) new_w ;

   for (i = 0; i < cw->composite.num_children; i++) {
       child = cw->composite.children[i];

       if (((care_visual = (XmCareVisualTrait)
	    XmeTraitGet((XtPointer) XtClass(child),
			XmQTcareParentVisual)) != NULL) && 
	   XtIsRealized(child)) 

	   redisplay |= care_visual->redraw(child, cur, new_w, visual_flag) ;

   }

   return (redisplay);
}

