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
/* $XConsortium: ScrolledWP.h /main/13 1995/09/19 23:08:30 cde-sun $ */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmScrolledWindowP_h
#define _XmScrolledWindowP_h


#ifndef MOTIF12_HEADERS


#include <Xm/ManagerP.h>
#include <Xm/ScrolledW.h>

#include <Xm/ScrollBar.h>
#include <Xm/DrawingA.h>

#include <Xm/ScrollFrameT.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*XmGetAutoDragRectsProc)(Widget, XRectangle **, Cardinal *) ;

#define XmInheritGetAutoDragRectsProc ((XmGetAutoDragRectsProc) _XtInherit)

#define XmScrolledWindowClassExtVersion 1L

typedef struct _XmScrolledWindowClassExtRec{
    XtPointer           next_extension;
    XrmQuark            record_type;
    long                version;
    Cardinal            record_size;
    XmGetAutoDragRectsProc get_hor_rects;
    XmGetAutoDragRectsProc get_vert_rects;
} XmScrolledWindowClassExtRec, *XmScrolledWindowClassExt;

/* New fields for the ScrolledWindow widget class record */
typedef struct {
     XtPointer extension;   /* extension to new class methods */
} XmScrolledWindowClassPart;

/****************
 *
 * Class record declaration
 *
 ****************/
typedef struct _XmScrolledWindowClassRec {
    CoreClassPart	core_class;
    CompositeClassPart  composite_class;
    ConstraintClassPart constraint_class;
    XmManagerClassPart  manager_class;
    XmScrolledWindowClassPart	swindow_class;
} XmScrolledWindowClassRec;

externalref XmScrolledWindowClassRec xmScrolledWindowClassRec;

/****************
 *
 * Scrolled Window instance structure.
 *
 ****************/
typedef struct {

   int vmin;		  /*  slider minimum coordinate position     */
   int vmax;		  /*  slider maximum coordinate position     */
   int vOrigin;		  /*  slider edge location                  */
   int vExtent;		  /*  slider size                           */

   int hmin;		  /*  Same as above for horizontal bar.     */
   int hmax;
   int hOrigin;
   int hExtent;

   Position hsbX,hsbY;
   Dimension hsbWidth,hsbHeight;    /* Dimensions for the horiz bar */

   Position vsbX,vsbY;
   Dimension vsbWidth,vsbHeight;    /* Dimensions for the vertical bar */

   Position    GivenHeight, GivenWidth;
   /* these 2 fields are now obsolete, we re-use them for
      a different purpose, to track the initial position
      of the scrollbars */
#define sw_prev_x GivenWidth
#define sw_prev_y GivenHeight

   Dimension	AreaWidth,AreaHeight;
   Dimension	WidthPad,HeightPad;
   Position	XOffset, YOffset;

   Dimension	pad;

   Boolean	hasHSB;
   Boolean	hasVSB;
   Boolean	InInit;
   Boolean	FromResize;

   unsigned char	VisualPolicy;
   unsigned char	ScrollPolicy;
   unsigned char	ScrollBarPolicy;
   unsigned char	Placement;
   
   XmScrollBarWidget   	hScrollBar;
   XmScrollBarWidget   	vScrollBar;
   XmDrawingAreaWidget 	ClipWindow;
   Widget              	WorkWindow;
   
   XtCallbackList       traverseObscuredCallback;

   XtEnum       auto_drag_model;
   XtIntervalId auto_drag_timer; 

   Boolean scroll_frame_inited ;
   XmScrollFrameData scroll_frame_data ;

   XtPointer	auto_drag_rects;
   XtPointer	auto_drag_closure;
} XmScrolledWindowPart;


/************************************************************************
 *									*
 * Full instance record declaration					*
 *									*
 ************************************************************************/

typedef struct _XmScrolledWindowRec {
    CorePart	    core;
    CompositePart   composite;
    ConstraintPart constraint;
    XmManagerPart   manager;
    XmScrolledWindowPart   swindow;
} XmScrolledWindowRec;


/*  Frame constraint records  */

typedef struct _XmScrolledWindowConstraintPart
{
   unsigned char child_type;
   Position orig_x;
   Position orig_y;
} XmScrolledWindowConstraintPart, * XmScrolledWindowConstraint;

