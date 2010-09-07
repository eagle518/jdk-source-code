/* $XConsortium: IconH.h /main/5 1995/07/15 20:52:21 drk $ */
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
#ifndef _XmIconH_h
#define _XmIconH_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif



/* Class record constants */
extern	WidgetClass	xmIconHeaderClass;

typedef struct _XmIconHeaderClassRec * XmIconHeaderClass;
typedef struct _XmIconHeaderRec      * XmIconHeader;

#ifndef XmIsIconHeader
#define XmIsIconHeader(w) XtIsSubclass(w, xmIconHeaderClass)
#endif /* XmIsIconHeader */

/********    Public Function Declarations    ********/
extern Widget XmCreateIconHeader(
                        Widget parent,
                        String name,
                        ArgList arglist,
                        Cardinal argcount) ;

/********    End Public Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmIconH_h */

/* DON'T ADD ANYTHING AFTER THIS #endif */
