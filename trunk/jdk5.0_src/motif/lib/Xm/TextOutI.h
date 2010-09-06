/* * COPYRIGHT NOTICE
 * Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 * ALL RIGHTS RESERVED (MOTIF).  See the file named COPYRIGHT.MOTIF
 * for the full copyright text.
 * 
 */
/*
 *  (c) Copyright 1995 FUJITSU LIMITED
 *  This is source code modified by FUJITSU LIMITED under the Joint
 *  Development Agreement for the CDEnext PST.
 *  This is unpublished proprietary source code of FUJITSU LIMITED
 */
/*
 * HISTORY
 */
/* $XConsortium: TextOutI.h /main/7 1995/11/02 12:05:38 cde-fuj $ */
#ifndef _XmTextOutI_h
#define _XmTextOutI_h

#include <Xm/TextOutP.h>

#ifdef __cplusplus
extern "C" {
#endif


/********    Private Function Declarations    ********/

extern void _XmTextFreeContextData( 
                        Widget w,
                        XtPointer clientData,
                        XtPointer callData) ;
extern void _XmTextResetClipOrigin(XmTextWidget, XmTextPosition, Boolean);
extern void _XmTextAdjustGC(XmTextWidget);
extern Boolean _XmTextShouldWordWrap( 
                        XmTextWidget widget) ;
extern Boolean _XmTextScrollable( 
                        XmTextWidget widget) ;
extern XmTextPosition _XmTextFindLineEnd( 
                        XmTextWidget widget,
                        XmTextPosition position,
                        LineTableExtra *extra) ;
extern void _XmTextOutputGetSecResData( 
                        XmSecondaryResourceData *secResDataRtn) ;
extern int _XmTextGetNumberLines( 
                        XmTextWidget widget) ;
extern void _XmTextMovingCursorPosition( 
                        XmTextWidget tw,
                        XmTextPosition position) ;
extern void _XmTextChangeBlinkBehavior(XmTextWidget, Boolean);
extern void _XmTextOutputCreate( 
                        Widget wid,
                        ArgList args,
                        Cardinal num_args) ;
extern Boolean _XmTextGetBaselines( 
                        Widget widget,
                        Dimension **baselines,
                        int *line_count) ;
extern Boolean _XmTextGetDisplayRect( 
                        Widget w,
                        XRectangle *display_rect) ;
extern void _XmTextMarginsProc( 
                        Widget w,
                        XmBaselineMargins *margins_rec) ;
extern void _XmTextChangeHOffset( 
                        XmTextWidget widget,
                        int length) ;
extern void _XmTextChangeVOffset( 
                        XmTextWidget widget,
                        int length) ;
extern void _XmTextToggleCursorGC( 
                        Widget widget) ;

#ifdef SUN_CTL
extern int _XmTextPosSegment(
			XmTextWidget    tw, 
			XmTextPosition  pos, 
			XSegment	*char_segment);
#endif /* CTL */


/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmTextOutI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
