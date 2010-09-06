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
/* $XConsortium: DragOverSI.h /main/6 1995/07/14 10:26:24 drk $ */
#ifndef _XmDragOverSI_h
#define _XmDragOverSI_h

#include <Xm/DragOverSP.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations for DragOverS.c    ********/

extern void _XmDragOverHide( 
                        Widget w,
#if NeedWidePrototypes
                        int clipOriginX,
                        int clipOriginY,
#else
                        Position clipOriginX,
                        Position clipOriginY,
#endif /* NeedWidePrototypes */
                        XmRegion clipRegion) ;
extern void _XmDragOverShow( 
                        Widget w,
#if NeedWidePrototypes
                        int clipOriginX,
                        int clipOriginY,
#else
                        Position clipOriginX,
                        Position clipOriginY,
#endif /* NeedWidePrototypes */
                        XmRegion clipRegion) ;
extern void _XmDragOverMove( 
                        Widget w,
#if NeedWidePrototypes
                        int x,
                        int y) ;
#else
                        Position x,
                        Position y) ;
#endif /* NeedWidePrototypes */
extern void _XmDragOverChange( 
                        Widget w,
#if NeedWidePrototypes
                        unsigned int dropSiteStatus) ;
#else
                        unsigned char dropSiteStatus) ;
#endif /* NeedWidePrototypes */
extern void _XmDragOverFinish( 
                        Widget w,
#if NeedWidePrototypes
                        unsigned int completionStatus) ;
#else
                        unsigned char completionStatus) ;
#endif /* NeedWidePrototypes */

extern Cursor _XmDragOverGetActiveCursor(
			Widget w) ;
extern void _XmDragOverSetInitialPosition(
			Widget w,
#if NeedWidePrototypes
			int initialX,
			int initialY) ;
#else
			Position initialX,
			Position initialY) ;
#endif /* NeedWidePrototypes */

extern void _XmDragOverUpdateCache();

/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDragOverSI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
