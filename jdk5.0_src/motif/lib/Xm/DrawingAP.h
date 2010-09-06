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
/*   $XConsortium: DrawingAP.h /main/13 1996/04/01 15:22:11 daniel $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmDrawingAreaP_h
#define _XmDrawingAreaP_h

#ifndef MOTIF12_HEADERS 

#include <Xm/ManagerP.h>
#include <Xm/DrawingA.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XmRESIZE_SWINDOW	10


/* Constraint part record for DrawingArea widget */

typedef struct _XmDrawingAreaConstraintPart
{
   char unused;
} XmDrawingAreaConstraintPart, * XmDrawingAreaConstraint;

/*  New fields for the DrawingArea widget class record  */

typedef struct
{
   XtPointer extension;   /* Pointer to extension record */
} XmDrawingAreaClassPart;


/* Full class record declaration */

typedef struct _XmDrawingAreaClassRec
{
  CoreClassPart		core_class;
  CompositeClassPart	composite_class;
  ConstraintClassPart	constraint_class;
  XmManagerClassPart	manager_class;
  XmDrawingAreaClassPart	drawing_area_class;
} XmDrawingAreaClassRec;

externalref XmDrawingAreaClassRec xmDrawingAreaClassRec;


/* New fields for the DrawingArea widget record */

typedef struct
{
  Dimension		margin_width;
  Dimension		margin_height;

  XtCallbackList	resize_callback;
  XtCallbackList	expose_callback;
  XtCallbackList	input_callback;

  unsigned char		resize_policy;
  
#ifndef XM_PART_BC
  XtCallbackList	convert_callback;
  XtCallbackList	destination_callback;
#endif
} XmDrawingAreaPart;


/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _XmDrawingAreaRec
{
  CorePart		core;
  CompositePart		composite;
  ConstraintPart	constraint;
  XmManagerPart		manager;
  XmDrawingAreaPart	drawing_area;
} XmDrawingAreaRec;


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#else /* MOTIF12_HEADERS */


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
/*   $XConsortium: DrawingAP.h /main/cde1_maint/2 1995/08/18 19:00:57 drk $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#include <Xm/ManagerP.h>
#include <Xm/DrawingA.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XmRESIZE_SWINDOW	10


/* Constraint part record for DrawingArea widget */

typedef struct _XmDrawingAreaConstraintPart
{
   char unused;
} XmDrawingAreaConstraintPart, * XmDrawingAreaConstraint;

/*  New fields for the DrawingArea widget class record  */

typedef struct
{
   XtPointer extension;   /* Pointer to extension record */
} XmDrawingAreaClassPart;


/* Full class record declaration */

typedef struct _XmDrawingAreaClassRec
{
	CoreClassPart		core_class;
	CompositeClassPart	composite_class;
	ConstraintClassPart	constraint_class;
	XmManagerClassPart	manager_class;
	XmDrawingAreaClassPart	drawing_area_class;
} XmDrawingAreaClassRec;

externalref XmDrawingAreaClassRec xmDrawingAreaClassRec;


/* New fields for the DrawingArea widget record */

typedef struct
{
	Dimension		margin_width;
	Dimension		margin_height;

	XtCallbackList		resize_callback;
	XtCallbackList		expose_callback;
	XtCallbackList		input_callback;

	unsigned char		resize_policy;
} XmDrawingAreaPart;


/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _XmDrawingAreaRec
{
	CorePart		core;
	CompositePart		composite;
	ConstraintPart		constraint;
	XmManagerPart		manager;
	XmDrawingAreaPart	drawing_area;
} XmDrawingAreaRec;



/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern void _XmDrawingAreaInput() ;

#else

extern void _XmDrawingAreaInput( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* MOTIF12_HEADERS */

#endif /* _XmDrawingAreaP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
