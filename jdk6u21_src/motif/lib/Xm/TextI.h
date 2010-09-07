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
/* $XConsortium: TextI.h /main/6 1996/05/29 13:45:16 pascale $ */
#ifndef _XmTextI_h
#define _XmTextI_h

#include <Xm/TextP.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations    ********/

extern XmTextPosition _XmTextFindScroll(XmTextWidget widget,
					XmTextPosition start,
					int delta);
extern int _XmTextGetTotalLines(Widget widget);
extern XmTextLineTable _XmTextGetLineTable(Widget widget,
					   int *total_lines);
extern void _XmTextSetUrlHighlightsFromList(Widget w,
					    UrlHighlightRec *urlHighlightList,
					    int size);
extern void _XmTextRealignLineTable(XmTextWidget widget,
				    XmTextLineTable *temp_table,
				    int *temp_table_size,
				    register unsigned int cur_index,
				    register XmTextPosition cur_start,
				    register XmTextPosition cur_end);
extern unsigned int _XmTextGetTableIndex(XmTextWidget widget,
					 XmTextPosition pos);
extern void _XmTextUpdateLineTable(Widget widget,
				   XmTextPosition start,
				   XmTextPosition end,
				   XmTextBlock block,
#if NeedWidePrototypes
				   int update
#else
				   Boolean update
#endif /* NeedWidePrototypes */
				   );
extern void _XmTextMarkRedraw(XmTextWidget widget,
			      XmTextPosition left,
			      XmTextPosition right);
extern LineNum _XmTextNumLines(XmTextWidget widget);
extern void _XmTextLineInfo(XmTextWidget widget,
			    LineNum line,
			    XmTextPosition *startpos,
			    LineTableExtra *extra);
extern LineNum _XmTextPosToLine(XmTextWidget widget,
				XmTextPosition position);
extern void _XmTextInvalidate(XmTextWidget widget,
			      XmTextPosition position,
			      XmTextPosition topos,
			      long delta);
extern void _XmTextSetTopCharacter(Widget widget,
				   XmTextPosition top_character);
extern size_t TextCountCharacters( Widget wid, /* Bug Id : 1217687/4128045/4154215 */
				   char *str,
				   size_t num_count_bytes);
extern size_t _XmTextCountCharacters( char *str, /* Wyoming 64-bit fix */ 
				     size_t num_count_bytes); /* Wyoming 64-bit fix */ 
extern Boolean _XmTextCheckErrorStatus(Widget w);
extern void _XmTextSetCursorPosition(Widget widget,
				     XmTextPosition position);
extern void _XmTextDisableRedisplay(XmTextWidget widget,
#if NeedWidePrototypes
				    int losesbackingstore);
#else
                                    Boolean losesbackingstore);
#endif /* NeedWidePrototypes */
extern void _XmTextEnableRedisplay(XmTextWidget widget);  

extern void _XmTextSetHighlight(Widget, XmTextPosition,
                                XmTextPosition, XmHighlightMode);
extern void _XmTextShowPosition(Widget, XmTextPosition);
extern void _XmTextSetEditable(Widget, Boolean, Boolean);
extern void _XmTextResetIC(Widget widget);
extern Boolean _XmTextNeedsPendingDeleteDis(XmTextWidget tw,
                                            XmTextPosition *left,
                                            XmTextPosition *right,
                                            int check_add_mode);
extern void _XmTextReplace(Widget widget,
                           XmTextPosition frompos,
                           XmTextPosition topos,
	                   char *value, 
#if NeedWidePrototypes
                           int is_wchar);
#else
                           Boolean is_wchar);
#endif /* NeedWidePrototypes */
extern void _XmTextValidate(XmTextPosition *start,
		            XmTextPosition *end,
		            long maxsize); /* Wyoming 64-bit fix */ 


/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmTextI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
