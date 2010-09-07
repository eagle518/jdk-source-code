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
/*   $XConsortium: Form.h /main/12 1995/07/14 10:34:46 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmForm_h
#define _XmForm_h


#include <Xm/BulletinB.h>

#ifdef __cplusplus
extern "C" {
#endif

/*  Form Widget  */

externalref WidgetClass xmFormWidgetClass;

typedef struct _XmFormClassRec * XmFormWidgetClass;
typedef struct _XmFormRec      * XmFormWidget;


/* ifndef for Fast Subclassing  */

#ifndef XmIsForm
#define XmIsForm(w)	XtIsSubclass(w, xmFormWidgetClass)
#endif  /* XmIsForm */

/********    Public Function Declarations    ********/

extern Widget XmCreateForm( 
                        Widget parent,
                        char *name,
                        ArgList arglist,
                        Cardinal argcount) ;
extern Widget XmCreateFormDialog( 
                        Widget parent,
                        char *name,
                        ArgList arglist,
                        Cardinal argcount) ;

/********    End Public Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmForm_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
