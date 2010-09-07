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
/* $XConsortium: CascadeBGI.h /main/6 1995/07/14 10:14:47 drk $ */
#ifndef _XmCascadeBGI_h
#define _XmCascadeBGI_h

#include <Xm/CascadeBGP.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations    ********/

extern int _XmArrowPixmapCacheCompare( 
                        XtPointer A,
                        XtPointer B) ;
extern void _XmArrowPixmapCacheDelete( 
                        XtPointer data) ;
extern void _XmCreateArrowPixmaps( 
                        Widget wid) ;

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmCascadeBGI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
