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
/*   $XConsortium: DialogS.h /main/13 1995/07/14 10:18:50 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
*  (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
/*
*  (c) Copyright 1988 MICROSOFT CORPORATION */
#ifndef _XmDialogShell_h
#define _XmDialogShell_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef XmIsDialogShell
#define XmIsDialogShell(w)	XtIsSubclass(w, xmDialogShellWidgetClass)
#endif /* XmIsDialogShell */

externalref WidgetClass xmDialogShellWidgetClass;

typedef struct _XmDialogShellClassRec       * XmDialogShellWidgetClass;
typedef struct _XmDialogShellRec            * XmDialogShellWidget;


/********    Public Function Declarations    ********/

extern Widget XmCreateDialogShell( 
                        Widget p,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;

/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDialogShell_h */
/* DON'T ADD STUFF AFTER THIS #endif */
