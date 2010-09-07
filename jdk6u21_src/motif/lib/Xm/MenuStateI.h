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
/* $XConsortium: MenuStateI.h /main/5 1995/07/13 17:36:46 drk $ */
#ifndef _XmMenuStateI_h
#define _XmMenuStateI_h

#include <Xm/MenuStateP.h>

#ifdef __cplusplus
extern "C" {
#endif


/********    Private Function Declarations    ********/

extern Widget _XmGetRC_PopupPosted (
                         Widget wid) ;
extern Boolean _XmGetInDragMode( 
                        Widget widget) ;
extern void _XmSetInDragMode( 
                        Widget widget,
#if NeedWidePrototypes
                        int mode) ;
#else
                        Boolean mode) ;
#endif /* NeedWidePrototypes */

extern XmMenuState _XmGetMenuState(
                        Widget widget) ;
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmMenuStateI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
