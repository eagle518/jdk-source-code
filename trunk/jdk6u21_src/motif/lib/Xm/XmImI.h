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
/* $XConsortium: XmImI.h /main/5 1995/07/13 18:23:12 drk $ */
#ifndef _XmImI_h
#define _XmImI_h

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations for XmIm.c    ********/

extern void _XmImFreeShellData(Widget w, XtPointer* data);
extern void _XmImChangeManaged(Widget vw) ;
extern void _XmImRealize(Widget vw) ;
extern void _XmImResize(Widget vw) ;
extern void _XmImRedisplay(Widget vw) ;

extern Boolean _XmImInputMethodExits(Widget vw);  /* leob fix for bug 4136711 */

/* Solaris 2.6 Motif diff bug #4077411 one lines */
extern int _XmImGetGeo(Widget vw);

/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmImI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
