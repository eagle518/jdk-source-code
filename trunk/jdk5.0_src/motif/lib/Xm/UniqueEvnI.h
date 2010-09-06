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
/* $XConsortium: UniqueEvnI.h /main/5 1995/07/13 18:17:21 drk $ */
#ifndef _XmUniqueEventI_h
#define _XmUniqueEventI_h

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations for UniqueEvnt.c    ********/

extern Boolean _XmIsEventUnique(XEvent *event) ;
extern void _XmRecordEvent(XEvent *event) ;

/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmUniqueEventI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
