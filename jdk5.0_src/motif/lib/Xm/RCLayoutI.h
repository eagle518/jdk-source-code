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
/* $XConsortium: RCLayoutI.h /main/5 1995/07/13 17:44:58 drk $ */
#ifndef _XmRCLayoutI_h
#define _XmRCLayoutI_h

#include <Xm/RCLayoutP.h>

#ifdef __cplusplus
extern "C" {
#endif


/********    Private Function Declarations    ********/

extern void _XmRCDoMarginAdjustment( 
                        XmRowColumnWidget m) ;
extern void _XmRCThinkAboutSize( 
                        register XmRowColumnWidget m,
                        Dimension *w,
                        Dimension *h,
                        Widget instigator,
                        XtWidgetGeometry *request) ;
extern void _XmRCPreferredSize( 
                        XmRowColumnWidget m,
                        Dimension *w,
                        Dimension *h) ;
extern void _XmRCAdaptToSize( 
                        XmRowColumnWidget m,
                        Widget instigator,
                        XtWidgetGeometry *request) ;
extern XmRCKidGeometry _XmRCGetKidGeo( 
                        Widget wid,
                        Widget instigator,
                        XtWidgetGeometry *request,
                        int uniform_border,
#if NeedWidePrototypes
                        int border,
#else
                        Dimension border,
#endif /* NeedWidePrototypes */
                        int uniform_width_margins,
                        int uniform_height_margins,
                        Widget help,
			Widget toc,
                        int geo_type) ;
extern void _XmRCSetKidGeo( 
                        XmRCKidGeometry kg,
                        Widget instigator) ;

extern void _XmRC_SetOrGetTextMargins( 
                        Widget wid,
#if NeedWidePrototypes
                        unsigned int op,
#else
                        unsigned char op,
#endif /* NeedWidePrototypes */
                        XmBaselineMargins *textMargins) ;

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmRCLayoutI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
