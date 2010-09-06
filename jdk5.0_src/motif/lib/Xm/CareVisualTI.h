/* $XConsortium: CareVisualTI.h /main/5 1995/07/15 20:48:25 drk $ */
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
#ifndef _XmCareVisualTI_h
#define _XmCareVisualTI_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations    ********/

extern Boolean _XmNotifyChildrenVisual( 
				       Widget cur,
				       Widget new_w,
				       Mask visual_flag) ;

/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmCareVisualTI_h */
