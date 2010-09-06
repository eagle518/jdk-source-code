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
/*   $XConsortium: XmRenderTI.h /main/5 1995/07/13 18:24:12 drk $ */
#ifndef _XmRenderTI_h
#define _XmRenderTI_h

#include <Xm/XmP.h>

#ifdef SUN_CTL
#include <sys/layout.h>
#include <Xm/XmXOC.h>
#endif /* CTL */
#include "TextP.h"
#ifdef SUN_TBR
#include "XmTBR.h"
#endif /*SUN_TBR */

#ifdef __cplusplus
extern "C" {
#endif


/*
 * Macros for Rendition data structure access
 */

#define _XmRendRefcount(r)	((_XmRendition)*(r))->refcount
#define _XmRendFontOnly(r)	((_XmRendition)*(r))->fontOnly
#define _XmRendLoadModel(r)	((_XmRendition)*(r))->loadModel
#define _XmRendTag(r)		((_XmRendition)*(r))->tag
#define _XmRendFontName(r)	((_XmRendition)*(r))->fontName
#define _XmRendFontType(r)	((_XmRendition)*(r))->fontType
#define _XmRendFont(r)		((_XmRendition)*(r))->font
#ifdef SUN_CTL
#define _XmRendLayoutActive(r)	((XmXOC)((_XmRendition)*(r))->font)->layout_active
#define _XmRendLayoutIsCTL(r)	((_XmRendition)*(r))->layoutIsCtl
#define _XmRendLayoutModifier(r)   ((_XmRendition)*(r))->layoutModifier
#define _XmRendXOC(r)		((XmXOC)((_XmRendition)*(r))->font)->xoc
#endif /* CTL */
#ifdef SUN_TBR
#define _XmRendIsTBR(r)	        ((((_XmRendition)*(r))->XmTBRObject) != NULL ? True: False)
#define _XmRendTBR(r)	        ((_XmRendition)*(r))->XmTBRObject
#endif /*SUN_TBR*/
#define _XmRendDisplay(r)	((_XmRendition)*(r))->display
#define _XmRendTabs(r)		((_XmRendition)*(r))->tabs
#define _XmRendBG(r)		((_XmRendition)*(r))->background
#define _XmRendFG(r)		((_XmRendition)*(r))->foreground
#define _XmRendBGState(r)	((_XmRendition)*(r))->backgroundState
#define _XmRendFGState(r)	((_XmRendition)*(r))->foregroundState
#define _XmRendUnderlineType(r)	((_XmRendition)*(r))->underlineType
#define _XmRendStrikethruType(r)((_XmRendition)*(r))->strikethruType
#define _XmRendGC(r)		((_XmRendition)*(r))->gc
#define _XmRendTags(r)		((_XmRendition)*(r))->tags
#define _XmRendTagCount(r)	((_XmRendition)*(r))->count
#define _XmRendHadEnds(r)	((_XmRendition)*(r))->hadEnds
#define _XmRendRefcountInc(r)	++(((_XmRendition)*(r))->refcount)
#define _XmRendRefcountDec(r)	--(((_XmRendition)*(r))->refcount)

typedef struct __XmRenditionRec
{
	/* flag indicating _XmFontRenditionRec */
	u_short			fontOnly;
	u_short			refcount;

	u_char			loadModel;
	XmStringTag		tag;
	String			fontName;
	XmFontType		fontType;
	XtPointer		font;
	Display *		display;
	GC					gc;
	XmStringTag *	tags;
	unsigned int	count;
	Boolean			hadEnds;

	XmTabList		tabs;
	Pixel				background;
	Pixel				foreground;
	u_char			underlineType;
	u_char			strikethruType;
	u_char			backgroundState;
	u_char			foregroundState;
#ifdef SUN_CTL
	Boolean			layoutIsCtl;
	String			layoutModifier;
#endif /* CTL */
#ifdef SUN_TBR
       XtPointer		XmTBRObject;
#endif /*SUN_TBR*/   
} _XmRenditionRec, *_XmRendition;

typedef struct __XmFontRenditionRec
{
	/* flag indicating _XmFontRenditionRec */
	u_short			fontOnly;
	u_short			refcount;

	u_char			loadModel;
	XmStringTag		tag;
	String			fontName;
	XmFontType		fontType;
	XtPointer		font;
	Display *		display;
	GC *				gc;
	XmStringTag *	tags;
	unsigned int	count;
} _XmFontRenditionRec, *_XmFontRendition;

/* Accessor macros. */

#define _XmRTCount(rt)		((_XmRenderTable)*(rt))->count
#define _XmRTRenditions(rt)	((_XmRenderTable)*(rt))->renditions
#define _XmRTDisplay(rt)	((_XmRenderTable)*(rt))->display
#define _XmRTMark(rt)		((_XmRenderTable)*(rt))->mark
#define _XmRTRefcount(rt)	((_XmRenderTable)*(rt))->refcount
#define _XmRTRefcountInc(rt)	++(((_XmRenderTable)*(rt))->refcount)
#define _XmRTRefcountDec(rt)	--(((_XmRenderTable)*(rt))->refcount)

#define RENDITIONS_IN_STRUCT	1

typedef struct __XmRenderTableRec
{
	u_short			mark;
	u_short			refcount;
	u_short			count;
	Display *		display;
	XmRendition		renditions[RENDITIONS_IN_STRUCT];
} _XmRenderTableRec, 		*_XmRenderTable;


/********    Private Function Declarations for XmRenderTable.c    ********/

extern XmRendition _XmRenderTableFindRendition( XmRenderTable table,
                                                XmStringTag tag,
                                                Boolean cached_tag,
                                                Boolean need_font,
                                                Boolean call,
                                                short *index
                                              );

extern XmRendition _XmRenditionCreate(Display *display,
				      Widget widget,
				      String resname,
				      String resclass,
				      XmStringTag tag,
				      ArgList arglist,
				      Cardinal argcount,
				      Boolean *in_db); 
extern XmRendition _XmRenderTableGetMerged(XmRenderTable rt,
					   XmStringTag base,
					   XmStringTag *tags,
#if NeedWidePrototypes
					   unsigned int tag_count 
#else			
                                           unsigned short tag_count 
#endif /* NeedWidePrototypes */
					   );
extern XmRendition _XmRenditionMerge(Display *d,
				     XmRendition *scr,
				     XmRendition base_rend,
				     XmRenderTable rt,
				     XmStringTag base_tag,
				     XmStringTag *tags,
#if NeedWidePrototypes
				     unsigned int tag_count,
                                     unsigned int copy
#else			
				     unsigned short tag_count,
                                     Boolean copy
#endif /* NeedWidePrototypes */
				     ); 
extern Widget _XmCreateRenderTable(Widget parent,
				   String name,
				   ArgList arglist,
				   Cardinal argcount); 
extern Widget _XmCreateRendition(Widget parent,
				 String name,
				 ArgList arglist,
				 Cardinal argcount); 
extern Display *_XmRenderTableDisplay(XmRenderTable table);
extern XmRendition _XmRenditionCopy(XmRendition rend,
				    Boolean shared);
extern Boolean _XmRenderTableFindFallback(XmRenderTable ,
					  XmStringTag tag,
#if NeedWidePrototypes
					  int cached_tag,
#else
					  Boolean cached_tag,
#endif /* NeedWidePrototypes */
					  short *indx,
					  XmRendition *rend_ptr) ;
extern Boolean _XmRenderTableFindFirstFont(XmRenderTable rendertable,
					   short *indx,
					   XmRendition *rend_ptr);
extern XmRenderTable _XmRenderTableRemoveRenditions(XmRenderTable oldtable,
						    XmStringTag *tags,
						    int tag_count,
#if NeedWidePrototypes
						    int chk_font,
#else
						    Boolean chk_font,
#endif /* NeedWidePrototypes */
						    XmFontType type,
						    XtPointer font);

#ifdef SUN_CTL 
extern Dimension _XmRenditionEscapement(
				XmRendition 	rend,
				char 		*text, 
				size_t 		 text_len, 
				Boolean 	 is_wchar,
				Dimension 	 tabwidth);
extern Dimension _XmRenditionPosToEscapement(
				XmRendition	rend,
				Position	offset,
				char 		*string,
				Boolean		is_wchar,
				XmTextPosition	index,
				XmTextPosition	length,
				Dimension	tabwidth,
				XmEDGE		edge,
				unsigned char	mode,
				Boolean		istext_rtaligned);
extern XmTextPosition _XmRenditionEscapementToPos(
				XmRendition		rend,
				Position		offset,
				Position		escapement, 
				char 			*text, 
				XmTextPosition		text_len,
				Boolean			is_wchar,
				Dimension		tabwidth,
				XmEDGE			edge,
				XmCURSOR_DIRECTION 	*ret_cursor_dir,
				Boolean			istext_rtaligned);

extern int _XmRenditionDraw(  XmRendition          rend,
                              Widget               w,
                              GC                   gc,
                              XmTextPart *         tpp,
                              Position             x,
                              Position             y,       /* baseline */
                              char *               draw_text,
                              uint                 draw_text_len,
                              Boolean              is_wchar,
                              Boolean              is_editable,
                              Boolean              image,
                              _XmHighlightData *   hl_data,
                              Dimension            tabwidth,
                              Boolean              istext_rtaligned);

extern XmTextPosition _XmRenditionScan(
				XmRendition 		rend,
			       char 			*text, 
			       unsigned int 		text_len,
			       Boolean 			is_wchar,
			       XmTextPosition		pos,
			       XmTextScanType		sType,
			       XmTextScanDirection	dir);
extern int _XmRenditionTextPerCharExtents(
				XmRendition    rend,
				char          *text,
				int            text_len,
				Boolean        is_wchar,
				XtPointer      logical_array,
				int            array_size,
				int           *num_chars_return,
				Position       xoffset,
				Dimension      tab_width,
				Boolean        istext_rtaligned,
				XtPointer      overall_logical_return);
extern int _XmRenditionPosCharExtents(
				XmRendition       rend,
				XmTextPosition    pos,
				char             *text,
				size_t            text_len,
				Boolean           is_wchar,
				int               tab_width,
				Boolean           istext_rtaligned,
				XSegment         *char_xsegment);
#endif /* CTL */

/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmRenderTI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
