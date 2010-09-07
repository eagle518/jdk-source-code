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
/* $XConsortium: CascadeBI.h /main/6 1995/07/14 10:15:09 drk $ */
#ifndef _XmCascadeBI_h
#define _XmCascadeBI_h

#include <Xm/CascadeBP.h>

#ifdef __cplusplus
extern "C" {
#endif


/********    Private Function Declarations    ********/

extern void _XmCBHelp( 
                        Widget w,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmCascadingPopup( 
                        Widget cb,
                        XEvent *event,
#if NeedWidePrototypes
                        int doCascade) ;
#else
                        Boolean doCascade) ;
#endif /* NeedWidePrototypes */

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmCascadeBI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
