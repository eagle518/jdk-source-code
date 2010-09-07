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
/* $XConsortium: LabelGI.h /main/5 1995/07/13 17:31:31 drk $ */
#ifndef _XMLABELGI_H
#define _XMLABELGI_H

#include <Xm/LabelGP.h>
#include <Xm/MenuT.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations    ********/

extern int _XmLabelCacheCompare( 
                        XtPointer A,
                        XtPointer B) ;
extern void _XmCalcLabelGDimensions( 
                        Widget wid) ;
extern void _XmReCacheLabG( 
                        Widget wid) ;
extern void _XmAssignLabG_MarginHeight( 
                        XmLabelGadget lw,
#if NeedWidePrototypes
                        int value) ;
#else
                        Dimension value) ;
#endif /* NeedWidePrototypes */
extern void _XmAssignLabG_MarginWidth( 
                        XmLabelGadget lw,
#if NeedWidePrototypes
                        int value) ;
#else
                        Dimension value) ;
#endif /* NeedWidePrototypes */
extern void _XmAssignLabG_MarginLeft( 
                        XmLabelGadget lw,
#if NeedWidePrototypes
                        int value) ;
#else
                        Dimension value) ;
#endif /* NeedWidePrototypes */
extern void _XmAssignLabG_MarginRight( 
                        XmLabelGadget lw,
#if NeedWidePrototypes
                        int value) ;
#else
                        Dimension value) ;
#endif /* NeedWidePrototypes */
extern void _XmAssignLabG_MarginTop( 
                        XmLabelGadget lw,
#if NeedWidePrototypes
                        int value) ;
#else
                        Dimension value) ;
#endif /* NeedWidePrototypes */
extern void _XmAssignLabG_MarginBottom( 
                        XmLabelGadget lw,
#if NeedWidePrototypes
                        int value) ;
#else
                        Dimension value) ;
#endif /* NeedWidePrototypes */
extern void _XmProcessDrag( 
                        Widget w,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern Boolean _XmLabelGCVTRedraw (Widget kid, 
				   Widget cur_parent,
				   Widget new_parent,
				   Mask visual_flag);

extern void _XmRedisplayLabG (Widget      w,
			      XEvent     *event,
			      Region      region,
			      LRectangle *background_box);

extern void _XmLabelGCloneMenuSavvy(WidgetClass, XmMenuSavvyTrait);

extern void _XmLabelSetBackgroundGC(XmLabelGadget lw);
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XMLABELGI_H */
/* DON'T ADD ANYTHING AFTER THIS #endif */
