
/* $XConsortium: MenuProc.c /main/8 1996/01/17 10:50:38 lehors $ */
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
/*
 * The following functions are used to separate the private class function
 * in RowColumn from the buttons that may be children of the RowColumn.
 * This is simply an interface supplied so that the buttons will not have
 * to have intimate knowledge of the RowColumn class functions.
 */

#include <Xm/XmP.h>
#include <Xm/MenuProcP.h>
#include "MenuProcI.h"
#include "GadgetUtiI.h"
#include "XmI.h"

static XtPointer menuProcEntry = NULL;

/*
 * this routine is called at RowColumn class init to
 * save the address of the menuProcedureEntry routine.
 */

void 
_XmSaveMenuProcContext(
        XtPointer address )
{
   menuProcEntry = address;
}


/*
 * This routine is used by the button children of the RowColumn (currently
 * all label and labelgadget subclasses) to get the address of the
 * menuProcedureEntry routine.  It is called by the buttons class init
 * routines.
 */
XtPointer 
_XmGetMenuProcContext( void )
{
   return menuProcEntry;
}


/* temp hold for core class translations used during subclass'
 * InitializePrehook & InitializePosthook
 */
static XContext SaveTranslationsContext = 0;


/************************************************************************
 *
 * _XmSaveCoreClassTranslations
 *
 *  Save away the core class translations during the initialization
 *  routines.  This is used by rowcol and subclasses of Label that set their
 *  translations during initialization depending on whether they are in
 *  a menu.  The InitializePrehook calls this routine to save the
 *  core class translations.
 ************************************************************************/
void
_XmSaveCoreClassTranslations(Widget widget)
{
   _XmProcessLock();
   if (SaveTranslationsContext == 0)
    {
      SaveTranslationsContext = XUniqueContext();
    }

   XSaveContext(XtDisplay(widget), (XID)widget, SaveTranslationsContext,
                (char *)(widget->core.widget_class->core_class.tm_table));
   _XmProcessUnlock();
}


/************************************************************************
 *
 * _XmDeleteCoreTranslations
 *
 *  Delete the core class translations saved for this widget.
 ************************************************************************/
void
_XmDeleteCoreClassTranslations(
	Widget widget)
{
    _XmProcessLock();
    if (SaveTranslationsContext == 0)
    {
    	_XmProcessUnlock();
    	return;
    }

    XDeleteContext(XtDisplay(widget), (XID)widget, SaveTranslationsContext);
    _XmProcessUnlock();
}


/************************************************************************
 *
 * _XmRestoreCoreClassTranslations
 *
 *  Restore the core class translations during the initialization
 *  routines.  This is used by rowcol and subclasses of Label that set their
 *  translations during initialization depending on whether they are in
 *  a menu.  The InitializePosthook calls this routine to restore the
 *  core class translations.
 ************************************************************************/
void
_XmRestoreCoreClassTranslations(Widget widget)
{
   String   saved_translations;

   _XmProcessLock();
   if (SaveTranslationsContext &&
       (!XFindContext(XtDisplay(widget), (XID)widget,
                      SaveTranslationsContext,
                      (XtPointer)&saved_translations)
       )
      )
    {
      widget->core.widget_class->core_class.tm_table = saved_translations;
      XDeleteContext(XtDisplay(widget), (XID)widget, SaveTranslationsContext);
      SaveTranslationsContext = 0;
    }
#ifdef DEBUG
   else  /* This should'nt happen ! */
    {
      abort();
    }
#endif
   _XmProcessUnlock();
}


/*************************************************
 * This function extracts a time from an event or
 * returns the last processed time if the event
 * is NULL or isn't an event with a timestamp
 *************************************************/

/*ARGSUSED*/
Time 
_XmGetDefaultTime(Widget wid,	
		  XEvent *event)
{
  if (event == NULL)
    return(XtLastTimestampProcessed(XtDisplay(wid)));
  else if (event -> type == ButtonPress ||
	   event -> type == ButtonRelease)
    return(event -> xbutton.time);
  else if (event -> type == KeyPress ||
	   event -> type == KeyRelease)
    return(event -> xkey.time);
  else if (event -> type == MotionNotify)
    return(event -> xmotion.time);
  else if (event -> type == EnterNotify ||
	   event -> type == LeaveNotify)
    return(event -> xcrossing.time);
  else
    return(XtLastTimestampProcessed(XtDisplay(wid)));
}
