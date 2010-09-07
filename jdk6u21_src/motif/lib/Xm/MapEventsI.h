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
/* $XConsortium: MapEventsI.h /main/5 1995/07/13 17:35:36 drk $ */
#ifndef _XmMapEventsI_h
#define _XmMapEventsI_h

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations for MapEvents.c    ********/

extern Boolean _XmMapBtnEvent( 
                        register String str,
                        int *eventType,
                        unsigned int *button,
                        Modifiers *modifiers) ;
extern int _XmMapKeyEvents( 
                        register String str,
                        int **eventType,
                        KeySym **keysym,
                        Modifiers **modifiers) ;
extern Boolean _XmMatchBtnEvent( 
                        XEvent *event,
                        int eventType,
                        unsigned int button,
                        Modifiers modifiers) ;
extern Boolean _XmMatchKeyEvent( 
                        XEvent *event,
                        int eventType,
                        unsigned int key,
                        Modifiers modifiers) ;

/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmMapEventsI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
