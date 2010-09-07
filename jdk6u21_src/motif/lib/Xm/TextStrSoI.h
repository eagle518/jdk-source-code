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
/* $XConsortium: TextStrSoI.h /main/5 1995/07/13 18:10:59 drk $ */
/* (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmTextStrSoI_h
#define _XmTextStrSoI_h

#include <Xm/TextStrSoP.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations    ********/

extern char  * _XmStringSourceGetString(XmTextWidget tw,
				        XmTextPosition from,
				        XmTextPosition to,
#if NeedWidePrototypes
				        int want_wchar);
#else
                                        Boolean want_wchar);
#endif /* NeedWidePrototypes */
extern Boolean _XmTextFindStringBackwards(Widget w,
					  XmTextPosition start,
					  char *search_string,
					  XmTextPosition *position);
extern Boolean _XmTextFindStringForwards(Widget w,
					 XmTextPosition start,
					 char *search_string,
					 XmTextPosition *position);
extern void    _XmStringSourceSetGappedBuffer(XmSourceData data,
					      XmTextPosition position);
extern Boolean _XmTextModifyVerify(XmTextWidget initiator,
				   XEvent *event,
				   XmTextPosition *start,
				   XmTextPosition *end,
				   XmTextPosition *cursorPos,
				   XmTextBlock block,
				   XmTextBlock newblock,
				   Boolean *freeBlock);
extern XmTextSource StringSourceCreate(Widget wid, /* Bug Id : 1217687/4128045/4154215 */
				       char *value,
#if NeedWidePrototypes
				       int is_wchar);
#else
                                       Boolean is_wchar);
#endif /* NeedWidePrototypes */
extern XmTextSource _XmStringSourceCreate(char *value,
#if NeedWidePrototypes
					  int is_wchar);
#else
                                          Boolean is_wchar);
#endif /* NeedWidePrototypes */
extern void    _XmStringSourceDestroy(XmTextSource source);
extern char  * _XmStringSourceGetValue(XmTextSource source,
#if NeedWidePrototypes
				       int want_wchar);
#else
                                       Boolean want_wchar);
#endif /* NeedWidePrototypes */
extern void    _XmStringSourceSetValue(XmTextWidget widget,
				       char *value);
extern Boolean _XmStringSourceHasSelection(XmTextSource source);
extern Boolean _XmStringSourceGetEditable(XmTextSource source);
extern void    _XmStringSourceSetEditable(XmTextSource source,
#if NeedWidePrototypes
					  int editable);
#else
                                         Boolean editable);
#endif /* NeedWidePrototypes */
extern int     _XmStringSourceGetMaxLength(XmTextSource source);
extern void    _XmStringSourceSetMaxLength(XmTextSource source,
					   int max);
extern int _XmTextBytesToCharacters(char *characters,
				    char *bytes,
				    long num_chars, /* Wyoming 64-bit fix */ 
#if NeedWidePrototypes
				    int add_null_terminator,
#else
				    Boolean add_null_terminator,
#endif /* NeedWidePrototypes */
				    int max_char_size);
extern int _XmTextCharactersToBytes(char *bytes,
				    char *characters,
				    long num_chars, /* Wyoming 64-bit fix */ 
				    long max_char_size); /* Wyoming 64-bit fix */ 
extern void    _XmTextValueChanged(XmTextWidget initiator,
				   XEvent *event);
extern Boolean *_XmStringSourceGetPending(XmTextWidget widget);
extern void    _XmStringSourceSetPending(XmTextWidget widget,
					 Boolean *pending);
#ifdef SUN_CTL
extern Boolean
_XmCTLGetLine(XmTextWidget	tw,
		int		pos,
		XmTextPosition	*line_start,
		XmTextPosition	*next_line_start,
		char		**text,
		int		*text_len);
extern XmTextPosition
_XmTextVisualConstScan(XmTextSource	source,
		   XmTextPosition pos,
		   int		posType);
extern XmTextPosition 
_XmTextVisualScan(XmTextSource	source,
	      XmTextPosition	pos,
	      XmTextScanType	sType,
	      XmTextScanDirection dir,
	      int 		count,
#if NeedWidePrototypes
	      int 		include);
#else 
	      Boolean 		include);
#endif /* NeedWidePrototypes */
#endif /* CTL */

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /*  _XmTextStrSoI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
