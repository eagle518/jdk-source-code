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
/*   $XConsortium: GMUtilsI.h /main/9 1995/07/13 17:26:45 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmGMUtilsI_h
#define _XmGMUtilsI_h


/* Include files:
*/
#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations    ********/

extern void _XmGMCalcSize( 
                        XmManagerWidget manager,
#if NeedWidePrototypes
                        int margin_width,
                        int margin_height,
#else
                        Dimension margin_width,
                        Dimension margin_height,
#endif /* NeedWidePrototypes */
                        Dimension *replyWidth,
                        Dimension *replyHeight) ;
extern Boolean _XmGMDoLayout( 
                        XmManagerWidget manager,
#if NeedWidePrototypes
                        int margin_width,
                        int margin_height,
#else
                        Dimension margin_width,
                        Dimension margin_height,
#endif /* NeedWidePrototypes */
                        int resize_policy,
#if NeedWidePrototypes
                        int queryonly) ;
#else
                        Boolean queryonly) ;
#endif /* NeedWidePrototypes */
extern void _XmGMEnforceMargin( 
                        XmManagerWidget manager,
#if NeedWidePrototypes
                        int margin_width,
                        int margin_height,
                        int setvalue) ;
#else
                        Dimension margin_width,
                        Dimension margin_height,
                        Boolean setvalue) ;
#endif /* NeedWidePrototypes */
extern XtGeometryResult _XmGMHandleQueryGeometry( 
                        Widget widget,
                        XtWidgetGeometry *intended,
                        XtWidgetGeometry *desired,
#if NeedWidePrototypes
                        int margin_width,
                        int margin_height,
#else
                        Dimension margin_width,
                        Dimension margin_height,
#endif /* NeedWidePrototypes */
                        int resize_policy) ;
extern Boolean _XmGMOverlap( 
                        XmManagerWidget manager,
                        Widget w) ;
extern XtGeometryResult _XmGMHandleGeometryManager( 
                        Widget parent,
                        Widget w,
                        XtWidgetGeometry *request,
                        XtWidgetGeometry *reply,
#if NeedWidePrototypes
                        int margin_width,
                        int margin_height,
#else
                        Dimension margin_width,
                        Dimension margin_height,
#endif /* NeedWidePrototypes */
                        int resize_policy,
                        int allow_overlap) ;
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmGMUtilsI_h */
 /* DON'T ADD STUFF AFTER THIS #endif */
