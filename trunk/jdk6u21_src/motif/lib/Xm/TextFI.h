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
/* $XConsortium: TextFI.h /main/5 1995/07/13 18:05:32 drk $ */
#ifndef _XmTextFI_h
#define _XmTextFI_h

#include <Xm/TextFP.h>

#ifdef __cplusplus
extern "C" {
#endif


/********    Private Function Declarations    ********/

#ifdef SUN_CTL
extern int _XmTextFieldFindPixelPosition(
			XmTextFieldWidget tf,
			char 		  *string,
			XmTextPosition	  length,
			XmEDGE		  edge );
#endif /* CTL */

extern int _XmTextFieldCountBytes( 
                        XmTextFieldWidget tf,
                        wchar_t *wc_value,
                        int num_chars) ;
extern void _XmTextFToggleCursorGC( 
                        Widget widget) ;
extern void _XmTextFieldDrawInsertionPoint( 
                        XmTextFieldWidget tf,
#if NeedWidePrototypes
                        int turn_on) ;
#else
                        Boolean turn_on) ;
#endif /* NeedWidePrototypes */
extern void _XmTextFieldSetClipRect( 
                        XmTextFieldWidget tf) ;
extern void _XmTextFieldSetCursorPosition( 
                        XmTextFieldWidget tf,
                        XEvent *event,
                        XmTextPosition position,
#if NeedWidePrototypes
                        int adjust_flag,
                        int call_cb) ;
#else
                        Boolean adjust_flag,
                        Boolean call_cb) ;
#endif /* NeedWidePrototypes */
extern Boolean _XmTextFieldReplaceText( 
                        XmTextFieldWidget tf,
                        XEvent *event,
                        XmTextPosition replace_prev,
                        XmTextPosition replace_next,
                        char *insert,
                        long insert_length, /* Wyoming 64-bit fix */
#if NeedWidePrototypes
                        int move_cursor) ;
#else
                        Boolean move_cursor) ;
#endif /* NeedWidePrototypes */
extern void _XmTextFieldDeselectSelection( 
                        Widget w,
#if NeedWidePrototypes
                        int disown,
#else
                        Boolean disown,
#endif /* NeedWidePrototypes */
                        Time sel_time) ;
extern Boolean _XmTextFieldSetDestination( 
                        Widget w,
                        XmTextPosition position,
                        Time set_time) ;
extern void _XmTextFieldStartSelection( 
                        XmTextFieldWidget tf,
                        XmTextPosition left,
                        XmTextPosition right,
                        Time sel_time) ;
extern void _XmTextFieldSetSel2( 
                        Widget w,
                        XmTextPosition left,
                        XmTextPosition right,
#if NeedWidePrototypes
                        int disown,
#else
                        Boolean disown,
#endif /* NeedWidePrototypes */
                        Time sel_time) ;
extern void _XmTextFieldHandleSecondaryFinished(Widget w,
						XEvent *event);
extern int _XmTextFieldCountCharacters(XmTextFieldWidget tf,
				       char *ptr,
				       int n_bytes);

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmTextFI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
