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
/*   $XConsortium: DrawnB.h /main/12 1995/07/14 10:29:23 drk $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/***********************************************************************
 *
 * DrawnButton Widget
 *
 ***********************************************************************/

#ifndef _XmDButton_h
#define _XmDButton_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XmIsDrawnButton
#define XmIsDrawnButton(w) XtIsSubclass(w, xmDrawnButtonWidgetClass)
#endif /* XmIsDrawnButton */

/* DrawnButon Widget */

externalref WidgetClass xmDrawnButtonWidgetClass;

typedef struct _XmDrawnButtonClassRec *XmDrawnButtonWidgetClass;
typedef struct _XmDrawnButtonRec      *XmDrawnButtonWidget;


/********    Public Function Declarations    ********/

extern Widget XmCreateDrawnButton( 
                        Widget parent,
                        char *name,
                        ArgList arglist,
                        Cardinal argcount) ;

/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDButton_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
