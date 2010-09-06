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
/*   $XConsortium: ToggleB.h /main/12 1995/07/13 18:11:58 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/***********************************************************************
 *
 * Toggle Widget
 *
 ***********************************************************************/
#ifndef _XmToggle_h
#define _XmToggle_h

#ifndef MOTIF12_HEADERS

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref WidgetClass xmToggleButtonWidgetClass;

typedef struct _XmToggleButtonClassRec *XmToggleButtonWidgetClass;
typedef struct _XmToggleButtonRec      *XmToggleButtonWidget;

/*fast subclass define */
#ifndef XmIsToggleButton
#define XmIsToggleButton(w)     XtIsSubclass(w, xmToggleButtonWidgetClass)
#endif /* XmIsToggleButton */


/********    Public Function Declarations    ********/

extern Boolean XmToggleButtonGetState( 
                        Widget w) ;
extern void XmToggleButtonSetState( 
                        Widget w,
#if NeedWidePrototypes
                        int newstate,
                        int notify) ;
#else
                        Boolean newstate,
                        Boolean notify) ;
#endif /* NeedWidePrototypes */

extern Boolean
XmToggleButtonSetValue(
        Widget w,
#if NeedWidePrototypes
        int newstate,
        int notify );
#else
        XmToggleButtonState newstate,
        Boolean notify );
#endif /* NeedWidePrototypes */
extern Widget XmCreateToggleButton( 
                        Widget parent,
                        char *name,
                        Arg *arglist,
                        Cardinal argCount) ;

/********    End Public Function Declarations    ********/


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
/*   $XConsortium: ToggleB.h /main/cde1_maint/2 1995/08/18 19:29:33 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/***********************************************************************
 *
 * Toggle Widget
 *
 ***********************************************************************/

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref WidgetClass xmToggleButtonWidgetClass;

typedef struct _XmToggleButtonClassRec *XmToggleButtonWidgetClass;
typedef struct _XmToggleButtonRec      *XmToggleButtonWidget;

/*fast subclass define */
#ifndef XmIsToggleButton
#define XmIsToggleButton(w)     XtIsSubclass(w, xmToggleButtonWidgetClass)
#endif /* XmIsToggleButton */


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Boolean XmToggleButtonGetState() ;
extern void XmToggleButtonSetState() ;
extern Widget XmCreateToggleButton() ;

#else

extern Boolean XmToggleButtonGetState( 
                        Widget w) ;
extern void XmToggleButtonSetState( 
                        Widget w,
#if NeedWidePrototypes
                        int newstate,
                        int notify) ;
#else
                        Boolean newstate,
                        Boolean notify) ;
#endif /* NeedWidePrototypes */
extern Widget XmCreateToggleButton( 
                        Widget parent,
                        char *name,
                        Arg *arglist,
                        Cardinal argCount) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* MOTIF12_HEADERS */

#endif /* _XmToggle_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
