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
/* $XConsortium: TextSelI.h /main/5 1995/07/13 18:10:00 drk $ */
#ifndef _XmTextSelI_h
#define _XmTextSelI_h

#include <Xm/TextSelP.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations    ********/

extern Boolean _XmTextConvert( 
                        Widget w,
                        Atom *selection,
                        Atom *target,
                        Atom *type,
                        XtPointer *value,
                        unsigned long *length,
                        int *format,
			Widget dc,
			XEvent *event) ;
extern void _XmTextLoseSelection( 
                        Widget w,
                        Atom *selection) ;
extern Widget _XmTextGetDropReciever( 
                        Widget w) ;
extern void _XmTextInstallTransferTrait(void);

/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmTextSelI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
