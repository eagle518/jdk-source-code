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
/*   $XConsortium: MessageB.h /main/11 1995/07/13 17:37:42 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmMessage_h
#define _XmMessage_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Class record constants */

externalref WidgetClass xmMessageBoxWidgetClass;

typedef struct _XmMessageBoxClassRec * XmMessageBoxWidgetClass;
typedef struct _XmMessageBoxRec      * XmMessageBoxWidget;

/* fast XtIsSubclass define */
#ifndef XmIsMessageBox
#define XmIsMessageBox(w) XtIsSubclass (w, xmMessageBoxWidgetClass)
#endif 


/********    Public Function Declarations    ********/

extern Widget XmCreateMessageBox( 
                        Widget parent,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmCreateMessageDialog( 
                        Widget parent,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmCreateErrorDialog( 
                        Widget parent,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmCreateInformationDialog( 
                        Widget parent,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmCreateQuestionDialog( 
                        Widget parent,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmCreateWarningDialog( 
                        Widget parent,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmCreateWorkingDialog( 
                        Widget parent,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmCreateTemplateDialog( 
                        Widget parent,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmMessageBoxGetChild( 
                        Widget widget,
#if NeedWidePrototypes
                        unsigned int child) ;
#else
                        unsigned char child) ;
#endif /* NeedWidePrototypes */

/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmMessage_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
