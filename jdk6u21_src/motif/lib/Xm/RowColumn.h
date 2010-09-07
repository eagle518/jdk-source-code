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
/*   $XConsortium: RowColumn.h /main/12 1995/07/13 17:51:13 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
#ifndef _XmRowColumn_h
#define _XmRowColumn_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref WidgetClass xmRowColumnWidgetClass;

typedef struct _XmRowColumnClassRec * XmRowColumnWidgetClass;
typedef struct _XmRowColumnRec      * XmRowColumnWidget;

#ifndef XmIsRowColumn
#define XmIsRowColumn(w) XtIsSubclass((w),xmRowColumnWidgetClass)
#endif


/********    Public Function Declarations    ********/

extern void XmMenuPosition( 
                        Widget p,
                        XButtonPressedEvent *event) ;
extern Widget XmCreateRowColumn( 
                        Widget p,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmCreateWorkArea( 
                        Widget p,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmCreateRadioBox( 
                        Widget p,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmCreateOptionMenu( 
                        Widget p,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmOptionLabelGadget( 
                        Widget m) ;
extern Widget XmOptionButtonGadget( 
                        Widget m) ;
extern Widget XmCreateMenuBar( 
                        Widget p,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmCreatePopupMenu( 
                        Widget p,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmCreatePulldownMenu( 
                        Widget p,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern Widget XmGetPostedFromWidget( 
                        Widget menu) ;
extern Widget XmGetTearOffControl(
			Widget menu) ;


/* start fix for bug 4229966 - leob */
extern void   XmAddToPostFromList(
			Widget menu,
			Widget item);

extern void   XmRemoveFromPostFromList(
			Widget menu,
			Widget item);
/* finish fix for bug 4229966 - leob */

/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmRowColumn_h  */
/* DON'T ADD STUFF AFTER THIS #endif */
