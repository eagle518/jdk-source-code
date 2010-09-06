/*
 * COPYRIGHT NOTICE
 * Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 * ALL RIGHTS RESERVED (MOTIF).  See the file named COPYRIGHT.MOTIF
 * for the full copyright text.
 * 
 */
/*
 * HISTORY
 */
/* $XConsortium: TearOffI.h /main/5 1995/07/13 18:02:12 drk $ */
#ifndef _XmTearOffI_h
#define _XmTearOffI_h

#include <Xm/TearOffP.h>

#ifdef __cplusplus
extern "C" {
#endif


/********    Private Function Declarations    ********/

extern void _XmTearOffBtnDownEventHandler( 
                        Widget reportingWidget,
                        XtPointer data,
                        XEvent *event,
                        Boolean *cont) ;
extern void _XmTearOffBtnUpEventHandler( 
                        Widget reportingWidget,
                        XtPointer data,
                        XEvent *event,
                        Boolean *cont) ;
extern void _XmDestroyTearOffShell( 
                        Widget wid) ;
extern void _XmDismissTearOff( 
                        Widget shell,
                        XtPointer closure,
                        XtPointer call_data) ;
extern void _XmTearOffInitiate( 
                        Widget wid,
                        XEvent *event) ;
extern void _XmAddTearOffEventHandlers( 
                        Widget wid) ;
extern Boolean _XmIsTearOffShellDescendant( 
                        Widget wid) ;
extern void _XmLowerTearOffObscuringPoppingDownPanes(
			Widget ancestor,
			Widget tearOff ) ;
extern void _XmRestoreExcludedTearOffToToplevelShell( 
                        Widget wid,
                        XEvent *event) ;
extern void _XmRestoreTearOffToToplevelShell( 
                        Widget wid,
                        XEvent *event) ;
extern void _XmRestoreTearOffToMenuShell( 
                        Widget wid,
                        XEvent *event) ;

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmTearOffI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
