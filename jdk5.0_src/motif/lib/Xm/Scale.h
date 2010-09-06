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
/*   $XConsortium: Scale.h /main/11 1995/07/13 17:52:48 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmScale_h
#define _XmScale_h


#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Class record constants */

externalref WidgetClass xmScaleWidgetClass;

/* fast XtIsSubclass define */
#ifndef XmIsScale
#define XmIsScale(w) XtIsSubclass (w, xmScaleWidgetClass)
#endif

typedef struct _XmScaleClassRec * XmScaleWidgetClass;
typedef struct _XmScaleRec      * XmScaleWidget;


/********    Public Function Declarations    ********/

extern void XmScaleSetValue( 
                        Widget w,
                        int value) ;
extern void XmScaleGetValue( 
                        Widget w,
                        int *value) ;
extern Widget XmCreateScale( 
                        Widget parent,
                        char *name,
                        ArgList arglist,
                        Cardinal argcount) ;
extern void XmScaleSetTicks(
			    Widget scale,
			    int big_every,
			    Cardinal num_med,
			    Cardinal num_small, 
			    Dimension  size_big,
			    Dimension  size_med,
			    Dimension  size_small);
/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmScale_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
