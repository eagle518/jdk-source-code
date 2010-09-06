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
/*   $XConsortium: CascadeB.h /main/12 1995/07/14 10:13:57 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmCascadeB_h
#define _XmCascadeB_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref WidgetClass xmCascadeButtonWidgetClass;

typedef struct _XmCascadeButtonRec      * XmCascadeButtonWidget;
typedef struct _XmCascadeButtonClassRec * XmCascadeButtonWidgetClass;

/* fast subclass define */
#ifndef XmIsCascadeButton 
#define XmIsCascadeButton(w) XtIsSubclass(w, xmCascadeButtonWidgetClass)
#endif /* XmIsCascadeButton */


/********    Public Function Declarations    ********/

extern Widget XmCreateCascadeButton( 
                        Widget parent,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern void XmCascadeButtonHighlight( 
                        Widget cb,
#if NeedWidePrototypes
                        int highlight) ;
#else
                        Boolean highlight) ;
#endif /* NeedWidePrototypes */

/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmCascadeB_h */
/* DON'T ADD STUFF AFTER THIS #endif */
