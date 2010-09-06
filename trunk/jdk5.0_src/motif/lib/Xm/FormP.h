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
/*   $XConsortium: FormP.h /main/12 1995/07/14 10:35:01 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmFormP_h
#define _XmFormP_h


#include <Xm/Form.h>
#include <Xm/BulletinBP.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _XmFormAttachmentRec 
{
   unsigned char type;
   Widget w;
   int percent;
   int offset;
   int value;
   int tempValue;
} XmFormAttachmentRec, * XmFormAttachment;


#ifdef att
#undef att
#endif

typedef struct _XmFormConstraintPart
{
   XmFormAttachmentRec att[4];
   Widget next_sibling;
   Boolean sorted;
   Boolean resizable;
   Dimension preferred_width, preferred_height;
} XmFormConstraintPart, * XmFormConstraint;

typedef struct _XmFormConstraintRec
{
   XmManagerConstraintPart manager;
   XmFormConstraintPart    form;
} XmFormConstraintRec, * XmFormConstraintPtr;


/*  Form class structure  */

typedef struct _XmFormClassPart
{
   XtPointer extension;   /* Pointer to extension record */
} XmFormClassPart;


/*  Full class record declaration for form class  */

typedef struct _XmFormClassRec
{
   CoreClassPart       core_class;
   CompositeClassPart  composite_class;
   ConstraintClassPart constraint_class;
   XmManagerClassPart  manager_class;
   XmBulletinBoardClassPart  bulletin_board_class;
   XmFormClassPart     form_class;
} XmFormClassRec;

externalref XmFormClassRec xmFormClassRec;


/*  The Form instance record  */

typedef struct _XmFormPart
{
   Dimension horizontal_spacing;
   Dimension vertical_spacing;
   int fraction_base;
   Boolean rubber_positioning;
   Widget first_child;
   Boolean initial_width, initial_height;
   Boolean processing_constraints;
} XmFormPart;


/*  Full instance record declaration  */

typedef struct _XmFormRec
{
   CorePart	  core;
   CompositePart  composite;
   ConstraintPart constraint;
   XmManagerPart  manager;
   XmBulletinBoardPart  bulletin_board;
   XmFormPart     form;
} XmFormRec;

/********    Private Function Declarations    ********/


/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmFormP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
