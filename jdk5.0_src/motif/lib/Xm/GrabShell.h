/* $XConsortium: GrabShell.h /main/5 1995/07/15 20:51:22 drk $ */
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
#ifndef _XmGrabShell_h
#define _XmGrabShell_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref WidgetClass xmGrabShellWidgetClass;

typedef struct _XmGrabShellClassRec       * XmGrabShellWidgetClass;
typedef struct _XmGrabShellWidgetRec      * XmGrabShellWidget;

#ifndef XmIsGrabShell
#define XmIsGrabShell(w) XtIsSubclass(w, xmGrabShellWidgetClass)
#endif /* XmIsGrabShell */


/********    Public Function Declarations    ********/

extern Widget XmCreateGrabShell( 
                        Widget parent,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;

/********    End Public Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmGrabShell_h */
/* DON'T ADD STUFF AFTER THIS #endif */
