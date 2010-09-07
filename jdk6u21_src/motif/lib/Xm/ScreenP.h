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
/* $XConsortium: ScreenP.h /main/8 1995/07/13 17:53:51 drk $ */
/* (c) Copyright 1989, 1990  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/* (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
/* (c) Copyright 1988 MICROSOFT CORPORATION */
#ifndef _XmScreenP_h
#define _XmScreenP_h


#ifndef MOTIF12_HEADERS

#include <Xm/DesktopP.h>
#include <Xm/Screen.h>
#include <Xm/DragIcon.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _XmScreenClassPart {
    XtPointer		extension;
} XmScreenClassPart, *XmScreenClassPartPtr;

typedef struct _XmScreenClassRec {
/*    ObjectClassPart		object_class;
    XmExtClassPart		ext_class; */
    CoreClassPart               core_class ;
    XmDesktopClassPart 		desktop_class;
    XmScreenClassPart		screen_class;
} XmScreenClassRec;

typedef struct _XmDragCursorRec {
    struct _XmDragCursorRec	*next;
    Cursor			cursor;
    XmDragIconObject		stateIcon;
    XmDragIconObject		opIcon;
    XmDragIconObject		sourceIcon;
    Boolean			dirty;
} XmDragCursorRec, *XmDragCursorCache;

typedef struct _XmScratchPixmapKeyRec *XmScratchPixmapKey;

typedef struct _XmScratchPixmapKeyRec {
    Cardinal		depth;
    Dimension           width;
    Dimension           height;
} XmScratchPixmapKeyRec;

typedef struct {
    Boolean		mwmPresent;
    unsigned short	numReparented;
    int			darkThreshold;
    int			foregroundThreshold;
    int			lightThreshold;
    XmDragIconObject	defaultNoneCursorIcon;
    XmDragIconObject	defaultValidCursorIcon;
    XmDragIconObject	defaultInvalidCursorIcon;
    XmDragIconObject	defaultMoveCursorIcon;
    XmDragIconObject	defaultCopyCursorIcon;
    XmDragIconObject	defaultLinkCursorIcon;
    XmDragIconObject	defaultSourceCursorIcon;

    Cursor		nullCursor;
    XmDragCursorRec	*cursorCache;
    Cardinal		maxCursorWidth;
    Cardinal		maxCursorHeight;

    Cursor		menuCursor;
    unsigned char	unpostBehavior;
    XFontStruct *	font_struct;
    int			h_unit;
    int			v_unit;
    XtPointer		scratchPixmaps;
    unsigned char       moveOpaque;
    XmScreenColorProc   color_calc_proc;
    XmAllocColorProc    color_alloc_proc;
    XtEnum              bitmap_conversion_model;

    /* to save internally-created XmDragIcons */

    XmDragIconObject	xmStateCursorIcon;
    XmDragIconObject	xmMoveCursorIcon;
    XmDragIconObject	xmCopyCursorIcon;
    XmDragIconObject	xmLinkCursorIcon;
    XmDragIconObject	xmSourceCursorIcon;

    GC			imageGC;		/* OBSOLETE FIELD */
    int			imageGCDepth;           /* OBSOLETE FIELD */
    Pixel		imageForeground;        /* OBSOLETE FIELD */
    Pixel		imageBackground;        /* OBSOLETE FIELD */

    XtPointer		screenInfo;		/* extension */

    XtPointer           user_data;

    Pixmap              insensitive_stipple_bitmap;

#ifdef DEFAULT_GLYPH_PIXMAP
   Pixmap           default_glyph_pixmap ;
   unsigned int     default_glyph_pixmap_width ;
   unsigned int     default_glyph_pixmap_height ;
#endif

   XtPointer		inUsePixmaps;
} XmScreenPart, *XmScreenPartPtr;

typedef struct _XmScreenInfo {
	/* so much for information hiding */
	XtPointer	menu_state;		/* MenuUtil.c */
	Boolean		destroyCallbackAdded;	/* ImageCache.c */
} XmScreenInfo;

externalref XmScreenClassRec 	xmScreenClassRec;

typedef struct _XmScreenRec {
/*    ObjectPart			object;
    XmExtPart			ext; */
    CorePart                    core ;
    XmDesktopPart		desktop;
    XmScreenPart		screen;
} XmScreenRec;


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
/*   $XConsortium: ScreenP.h /main/cde1_maint/2 1995/08/18 19:20:28 drk $ */
/*
*  (c) Copyright 1989, 1990  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
*  (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
/*
*  (c) Copyright 1988 MICROSOFT CORPORATION */

#include <Xm/DesktopP.h>
#include <Xm/Screen.h>
#include <Xm/DragIcon.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _XmScreenClassPart{
    XtPointer		extension;
}XmScreenClassPart, *XmScreenClassPartPtr;

typedef struct _XmScreenClassRec{
    CoreClassPart		core_class;
    XmDesktopClassPart 		desktop_class;
    XmScreenClassPart		screen_class;
}XmScreenClassRec;

typedef struct _XmDragCursorRec{
    struct _XmDragCursorRec	*next;
    Cursor			cursor;
    XmDragIconObject		stateIcon;
    XmDragIconObject		opIcon;
    XmDragIconObject		sourceIcon;
    Boolean			dirty;
}XmDragCursorRec, *XmDragCursorCache;

typedef struct _XmScratchPixmapRec *XmScratchPixmap;

