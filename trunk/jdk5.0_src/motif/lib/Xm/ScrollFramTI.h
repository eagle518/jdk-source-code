/* $XConsortium: ScrollFramTI.h /main/5 1995/07/15 20:55:16 drk $ */
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
#ifndef _XmScrollFrameTI_h
#define _XmScrollFrameTI_h

#include <Xm/Xm.h>
#include <Xm/ScrollFrameT.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations    ********/

extern void _XmSFAddNavigator(
			      Widget sf,
			      Widget nav,
			      Mask dimMask,
			      XmScrollFrameData scroll_frame_data);
extern void _XmSFRemoveNavigator(
				 Widget sf,
				 Widget nav,
				 XmScrollFrameData scroll_frame_data);
extern void _XmSFUpdateNavigatorsValue(
				       Widget sf,
				       XmNavigatorData nav_data,
				       Boolean notify);

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmScrollFrameTI_h */
