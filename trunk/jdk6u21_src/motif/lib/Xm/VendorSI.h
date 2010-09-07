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
/* $XConsortium: VendorSI.h /main/5 1995/07/13 18:19:43 drk $ */
#ifndef _XmVendorSI_h
#define _XmVendorSI_h

#include <Xm/VendorSP.h>

#ifdef __cplusplus
extern "C" {
#endif

/* used in visual resources declaration and in _XmDefaultVisualResources.
   Cannot use 0 which is None = CopyFromParent, the Xt default */
#define INVALID_VISUAL ((Visual*)-1)

/********    Private Function Declarations    ********/

extern void _XmAddGrab( 
                        Widget wid,
#if NeedWidePrototypes
                        int exclusive,
                        int spring_loaded) ;
#else
                        Boolean exclusive,
                        Boolean spring_loaded) ;
#endif /* NeedWidePrototypes */
extern void _XmRemoveGrab( 
                        Widget wid) ;
extern void _XmDefaultVisualResources(Widget widget) ;

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmVendorSI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