typedef struct _XmScrolledWindowConstraintRec
{
   XmManagerConstraintPart manager;
   XmScrolledWindowConstraintPart   swindow;
} XmScrolledWindowConstraintRec, * XmScrolledWindowConstraintPtr;


#define DEFAULT_HEIGHT	20
#define DEFAULT_WIDTH	20

#define RESOURCE_DEFAULT (-1)

#define GetSWConstraint(w) \
        (&((XmScrolledWindowConstraintPtr) (w)->core.constraints)->swindow)


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#else  /* MOTIF12_HEADERS */

/* 
 * @OSF_COPYRIGHT@
 * (c) Copyright 1990, 1991, 1992, 1993, 1994 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *  
*/ 
/*
 * HISTORY
 * Motif Release 1.2.5
*/
/*   $XConsortium: ScrolledWP.h /main/cde1_maint/2 1995/08/18 19:21:29 drk $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#include <Xm/ManagerP.h>
#include <Xm/ScrolledW.h>

#include <Xm/ScrollBar.h>
#include <Xm/DrawingA.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Constraint part record for ScrolledWindow widget */
typedef struct _XmScrolledWindowConstraintPart
{
   unsigned char child_type;
} XmScrolledWindowConstraintPart, * XmScrolledWindowConstraint;


/* New fields for the ScrolledWindow widget class record */
typedef struct {
   XtPointer extension;   /* Pointer to extension record */
} XmScrolledWindowClassPart;

/****************
 *
 * Class record declaration
 *
 ****************/
typedef struct _XmScrolledWindowClassRec {
    CoreClassPart	core_class;
    CompositeClassPart  composite_class;
    ConstraintClassPart constraint_class;
    XmManagerClassPart  manager_class;
    XmScrolledWindowClassPart	swindow_class;
} XmScrolledWindowClassRec;

externalref XmScrolledWindowClassRec xmScrolledWindowClassRec;

/****************
 *
 * Scrolled Window instance structure.
 *
 ****************/
typedef struct {

   int vmin;		  /*  slider minimum coordiate position     */
   int vmax;		  /*  slider maximum coordiate position     */
   int vOrigin;		  /*  slider edge location                  */
   int vExtent;		  /*  slider size                           */

   int hmin;		  /*  Same as above for horizontal bar.     */
   int hmax;
   int hOrigin;
   int hExtent;

   Position hsbX,hsbY;
   Dimension hsbWidth,hsbHeight;    /* Dimensions for the horiz bar */

   Position vsbX,vsbY;
   Dimension vsbWidth,vsbHeight;    /* Dimensions for the vertical bar */

   Dimension    GivenHeight, GivenWidth;

   Dimension	AreaWidth,AreaHeight;
   Dimension	WidthPad,HeightPad;
   Position	XOffset, YOffset;

   Dimension	pad;

   Boolean	hasHSB;
   Boolean	hasVSB;
   Boolean	InInit;
   Boolean	FromResize;

   unsigned char	VisualPolicy;
   unsigned char	ScrollPolicy;
   unsigned char	ScrollBarPolicy;
   unsigned char	Placement;
   
   XmScrollBarWidget   	hScrollBar;
   XmScrollBarWidget   	vScrollBar;
   XmDrawingAreaWidget 	ClipWindow;
   Widget              	WorkWindow;
   
   XtCallbackList       traverseObscuredCallback;
} XmScrolledWindowPart;


/************************************************************************
 *									*
 * Full instance record declaration					*
 *									*
 ************************************************************************/

typedef struct _XmScrolledWindowRec {
    CorePart	    core;
    CompositePart   composite;
    ConstraintPart constraint;
    XmManagerPart   manager;
    XmScrolledWindowPart   swindow;
} XmScrolledWindowRec;

#define DEFAULT_HEIGHT 20
#define DEFAULT_WIDTH 20


/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern char * _XmGetRealXlations() ;
extern void _XmInitializeScrollBars() ;

#else

extern char * _XmGetRealXlations( 
                        Display *dpy,
                        _XmBuildVirtualKeyStruct *keys,
                        int num_keys) ;
extern void _XmInitializeScrollBars( 
                        Widget w) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif  /* MOTIF12_HEADERS */


#endif /* _XmScrolledWindowP_h */
/* DON'T ADD STUFF AFTER THIS #endif */
