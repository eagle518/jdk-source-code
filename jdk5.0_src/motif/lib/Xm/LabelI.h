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
/* $XConsortium: LabelI.h /main/5 1995/07/13 17:31:52 drk $ */
#ifndef _XmLabelI_h
#define _XmLabelI_h

#include <Xm/LabelP.h>
#include <Xm/MenuT.h>

#ifdef __cplusplus
extern "C" {
#endif


/********    Private Function Declarations    ********/

extern void _XmCalcLabelDimensions(Widget wid) ;
extern void _XmLabelCloneMenuSavvy(WidgetClass, XmMenuSavvyTrait);
extern char* _XmCBNameActivate(void);
extern char* _XmCBNameValueChanged(void);


/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmLabelI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
