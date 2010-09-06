/* $XConsortium: Primitive.h /main/5 1995/07/15 20:54:22 drk $ */
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
#ifndef _XmPrimitive_h
#define _XmPrimitive_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XmIsPrimitive
#define XmIsPrimitive(w) XtIsSubclass(w, xmPrimitiveWidgetClass)
#endif /* XmIsPrimitive */

externalref WidgetClass xmPrimitiveWidgetClass;

typedef struct _XmPrimitiveClassRec * XmPrimitiveWidgetClass;
typedef struct _XmPrimitiveRec      * XmPrimitiveWidget;


/********    Public Function Declarations    ********/


/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmPrimitive_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