typedef struct _XmScratchPixmapRec{
    XmScratchPixmap     next;
    Pixmap              pixmap;
    Cardinal		depth;
    Dimension           width;
    Dimension           height;
    Boolean             inUse;
}XmScratchPixmapRec;

typedef struct {
    Boolean		mwmPresent;
    unsigned short	numReparented;
    int			darkThreshold;
    int			foregroundThreshold;
    int			lightThreshold;
    XmDragIconObject	defaultNoneCursorIcon;
    XmDragIconObject	defaultValidCursorIcon;
    XmDragIconObject	defaultInvalidCursorIcon;
    XmDragIconObject	defaultMoveCursorIcon;
    XmDragIconObject	defaultCopyCursorIcon;
    XmDragIconObject	defaultLinkCursorIcon;
    XmDragIconObject	defaultSourceCursorIcon;

    Cursor		nullCursor;
    XmDragCursorRec	*cursorCache;
    Cardinal		maxCursorWidth;
    Cardinal		maxCursorHeight;

    Cursor		menuCursor;
    unsigned char	unpostBehavior;
    XFontStruct *	font_struct;
    int			h_unit;
    int			v_unit;
    XmScratchPixmap	scratchPixmaps;
    unsigned char     moveOpaque;

    /* to save internally-created XmDragIcons */

    XmDragIconObject	xmStateCursorIcon;
    XmDragIconObject	xmMoveCursorIcon;
    XmDragIconObject	xmCopyCursorIcon;
    XmDragIconObject	xmLinkCursorIcon;
    XmDragIconObject	xmSourceCursorIcon;

    GC			imageGC;		/* ImageCache.c */
    int			imageGCDepth;
    Pixel		imageForeground;
    Pixel		imageBackground;

    XtPointer		screenInfo;		/* extension */
} XmScreenPart, *XmScreenPartPtr;

typedef struct _XmScreenInfo {
	/* so much for information hiding */
	XtPointer	menu_state;		/* MenuUtil.c */
	Boolean		destroyCallbackAdded;	/* ImageCache.c */
} XmScreenInfo;

externalref XmScreenClassRec 	xmScreenClassRec;

typedef struct _XmScreenRec{
    CorePart			core;
    XmDesktopPart		desktop;
    XmScreenPart		screen;
}XmScreenRec;

externalref XrmQuark _XmInvalidCursorIconQuark ;
externalref XrmQuark _XmValidCursorIconQuark ;
externalref XrmQuark _XmNoneCursorIconQuark ;
externalref XrmQuark _XmDefaultDragIconQuark ;
externalref XrmQuark _XmMoveCursorIconQuark ;
externalref XrmQuark _XmCopyCursorIconQuark ;
externalref XrmQuark _XmLinkCursorIconQuark ;


/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern XmDragIconObject _XmScreenGetOperationIcon() ;
extern XmDragIconObject _XmScreenGetStateIcon() ;
extern XmDragIconObject _XmScreenGetSourceIcon() ;
extern Pixmap _XmAllocScratchPixmap() ;
extern void _XmFreeScratchPixmap() ;
extern XmDragCursorCache * _XmGetDragCursorCachePtr() ;
extern void _XmGetMaxCursorSize() ;
extern Cursor _XmGetNullCursor() ;
extern Cursor _XmGetMenuCursorByScreen() ;
extern Boolean _XmGetMoveOpaqueByScreen() ;
extern unsigned char _XmGetUnpostBehavior() ;
extern int _XmGetFontUnit() ;
extern void _XmScreenRemoveFromCursorCache() ;

#else

extern XmDragIconObject _XmScreenGetOperationIcon( 
                        Widget w,
#if NeedWidePrototypes
                        unsigned int operation) ;
#else
                        unsigned char operation) ;
#endif /* NeedWidePrototypes */
extern XmDragIconObject _XmScreenGetStateIcon( 
                        Widget w,
#if NeedWidePrototypes
                        unsigned int state) ;
#else
                        unsigned char state) ;
#endif /* NeedWidePrototypes */
extern XmDragIconObject _XmScreenGetSourceIcon( 
                        Widget w) ;
extern Pixmap _XmAllocScratchPixmap( 
                        XmScreen xmScreen,
#if NeedWidePrototypes
                        unsigned int depth,
                        int width,
                        int height) ;
#else
                        Cardinal depth,
                        Dimension width,
                        Dimension height) ;
#endif /* NeedWidePrototypes */
extern void _XmFreeScratchPixmap( 
                        XmScreen xmScreen,
                        Pixmap pixmap) ;
extern XmDragCursorCache * _XmGetDragCursorCachePtr( 
                        XmScreen xmScreen) ;
extern void _XmGetMaxCursorSize( 
                        Widget w,
                        Dimension *width,
                        Dimension *height) ;
extern Cursor _XmGetNullCursor( 
                        Widget w) ;
extern Cursor _XmGetMenuCursorByScreen( 
                        Screen *screen) ;
extern Boolean _XmGetMoveOpaqueByScreen( 
                        Screen *screen) ;
extern unsigned char _XmGetUnpostBehavior( 
                        Widget wid) ;
extern int _XmGetFontUnit( 
                        Screen *screen,
                        int dimension) ;
extern void _XmScreenRemoveFromCursorCache(
			XmDragIconObject icon) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* MOTIF12_HEADERS */

#endif /* _XmScreenP_h */
/* DON'T ADD STUFF AFTER THIS #endif */
