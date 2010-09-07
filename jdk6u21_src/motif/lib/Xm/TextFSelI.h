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
/* $XConsortium: TextFSelI.h /main/5 1995/07/13 18:06:08 drk $ */
#ifndef _XmTextFSelI_h
#define _XmTextFSelI_h

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations    ********/

extern Boolean _XmTextFieldConvert( 
                        Widget w,
                        Atom *selection,
                        Atom *target,
                        Atom *type,
                        XtPointer *value,
                        unsigned long *length,
                        int *format,
			Widget drag_context,
			XEvent *event) ;
extern void _XmTextFieldLoseSelection( 
                        Widget w,
                        Atom *selection) ;
extern Widget _XmTextFieldGetDropReciever( 
                        Widget w) ;
extern void _XmTextFieldInstallTransferTrait(void);

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmTextFSelI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
