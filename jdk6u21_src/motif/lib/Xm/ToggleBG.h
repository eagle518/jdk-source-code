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
/*   $XConsortium: ToggleBG.h /main/12 1995/07/13 18:12:49 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/***********************************************************************
 *
 * Toggle Gadget
 *
 ***********************************************************************/
#ifndef _XmToggleG_h
#define _XmToggleG_h

#ifndef MOTIF12_HEADERS

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif


externalref WidgetClass xmToggleButtonGadgetClass;

typedef struct _XmToggleButtonGadgetClassRec     *XmToggleButtonGadgetClass;
typedef struct _XmToggleButtonGadgetRec          *XmToggleButtonGadget;
typedef struct _XmToggleButtonGCacheObjRec       *XmToggleButtonGCacheObject;


/*fast subclass define */
#ifndef XmIsToggleButtonGadget
#define XmIsToggleButtonGadget(w)     XtIsSubclass(w, xmToggleButtonGadgetClass)
#endif /* XmIsToggleButtonGadget */


/********    Public Function Declarations    ********/

extern Boolean XmToggleButtonGadgetGetState( 
                        Widget w) ;
extern void XmToggleButtonGadgetSetState( 
                        Widget w,
#if NeedWidePrototypes
                        int newstate,
                        int notify) ;
#else
                        Boolean newstate,
                        Boolean notify) ;
#endif /* NeedWidePrototypes */

extern Boolean XmToggleButtonGadgetSetValue(
					    Widget w,
#if NeedWidePrototypes
					    int newstate,
					    int notify);
#else
					    XmToggleButtonState newstate,
					    Boolean notify);
#endif /* NeedWidePrototypes */

extern Widget XmCreateToggleButtonGadget( 
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
/*   $XConsortium: ToggleBG.h /main/cde1_maint/2 1995/08/18 19:29:58 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/***********************************************************************
 *
 * Toggle Gadget
 *
 ***********************************************************************/

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif


externalref WidgetClass xmToggleButtonGadgetClass;

typedef struct _XmToggleButtonGadgetClassRec     *XmToggleButtonGadgetClass;
typedef struct _XmToggleButtonGadgetRec          *XmToggleButtonGadget;
typedef struct _XmToggleButtonGCacheObjRec       *XmToggleButtonGCacheObject;


/*fast subclass define */
#ifndef XmIsToggleButtonGadget
#define XmIsToggleButtonGadget(w)     XtIsSubclass(w, xmToggleButtonGadgetClass)
#endif /* XmIsToggleButtonGadget */


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Boolean XmToggleButtonGadgetGetState() ;
extern void XmToggleButtonGadgetSetState() ;
extern Widget XmCreateToggleButtonGadget() ;

#else

extern Boolean XmToggleButtonGadgetGetState( 
                        Widget w) ;
extern void XmToggleButtonGadgetSetState( 
                        Widget w,
#if NeedWidePrototypes
                        int newstate,
                        int notify) ;
#else
                        Boolean newstate,
                        Boolean notify) ;
#endif /* NeedWidePrototypes */
extern Widget XmCreateToggleButtonGadget( 
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

#endif /* _XmToggleG_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
