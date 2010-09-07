/* $XConsortium: Gadget.h /main/5 1995/07/15 20:51:03 drk $ */
/*
 *  @OSF_COPYRIGHT@
 *  COPYRIGHT NOTICE
 *  Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 *  ALL RIGHTS RESERVED (MOTIF). See the file named COPYRIGHT.MOTIF for
 *  the full copyright text.
 */
/*
 * HISTORY
 */
#ifndef _XmGadget_h
#define _XmGadget_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XmIsGadget
#define XmIsGadget(w) XtIsSubclass(w, xmGadgetClass)
#endif /* XmIsGadget */

externalref WidgetClass xmGadgetClass;

typedef struct _XmGadgetClassRec * XmGadgetClass;
typedef struct _XmGadgetRec      * XmGadget;


/********    Public Function Declarations    ********/


/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmGadget_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
