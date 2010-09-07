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
/*   $XConsortium: Label.h /main/11 1995/07/13 17:30:38 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmLabel_h
#define _XmLabel_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

/*  Widget class and record definitions  */

externalref WidgetClass xmLabelWidgetClass;

typedef struct _XmLabelClassRec     * XmLabelWidgetClass;
typedef struct _XmLabelRec      * XmLabelWidget;

/*fast subclass define */
#ifndef XmIsLabel
#define XmIsLabel(w)     XtIsSubclass(w, xmLabelWidgetClass)
#endif /* XmIsLabel */


/********    Public Function Declarations    ********/

extern Widget XmCreateLabel( 
                        Widget parent,
                        char *name,
                        Arg *arglist,
                        Cardinal argCount) ;

/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmLabel_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
