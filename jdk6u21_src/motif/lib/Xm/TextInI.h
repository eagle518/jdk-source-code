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
/* $XConsortium: TextInI.h /main/5 1995/07/13 18:07:40 drk $ */
#ifndef _XmTextInI_h
#define _XmTextInI_h

#include <Xm/TextInP.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations    ********/

extern Boolean _XmTextHasDestination( 
                        Widget w) ;
extern Boolean _XmTextSetDestinationSelection( 
                        Widget w,
                        XmTextPosition position,
#if NeedWidePrototypes
                        int disown,
#else
                        Boolean disown,
#endif /* NeedWidePrototypes */
                        Time set_time) ;
extern Boolean _XmTextSetSel2( 
                        XmTextWidget tw,
                        XmTextPosition left,
                        XmTextPosition right,
                        Time set_time) ;
extern Boolean _XmTextGetSel2( 
                        XmTextWidget tw,
                        XmTextPosition *left,
                        XmTextPosition *right) ;
extern void _XmTextInputGetSecResData( 
                        XmSecondaryResourceData *secResDataRtn) ;
extern void _XmTextInputCreate( 
                        Widget wid,
                        ArgList args,
                        Cardinal num_args) ;
extern void _XmTextHandleSecondaryFinished(Widget w,
					   XEvent *event);

/* fix for bug 4367450 - leob */
extern Boolean VerifyLeave(Widget w, XEvent *event);


/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmTextInI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
