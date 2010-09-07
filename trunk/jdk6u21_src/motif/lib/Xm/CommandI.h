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
/* $XConsortium: CommandI.h /main/6 1995/07/14 10:16:31 drk $ */
#ifndef _XmCommandI_h
#define _XmCommandI_h

#include <Xm/CommandP.h>

#ifdef __cplusplus
extern "C" {
#endif


/********    Private Function Declarations    ********/

extern void _XmCommandReturn( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *numParams) ;
extern void _XmCommandUpOrDown( 
                        Widget wid,
                        XEvent *event,
                        String *argv,
                        Cardinal *argc) ;

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmCommandI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
