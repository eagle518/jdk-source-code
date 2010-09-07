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
/* $XConsortium: PixConvI.h /main/6 1996/01/29 10:08:12 daniel $ */
#ifndef _XmPixConvI_h
#define _XmPixConvI_h

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations for PixConv.c    ********/

extern void _XmRegisterPixmapConverters( void ) ;
extern void _XmTopShadowPixmapDefault( 
                        Widget widget,
                        int offset,
                        XrmValue *value) ;
extern void _XmHighlightPixmapDefault( 
                        Widget widget,
                        int offset,
                        XrmValue *value) ;
extern GC _XmGetPixmapBasedGC(
			      Widget w,
			      Pixel foreground,
			      Pixel background,
			      Pixmap pixmap);

/********    End Private Function Declarations    ********/


/* tmp - go to XmStrDef */
#define XmRNoScalingBitmap "NoScalingBitmap"
#define XmRNoScalingDynamicPixmap "NoScalingDynamicPixmap"



#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmPixConvI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
