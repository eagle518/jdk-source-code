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
/* $XConsortium: ScreenI.h /main/5 1995/07/13 17:53:44 drk $ */
#ifndef _XmScreenI_h
#define _XmScreenI_h

#include <Xm/ScreenP.h>

#ifdef __cplusplus
extern "C" {
#endif


externalref XrmQuark _XmInvalidCursorIconQuark ;
externalref XrmQuark _XmValidCursorIconQuark ;
externalref XrmQuark _XmNoneCursorIconQuark ;
externalref XrmQuark _XmDefaultDragIconQuark ;
externalref XrmQuark _XmMoveCursorIconQuark ;
externalref XrmQuark _XmCopyCursorIconQuark ;
externalref XrmQuark _XmLinkCursorIconQuark ;


/********    Private Function Declarations    ********/

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
extern XmScreenColorProc _XmGetColorCalculationProc( 
                        Screen *screen) ;
extern XmAllocColorProc _XmGetColorAllocationProc(
			Screen *screen) ;
extern Pixmap _XmGetInsensitiveStippleBitmap(
			Widget w) ;
extern XtEnum _XmGetBitmapConversionModel(
			Screen *screen) ;

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmScreenI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
