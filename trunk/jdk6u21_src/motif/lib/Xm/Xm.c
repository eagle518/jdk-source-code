/* $XConsortium: Xm.c /main/6 1995/10/25 20:28:03 cde-sun $ */
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

#include "XmI.h"
#include "MessagesI.h"
#include <Xm/PrimitiveP.h>
#include <Xm/ManagerP.h>
#include <Xm/GadgetP.h>

/**************************************************************************
 *   This is Xm.c
 *    It contains global API that:
 *      - it's not widget specific 
 *      - it's already used by various widgets (you don't get useless
 *          code by linking with this module: all the functions
 *          are useful and used. 
 *   For example, TrackingLocate or ResolvePartOffset do not belong
 *   here because they are not used by everybody.
 *************************************************************************/



/**************************************************************************
 *                                                                        *
 * _XmSocorro - Help dispatch function.  Start at the widget help was     *
 *   invoked on, find the first non-null help callback list, and call it. *
 *   -- Called by various widgets across Xm                               *
 *                                                                        *
 *************************************************************************/
/* ARGSUSED */
void 
_XmSocorro(
        Widget w,
        XEvent *event,
        String *params,		/* unused */
        Cardinal *num_params )	/* unused */
{
    XmAnyCallbackStruct cb;    

    if (w == NULL) return;

    cb.reason = XmCR_HELP;
    cb.event = event;

    do {
        if ((XtHasCallbacks(w, XmNhelpCallback) == XtCallbackHasSome))
        {
            XtCallCallbacks (w, XmNhelpCallback, &cb);
            return;
        }
        else
            w = XtParent(w);
    }    
    while (w != NULL);
}


/****************************************************************
 *
 * _XmParentProcess
 *    This is the entry point for parent processing.
 *   -- Called by various widgets across Xm                              
 *
 ****************************************************************/
Boolean 
_XmParentProcess(
        Widget widget,
        XmParentProcessData data )
{
    XmManagerWidgetClass manClass ;
	    
    manClass = (XmManagerWidgetClass) widget->core.widget_class ;
    
    if(    XmIsManager( widget)
       && manClass->manager_class.parent_process    ) {   
	return( (*manClass->manager_class.parent_process)( widget, data)) ;
    } 
	    
    return( FALSE) ;
}



/************************************************************************
 *
 *  _XmDestroyParentCallback
 *     Destroy parent. Used by various dialog subclasses
 *
 ************************************************************************/
/* ARGSUSED */
void 
_XmDestroyParentCallback(
        Widget w,
        XtPointer client_data,	/* unused */
        XtPointer call_data )	/* unused */
{
   XtDestroyWidget (XtParent (w));
}



/************************************************************************
 *
 *  _XmClearShadowType
 *	Clear the right and bottom border area and save 
 *	the old width, height and shadow type.
 *      Used by various subclasses for resize larger situation, where the
 *      inside shadow is not exposed.
 *   Maybe that should be moved in Draw.c, maybe not, since it's a widget API
 *
 ************************************************************************/
void 
_XmClearShadowType(
        Widget w,
#if NeedWidePrototypes
        int old_width,
        int old_height,
        int old_shadow_thickness,
        int old_highlight_thickness )
#else
        Dimension old_width,
        Dimension old_height,
        Dimension old_shadow_thickness,
        Dimension old_highlight_thickness )
#endif /* NeedWidePrototypes */
{

    if (old_shadow_thickness == 0) return;

    if (XtIsRealized(w)) {
      if (old_width <= w->core.width)
	  XClearArea (XtDisplay (w), XtWindow (w),
		      old_width - old_shadow_thickness - 
		      old_highlight_thickness, 0,
		      old_shadow_thickness, old_height - 
		      old_highlight_thickness, 
		      False);

      if (old_height <= w->core.height)
	  XClearArea (XtDisplay (w), XtWindow (w),
		      0, old_height - old_shadow_thickness - 
		      old_highlight_thickness, 
		      old_width - old_highlight_thickness, 
		      old_shadow_thickness, 
		      False);
   }
}



/**********************************************************************
 *
 * _XmReOrderResourceList
 *   This procedure moves the given resource right after the
 *   insert_after name in this class resource list.  
 *   (+ insert_after NULL means insert in front)
 *
 *   ----Replace by a call to an Xt function in R6.-----
 **********************************************************************/
