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
/*   $XConsortium: DesktopP.h /main/10 1995/07/14 10:17:44 drk $ */
/*
*  (c) Copyright 1989, 1990  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
*  (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
/*
*  (c) Copyright 1988 MICROSOFT CORPORATION */
#ifndef  _XmDesktopP_h
#define _XmDesktopP_h

#include <Xm/ExtObjectP.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef XmIsDesktopObject
#define XmIsDesktopObject(w)	XtIsSubclass(w, xmDesktopClass)
#endif /* XmIsDesktopObject */

typedef struct _XmDesktopRec *XmDesktopObject;
typedef struct _XmDesktopClassRec *XmDesktopObjectClass;
externalref WidgetClass xmDesktopClass;


typedef struct _XmDesktopClassPart{
    WidgetClass		child_class;
    XtWidgetProc	insert_child;	  /* physically add child to parent  */
    XtWidgetProc      	delete_child;	  /* physically remove child	     */
    XtPointer		extension;
}XmDesktopClassPart, *XmDesktopClassPartPtr;

typedef struct _XmDesktopClassRec{
    ObjectClassPart		object_class;
    XmExtClassPart		ext_class;
    XmDesktopClassPart 		desktop_class;
}XmDesktopClassRec;

typedef struct {
    Widget		parent;
    Widget		*children;
    Cardinal		num_children;
    Cardinal		num_slots;
} XmDesktopPart, *XmDesktopPartPtr;

externalref XmDesktopClassRec 	xmDesktopClassRec;

typedef struct _XmDesktopRec{
    ObjectPart			object;
    XmExtPart			ext;
    XmDesktopPart		desktop;
}XmDesktopRec;


/********    Private Function Declarations    ********/


/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif  /* _XmDesktopP_h */
