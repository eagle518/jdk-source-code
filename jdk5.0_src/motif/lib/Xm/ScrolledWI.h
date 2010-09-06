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
/* $XConsortium: ScrolledWI.h /main/5 1995/07/13 17:56:39 drk $ */
#ifndef _XmScrolledWI_h
#define _XmScrolledWI_h

#include <Xm/ScrolledWP.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SWCEPTR(wc)  ((XmScrolledWindowClassExt *)(&(((XmScrolledWindowWidgetClass)(wc))->swindow_class.extension)))

#define _XmGetScrolledWindowClassExtPtr(wc, owner) \
  ((*SWCEPTR(wc) && (((*SWCEPTR(wc))->record_type) == owner))\
   ? SWCEPTR(wc) \
   :((XmScrolledWindowClassExt *) _XmGetClassExtensionPtr(((XmGenericClassExt *)SWCEPTR(wc)), owner)))


/********    Private Function Declarations    ********/

extern void _XmSWNotifyGeoChange(Widget sw,
				 Widget child,
				 XtWidgetGeometry *request);
extern Boolean _XmSWGetClipArea(Widget widget, 
				XRectangle *rect);

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmScrolledWI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
