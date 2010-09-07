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
/* $XConsortium: DrawingAI.h /main/6 1995/07/14 10:28:32 drk $ */
#ifndef _XmDrawingAI_h
#define _XmDrawingAI_h

#include <Xm/DrawingAP.h>

#ifdef __cplusplus
extern "C" {
#endif


/********    Private Function Declarations    ********/

extern void _XmDrawingAreaInput( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDrawingAI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
