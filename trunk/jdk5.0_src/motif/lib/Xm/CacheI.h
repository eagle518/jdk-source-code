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
/* $XConsortium: CacheI.h /main/6 1995/07/14 10:12:39 drk $ */
#ifndef _XmCacheI_h
#define _XmCacheI_h

#include <Xm/XmP.h>
#include <Xm/CacheP.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations    ********/

extern void _XmCacheDelete( 
                        XtPointer data) ;
extern void _XmCacheCopy( 
                        XtPointer src,
                        XtPointer dest,
                        size_t size) ;
extern XtPointer _XmCachePart( 
                        XmCacheClassPartPtr cp,
                        XtPointer cpart,
                        size_t size) ;

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmCacheI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
