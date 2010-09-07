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
/*   $XConsortium: Display.h /main/10 1995/07/14 10:20:21 drk $ */
/*
*  (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#ifndef _XmDisplay_h
#define _XmDisplay_h

#include <Xm/Xm.h>
#include <X11/Shell.h>
#include <Xm/DragC.h>
#include <Xm/DropSMgr.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XmIsDisplay
#define XmIsDisplay(w) (XtIsSubclass(w, xmDisplayClass))
#endif /* XmIsXmDisplay */

enum {
	XmDRAG_NONE,
	XmDRAG_DROP_ONLY,
	XmDRAG_PREFER_PREREGISTER,
	XmDRAG_PREREGISTER,
	XmDRAG_PREFER_DYNAMIC,
	XmDRAG_DYNAMIC,
	XmDRAG_PREFER_RECEIVER
};

/* Class record constants */

typedef struct _XmDisplayRec *XmDisplay;
typedef struct _XmDisplayClassRec *XmDisplayClass;
externalref 	WidgetClass xmDisplayClass;

#define XmGetDisplay(w) XmGetXmDisplay(XtDisplayOfObject(w))

/********    Public Function Declarations    ********/

extern Widget XmGetDragContext( 
                        Widget w,
                        Time time) ;
extern Widget XmGetXmDisplay( 
                        Display *display) ;

/********    End Public Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDisplay_h */


