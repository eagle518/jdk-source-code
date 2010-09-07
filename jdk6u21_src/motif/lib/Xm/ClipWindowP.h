/* $XConsortium: ClipWindowP.h /main/5 1995/07/15 20:48:39 drk $ */
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
#ifndef _XmClipWindowP_h
#define _XmClipWindowP_h

#include <Xm/DrawingAP.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref WidgetClass xmClipWindowWidgetClass;

typedef struct _XmClipWindowClassRec * XmClipWindowWidgetClass;
typedef struct _XmClipWindowRec      * XmClipWindowWidget;


#ifndef XmIsClipWindow
#define XmIsClipWindow(w)  (XtIsSubclass (w, xmClipWindowWidgetClass))
#endif


/*  New fields for the ClipWindow widget class record  */

typedef struct
{
   XtPointer extension;   
} XmClipWindowClassPart;


/* Full class record declaration */

typedef struct _XmClipWindowClassRec
{
	CoreClassPart		core_class;
	CompositeClassPart	composite_class;
	ConstraintClassPart	constraint_class;
	XmManagerClassPart	manager_class;
	XmDrawingAreaClassPart	drawing_area_class;
	XmClipWindowClassPart	clip_window_class;
} XmClipWindowClassRec;

externalref XmClipWindowClassRec xmClipWindowClassRec;


/* New fields for the ClipWindow widget record */

typedef struct
{
	unsigned char flags;
	Dimension old_width ;
} XmClipWindowPart;


/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _XmClipWindowRec
{
	CorePart		core;
	CompositePart		composite;
	ConstraintPart		constraint;
	XmManagerPart		manager;
	XmDrawingAreaPart	drawing_area;
	XmClipWindowPart	clip_window;
} XmClipWindowRec;


/********    Private Function Declarations    ********/
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmClipWindowP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
