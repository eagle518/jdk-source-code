/* $XConsortium: XmIm.h /main/7 1996/05/21 12:13:36 pascale $ */
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
#ifndef _XmIm_h
#define _XmIm_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Public Function Declarations    ********/

extern void XmImRegister( 
                        Widget w,
			unsigned int reserved) ;
extern void XmImUnregister( 
                        Widget w) ;
extern void XmImSetFocusValues( 
                        Widget w,
                        ArgList args,
                        Cardinal num_args) ;
extern void XmImSetValues( 
                        Widget w,
                        ArgList args,
                        Cardinal num_args) ;
extern void XmImUnsetFocus( 
                        Widget w) ;
extern XIM XmImGetXIM( 
                        Widget w) ;
extern void XmImCloseXIM(
                        Widget w) ;

extern int XmImMbLookupString( 
                        Widget w,
                        XKeyPressedEvent *event,
                        char *buf,
                        int nbytes,
                        KeySym *keysym,
                        int *status) ;
extern void XmImVaSetFocusValues( 
                        Widget w,
                        ...) ;
extern void XmImVaSetValues( 
                        Widget w,
                        ...) ;
extern XIC XmImGetXIC(
		        Widget 		w,
#if NeedWidePrototypes
		        unsigned int 	input_policy,
#else
		        XmInputPolicy	input_policy,
#endif /*NeedWidePrototypes*/
		        ArgList		args,
		        Cardinal	num_args) ;
extern XIC XmImSetXIC(
			Widget w,
			XIC    input_context) ;
extern void XmImFreeXIC(
			Widget w,
			XIC    input_context) ;

extern void XmImMbResetIC(
			Widget w,
			char **mb);

/********    End Public Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmIm_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
