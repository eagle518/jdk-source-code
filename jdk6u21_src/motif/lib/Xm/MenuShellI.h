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
/* $XConsortium: MenuShellI.h /main/5 1995/07/13 17:36:29 drk $ */
#ifndef _XmMenuShellI_h
#define _XmMenuShellI_h

#include <Xm/MenuShellP.h>

#ifdef __cplusplus
extern "C" {
#endif


/********    Private Function Declarations    ********/

extern void _XmEnterRowColumn( 
                        Widget widget,
                        XtPointer closure,
                        XEvent *event,
                        Boolean *cont) ;
extern void _XmClearTraversal( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmSetLastManagedMenuTime(
			Widget wid,
			Time newTime ) ;
extern void _XmPopupSpringLoaded(
			Widget shell ) ;
extern void _XmPopdown(
		        Widget shell ) ;

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmMenuShellI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
