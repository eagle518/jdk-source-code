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
/*   $XConsortium: DragOverSP.h /main/9 1995/07/14 10:26:38 drk $ */
/*
*  (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmDragOverSP_h
#define _XmDragOverSP_h

#ifndef MOTIF12_HEADERS

#include <X11/Shell.h>
#include <X11/ShellP.h>
#include <Xm/XmP.h>
#include <Xm/DragIconP.h>
#include <Xm/DragOverS.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DOExpose(do) \
	((XtClass(do))->core_class.expose) ((Widget)(do), NULL, NULL)

/* 
 * DRAGOVER SHELL
 */
typedef struct 
{
    XtPointer				extension;
} XmDragOverShellClassPart;

/* Full class record declaration */

typedef struct _XmDragOverShellClassRec 
{
    CoreClassPart 		core_class;
    CompositeClassPart 		composite_class;
    ShellClassPart 		shell_class;
    WMShellClassPart	        wm_shell_class;
    VendorShellClassPart 	vendor_shell_class;
    XmDragOverShellClassPart 	dragOver_shell_class;
} XmDragOverShellClassRec;

externalref XmDragOverShellClassRec xmDragOverShellClassRec;

typedef struct _XmBackingRec{
    Position	x, y;
    Pixmap	pixmap;
}XmBackingRec, *XmBacking;

typedef struct _XmDragOverBlendRec{
    XmDragIconObject		sourceIcon;	/* source icon */
    Position			sourceX;	/* source location in blend */
    Position			sourceY;	/* source location in blend */
    XmDragIconObject		mixedIcon;	/* blended icon */
    GC				gc;		/* appropriate depth */
}XmDragOverBlendRec, *XmDragOverBlend;

typedef struct _XmDragOverShellPart{
    Position		hotX;		/* current hotX */
    Position		hotY;		/* current hotY */
    unsigned char	cursorState;	/* current cursor state */
    unsigned char	mode;
    unsigned char	activeMode;

    Position		initialX;	/* initial hotX */
    Position		initialY;	/* initial hotY */

    XmDragIconObject	stateIcon;	/* current state icon */
    XmDragIconObject	opIcon;		/* current operation icon */

    XmDragOverBlendRec	cursorBlend;	/* cursor blending */
    XmDragOverBlendRec	rootBlend;	/* pixmap or window blending */
    Pixel		cursorForeground;
    Pixel		cursorBackground;
    Cursor		ncCursor;	/* noncached cursor */
    Cursor		activeCursor;	/* the current cursor */

    XmBackingRec	backing; 	/* backing store for pixdrag */
    Pixmap		tmpPix;		/* temp storage for pixdrag */
    Pixmap		tmpBit;		/* temp storage for pixdrag */
    Boolean             isVisible;	/* shell is visible */

    /* Added for ShapedWindow dragging */
    /* Resources */
    Boolean		installColormap;/* Install the colormap */

    /* locals */
    Boolean		holePunched;	/* true if hole is punched */

    /* the following variables are used to make sure the correct colormap */
    /* is installed.  colormapWidget is initially the parent widget, but */
    /* can be changed by calling DragShellColormapWidget.		*/
    Widget		colormapWidget;	/* The widget I'm dragging from */
    Widget		colormapShell;	/* It's shell, install colormap here */
    Boolean		colormapOverride; /* shell is override rediirect */
    Colormap*		savedColormaps;	/* used with override redirect */
    int			numSavedColormaps;
}XmDragOverShellPart;

typedef  struct _XmDragOverShellRec{
    CorePart	 	core;
    CompositePart 	composite;
    ShellPart 		shell;
    WMShellPart		wm;
    VendorShellPart	vendor;
    XmDragOverShellPart	drag;
} XmDragOverShellRec;

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#else /* MOTIF12_HEADERS */


/* 
 * @OSF_COPYRIGHT@
 * (c) Copyright 1990, 1991, 1992, 1993, 1994 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *  
*/ 
/*
 * HISTORY
 * Motif Release 1.2.5
*/
/*   $XConsortium: DragOverSP.h /main/cde1_maint/2 1995/08/18 18:59:48 drk $ */
/*
*  (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#include <X11/Shell.h>
#include <X11/ShellP.h>
#include <Xm/XmP.h>
#include <Xm/DragIconP.h>
#include <Xm/DragOverS.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DOExpose(do) \
	((XtClass(do))->core_class.expose) ((Widget)(do), NULL, NULL)

/* 
 * DRAGOVER SHELL
 */
typedef struct 
{
    XtPointer				extension;
} XmDragOverShellClassPart;

/* Full class record declaration */

typedef struct _XmDragOverShellClassRec 
{
    CoreClassPart 		core_class;
    CompositeClassPart 		composite_class;
    ShellClassPart 		shell_class;
    WMShellClassPart	        wm_shell_class;
    VendorShellClassPart 	vendor_shell_class;
    XmDragOverShellClassPart 	dragOver_shell_class;
} XmDragOverShellClassRec;

externalref XmDragOverShellClassRec xmDragOverShellClassRec;

typedef struct _XmBackingRec{
    Position	x, y;
    Pixmap	pixmap;
}XmBackingRec, *XmBacking;

typedef struct _XmDragOverBlendRec{
    XmDragIconObject		sourceIcon;	/* source icon */
    Position			sourceX;	/* source location in blend */
    Position			sourceY;	/* source location in blend */
    XmDragIconObject		mixedIcon;	/* blended icon */
    GC				gc;		/* appropriate depth */
}XmDragOverBlendRec, *XmDragOverBlend;

typedef struct _XmDragOverShellPart{
    Position			hotX;		/* current hotX */
    Position			hotY;		/* current hotY */
    unsigned char		cursorState;	/* current cursor state */
    unsigned char		mode;
    unsigned char		activeMode;

    Position			initialX;	/* initial hotX */
    Position			initialY;	/* initial hotY */

    XmDragIconObject		stateIcon;	/* current state icon */
    XmDragIconObject		opIcon;		/* current operation icon */

    XmDragOverBlendRec		cursorBlend;	/* cursor blending */
    XmDragOverBlendRec		rootBlend;	/* pixmap or window blending */
    Pixel			cursorForeground;
    Pixel			cursorBackground;
    Cursor			ncCursor;	/* noncached cursor */
    Cursor			activeCursor;	/* the current cursor */

    XmBackingRec		backing; 	/* backing store for pixdrag */
    Pixmap			tmpPix;		/* temp storage for pixdrag */
    Pixmap			tmpBit;		/* temp storage for pixdrag */
    Boolean                     isVisible;	/* shell is visible */
}XmDragOverShellPart;

typedef  struct _XmDragOverShellRec{
    CorePart	 	core;
    CompositePart 	composite;
    ShellPart 		shell;
    WMShellPart		wm;
    VendorShellPart	vendor;
    XmDragOverShellPart	drag;
} XmDragOverShellRec;

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* MOTIF12_HEADERS */

#endif /* _XmDragOverSP_h */
