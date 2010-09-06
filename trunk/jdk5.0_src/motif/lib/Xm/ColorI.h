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
/* $XConsortium: ColorI.h /main/6 1995/07/14 10:15:47 drk $ */
#ifndef _XmColorI_h
#define _XmColorI_h

#include <Xm/ColorP.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations for Color.c    ********/

extern void _XmForegroundColorDefault( 
                        Widget widget,
                        int offset,
                        XrmValue *value) ;
extern void _XmHighlightColorDefault( 
                        Widget widget,
                        int offset,
                        XrmValue *value) ;
extern void _XmBackgroundColorDefault( 
                        Widget widget,
                        int offset,
                        XrmValue *value) ;
extern void _XmTopShadowColorDefault( 
                        Widget widget,
                        int offset,
                        XrmValue *value) ;
extern void _XmBottomShadowColorDefault( 
                        Widget widget,
                        int offset,
                        XrmValue *value) ;
extern void _XmSelectColorDefault( 
                        Widget widget,
                        int offset,
                        XrmValue *value) ;
extern Boolean _XmSearchColorCache(
                        unsigned int which,
                        XmColorData *values,
                        XmColorData **ret) ;
extern XmColorData * _XmAddToColorCache( 
                        XmColorData *new_rec) ;
/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmColorI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
