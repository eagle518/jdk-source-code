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
/* $XConsortium: DragIconI.h /main/6 1995/07/14 10:25:29 drk $ */
#ifndef _XmDragIconI_h
#define _XmDragIconI_h

#include <Xm/DragIconP.h>

#ifdef __cplusplus
extern "C" {
#endif


/********    Private Function Declarations    ********/

extern void _XmDestroyDefaultDragIcon(XmDragIconObject icon) ;
extern Boolean _XmDragIconIsDirty(XmDragIconObject icon) ;
extern void _XmDragIconClean(XmDragIconObject icon1,
			     XmDragIconObject icon2,
			     XmDragIconObject icon3) ;

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDragIconI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
