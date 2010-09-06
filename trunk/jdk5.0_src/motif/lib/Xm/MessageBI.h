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
/* $XConsortium: MessageBI.h /main/5 1995/07/13 17:37:52 drk $ */
#ifndef _XmMessageBI_h
#define _XmMessageBI_h

#include <Xm/MessageBP.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations    ********/

extern XmGeoMatrix _XmMessageBoxGeoMatrixCreate( 
                        Widget wid,
                        Widget instigator,
                        XtWidgetGeometry *desired) ;
extern Boolean _XmMessageBoxNoGeoRequest( 
                        XmGeoMatrix geoSpec) ;

/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmMessageBI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
