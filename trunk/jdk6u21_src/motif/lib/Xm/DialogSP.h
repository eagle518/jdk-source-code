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
/*   $XConsortium: DialogSP.h /main/13 1995/07/14 10:19:42 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
*  (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
/*
*  (c) Copyright 1988 MICROSOFT CORPORATION */
#ifndef _XmDialogShellP_h
#define _XmDialogShellP_h

#include <Xm/XmP.h>
#include <Xm/DialogS.h>
#include <X11/ShellP.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The DialogShell instance record */

typedef struct 
{
    /* internal fields */
    XtGrabKind 		grab_kind;
    Position		old_x, old_y;
} XmDialogShellPart;


/* Full instance record declaration */

typedef  struct _XmDialogShellRec 
{	
    CorePart		    core;
    CompositePart	    composite;
    ShellPart		    shell;
    WMShellPart		    wm;
    VendorShellPart	    vendor;
    TransientShellPart	    transient;
    XmDialogShellPart	    dialog;
} XmDialogShellRec;

typedef  struct _XmDialogShellWidgetRec /* OBSOLETE (for compatibility only).*/
{	
    CorePart		    core;
    CompositePart	    composite;
    ShellPart		    shell;
    WMShellPart		    wm;
    VendorShellPart	    vendor;
    TransientShellPart	    transient;
    XmDialogShellPart	    dialog;
} XmDialogShellWidgetRec;



/* DialogShell class structure */

typedef struct 
{
    XtPointer			extension;	 /* Pointer to extension record */
} XmDialogShellClassPart;


/* Full class record declaration */

typedef struct _XmDialogShellClassRec 
{
    CoreClassPart 		core_class;
    CompositeClassPart 		composite_class;
    ShellClassPart 		shell_class;
    WMShellClassPart	        wm_shell_class;
    VendorShellClassPart 	vendor_shell_class;
    TransientShellClassPart  	transient_shell_class;
    XmDialogShellClassPart 	dialog_shell_part;
} XmDialogShellClassRec;


externalref XmDialogShellClassRec  xmDialogShellClassRec;


/********    Private Function Declarations    ********/


/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDialogShellP_h */
/* DON'T ADD STUFF AFTER THIS #endif */
