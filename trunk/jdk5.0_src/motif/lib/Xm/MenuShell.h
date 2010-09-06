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
/*   $XConsortium: MenuShell.h /main/11 1995/07/13 17:36:21 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmMenuShell_h
#define _XmMenuShell_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref WidgetClass xmMenuShellWidgetClass;

typedef struct _XmMenuShellClassRec       * XmMenuShellWidgetClass;
typedef struct _XmMenuShellWidgetRec      * XmMenuShellWidget;

#ifndef XmIsMenuShell
#define XmIsMenuShell(w) XtIsSubclass(w, xmMenuShellWidgetClass)
#endif /* XmIsMenuShell */


/********    Public Function Declarations    ********/

extern Widget XmCreateMenuShell( 
                        Widget parent,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;

/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmMenuShell_h */
/* DON'T ADD STUFF AFTER THIS #endif */
