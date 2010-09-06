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
/*   $XConsortium: TravActI.h /main/9 1995/07/13 18:15:46 drk $ */
/*
*  (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmTravActI_h
#define _XmTravActI_h

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XmFOCUS_RESET	1<<0
#define XmFOCUS_IGNORE	1<<1

/********    Private Function Declarations    ********/

extern unsigned short _XmGetFocusFlag(
			Widget w,
			unsigned int mask) ;
extern void _XmSetFocusFlag(
			Widget w,
			unsigned int mask,
#if NeedWidePrototypes
        		int value ) ;
#else
        		Boolean value ) ;
#endif /* NeedWidePrototypes */
extern void _XmTrackShellFocus( 
                        Widget widget,
                        XtPointer client_data,
                        XEvent *event,
                        Boolean *dontSwallow) ;
extern void _XmPrimitiveEnter( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmPrimitiveLeave( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmPrimitiveUnmap( 
                        Widget pw,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmPrimitiveFocusInInternal( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmPrimitiveFocusOut( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmPrimitiveFocusIn( 
                        Widget pw,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmEnterGadget( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmLeaveGadget( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmFocusInGadget( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmFocusOutGadget( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmManagerEnter( 
                        Widget wid,
                        XEvent *event_in,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmManagerLeave( 
                        Widget wid,
                        XEvent *event_in,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmManagerFocusInInternal( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmManagerFocusIn( 
                        Widget mw,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmManagerFocusOut( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
extern void _XmManagerUnmap( 
                        Widget mw,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmTravActI_h */
