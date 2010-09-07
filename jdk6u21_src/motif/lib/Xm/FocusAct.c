/* $XConsortium: FocusAct.c /main/5 1995/07/15 20:50:59 drk $ */
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
#include "PrimitiveI.h"
#include "UniqueEvnI.h"

/**********************************************************************
 *
 * _XmMenuButtonTakeFocus
 *
 *********************************************************************/
/*ARGSUSED*/
void 
_XmMenuButtonTakeFocus(
        Widget wid,
        XEvent *event,
        String *params,		/* unused */
        Cardinal *num_params )	/* unused */
{
   /* Support menu replay, free server input queue until next button event */
   XAllowEvents(XtDisplay(wid), SyncPointer, CurrentTime);

   XmProcessTraversal(wid, XmTRAVERSE_CURRENT);

   _XmRecordEvent (event);
}

/**********************************************************************
 *
 * _XmMenuButtonTakeFocusUp
 *
 *********************************************************************/
/*ARGSUSED*/
void 
_XmMenuButtonTakeFocusUp(
        Widget wid,
        XEvent *event,
        String *params,		/* unused */
        Cardinal *num_params )	/* unused */
{
   /* Support menu replay, free server input queue until next button event */
   XAllowEvents(XtDisplay(wid), SyncPointer, CurrentTime);
   _XmRecordEvent (event);
}



