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
/* $XConsortium: DrawI.h /main/6 1995/07/14 10:27:37 drk $ */
#ifndef _XmDrawI_h
#define _XmDrawI_h

#include <Xm/DrawP.h>

#ifdef __cplusplus
extern "C" {
#endif


/********    Private Function Declarations    ********/

extern void _XmDrawHighlight( 
                        Display *display,
                        Drawable d,
                        GC gc,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int width,
                        int height,
                        int highlight_thick,
#else
                        Position x,
                        Position y,
                        Dimension width,
                        Dimension height,
                        Dimension highlight_thick,
#endif /* NeedWidePrototypes */
                        int line_style) ;

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDrawI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
