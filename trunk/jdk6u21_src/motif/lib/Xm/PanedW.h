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
/*   $XConsortium: PanedW.h /main/11 1995/07/13 17:40:28 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
*  (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
/****************************************************************
 *
 * Vertical Paned Widget (SubClass of CompositeClass)
 *
 ****************************************************************/
#ifndef _XmPanedW_h
#define _XmPanedW_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Class record constant */
externalref WidgetClass xmPanedWindowWidgetClass;

#ifndef XmIsPanedWindow
#define XmIsPanedWindow(w)	XtIsSubclass(w, xmPanedWindowWidgetClass)
#endif /* XmIsPanedWindow */

typedef struct _XmPanedWindowClassRec  *XmPanedWindowWidgetClass;
typedef struct _XmPanedWindowRec	*XmPanedWindowWidget;


/********    Public Function Declarations    ********/

extern Widget XmCreatePanedWindow( 
                        Widget parent,
                        char *name,
                        ArgList args,
                        Cardinal argCount) ;

/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmPanedWindow_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
