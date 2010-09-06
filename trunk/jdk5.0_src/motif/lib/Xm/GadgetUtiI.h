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
/* $XConsortium: GadgetUtiI.h /main/5 1995/07/13 17:27:26 drk $ */
#ifndef _XmGadgetUtilI_h
#define _XmGadgetUtilI_h

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations for GadgetUtil.c    ********/

extern XmGadget _XmInputForGadget(Widget cw, int x, int y) ;
extern void _XmDispatchGadgetInput(Widget g, XEvent *event, Mask mask) ;
extern Time _XmGetDefaultTime(Widget, XEvent*) ;

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmGadgetUtilI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
