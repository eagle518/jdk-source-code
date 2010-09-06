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
/*   $XConsortium: Command.h /main/12 1995/07/14 10:16:15 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmCommand_h
#define _XmCommand_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Class record constants */

externalref WidgetClass xmCommandWidgetClass;

typedef struct _XmCommandClassRec * XmCommandWidgetClass;
typedef struct _XmCommandRec      * XmCommandWidget;


#ifndef XmIsCommand
#define XmIsCommand(w)  (XtIsSubclass (w, xmCommandWidgetClass))
#endif



/********    Public Function Declarations    ********/

extern Widget XmCreateCommand( 
                        Widget parent,
                        String name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmCommandGetChild( 
                        Widget widget,
#if NeedWidePrototypes
                        unsigned int child) ;
#else
                        unsigned char child) ;
#endif /* NeedWidePrototypes */
extern void XmCommandSetValue( 
                        Widget widget,
                        XmString value) ;
extern void XmCommandAppendValue( 
                        Widget widget,
                        XmString value) ;
extern void XmCommandError( 
                        Widget widget,
                        XmString error) ;
extern Widget XmCreateCommandDialog( 
                        Widget ds_p,
                        String name,
                        ArgList fsb_args,
                        Cardinal fsb_n) ;


/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmCommand_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
