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
/* $XConsortium: DragUnderI.h /main/11 1995/07/14 10:27:08 drk $ */
/* (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#ifndef _XmDragUnderI_h
#define _XmDragUnderI_h

#include <Xm/XmP.h>
#include <Xm/Screen.h>		/* for XmScreen */

#ifdef __cplusplus
extern "C" {
#endif

/* Structure describing a pixmap */

typedef struct _DragPixmapData {
    Pixmap		pixmap;
    int			x, y;
    unsigned int	width, height;
} DragPixmapData;

typedef struct _XmAnimationSaveData {
    Display		*display;
    XmScreen		xmScreen;
    Window		window;
    Position		windowX;
    Position		windowY;
    unsigned int	windowDepth;
    XmRegion		clipRegion;
    XmRegion		dropSiteRegion;
    Dimension		shadowThickness;
    Dimension		highlightThickness;
    Pixel		background;
    Pixel		foreground;
    Pixel		highlightColor;
    Pixmap		highlightPixmap;
    Pixel		topShadowColor;
    Pixmap		topShadowPixmap;
    Pixel		bottomShadowColor;
    Pixmap		bottomShadowPixmap;

    Dimension		borderWidth;
    Pixmap		animationMask;
    Pixmap		animationPixmap;
    unsigned int	animationPixmapDepth;
    unsigned char	animationStyle;
    Widget		dragOver;

    GC			highlightGC;
    GC			topShadowGC;
    GC			bottomShadowGC;
    GC			drawGC;
    DragPixmapData	*savedPixmaps;
    Cardinal		numSavedPixmaps;
    Widget		dragUnder;
    unsigned char	activeMode;
} XmAnimationSaveDataRec, *XmAnimationSaveData;


/********    Private Function Declarations for DragUnder.c    ********/

extern void _XmDragUnderAnimation( 
                        Widget w,
                        XtPointer clientData,
                        XtPointer callData) ;

/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDragUnderI_h */