void 
_XmReOrderResourceList(
	WidgetClass widget_class,
        String res_name,
        String insert_after)
{
    XrmResource **list;
    int len;
    XrmQuark res_nameQ = XrmPermStringToQuark(res_name);
    XrmResource *tmp ;
    int n ;

    _XmProcessLock();
    list = (XrmResource **) widget_class->core_class.resources ;
    len = widget_class->core_class.num_resources;

    /* look for the named resource slot */
    n = 0; 
    while ((n < len) && (list[n]->xrm_name != res_nameQ))
      n++;

    if (n < len) {
	int m, i;
	XrmQuark insert_afterQ ;  
		
	if (insert_after) {
	    insert_afterQ = XrmPermStringToQuark(insert_after) ;
	    /* now look for the insert_after resource slot */
	    m = 0; 
	    while ((m < len) && (list[m]->xrm_name != insert_afterQ))
	      m++;
	} else m = len ;
	if (m == len) m = -1 ;

	/* now do the insertion/packing, both cases */
	tmp = list[n] ;

	if (n > m) {
	    for (i = n; i > m+1; i--)
		list[i] = list[i-1];
	    list[m+1] = tmp;
	} else {
	    for (i = n; i < m; i++)
		list[i] = list[i+1];
	    list[m] = tmp;
	}
    }
    _XmProcessUnlock();
}



/************************************************************************
 *
 *  _XmWarningMsg
 *	Add XME_WARNING to Message list so MotifWarningHandler will
 *      add Name: & Class:
 *
 ************************************************************************/
void 
_XmWarningMsg(Widget w,
	      char *type,
	      char *message,
	      char **params,
	      Cardinal num_params)
{
  Display * dpy;
  char *new_params[11];
  Cardinal num_new_params = num_params + 1;
  int i;

  if (num_new_params > 11) num_new_params = 11;
  for (i = 0; i < num_new_params-1; i++)
    new_params[i] = params[i];
  new_params[num_new_params-1] = XME_WARNING;

  if (w != NULL) {
    XtAppWarningMsg (XtWidgetToApplicationContext(w),
		     XrmQuarkToString (w->core.xrm_name),
		     type,
		     w->core.widget_class->core_class.class_name, 
		     message, new_params, &num_new_params);
  } else
    XtWarning(message);
}



/***************************************/
/********---- PUBLIC API ----**********/
/*************************************/




/************************************************************************
 *
 *  XmeWarning
 *	Build up a warning message and call Xt to get it displayed.
 *
 ************************************************************************/
void 
XmeWarning(Widget w,
	   char *message )
{
  char *params[1];
  Cardinal num_params = 0;
  Display * dpy;
  
  if (w != NULL) {
    /* the MotifWarningHandler installed in VendorS.c knows about
       this convention */
    params[0] = XME_WARNING;
    num_params++;

    XtAppWarningMsg (XtWidgetToApplicationContext(w),
		     XrmQuarkToString (w->core.xrm_name),
		     "XmeWarning",
		     w->core.widget_class->core_class.class_name, 
		     message, params, &num_params);
  } else 
    	XtWarning(message);
}


/************************************************************************
 *
 *  XmObjectAtPoint
 *	new implementation that ask a manager class method
 *   -- Called by various widgets across Xm                            
 *
 ************************************************************************/
Widget 
XmObjectAtPoint(
        Widget wid,
        Position x,
        Position y )
{
    XmManagerWidgetClass mw = (XmManagerWidgetClass) XtClass(wid);
    XmManagerClassExt *mext ;
    Widget return_wid = NULL;
    _XmWidgetToAppContext(wid);
     
    _XmAppLock(app);

    if (!XmIsManager(wid)) {
	_XmAppUnlock(app);
	return NULL;
    }

    mext = (XmManagerClassExt *)
	_XmGetClassExtensionPtr( (XmGenericClassExt *)
				&(mw->manager_class.extension), NULLQUARK) ;
    if (!*mext) {
	_XmAppUnlock(app);
	return NULL;
    }
    
    if ((*mext)->object_at_point)
	return_wid = ((*mext)->object_at_point)(wid, x, y);
	
    _XmAppUnlock(app);
    return return_wid;
}
