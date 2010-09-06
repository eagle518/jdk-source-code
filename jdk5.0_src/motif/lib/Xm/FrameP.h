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
/*   $XConsortium: FrameP.h /main/13 1995/09/19 23:03:22 cde-sun $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmFrameP_h
#define _XmFrameP_h

#include <Xm/Frame.h>
#include <Xm/ManagerP.h>

#ifdef __cplusplus
extern "C" {
#endif



/* Full class records */

typedef struct
{
   XtPointer extension;
} XmFrameClassPart;

typedef struct _XmFrameClassRec
{
   CoreClassPart       core_class;
   CompositeClassPart  composite_class;
   ConstraintClassPart constraint_class;
   XmManagerClassPart  manager_class;
   XmFrameClassPart    frame_class;
} XmFrameClassRec;

externalref XmFrameClassRec xmFrameClassRec;


/*  Frame instance records  */

typedef struct
{
   Dimension margin_width;
   Dimension margin_height;
   unsigned char shadow_type;
   Dimension old_width;
   Dimension old_height;
   Dimension old_shadow_thickness;
   Position old_shadow_x;
   Position old_shadow_y;
   Widget work_area;
   Widget title_area;
   Boolean processing_constraints;
} XmFramePart;

typedef struct _XmFrameRec
{
    CorePart	   core;
    CompositePart  composite;
    ConstraintPart constraint;
    XmManagerPart  manager;
    XmFramePart    frame;
} XmFrameRec;


/*  Frame constraint records  */

typedef struct _XmFrameConstraintPart
{
   /* "unused" is actually being used in the CheckSetChildType defaultproc ! */
   int unused;
   unsigned char child_type;
   unsigned char child_h_alignment;
   Dimension child_h_spacing;
   unsigned char child_v_alignment;
} XmFrameConstraintPart, * XmFrameConstraint;

typedef struct _XmFrameConstraintRec
{
   XmManagerConstraintPart manager;
   XmFrameConstraintPart   frame;
} XmFrameConstraintRec, * XmFrameConstraintPtr;


/********    Private Function Declarations    ********/


/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmFrameP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
