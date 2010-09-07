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
/* $XConsortium: DisplayI.h /main/9 1995/07/14 10:20:35 drk $ */
/* (c) Copyright 1989, 1990  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/* (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
/* (c) Copyright 1988 MICROSOFT CORPORATION */
#ifndef _XmDisplayI_h
#define _XmDisplayI_h

#include <Xm/DisplayP.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _XmDisplayEventQueryStruct {
  XmDisplay			dd;
  XmDragContext			dc;
  XmTopLevelEnterCallbackStruct	*enterCB;
  XmDragMotionCallbackStruct	*motionCB;
  XmTopLevelLeaveCallbackStruct	*leaveCB;
  XmDropStartCallbackStruct	*dropStartCB;
  Boolean			hasEnter;
  Boolean			hasMotion;
  Boolean			hasLeave;
  Boolean			hasDropStart;
} XmDisplayEventQueryStruct;


/********    Private Function Declarations    ********/

extern XmDropSiteManagerObject _XmGetDropSiteManagerObject( 
                        XmDisplay xmDisplay) ;
extern unsigned char _XmGetDragProtocolStyle( 
                        Widget w) ;
extern unsigned char _XmGetDragTrackingMode( 
                        Widget w) ;
extern Widget _XmGetDragContextFromHandle( 
                        Widget w,
                        Atom iccHandle) ;
extern WidgetClass _XmGetXmDisplayClass( void ) ;
extern WidgetClass _XmSetXmDisplayClass( 
                        WidgetClass wc) ;

/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDisplayI_h */
/* DON'T ADD STUFF AFTER THIS #endif */

