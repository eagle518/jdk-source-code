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

/*    File:   TextOut.c version 1.34, last updated (yy/dmm/dd) 01/04/26
 *
 * (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
 * (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY
 *
 *  (c) Copyright 1995 FUJITSU LIMITED
 *  This is source code modified by FUJITSU LIMITED under the Joint
 *  Development Agreement for the CDEnext PST.
 *  This is unpublished proprietary source code of FUJITSU LIMITED
 */

#include <stdio.h>
#include <limits.h>
#include <Xm/XmosP.h>
#include <X11/Shell.h>
#include <X11/Vendor.h>
#include <X11/Xatom.h>
#include <Xm/AccColorT.h>
#include <Xm/AtomMgr.h>
#include <Xm/Display.h>
#include <Xm/DrawP.h>
#include <Xm/NavigatorT.h>
#include <Xm/ScrollBar.h>
#include <Xm/ScrollFrameT.h>
#include <Xm/ScrolledWP.h>
#include <Xm/TextStrSoP.h>
#include <Xm/TraitP.h>
#include <Xm/ColorObj.h>
#include <Xm/ColorObjP.h>
#include "BaseClassI.h"
#include "ImageCachI.h"
#include "MessagesI.h"
#include "ScreenI.h"
#include "ScrollFramTI.h"
#include "TextI.h"
#include "TextInI.h"
#include "TextOutI.h"
#include "TraversalI.h"
#include "XmI.h"
#ifdef SUN_CTL
#include "XmRenderTI.h"		/* for _XmRendFontType() */
#endif /* CTL */


#define MSG1	_XmMMsgTextOut_0000
#define MSG2	_XmMMsgTextF_0001
#define MSG3	_XmMMsgTextF_0002
#define MSG4	_XmMMsgTextF_0003

#define XmDYNAMIC_BOOL        ((Boolean) (255))

 /* Text is right aligned in two cases
       1. When Widget layout direction is Right to Left and the Alignment
          is Beginnning
       2. When Widget layout direction is Left to Right and the Alignment
          is End
	  */
#ifdef SUN_CTL
#define ISTEXT_RIGHTALIGNED(tw)\
        (tw->text.output->data->ctl_direction)
#define IS_DEF_TEXT_RIGHTALIGNED(tw)\
         ((XmDirectionMatch(XmPrim_layout_direction(tw), XmRIGHT_TO_LEFT)&& \
           tw->text.output->data->alignment == XmALIGNMENT_BEGINNING) || \
          (XmDirectionMatch(XmPrim_layout_direction(tw), XmLEFT_TO_RIGHT)&& \
           tw->text.output->data->alignment == XmALIGNMENT_END))
#endif /* CTL */


void _Setup_hl1(Widget, XmTextPart *, Display *, Screen *);
void _Setup_hl2(Widget, XmTextPart *, Display *, Screen *);


/********    Static Function Declarations    ********/

static void SliderMove(Widget, XtPointer, XtPointer);
static void _XmTextDrawShadow(XmTextWidget);
static void SetFullGC(XmTextWidget, GC);
static void GetRect(XmTextWidget, XRectangle *);
static void SetMarginGC(XmTextWidget, GC);
static void SetNormGC(XmTextWidget, GC, Boolean, Boolean);
static void InvertImageGC(XmTextWidget);
static void SetInvGC(XmTextWidget, GC);
static int _FontStructFindWidth(XmTextWidget, int, XmTextBlock, size_t, size_t);
static int FindWidth(XmTextWidget, int, XmTextBlock, size_t, size_t);
static XmTextPosition XYToPos(XmTextWidget, Position, Position);
static Boolean PosToXY(XmTextWidget, XmTextPosition, Position *, Position *);
static XtGeometryResult TryResize(XmTextWidget, Dimension, Dimension);
static int CountLines(XmTextWidget, XmTextPosition, XmTextPosition);
static void TextFindNewWidth(XmTextWidget, Dimension *);
static void TextFindNewHeight(XmTextWidget, XmTextPosition, Dimension *);
static void CheckForNewSize(XmTextWidget, XmTextPosition);
static XtPointer OutputBaseProc(Widget, XtPointer);
static Boolean MeasureLine(XmTextWidget, LineNum, XmTextPosition, XmTextPosition *, LineTableExtra *);
static void Draw(XmTextWidget, LineNum, XmTextPosition, XmTextPosition,
#ifdef SUN_CTL
		 _XmHighlightData *);
#else  /* CTL */
		 XmHighlightMode);
#endif /* CTL */
static OnOrOff CurrentCursorState(XmTextWidget);
static void PaintCursor(XmTextWidget);
static void ChangeHOffset(XmTextWidget, int, Boolean);
static void ChangeVOffset(XmTextWidget, int, Boolean);
static void DrawInsertionPoint(XmTextWidget, XmTextPosition, OnOrOff);
static void MakePositionVisible(XmTextWidget, XmTextPosition);
static void BlinkInsertionPoint(XmTextWidget);
static Boolean MoveLines(XmTextWidget, LineNum, LineNum, LineNum);
static void OutputInvalidate(XmTextWidget, XmTextPosition, XmTextPosition, long);
static void RefigureDependentInfo(XmTextWidget);
static void SizeFromRowsCols(XmTextWidget, Dimension *, Dimension *);
static void LoadFontMetrics(XmTextWidget);
static void LoadGCs(XmTextWidget, Pixel, Pixel);
static void MakeIBeamOffArea(XmTextWidget, Dimension, Dimension);
static Pixmap FindPixmap(Screen *, char *, Pixel, Pixel, int);
static void MakeIBeamStencil(XmTextWidget, int);
static void MakeAddModeCursor(XmTextWidget, int);
static void MakeCursors(XmTextWidget);
static void OutputGetValues(Widget, ArgList, Cardinal);
static Boolean CKCols(ArgList, Cardinal);
static Boolean CKRows(ArgList, Cardinal);
static Boolean OutputSetValues(Widget, Widget, Widget, ArgList, Cardinal *);
static void NotifyResized(Widget, Boolean);
static void HandleTimer(XtPointer, XtIntervalId *);
static void HandleFocusEvents(Widget, XtPointer, XEvent *, Boolean *);
static void HandleGraphicsExposure(Widget, XtPointer, XEvent *, Boolean *);
static void OutputRealize(Widget, XtValueMask *, XSetWindowAttributes *);
static void OutputDestroy(Widget);
static void RedrawRegion(XmTextWidget, int, int, int, int);
static void OutputExpose(Widget, XEvent *, Region);
static void GetPreferredSize(Widget, Dimension *, Dimension *);
static void HandleVBarButtonRelease(Widget, XtPointer, XEvent *, Boolean *);
static void HandleHBarButtonRelease(Widget, XtPointer, XEvent *, Boolean *);
static int _FontStructFindHeight(XmTextWidget, int, XmTextBlock, size_t, size_t);
static int FindHeight(XmTextWidget, int, XmTextBlock, size_t, size_t);
static Boolean _FontStructPerCharExtents(XmTextWidget, char *, int, XCharStruct *);
static Boolean SetXOCOrientation(XmTextWidget, XOC, XOrientation);



/********    End Static Function Declarations    ********/

#define EraseInsertionPoint(tw)\
(*tw->text.output->DrawInsertionPoint)(tw,\
					   tw->text.cursor_position, off)

#define TextDrawInsertionPoint(tw)\
(*tw->text.output->DrawInsertionPoint)(tw,\
					   tw->text.cursor_position, on)

static XmTextWidget posToXYCachedWidget = NULL;
static XmTextPosition posToXYCachedPosition;
static Position posToXYCachedX;
static Position posToXYCachedY;
#ifdef SUN_CTL
static XmDirection posToXYCachedCtlDirection;
#endif /*SUN_CTL*/
static XtResource output_resources[] =
{
    {
      XmNfontList, XmCFontList, XmRFontList, sizeof(XmFontList),
      XtOffsetOf(OutputDataRec, fontlist),
      XmRImmediate, (XtPointer)NULL
    },

    {
      XmNrenderTable, XmCRenderTable, XmRRenderTable, sizeof(XmRenderTable),
      XtOffsetOf(OutputDataRec, rendertable),
      XmRImmediate, (XtPointer)NULL
    },

    {
      XmNwordWrap, XmCWordWrap, XmRBoolean, sizeof(Boolean),
      XtOffsetOf(struct _OutputDataRec, wordwrap),
      XmRImmediate, (XtPointer) False
    },

    {
      XmNblinkRate, XmCBlinkRate, XmRInt, sizeof(int),
      XtOffsetOf(struct _OutputDataRec, blinkrate),
      XmRImmediate, (XtPointer) 500
    },

    {
      XmNcolumns, XmCColumns, XmRShort, sizeof(short),
      XtOffsetOf(struct _OutputDataRec, columns),
      XmRImmediate, (XtPointer) 0
    },

    {
      XmNrows, XmCRows, XmRShort, sizeof(short),
      XtOffsetOf(struct _OutputDataRec, rows),
      XmRImmediate, (XtPointer) 0
    },

    {
      XmNresizeWidth, XmCResizeWidth, XmRBoolean, sizeof(Boolean),
      XtOffsetOf(struct _OutputDataRec, resizewidth),
      XmRImmediate, (XtPointer) False
    },

    {
      XmNresizeHeight, XmCResizeHeight, XmRBoolean, sizeof(Boolean),
      XtOffsetOf(struct _OutputDataRec, resizeheight),
      XmRImmediate, (XtPointer) False
    },

    {
      XmNscrollVertical, XmCScroll, XmRBoolean, sizeof(Boolean),
      XtOffsetOf(struct _OutputDataRec, scrollvertical),
      XmRImmediate,(XtPointer) True
    },

    {
      XmNscrollHorizontal, XmCScroll, XmRBoolean, sizeof(Boolean),
      XtOffsetOf(struct _OutputDataRec, scrollhorizontal),
      XmRImmediate, (XtPointer) True
    },

    {
      XmNscrollLeftSide, XmCScrollSide, XmRBoolean, sizeof(Boolean),
      XtOffsetOf(struct _OutputDataRec, scrollleftside),
      XmRImmediate,(XtPointer) XmDYNAMIC_BOOL
    },

    {
      XmNscrollTopSide, XmCScrollSide, XmRBoolean, sizeof(Boolean),
      XtOffsetOf(struct _OutputDataRec, scrolltopside),
      XmRImmediate, (XtPointer) False
    },

    {
      XmNcursorPositionVisible, XmCCursorPositionVisible, XmRBoolean,
      sizeof(Boolean),
      XtOffsetOf(struct _OutputDataRec, cursor_position_visible),
      XmRImmediate, (XtPointer) True
    }
#ifdef SUN_CTL
 ,{
   XmNalignment, XmCAlignment, XmRAlignment,
   sizeof(unsigned char), XtOffsetOf(struct _OutputDataRec, alignment),
   XmRImmediate, (XtPointer) XmALIGNMENT_BEGINNING
 }
#endif /* CTL */
};



void
_XmTextFreeContextData(Widget w, XtPointer clientData, XtPointer callData)
{
	XmTextContextData		ctx_data=(XmTextContextData)clientData;
	Display *				display=DisplayOfScreen(ctx_data->screen);
	XtPointer				data_ptr;

	if (XFindContext(display, (Window)ctx_data->screen,
						  ctx_data->context, (char **) &data_ptr))
	 {
		if (ctx_data->type != '\0')
		 {
			if (data_ptr)
			 {
				XtFree((char *) data_ptr);
			 }
		 }
		XDeleteContext (display, (Window) ctx_data->screen, ctx_data->context);
	 }
  
	XtFree ((char *) ctx_data);
}


/*****************************************************************************
 * To make TextOut a true "Object" this function should be a class function. *
 *****************************************************************************/
static void 
_XmTextDrawShadow(XmTextWidget tw)
{
	if (XtIsRealized((Widget)tw))
	 {
		if (tw->primitive.shadow_thickness > 0)
		 {
			XmeDrawShadows(XtDisplay(tw), XtWindow(tw),
								tw->primitive.bottom_shadow_GC,
								tw->primitive.top_shadow_GC,
								tw->primitive.highlight_thickness,
								tw->primitive.highlight_thickness,
								tw->core.width - 2 * tw->primitive.highlight_thickness,
								tw->core.height - 2 * tw->primitive.highlight_thickness,
								tw->primitive.shadow_thickness,
								XmSHADOW_OUT);
		 }

		if (tw->primitive.highlighted)
		 {   
			(*(((XmPrimitiveWidgetClass)XtClass((Widget)tw))->
									primitive_class.border_highlight))((Widget)tw);
		 }
		else
		 {   
			(*(((XmPrimitiveWidgetClass) XtClass((Widget)tw))->
									primitive_class.border_unhighlight))((Widget)tw);
		 }
	 }
}


void
_XmTextResetClipOrigin(XmTextWidget tw, XmTextPosition position, Boolean clip_mask_reset)
{
	OutputData	data = tw->text.output->data;
	int			x, y;
	Position		x_pos, y_pos;
  
	if (!XtIsRealized((Widget)tw))
	 {
		return;
	 }
  
	if (!PosToXY(tw, tw->text.cursor_position, &x_pos, &y_pos))
	 {
		return;
	 }
  
	x = (int) x_pos; y = (int) y_pos;
	x -= (data->cursorwidth >> 1) + 1;
	y = (y + data->font_descent) - data->cursorheight;
	XSetTSOrigin(XtDisplay((Widget)tw), data->imagegc, x, y);
}


static void 
SetFullGC(XmTextWidget tw, GC gc)
{
	XRectangle	ClipRect;

	ClipRect.x = tw->primitive.highlight_thickness + tw->primitive.shadow_thickness;
	ClipRect.y = tw->primitive.highlight_thickness + tw->primitive.shadow_thickness;
	ClipRect.width = tw->core.width - (2 *(tw->primitive.highlight_thickness +
								tw->primitive.shadow_thickness));
	ClipRect.height = tw->core.height - (2 *(tw->primitive.highlight_thickness +
								tw->primitive.shadow_thickness));
  
	XSetClipRectangles(XtDisplay(tw), gc, 0, 0, &ClipRect, 1, Unsorted);
}


static void 
GetRect(XmTextWidget tw, XRectangle *rect)
{
	Dimension	margin_width=tw->text.margin_width + tw->primitive.shadow_thickness +
																	 tw->primitive.highlight_thickness;
	Dimension	margin_height=tw->text.margin_height + tw->primitive.shadow_thickness +
																		tw->primitive.highlight_thickness;
  
	if (margin_width < tw->core.width)
	 {
		rect->x = margin_width;
	 }
	else
	 {
		rect->x = tw->core.width;
	 }
  
	if (margin_height < tw->core.height)
	 {
		rect->y = margin_height;
	 }
	else
	 {
		rect->y = tw->core.height;
	 }
  
	if ((int) (2 * margin_width) < (int) tw->core.width)
	 {
		rect->width = (int) tw->core.width - (2 * margin_width);
	 }
	else
	 {
		rect->width = 0;
	 }
  
	if ((int) (2 * margin_height) < (int) tw->core.height)
	 {
		rect->height = (int) tw->core.height - (2 * margin_height);
	 }
	else
	 {
		rect->height = 0;
	 }
}


static void 
SetMarginGC(XmTextWidget tw, GC gc)
{
	XRectangle	ClipRect;
  
	GetRect(tw, &ClipRect);
	XSetClipRectangles(XtDisplay(tw), gc, 0, 0, &ClipRect, 1, Unsorted);
}


/*****************************************************************************
 * To make TextOut a true "Object" this function should be a class function. *
 *****************************************************************************/
/*
 * Set new clipping rectangle for text widget.  This is
 * done on each focus in event since the text widgets
 * share the same GC.
 */
void 
_XmTextAdjustGC(XmTextWidget tw)
{
	OutputData		data=tw->text.output->data;
	unsigned long	valueMask=(GCForeground | GCBackground);
	XGCValues		values;


	if (!XtIsRealized((Widget)tw))
	 {
		return;
	 }

	SetMarginGC(tw, data->gc);

	/* Restore cached text gc to state correct for this instantiation */

	if (data->gc)
	 {
		values.foreground = tw->primitive.foreground ^ tw->core.background_pixel;
		values.background = 0;
		XChangeGC(XtDisplay(tw), data->gc, valueMask, &values);
	 }
}


static void 
SetNormGC(XmTextWidget tw, GC gc, Boolean change_stipple, Boolean stipple)
{
	unsigned long	valueMask=(GCForeground | GCBackground);
	XGCValues		values;
	OutputData		data=tw->text.output->data;
  
	values.foreground = tw->primitive.foreground;
	values.background = tw->core.background_pixel;
	if (change_stipple)
	 {
		valueMask |= GCFillStyle;
		if (stipple)
		 {
			values.fill_style = FillStippled;
			valueMask |= GCStipple;
			values.stipple = data->stipple_tile;
		 }
		else 
		 {
			values.fill_style = FillSolid;
		 }
	 }
  
	XChangeGC(XtDisplay(tw), gc, valueMask, &values);
}


static void 
InvertImageGC(XmTextWidget tw)
{
	OutputData	data=tw->text.output->data;

	data->have_inverted_image_gc = !data->have_inverted_image_gc;
}


static void
SetInvGC(XmTextWidget tw, GC gc)
{
	unsigned long	valueMask=(GCForeground | GCBackground);
	XGCValues		values;

	values.foreground = tw->core.background_pixel;
	values.background = tw->primitive.foreground;

	XChangeGC(XtDisplay(tw), gc, valueMask, &values);
}


#ifdef SUN_CTL
/* This function should be called with to == block->length to ensure 
   proper results.
   */
static int 
CTLFindWidth(XmTextWidget	tw,
				 int				x,			/*	Starting position (needed for tabs)	*/
				 XmTextBlock	block,
				 size_t			from,		/* How many bytes in to start measuring*/
				 size_t			to			/* How many bytes in to stop measuring */
				)
{
	size_t	end=to;
	size_t	ret=0;
  

   if (end > 0)
	 {
		OutputData	data     = tw->text.output->data;
		Boolean		is_wchar = (tw->text.char_size > 1);

		/*++ This idiom is repeated before all the calls to _XmRendition	*/
		/*	functions. We need this because ReadSource returns mbs.			*/

		char *		text=block->ptr;
		char			tmp_cache[200];
		char *		tmp_ch;
		wchar_t		tmp_wcache[200];
		wchar_t *	tmp_wc;
      
		/* block->ptr is not null-terminated - but we can't just go		*/
		/*	write a null into it														*/

		if (is_wchar && _XmRendLayoutIsCTL(data->rendition))
		 {
			tmp_ch = XmStackAlloc((end + 1), tmp_cache);
			memcpy(tmp_ch, text, end);			/* we know there's no overlap so	*/
														/*	don't pay for memmove			*/
			tmp_ch[end] = 0;
      
			tmp_wc = (wchar_t*)XmStackAlloc(((end + 1) * sizeof(wchar_t)), tmp_wcache);
			end    = mbstowcs(tmp_wc, tmp_ch, end);

			if (end == -1)
			 {
				end = _Xm_mbs_invalid(tmp_wc, tmp_ch, to);
			 }
			tmp_wc[end] = (wchar_t)0;
	  		text = (char*)tmp_wc;
		 }

		ret = _XmRenditionEscapement(data->rendition, text, end, is_wchar, data->tabwidth);
		if (is_wchar)
		 {
			XmStackFree(tmp_ch, tmp_cache);
			XmStackFree((char*)tmp_wc, tmp_wcache);
		 }
	 }

	return ret;
}


static int 
FindPosition(XmTextWidget		tw,
				 int					x,		/* Starting position (needed for tabs)	*/
				 XmTextBlock		block,
				 XmTextPosition	pos,
				 XmEDGE				edge)
{
	OutputData	data     = tw->text.output->data;
	Boolean		is_wchar = (tw->text.char_size > 1);
	int			ret      = 0;
	int			end      = block->length;
    

	if (end > 0)
	 {
		char *		text=block->ptr;
		char			tmp_cache[200];
		char *		tmp_ch=NULL;
		wchar_t		tmp_wcache[200];
		wchar_t *	tmp_wc=NULL;
	
		if (is_wchar && _XmRendLayoutIsCTL(data->rendition))
		 {
			tmp_ch = XmStackAlloc((end + 1), tmp_cache);
			memcpy(tmp_ch, text, end);
			tmp_ch[end] = 0;
	    
			tmp_wc = (wchar_t*)XmStackAlloc(((end + 1) * sizeof(wchar_t)), tmp_wcache);
			end	 = mbstowcs(tmp_wc, tmp_ch, end);
	    
			if (end == -1)
			 {
				end = _Xm_mbs_invalid(tmp_wc, tmp_ch, block->length);
			 }
			tmp_wc[end] = (wchar_t)0;
			text = (char*)tmp_wc;
		 }

		ret = _XmRenditionPosToEscapement(data->rendition, x, text, is_wchar,
													 pos, end, data->tabwidth, edge,
													 tw->text.input->data->edit_policy,
													 ISTEXT_RIGHTALIGNED(tw));
		if (is_wchar)
		 {
			XmStackFree((char*)tmp_ch, tmp_cache);
			XmStackFree((char*)tmp_wc, tmp_wcache);
		 }
	 }

	return ret;
}
#endif /* CTL */


static int 
_FontStructFindWidth(XmTextWidget	tw,
							int				x,		/* Starting position (needed for tabs) */
							XmTextBlock		block,
							size_t			from,	/* How many bytes in to start measuring */ /* Wyoming 64-bit fix */
							size_t			to		/* How many bytes in to stop measuring */ /* Wyoming 64-bit fix */
						  )
{
	OutputData		data=tw->text.output->data;
	XFontStruct *	font=data->font;
	char *			ptr;
	unsigned char	c;
	size_t			i;
	int				csize;
	int				result=0;
  
	if (tw->text.char_size != 1)
	 {
		int			dummy;
		XCharStruct	overall;
    
		for (i=from, ptr=block->ptr + from; i<to; i +=csize, ptr += csize)
		 {
			csize = mblen(ptr, tw->text.char_size);
			if (csize == 0)
			 {
				break;
			 }
			if (csize == -1)
			 {
				csize = 1;
			 }
			c = (unsigned char) *ptr;
			if (csize == 1)
			 {
				if (c == '\t')
				 {
					result += (data->tabwidth -
									((x + result - data->leftmargin) % data->tabwidth));
				 }
				else
				 {
					if (font->per_char && (c >= font->min_char_or_byte2) && 
						 (c <= font->max_char_or_byte2))
					 {
						result += font->per_char[c - font->min_char_or_byte2].width;
					 }
					else
					 {
						result += font->min_bounds.width;
					 }
				 }
			 }
			else
			 {
				XTextExtents(data->font, ptr, csize, &dummy, &dummy, &dummy, &overall);
				result += overall.width;
			 }
		 }
	 }
	else
	 {
		for (i=from, ptr=block->ptr + from; i<to; i++, ptr++)
		 {
			c = (unsigned char) *ptr;
			if (c == '\t')
			 {
				result += (data->tabwidth - ((x + result - data->leftmargin) % data->tabwidth));
			 }
			else
			 {
				if (font->per_char)
				 {
					if ((c >= font->min_char_or_byte2) &&
						 (c <= font->max_char_or_byte2))
					 {
						result += font->per_char[c - font->min_char_or_byte2].width;
					 }
					else if ((font->default_char >= font->min_char_or_byte2) &&
								(font->default_char <= font->max_char_or_byte2))
					 {
						result += font->per_char[font->default_char - font->min_char_or_byte2].width;
					 }
					else
					 {
						result += font->min_bounds.width;
					 }
				 }
				else
				 {
					result += font->min_bounds.width;
				 }
			 }
		 }
	 }

	return result;
}


static int 
#ifdef SUN_CTL
NONCTLFindWidth(XmTextWidget tw, /* another name for CTL version*/
#else /* CTL */
FindWidth(XmTextWidget tw,
#endif
	  int x,                  /* Starting position (needed for tabs) */
	  XmTextBlock block,
	  size_t from,               /* How many bytes in to start measuring */
	  size_t to)                 /* How many bytes in to stop measuring */
{
	OutputData		data=tw->text.output->data;
	unsigned char	c;
	char *			ptr;
	size_t			tmp;
	long				csize=1;
	long				i;
	int				result=0;
  

	if (!data->use_fontset)
	 {
		return _FontStructFindWidth(tw, x, block, from, to);
	 }
  
	if (to > block->length)
	 {
		to = block->length;
	 }

	if (from > to)
	 {
		tmp = to;
		to = from;
		from = tmp;
	 }
  
	if ((to == from) || (to == 0))
	 {
		return 0;
	 }
  
	if (tw->text.char_size != 1)
	 {
		for (i=from, ptr=block->ptr + from; i<to; i+=csize, ptr+=csize)
		 {
			csize = mblen(ptr, tw->text.char_size);
			if (csize == 0)
			 {
				break;
			 }
			if (csize == -1)
			 {
				csize = 1;
			 }
			c = (unsigned char) *ptr;
			if ((csize == 1) && (c == '\t'))
			 {
				result += (data->tabwidth -
									((x + result - data->leftmargin) % data->tabwidth));
			 }
			else
			 {
				result += XmbTextEscapement((XFontSet)data->font, ptr, (int)csize);
			 }
		 }
	 }
	else
	 {				/*	no need to pay for mblen if we know all chars are 1 byte */
		for (i=from, ptr=block->ptr + from; i<to; i++, ptr++)
		 {
			c = (unsigned char) *ptr;
			if (c == '\t')
			 {
				result += (data->tabwidth -
										((x + result - data->leftmargin) % data->tabwidth));
			 }
			else
			 {
				result += XmbTextEscapement((XFontSet)data->font, ptr, 1);
			 }
		 }
	 } 

	return result;
}


static int 
_FontStructFindHeight(XmTextWidget	tw,
							 int				y,
							 XmTextBlock	block,
							 size_t			from,
							 size_t			to)
{
	OutputData		data=tw->text.output->data;
	XFontStruct *	font=data->font;
	char *			ptr=NULL;
	unsigned char	c;
	size_t			i=0;
	long				csize=0;
	int				result=0;
	XCharStruct		overall;
  
	if (tw->text.char_size != 1)
	 {
		for (i=from, ptr=block->ptr + from; i<to; i+=csize, ptr+=csize)
		 {
			csize = mblen(ptr, tw->text.char_size);
			if (csize == 0)
			 {
				break;
			 }
			if (csize == -1)
			 {
				csize = 1;
			 }
			if ((csize == 1) && ((unsigned char)*ptr == '\t'))
			 {
				result += (data->tabheight -
										((y + result - data->topmargin) % data->tabheight));
			 }
			else
			 {
				_FontStructPerCharExtents(tw, ptr, (int)csize, &overall);
				result += overall.ascent + overall.descent;
			 }
		 }
	 }
	else
	 {
		for (i=from, ptr=block->ptr + from; i<to; i++, ptr++)
		 {
			c = (unsigned char) *ptr;
			if ((unsigned char) *ptr == '\t')
			 {
				result += (data->tabheight -
									((y + result - data->topmargin) % data->tabheight));
			 }
			else
			 {
				_FontStructPerCharExtents(tw, ptr, 1, &overall);
				result += overall.ascent + overall.descent;
			 }
		 }
	 }

	return result;
}


static int 
FindHeight(XmTextWidget	tw,
			  int				y,
			  XmTextBlock	block,
			  size_t			from,
			  size_t			to)
{
	OutputData		data = tw->text.output->data;
	char *			ptr=NULL;
	unsigned char	c;
	int				result=0;
	long				tmp=0, csize=1, i=0;
	XOrientation	orient;
  
	if (!data->use_fontset)
	 {
		return _FontStructFindHeight(tw, y, block, from, to);
	 }
  
	if (to > block->length)
	 {
		to = block->length;
	 }
	if (from > to)
	 {
		tmp = to;
		to = from;
		from = tmp;
	 }
  
	if ((to == from) || (to == 0))
	 {
		return 0;
	 }
  
	if(data->use_fontset == True)
	 {
		XGetOCValues((XOC)data->font, XNOrientation, &orient, NULL);
		SetXOCOrientation(tw, (XOC)data->font, XOMOrientation_TTB_RTL);
	 }
	if (tw->text.char_size != 1)
	 {
		for (i=from, ptr=block->ptr + from; i<to; i+=csize, ptr+=csize)
		 {
			csize = mblen(ptr, tw->text.char_size);
			if (csize == 0)
			 {
				break;
			 }
			if (csize == -1)
			 {
				csize = 1;
			 }
			c = (unsigned char) *ptr;
			if ((csize == 1) && (c == '\t'))
			 {
				result += (data->tabheight -
									((y + result - data->topmargin) % data->tabheight));
			 }
			else
			 {
				result += XmbTextEscapement((XFontSet)data->font, ptr, (int)csize);
			 }
		 }
	 }
	else
	 {
		for (i=from, ptr=block->ptr + from; i<to; i++, ptr++)
		 {
			c = (unsigned char) *ptr;
			if (c == '\t')
			 {
				result += (data->tabheight -
									((y + result - data->topmargin) % data->tabheight));
			 }
			else
			 {
				result += XmbTextEscapement((XFontSet)data->font, ptr, 1);
			 }
		 }
	 } 

	if(data->use_fontset == True)
	 {
		SetXOCOrientation(tw, (XOC)data->font, orient);
	 }

	return result;
}


#ifdef SUN_CTL
static int 
FindWidth(XmTextWidget	tw,
			 int				x,
			 XmTextBlock	block,
			 size_t			from,
			 size_t			to)
{
	if (TextW_LayoutActive (tw))
	 {
		return CTLFindWidth(tw, x, block, from, to);
	 }
	else
	 {
		return NONCTLFindWidth(tw, x, block, from, to);
	 }
}
#endif /* CTL */



/* Semi-public routines. */

static XmTextPosition 
XYToPos(XmTextWidget tw, Position x, Position y)
{
	OutputData			data=tw->text.output->data;
	LineTableExtra		extra=(LineTableExtra)NULL;
	int					i=0, width=0, lastwidth=0, length=0;
	int					height=0, lastheight=0,
							num_chars=0, num_bytes=0;
	LineNum				line=0;
	XmTextPosition		start, end, laststart;
	XmTextBlockRec		block;
	int					delta=0;

  
	start = end = laststart = 0;
  
	if (XmDirectionMatch(XmPrim_layout_direction(tw),
								XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
	 {
		y += data->voffset;

		/* take care of negative x case */
		if (data->linewidth)
		 {
			int	rightedge=tw->text.inner_widget->core.width - data->rightmargin;
			if (x > rightedge)
			 {
				delta = ((int)(rightedge - x - 1)/ (int) data->linewidth) + 1;
				x = rightedge;
			 }
			line = (rightedge - x) / (int)(data->linewidth);
		 }

		if (line > _XmTextNumLines(tw))
		 {
			line = _XmTextNumLines(tw);
		 }
		_XmTextLineInfo(tw, line, &start, &extra);
		if (start == PASTENDPOS)
		 {
			return (*tw->text.source->Scan)(tw->text.source, 0,
													  XmSELECT_ALL, XmsdRight, 1, False);
		 }

		_XmTextLineInfo(tw, line+1, &end, &extra);
		end = (*tw->text.source->Scan)(tw->text.source, end,
												 XmSELECT_POSITION, XmsdLeft, 1, True);
		height = lastheight = data->topmargin;
		if ((start >= end) && !delta)
		 {
			return start;
		 }
  
		/* if original y was negative, we need to find new laststart */
		if (delta && start > 0)
		 {
			end = (*tw->text.source->Scan)(tw->text.source, start,
													 XmSELECT_POSITION, XmsdLeft,
													 1, True);
			start = _XmTextFindScroll(tw, start, delta);
		 }
  
		do
		 {
			laststart = start;
			start = (*tw->text.source->ReadSource)(tw->text.source, start,
																end, &block);
			length = block.length;
			if ((int)tw->text.char_size > 1)
			 {
				num_bytes = mblen(block.ptr, (int)tw->text.char_size);
				if (num_bytes == -1)
				 {
					num_bytes = 1;
				 }

				for (i=num_chars=0; i<length && height<y && num_bytes>=0; )
				 {
					lastheight = height;
					height += FindHeight(tw, height, &block, i, i + num_bytes);
					i += num_bytes;
					num_chars++;
					num_bytes = mblen(&block.ptr[i], (int)tw->text.char_size);
					if (num_bytes == -1)
					 {
						num_bytes = 1;
					 }
				 }

				i = num_chars;
			 }
			else
			 {
				for (i=0; i<length && height<y; i++)
				 {
					lastheight = height;
					height += FindHeight(tw, height, &block, i, i+1);
				 }
			 }
		 }
		while ((height < y) && (start < end) && (laststart != end));
  
		if (abs(lastheight - y) < abs(height - y))
		 {
			i--;
		 }
	 }
	else
	 {
  		x += data->hoffset;
		y -= data->topmargin;

		/* take care of negative y case */
		if (data->lineheight)
		 {
			if (y < 0)
			 {
				delta = ((int)(y + 1)/ (int) data->lineheight) - 1;
				y = 0;
			 }
			line = y / (int) data->lineheight;
		 }

		if (line > _XmTextNumLines(tw))
		 {
			line = _XmTextNumLines(tw);
		 }

		_XmTextLineInfo(tw, line, &start, &extra);

		if (start == PASTENDPOS)
		 {
			return (*tw->text.source->Scan)(tw->text.source, 0,
													  XmSELECT_ALL, XmsdRight, 1, False);
		 }

		_XmTextLineInfo(tw, line+1, &end, &extra);
		end = (*tw->text.source->Scan)(tw->text.source, end,
												 XmSELECT_POSITION, XmsdLeft, 1, True);
		width = lastwidth = data->leftmargin;
		if (start >= end && !delta)
		 {
			return start;
		 }
  

		/* if original y was negative, we need to find new laststart */

		if (delta && start > 0)
		 {
			end = (*tw->text.source->Scan)(tw->text.source, start,
													 XmSELECT_POSITION, XmsdLeft, 1, True);
			start = _XmTextFindScroll(tw, start, delta);
		 }

#ifdef SUN_CTL
		if (TextW_LayoutActive(tw))
		 {
			int			line_pos;
			Dimension	line_width=0;
			Boolean		istext_rightaligned=ISTEXT_RIGHTALIGNED(tw);
			int			count;
			char *		text;
			Boolean		is_wchar;
			char			tmp_cache[200];
			char *		tmp_ch;
			wchar_t		tmp_wcache[200];
			wchar_t *	tmp_wc;
			XmEDGE		edge;


			(*tw->text.source->ReadSource)(tw->text.source, start, end, &block);
			x -= data->leftmargin;

			/* When the text is right aligned note that first character (in	*/
			/*	logical buffer) comes at the end on the drawing area (i.e		*/
			/*	window) so we need a different logic to compute the position	*/
			/*	from (x,y)																		*/

			if (istext_rightaligned)
			 {
				int	drawable_width,
						x_inline;


				/*	drawable_width denotes the drawable width on the drawing		*/
				/*	window which is equal to window width - the right margin		*/
				/*	and left margin															*/
	
				/* Imagine a hypothetical right edge(HRE) on the right (which	*/
				/*	is parallel to the right edge of drawing window and could	*/
				/*	be beyond the right_edge of the drawing window) from which	*/
				/*	all the logical text is drawn(originates). x_inline denotes	*/
				/*	the horizontal distance of (x,y) [on draw window] from HRE.	*/
				/*																					*/
				/*	note1: hoffset denotes the distance between the HRE.			*/
				/*	note2: hoffset in left alinged text is the distance between	*/
				/*			 the hypothetical left edge  (which is parallel to		*/
				/*			 the left edge of drawing window and could be to the	*/
				/*			 left of the drawing window) from which all the			*/
				/*			 logical is drawn.												*/
	
				drawable_width = tw->text.inner_widget->core.width -
											data->leftmargin - data->rightmargin;
				x_inline = data->hoffset + drawable_width - x;
				x = x_inline + data->hoffset;
			 }

			count    = block.length;
			text     = block.ptr;
			is_wchar = (tw->text.char_size > 1);

      	if (is_wchar && _XmRendLayoutIsCTL(data->rendition))
			 {
				tmp_ch = XmStackAlloc((count + 1), tmp_cache);
				memcpy(tmp_ch, text, count);
				tmp_ch[count] = 0;
				tmp_wc = (wchar_t*)XmStackAlloc(((count + 1) * sizeof(wchar_t)), tmp_wcache);
				count  = mbstowcs(tmp_wc, tmp_ch, count);

				if (count == -1)
				 {
					count = _Xm_mbs_invalid(tmp_wc, tmp_ch, block.length);
				 }
				tmp_wc[count] = (wchar_t)0;
				text = (char*)tmp_wc;
			 }
	 
			edge = ((tw->text.input->data->edit_policy == XmEDIT_VISUAL) &&
					  _XmRendLayoutIsCTL(data->rendition)) ? XmEDGE_LEFT : XmEDGE_NEAREST;
			line_pos = _XmRenditionEscapementToPos(data->rendition, 0, x, 
																text, count, is_wchar,
																data->tabwidth, edge, 
																NULL, istext_rightaligned);
			if (is_wchar)
			 {
				XmStackFree(tmp_ch, tmp_cache);
				XmStackFree((char*)tmp_wc, tmp_wcache);
			 }
			i = start + line_pos;
		 } 
		else
		 {
#endif /* CTL */

		do
		 {
			laststart = start;
			start = (*tw->text.source->ReadSource)(tw->text.source, start,
																end, &block);
			length = block.length;
			if ((int)tw->text.char_size > 1)
			 {
				num_bytes = mblen(block.ptr, (int)tw->text.char_size);
				if (num_bytes == -1)
				 {
					num_bytes = 1;
				 }

				for (i=num_chars=0; i<length && width<x && num_bytes>=0; )
				 {
					lastwidth = width;
					width += FindWidth(tw, width, &block, i, i + num_bytes);
					i += num_bytes;
					num_chars++;
					num_bytes = mblen(&block.ptr[i], (int)tw->text.char_size);
					if (num_bytes == -1)
					 {
						num_bytes = 1;
					 }
				 }
				i = num_chars;
			 }
			else
			 {
				for (i=0; i<length && width<x; i++)
				 {
					lastwidth = width;
					width += FindWidth(tw, width, &block, i, i+1);
				 }
			 }
		 }
		while (width<x && start<end && laststart!=end);
  
		if (abs(lastwidth - x) < abs(width - x))
		 {
			i--;
		 }
#ifdef SUN_CTL
	 }
#endif /* CTL */
	 }

	return (*tw->text.source->Scan)(tw->text.source, laststart,
											  XmSELECT_POSITION, (i < 0) ?
											  XmsdLeft : XmsdRight, abs(i), True);
}


/*****************************************************************************
 * To make TextOut a true "Object" this function should be a class function. *
 *****************************************************************************/
Boolean 
_XmTextShouldWordWrap(XmTextWidget tw)
{
	OutputData data = tw->text.output->data;

	return ShouldWordWrap(data, tw);
}


/*****************************************************************************
 * To make TextOut a true "Object" this function should be a class function. *
 *****************************************************************************/
Boolean 
_XmTextScrollable(XmTextWidget tw)
{
	OutputData data = tw->text.output->data;

	if(XmDirectionMatch(XmPrim_layout_direction(tw),
							  XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
	 {
		return (data->scrollhorizontal && XmIsScrolledWindow(XtParent(tw)));
	 }
	else
	 {
		return (data->scrollvertical && XmIsScrolledWindow(XtParent(tw)));
	 }
}


static Boolean 
PosToXY(XmTextWidget tw, XmTextPosition position, Position* x, Position* y)
{
	OutputData		data=tw->text.output->data;
	LineNum			line;
	XmTextPosition	linestart;
	LineTableExtra	extra;
	XmTextBlockRec	block;
  
#ifdef SUN_CTL
	Boolean istext_rightaligned = ISTEXT_RIGHTALIGNED(tw);
#endif /* CTL */
  
	if(XmDirectionMatch(XmPrim_layout_direction(tw),
							  XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
	 {
		_XmProcessLock();
		if ((tw == posToXYCachedWidget) && (position == posToXYCachedPosition))
		 {
			*x = posToXYCachedX;
			*y = posToXYCachedY;
			_XmProcessUnlock();
			return True;
		 } 
		_XmProcessUnlock();

		line = _XmTextPosToLine(tw, position);
		if ((line == NOLINE) || (line >= data->number_lines))
		 {
			return False;
		 }
		*y = data->topmargin;
		*x = tw->text.inner_widget->core.width - (data->rightmargin +
					line * data->linewidth + data->linewidth * 0.5);
		_XmTextLineInfo(tw, line, &linestart, &extra);
		while (linestart < position)
		 {
			linestart = (*tw->text.source->ReadSource)(tw->text.source,
																	 linestart, position, &block);
			*y += FindHeight(tw, *y, &block, 0, block.length);
		 }

		*y -= data->voffset;
	 }
	else
	 {
		_XmProcessLock();
		if ((tw == posToXYCachedWidget) && (position == posToXYCachedPosition))
		 {
#ifdef SUN_CTL
			if (!TextW_LayoutActive(tw) ||
				 (TextW_LayoutActive(tw) &&
				  tw->text.output->data->ctl_direction ==
								posToXYCachedCtlDirection))
			 {
#endif /* CTL */ 
				*x = posToXYCachedX;
				*y = posToXYCachedY;
				_XmProcessUnlock();
				return True;
#ifdef SUN_CTL
			 }
#endif /* CTL */ 
		 }
		_XmProcessUnlock();

		line = _XmTextPosToLine(tw, position);
		if ((line == NOLINE) || (line >= data->number_lines))
		 {
			return False;
		 }
		*y = data->topmargin + line * data->lineheight + data->font_ascent;
		*x = data->leftmargin;

		_XmTextLineInfo(tw, line, &linestart, &extra);
#ifdef SUN_CTL
		if (TextW_LayoutActive(tw))
		 {
			XmTextPosition	nextlinestart;
			int				line_width=0, line_pos_x;

   
			istext_rightaligned = ISTEXT_RIGHTALIGNED(tw);
			_XmTextLineInfo(tw, line + 1, &nextlinestart, NULL);

			if (nextlinestart == PASTENDPOS)
			 {
				nextlinestart = tw->text.last_position;
			 }
      
			if (nextlinestart > linestart)		/* if zero-length, no need to	*/
			 {												/*	measure							*/
				(*tw->text.source->ReadSource)(tw->text.source, linestart,
														 nextlinestart, &block);
			 }
			else
			 {
				block.length = 0;						/* (Checked a few lines down)	*/
			 }
      

			/* Now measure to our desired position */

#ifdef SUN_CTL_NYI
			if (tw->text.input->data->edit_policy == XmEDIT_VISUAL)
			 {
				line_pos_x =  FindPosition(tw, 0, &block, position - linestart, XmEDGE_LEFT);
			 }
			else
#endif
			line_pos_x =  FindPosition(tw, 0, &block, position - linestart, XmEDGE_BEG);
			if (istext_rightaligned)
			 {
				line_width = FindWidth(tw, 0, &block, 0, block.length);
				*x = (int)tw->text.inner_widget->core.width - data->rightmargin -
								((line_width - line_pos_x) - (data->hoffset));
				*x += data->hoffset;			/* to take care of x-= data->hoffset */
			 }
			else
			 {
				*x += line_pos_x;
			 }
		 }
		else
		 {
#endif /* CTL */
			while (linestart < position)
			 {
				linestart = (*tw->text.source->ReadSource)(tw->text.source,
																		 linestart, position, 
																		 &block);
				*x += FindWidth(tw, *x, &block, 0, block.length);
			 }
#ifdef SUN_CTL
		 }
#endif /* CTL */
		*x -= data->hoffset;
	 }
	_XmProcessLock();
	posToXYCachedWidget = tw;
	posToXYCachedPosition = position;
	posToXYCachedX = *x;
	posToXYCachedY = *y;
#ifdef SUN_CTL
	posToXYCachedCtlDirection = tw->text.output->data->ctl_direction;
#endif /* CTL */
	_XmProcessUnlock();

	return True;
}


#ifdef SUN_CTL
LineNum PosToAbsLine(XmTextWidget tw, XmTextPosition pos)
{
	LineNum	curr_index;

	XmTextLineTable line_table = tw->text.line_table;
	LineNum max_index = tw->text.total_lines;

	if (pos > tw->text.last_position)
	 {
		return NOLINE;
	 }
    
	for (curr_index=0; curr_index<max_index-1; curr_index++)
	 {
		if ((pos >= line_table[curr_index].start_pos) && 
			 (pos < line_table[curr_index+1].start_pos))
		 {
			return curr_index;
		 }
	 }

	return max_index - 1;
}


void CTLLineInfo(XmTextWidget tw, LineNum line, XmTextPosition* linestart)
{
	XmTextLineTable	line_table=tw->text.line_table;
	LineNum				max_index=tw->text.total_lines;

	if (line >= max_index)
	 {
		*linestart = tw->text.last_position;
		return;
	 }
	if (line < 0)
	 {
		*linestart = 0;
		return;
    }
    
	*linestart = line_table[line].start_pos;
	return;
}


Boolean
PosToAbsXY(XmTextWidget tw, XmTextPosition pos, XmEDGE edge, Position *x, Position *y)
{
	LineNum	line;
	Boolean	line_exists;
	char *	text;
	int		linestart, next_linestart, text_len;
    

	OutputData data  = tw->text.output->data;
	int line_height  = data->font_ascent + data->font_descent;
	Boolean is_wchar = (tw->text.char_size > 1);
    
	*x = *y = 0;
	line = PosToAbsLine(tw, pos);
	*y = line * line_height;
	line_exists = _XmCTLGetLine(tw, pos, &linestart, &next_linestart, &text, &text_len);
	if (!line_exists)
	 {
		return False;
	 }
	*x = _XmRenditionPosToEscapement(data->rendition, 0, text, is_wchar,
												pos - linestart,
												next_linestart - linestart,
												data->tabwidth, edge,
												tw->text.input->data->edit_policy,
												ISTEXT_RIGHTALIGNED(tw));
	return True;
}


/* The pre CTL PosToXY works without edge considerations */
static Boolean 
CTLPosToXY(XmTextWidget tw, XmTextPosition position, XmEDGE edge, Position *x, Position *y)
{
	OutputData		data=tw->text.output->data;
	LineNum			line;
	XmTextPosition	linestart;
	LineTableExtra	extra;
	XmTextBlockRec	block;
	Boolean			istext_rightaligned=ISTEXT_RIGHTALIGNED(tw);
    

	if (XmDirectionMatch(XmPrim_layout_direction(tw),
								XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
	 {
		_XmProcessLock();
		if ((tw == posToXYCachedWidget) && (position == posToXYCachedPosition))
		 {
			*x = posToXYCachedX;
			*y = posToXYCachedY;
			_XmProcessUnlock();
			return True;
		 }
		_XmProcessUnlock();
	
		line = _XmTextPosToLine(tw, position);
		if ((line == NOLINE) || (line >= data->number_lines))
		 {
			return False;
		 }
		*y = data->topmargin;
		*x = tw->text.inner_widget->core.width - (data->rightmargin +
						line * data->linewidth + data->linewidth * 0.5);
		_XmTextLineInfo(tw, line, &linestart, &extra);
		while (linestart < position)
		 {
			linestart = (*tw->text.source->ReadSource)(tw->text.source,
																	 linestart, position, 
																	 &block);
			*y += FindHeight(tw, *y, &block, 0, block.length);
		 }
		*y -= data->voffset;
    }
	else
	 {
		_XmProcessLock();
		if ((tw == posToXYCachedWidget) && (position == posToXYCachedPosition))
		 {
			*x = posToXYCachedX;
			*y = posToXYCachedY;
			_XmProcessUnlock();
			return True;
		 }
		_XmProcessUnlock();
	
		line = _XmTextPosToLine(tw, position);
		if ((line == NOLINE) || (line >= data->number_lines))
		 {
			return False;
		 }
		*y = data->topmargin + line * data->lineheight + data->font_ascent;
		*x = data->leftmargin;

		_XmTextLineInfo(tw, line, &linestart, &extra);
		if (TextW_LayoutActive(tw))
		 {
			XmTextPosition	nextlinestart;
			int				line_width=0, line_pos_x, right_edge;
	    
			istext_rightaligned = ISTEXT_RIGHTALIGNED(tw);
	    	_XmTextLineInfo(tw, line + 1, &nextlinestart, NULL);
	    	if (nextlinestart == PASTENDPOS)
			 {
				nextlinestart = tw->text.last_position;
			 }
	    
			if (nextlinestart > linestart)		/* if zero-length, no need to	*/
			 {												/*	measure							*/
				(*tw->text.source->ReadSource)(tw->text.source, linestart, nextlinestart, &block);
			 }
			else
			 {
				block.length = 0;						/* (Checked a few lines down)	*/
			 }
	    

			/* Now measure to our desired position */

			right_edge = (((int)tw->text.inner_widget->core.width) -
										data->rightmargin);

			/* right edge is the x on the drawing window beyond	*/
			/*	which the text is clipped on the right side.			*/

			line_pos_x =  FindPosition(tw, 0, &block, position - linestart, edge);
			if (istext_rightaligned)
			 {
				line_width = FindWidth(tw, 0, &block, 0, block.length);
				*x = (int)tw->text.inner_widget->core.width - data->rightmargin -
									((line_width - line_pos_x) - (data->hoffset));
				*x += data->hoffset;				/* to take care of x-= data->hoffset */
			 } 
			else
			 {
				*x += line_pos_x;
			 }
		 } 
		else
		 {
			while (linestart < position) 
			 {
				linestart = (*tw->text.source->ReadSource)(tw->text.source,
																		 linestart, position, 
																		 &block);
				*x += FindWidth(tw, *x, &block, 0, block.length);
			 }
		 }
		*x -= data->hoffset;
    }
	_XmProcessLock();
	posToXYCachedWidget = tw;
	posToXYCachedPosition = position;
	posToXYCachedX = *x;
	posToXYCachedY = *y;
	_XmProcessUnlock();
	return True;
}
#endif /* CTL */


/*****************************************************************************
 * To make TextOut a true "Object" this function should be a class function. *
 *****************************************************************************/
XmTextPosition 
_XmTextFindLineEnd(XmTextWidget tw, XmTextPosition position, LineTableExtra *extra)
{
	OutputData		data=tw->text.output->data;
	XmTextPosition	lastChar, lineEnd, nextLeft, nextBreak, lastBreak, oldpos;
	XmTextPosition	startpos=0;
	XmTextBlockRec	block;
	int				x, lastX, goalwidth,
						y, lastY, goalheight;
	long				length, i;
	int				num_bytes = 0;
  

	lastChar = (*tw->text.source->Scan)(tw->text.source, position,
													XmSELECT_LINE, XmsdRight, 1, False);
	lastBreak = startpos = position;
  
	if(XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
	 {
		y = lastY = data->topmargin;
		goalheight = tw->text.inner_widget->core.height - data->bottommargin;
		while (position < lastChar)
		 {
			nextLeft = (*tw->text.source->Scan)(tw->text.source, position,
															XmSELECT_WHITESPACE, XmsdRight,
															1, False);
			nextBreak = (*tw->text.source->Scan)(tw->text.source, nextLeft,
															 XmSELECT_WHITESPACE, XmsdRight,
															 1, True);
			while (position < nextLeft)
			 {
				position = (*tw->text.source->ReadSource)(tw->text.source,
																		position, nextLeft, &block);
				length = block.length;
				y += FindHeight(tw, y, &block, 0, block.length);
				if (y > goalheight)
				 {
					if (lastBreak > startpos)
					 {
						if (lastY <= goalheight) /* word wrap is being performed */
						 {
							return lastBreak;
						 }
						y = lastY;
						oldpos = position = lastBreak;
						while ((y > goalheight) && (position > startpos))
						 {
							oldpos = position;
							position = (*tw->text.source->Scan)(tw->text.source, position,
																			XmSELECT_POSITION,
																			XmsdLeft, 1, True);
							(*tw->text.source->ReadSource)(tw->text.source, position,
																	 oldpos, &block);
							num_bytes = mblen(block.ptr, (int)tw->text.char_size);
							if (num_bytes < 0)
							 {
								num_bytes = 1;
							 }
							y -= FindHeight(tw, y, &block, 0, num_bytes);
						 }

						if (extra)
						 {
							*extra = (LineTableExtra)XtMalloc(sizeof(LineTableExtraRec));
							(*extra)->wrappedbychar = True;
							(*extra)->width = 0;
						 }

						return oldpos;			/* (Allows one whitespace char to	*/
													/*	 appear partially off the edge.)	*/
					 }

					if (extra)
					 {
						*extra = (LineTableExtra)XtMalloc(sizeof(LineTableExtraRec));
						(*extra)->wrappedbychar = True;
						(*extra)->width = 0;
					 }

					if ((int)tw->text.char_size == 1)
					 {
						for (i=length - 1; i>=0 && y>goalheight; i--)
						 {
							y -= FindHeight(tw, y, &block, i, i + 1);
							position = (*tw->text.source->Scan)(tw->text.source,
																			position, XmSELECT_POSITION,
																			XmsdLeft, 1, True);
						 }
						return position;
					 }
					else
					 {
						char				tmp_cache[200];
						wchar_t *		tmp_wc;
						size_t			tmp_wc_size;
						char				tmp_char[MB_LEN_MAX];
						long				num_chars=0, org_chars=0; 
						XmTextBlockRec	mini_block;
	  
						/* If 16-bit data, convert the char* to wchar_t*... this	*/
						/*	allows us to scan backwards through the text one		*/
						/*	character at a time.  Without wchar_t, we would have	*/
						/*	to continually scan from the start of the string to	*/
						/*	find the byte offset of character n-1.						*/

						mini_block.ptr = tmp_char;
						org_chars = num_chars = TextCountCharacters((Widget)tw, block.ptr, block.length);
						tmp_wc_size = (num_chars + 1) * sizeof(wchar_t);
						tmp_wc = (wchar_t *) XmStackAlloc(tmp_wc_size, tmp_cache);
						num_chars = mbstowcs(tmp_wc, block.ptr, num_chars);
						if (num_chars < 0)
						 {
							num_chars = _Xm_mbs_invalid(tmp_wc, block.ptr, org_chars); /* fix for bug 4277497 - leob */
							if (num_chars > 0)
							 {
								for (i=num_chars - 1; i>=0 && y>goalheight; i--)
								 {
									mini_block.length = wctomb(mini_block.ptr, tmp_wc[i]);
									if (mini_block.length < 0)
									 {
										mini_block.length = 1;
										mini_block.ptr[0] = tmp_wc[i];
									 }
									y -= FindHeight(tw, y, &mini_block, 0, mini_block.length);
									position = (*tw->text.source->Scan)(tw->text.source,
																					position, 
																					XmSELECT_POSITION,
																					XmsdLeft, 1, True);
								 }
							 }
						 }

						XmStackFree((char*)tmp_wc, tmp_cache);
					 }											/*	end multi-byte handling	*/
					return position;
				 }
			 }

			while (position < nextBreak)
			 {
				position = (*tw->text.source->ReadSource)(tw->text.source,
																		position, nextBreak, &block);
				length = block.length;
				y += FindHeight(tw, y, &block, 0, block.length);
			 }
			lastBreak = nextBreak;
			lastY = y;
		 }
	 }
	else
	 {
  		x = lastX = data->leftmargin;
		goalwidth = tw->text.inner_widget->core.width - data->rightmargin;
		while (position < lastChar)
		 {
			nextLeft = (*tw->text.source->Scan)(tw->text.source, position,
															XmSELECT_WHITESPACE, XmsdRight,
															1, False);
			nextBreak = (*tw->text.source->Scan)(tw->text.source, nextLeft,
															 XmSELECT_WHITESPACE, XmsdRight,
															 1, True);
			while (position < nextLeft)
			 {
				position = (*tw->text.source->ReadSource)(tw->text.source,
																		position, nextLeft, 
																		&block);
				length = block.length;
				x += FindWidth(tw, x, &block, 0, block.length);
				if (x > goalwidth)
				 {
					if (lastBreak > startpos)
					 {
						if (lastX <= goalwidth)		/* word wrap is being performed */
						 {
							return lastBreak;
						 }
						x = lastX;
						oldpos = position = lastBreak;
						while ((x > goalwidth) && (position > startpos))
						 {
							oldpos = position;
							position = (*tw->text.source->Scan)(tw->text.source,
																			position, XmSELECT_POSITION,
																			XmsdLeft, 1, True);
							(*tw->text.source->ReadSource)(tw->text.source, position, oldpos, &block);
																	 num_bytes = mblen(block.ptr, 
																	 (int)tw->text.char_size);
							if (num_bytes < 0)
							 {
								num_bytes = 1;
							 }
							x -= FindWidth(tw, x, &block, 0, num_bytes);
						 }
						if (extra)
						 {
							*extra = (LineTableExtra)XtMalloc(sizeof(LineTableExtraRec));
							(*extra)->wrappedbychar = True;
							(*extra)->width = 0;
						 }
						return oldpos;				/* Allows one whitespace char to	*/
														/*	appear partially off the edge.*/
					 }
					if (extra)
					 {
						*extra = (LineTableExtra)XtMalloc(sizeof(LineTableExtraRec));
						(*extra)->wrappedbychar = True;
						(*extra)->width = 0;
					 }

					if ((int)tw->text.char_size == 1)
					 {
#ifdef SUN_CTL
						if (TextW_LayoutActive(tw))
						 {
							goalwidth = tw->text.inner_widget->core.width -
												data->rightmargin - data->leftmargin;
						 }
#endif /* CTL */
						for (i=length - 1; i>=0 && x>goalwidth; i--)
						 {
#ifdef SUN_CTL
							if (TextW_LayoutActive(tw))
							 {
								x = FindWidth(tw, x, &block, 0, i);
							 }
							else
#endif /* CTL */
							x -= FindWidth(tw, x, &block, i, i + 1);
							position = (*tw->text.source->Scan)(tw->text.source,
																			position, XmSELECT_POSITION,
																			XmsdLeft, 1, True);
						 }

						return position;
					 }
					else
					 {
						char				tmp_cache[200];
						wchar_t *		tmp_wc;
						long				tmp_wc_size;
						char				tmp_char[MB_LEN_MAX];
						long				num_chars=0, org_chars=0; 
						XmTextBlockRec	mini_block;
	  
						mini_block.ptr = tmp_char;
						org_chars = num_chars = TextCountCharacters((Widget)tw, block.ptr, block.length);
						tmp_wc_size = (num_chars + 1) * sizeof(wchar_t);
						tmp_wc = (wchar_t *) XmStackAlloc(tmp_wc_size, tmp_cache);
						num_chars = mbstowcs(tmp_wc, block.ptr, (int)num_chars);
						if (num_chars < 0)
						 {
							num_chars = _Xm_mbs_invalid(tmp_wc, block.ptr, org_chars);  /* fix for bug 4277497 - leob */
						 }
						if (num_chars > 0)
						 {
							for (i=num_chars - 1; i>=0 && x>goalwidth; i--)
							 {
								mini_block.length = wctomb(mini_block.ptr, tmp_wc[i]);
								if (mini_block.length < 0)
								 {
									mini_block.length = 1;
									mini_block.ptr[0] = tmp_wc[i];
								 }
								x -= FindWidth(tw, x, &mini_block, 0, mini_block.length);
#ifdef SUN_CTL
								if (TextW_LayoutActive (tw))
								 {
									position = _XmTextVisualScan(tw->text.source, position, XmSELECT_CELL, XmsdLeft, 1, True);
								 }
								else
#endif /* CTL */
								position = (*tw->text.source->Scan)(tw->text.source,
																				position, XmSELECT_POSITION,
																				XmsdLeft, 1, True);
							 }
						 }

						XmStackFree((char*)tmp_wc, tmp_cache);
					 }											/* end multi-byte handling	*/

					return position;
				 }
			 }

			while (position < nextBreak)
			 {
				position = (*tw->text.source->ReadSource)(tw->text.source,
																		position, nextBreak,
																		&block);
				length = block.length;
				x += FindWidth(tw, x, &block, 0, block.length);
			 }
			lastBreak = nextBreak;
			lastX = x;
		 }
	 }

	lineEnd = (*tw->text.source->Scan)(tw->text.source, lastChar,
												  XmSELECT_LINE, XmsdRight, 1, True);
	if (lineEnd != lastChar)
	 {
		return lineEnd;
	 }
	else
	 {
		return PASTENDPOS;
	 }
}


static XtGeometryResult 
TryResize(XmTextWidget tw, Dimension width, Dimension height)
{
	XtGeometryResult	result;
	Dimension			origwidth=tw->text.inner_widget->core.width,
							origheight=tw->text.inner_widget->core.height;
	XtWidgetGeometry	request, reply;
  
	if (origwidth != width)
	 {
		request.request_mode = CWWidth;
		request.width = width;
	 }
	else
	 {
		request.request_mode = (XtGeometryMask)0;
	 }

	if (origheight != height)
	 {
		request.request_mode |= CWHeight;
		request.height = height;
	 }
  
	if (request.request_mode == (XtGeometryMask)0)
	 {															/* requesting current size */
		return XtGeometryNo;
	 }

	result = XtMakeGeometryRequest(tw->text.inner_widget, &request, &reply);
  
	if (result == XtGeometryAlmost)
	 {
		if (request.request_mode & CWWidth)
		 {
			request.width = reply.width;
		 }
		if (request.request_mode & CWHeight)
		 {
			request.height = reply.height;
		 }
    
		result = XtMakeGeometryRequest(tw->text.inner_widget, &request, &reply);
		if (result == XtGeometryYes)
		 {
			result = XtGeometryNo;
			if (((request.request_mode & CWWidth) && (reply.width != origwidth))
					||
				 ((request.request_mode & CWHeight) && (reply.height != origheight)))
			 {
				result = XtGeometryYes;
			 }
		 }

		return result;
	 }
  
	if (result == XtGeometryYes)				/*	(Because Some brain damaged	*/
														/*	geometry managers return		*/
														/*	XtGeometryYes and don't			*/
	 {													/*	change the widget's size.)		*/
		if (((request.request_mode & CWWidth) &&
			  (tw->text.inner_widget->core.width != width)) ||
			 ((request.request_mode & CWHeight) &&
			  (tw->text.inner_widget->core.height != height)) ||
			 ((request.request_mode == (CWWidth & CWHeight)) &&
			  ((tw->text.inner_widget->core.width == origwidth) &&
			  (tw->text.inner_widget->core.height == origheight))))
		 {
			result = XtGeometryNo;
		 }
	 }

	return result;
}


void 
_XmRedisplayHBar(XmTextWidget tw)
{
	OutputData				data=tw->text.output->data;
	int						value, sliderSize, maximum, new_sliderSize;
	XmNavigatorDataRec	nav_data;
	XmNavigatorTrait		nav_trait;

	if (!(data->scrollhorizontal && XmIsScrolledWindow(XtParent(tw))) ||
		 data->suspend_hoffset || (tw->text.disable_depth != 0) ||
		 tw->core.being_destroyed || (data->hbar == NULL))
	 {
		return;
	 }
  
	ChangeHOffset(tw, data->hoffset, False);		/* Makes sure that hoffset	*/
																/*	still reasonable.			*/
  
	new_sliderSize = tw->text.inner_widget->core.width - (data->leftmargin + data->rightmargin);

  	if (new_sliderSize < 1)
	 {
		new_sliderSize = 1;
	 }
	if (new_sliderSize > data->scrollwidth)
	 {
		new_sliderSize = data->scrollwidth;
	 }
  
	nav_data.valueMask = NavValue|NavSliderSize|NavMaximum;
	nav_trait = (XmNavigatorTrait)XmeTraitGet((XtPointer)XtClass(data->hbar), XmQTnavigator);
	if (nav_trait)
	 {
		nav_trait->getValue(data->hbar, &nav_data);
		maximum = nav_data.maximum.x;
		sliderSize = nav_data.slider_size.x;
		value = nav_data.value.x;
	 }
	else
	 {
		return;
	 }
  
	if (((maximum != data->scrollwidth) || 
		  (value != data->hoffset) || 
		  (sliderSize != new_sliderSize)) &&
		 !((sliderSize == maximum) && (new_sliderSize == data->scrollwidth)))
	 {
    	data->ignorehbar = True;
		nav_data.value.x = data->hoffset;
		nav_data.minimum.x = 0;
		nav_data.maximum.x = data->scrollwidth;
		nav_data.slider_size.x = new_sliderSize;
		nav_data.increment.x = 0;							/* increments stay at current values */
		nav_data.page_increment.x = new_sliderSize;

		nav_data.dimMask = NavigDimensionX;
		nav_data.valueMask = NavValue|NavMinimum|NavMaximum|
									NavSliderSize|NavIncrement|NavPageIncrement;
		_XmSFUpdateNavigatorsValue(XtParent((Widget)tw), &nav_data, True); 
    
		data->ignorehbar = False;
	 }
}


void 
_XmRedisplayVBar(XmTextWidget tw)
{
	OutputData				data=tw->text.output->data;
	int						value=0, sliderSize=0,
								maximum=0, new_sliderSize=0;
	XmNavigatorDataRec	nav_data;
	XmNavigatorTrait		nav_trait;


	if (!(data->scrollvertical && XmIsScrolledWindow(XtParent(tw))) ||
		 data->suspend_voffset || (tw->text.disable_depth != 0) ||
		 tw->core.being_destroyed || (data->vbar == NULL))
	 {
		return;
	 }
  
	ChangeVOffset(tw, data->voffset, False);		/* Makes sure that voffset	*/
																/*	is still reasonable.		*/
	new_sliderSize = tw->text.inner_widget->core.height -
								(data->topmargin + data->bottommargin);
	if (new_sliderSize < 1)
	 {
		new_sliderSize = 1;
	 }
	if (new_sliderSize > data->scrollheight)
	 {
		new_sliderSize = data->scrollheight;
	 }
  
	nav_data.valueMask = NavValue|NavSliderSize|NavMaximum;
	nav_trait = (XmNavigatorTrait)XmeTraitGet((XtPointer)XtClass(data->vbar), XmQTnavigator);
	if (nav_trait)
	 {
		nav_trait->getValue(data->vbar, &nav_data);
		maximum = nav_data.maximum.y;
		sliderSize = nav_data.slider_size.y;
		value = nav_data.value.y;
	 }
	else
	 {
		return;
	 }
  
	if (((maximum != data->scrollheight) || 
		  (value != data->voffset) || 
		  (sliderSize != new_sliderSize)) &&
		 !((sliderSize == maximum) && (new_sliderSize == data->scrollheight)))
	 {
		data->ignorehbar = True;

		nav_data.value.y = data->voffset;
		nav_data.minimum.y = 0;
		nav_data.maximum.y = data->scrollheight;
		nav_data.slider_size.y = new_sliderSize;
		nav_data.increment.y = 0;							/* increments stay at current values */
		nav_data.page_increment.y = new_sliderSize;

		nav_data.dimMask = NavigDimensionY;
		nav_data.valueMask = NavValue|NavMinimum|NavMaximum|
									NavSliderSize|NavIncrement|NavPageIncrement;
		_XmSFUpdateNavigatorsValue(XtParent((Widget)tw), &nav_data, True); 
    
		data->ignorehbar = False;
	 }
}


static int 
CountLines(XmTextWidget tw, XmTextPosition start, XmTextPosition end)
{
	XmTextLineTable	line_table;
	unsigned int		t_index, max_index=0;
	int					numlines=0;
  

	line_table = tw->text.line_table;
	t_index = tw->text.table_index;
	max_index = tw->text.total_lines - 1;
  
	if (line_table[t_index].start_pos < TextPosToUInt(start))
	 {														/* look forward to find the	*/
															/*	current record.				*/
		while ((t_index <= max_index) &&
				 (line_table[t_index].start_pos < TextPosToUInt(start)))
		 {
			t_index++;
		 }
	 }
	else													/*	Else look backward for it.	*/
	 {
		while (t_index && (line_table[t_index].start_pos > TextPosToUInt(start)))
		 {
			t_index--;
		 }
	 }
  
	while(line_table[t_index].start_pos < end)
	 {
		t_index++;
		numlines++;
	 }
  
	return numlines;
}


void 
_XmChangeVSB(XmTextWidget tw)
{
	OutputData				data=tw->text.output->data;
	int						local_total, new_size;
	XmNavigatorDataRec	nav_data;
  
	if (tw->text.disable_depth != 0)
	 {
		return;
	 }
	if (tw->core.being_destroyed)
	 {
		return;
	 }
  
	if (!tw->text.top_character)
	 {
		tw->text.top_line = 0;
	 }
	else 
	 {
		tw->text.top_line = _XmTextGetTableIndex(tw, tw->text.top_character);
	 }
  
	if (tw->text.top_line > tw->text.total_lines)
	 {
		tw->text.top_line = tw->text.total_lines;
	 }
  
	if (tw->text.top_line + tw->text.number_lines > tw->text.total_lines)
	 {
		local_total = tw->text.top_line + tw->text.number_lines;
	 }
	else
	 {
		local_total = tw->text.total_lines;
	 }
  
	if (data->vbar)
	 {
		if (local_total >= tw->text.number_lines)
		 {
			new_size = tw->text.number_lines;
		 }
		else
		 {
			new_size = local_total;
		 }

		if (new_size + tw->text.top_line > local_total)
		 {
			new_size = local_total - tw->text.top_line;
		 }
    
		data->ignorevbar = True;

		nav_data.value.y = tw->text.top_line;
		nav_data.minimum.y = 0;
		nav_data.maximum.y = local_total;
		nav_data.slider_size.y = new_size;
		nav_data.increment.y = 0;
		nav_data.page_increment.y = (data->number_lines > 1) ?
												(data->number_lines - 1) : 1;
    
		nav_data.dimMask = NavigDimensionY;
		nav_data.valueMask = NavValue|NavMinimum|NavMaximum|
									NavSliderSize|NavIncrement|NavPageIncrement;
		_XmSFUpdateNavigatorsValue(XtParent((Widget)tw), &nav_data, True);
		data->ignorevbar = False;
	 }
}


void 
_XmChangeHSB(XmTextWidget tw)
{
	OutputData				data=tw->text.output->data;
	int						local_total=0, new_size=0,
								offset=0;
	XmNavigatorDataRec	nav_data;
  

	if (tw->text.disable_depth != 0)
	 {
		return;
	 }
	if (tw->core.being_destroyed)
	 {
		return;
	 }
  
	if (!tw->text.top_character)
	 {
		tw->text.top_line = 0;
	 }
	else 
	 {
		tw->text.top_line = _XmTextGetTableIndex(tw, tw->text.top_character);
	 }
  
	if (tw->text.top_line > tw->text.total_lines)
	 {
		tw->text.top_line = tw->text.total_lines;
	 }
  
	if (tw->text.top_line + tw->text.number_lines > tw->text.total_lines)
	 {
		local_total = tw->text.top_line + tw->text.number_lines;
	 }
	else
	 {
		local_total = tw->text.total_lines;
	 }
  
	if (data->hbar)
	 {
		if (local_total >= tw->text.number_lines)
		 {
			new_size = tw->text.number_lines;
		 }
		else
		 {
			new_size = local_total;
		 }
    
		if (new_size + tw->text.top_line > local_total)
		 {
			new_size = local_total - tw->text.top_line;
		 }
    
		data->ignorehbar = True;
		offset = local_total - (tw->text.number_lines + tw->text.top_line);
		nav_data.value.x = tw->text.top_line;
		nav_data.minimum.x = 0;
		nav_data.maximum.x = local_total;
		nav_data.slider_size.x = new_size;
		nav_data.increment.x = 0;
		nav_data.page_increment.x = (data->number_lines > 1)?
      (data->number_lines - 1): 1;
    
		nav_data.dimMask = NavigDimensionX;
		nav_data.valueMask = NavValue|NavMinimum|NavMaximum|
									NavSliderSize|NavIncrement|NavPageIncrement;
		_XmSFUpdateNavigatorsValue(XtParent((Widget)tw), &nav_data, True);
		data->ignorehbar = False;
	 }
}


static void 
TextFindNewWidth(XmTextWidget tw, Dimension *widthRtn)
{
	OutputData		data=tw->text.output->data;
	XmTextPosition	start;
	Dimension		newwidth;
  

	newwidth = 0;
  	if(XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
	 {
		XmTextPosition		first_position=0;
		LineTableExtra		extra;

		newwidth = (int)(tw->text.total_lines * data->linewidth) +
							data->leftmargin + data->rightmargin;
  
		_XmTextLineInfo(tw, (LineNum) 0, &start, &extra);
  
		if (start > 0)
		 {
			first_position = (*tw->text.source->Scan)(tw->text.source, start,
																	XmSELECT_ALL, XmsdLeft,
																	1, True);
			if (start > first_position)
			 {
				_XmTextSetTopCharacter((Widget)tw, start);
				return;
			 }
		 }
	 }
	else
	 {
		if (data->resizeheight && (tw->text.total_lines > data->number_lines))
		 {
			int				i;
			XmTextPosition	linestart, position;
			Dimension		text_width;
			XmTextBlockRec	block;
    
			i = _XmTextGetTableIndex(tw, tw->text.top_character);
			for (linestart = tw->text.top_character;
				  (i+1) < tw->text.total_lines; i++)
			 {
				text_width = data->leftmargin;
				position = tw->text.line_table[i + 1].start_pos - 1;
				while (linestart < position)
				 {
					linestart = (*tw->text.source->ReadSource)(tw->text.source,
																			 linestart, position,
																			 &block);
					text_width += FindWidth(tw, text_width, &block, 0, block.length);
				 }
				text_width += data->rightmargin;
				if (text_width > newwidth)
				 {
					newwidth = text_width;
				 }
			 }
			text_width = data->leftmargin;
			position = tw->text.last_position;
			while (linestart < position)
			 {
				linestart = (*tw->text.source->ReadSource)(tw->text.source,
																		 linestart, position,
																		 &block);
				text_width += FindWidth(tw, text_width, &block, 0, block.length);
			 }
			text_width += data->rightmargin;
			if (text_width > newwidth)
			 {
				newwidth = text_width;
			 }
		 }
		else
		 {
			LineNum				l;
			LineTableExtra		extra;
    
			for (l=0; l<data->number_lines; l++)
			 {
				_XmTextLineInfo(tw, l, &start, &extra);
				if (extra && newwidth < extra->width)
				 {
					newwidth = extra->width;
				 }
			 }
		 }
	 }
  
	*widthRtn = newwidth;
}


/*ARGSUSED*/
static void 
TextFindNewHeight(XmTextWidget tw, XmTextPosition position, Dimension *heightRtn)
{
	OutputData			data=tw->text.output->data;
	XmTextPosition		first_position, start;
	LineTableExtra		extra;
  

	Dimension newheight = 0;

	if(XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
	 {
		if (data->resizeheight && (tw->text.total_lines > data->number_lines))
		 {
			int					i;
			XmTextPosition		linestart, pos;
			Dimension			text_height;
			XmTextBlockRec		block;
    
			i = _XmTextGetTableIndex(tw, tw->text.top_character);
			for (linestart = tw->text.top_character;
				  (i+1) < tw->text.total_lines; i++)
			 {
				text_height = data->topmargin;
				pos = tw->text.line_table[i + 1].start_pos - 1;
				while (linestart < pos)
				 {
					linestart = (*tw->text.source->ReadSource)(tw->text.source,
																			 linestart, pos, &block);
					text_height += FindHeight(tw, text_height, &block, 0, block.length);
				 }
				text_height += data->bottommargin;
				if (text_height > newheight)
				 {
					newheight = text_height;
				 }
			 }
			text_height = data->topmargin;
			pos = tw->text.last_position;
			while (linestart < pos)
			 {
				linestart = (*tw->text.source->ReadSource)(tw->text.source,
																		 linestart, pos, &block);
				text_height += FindHeight(tw, text_height, &block, 0, block.length);
			 }
			text_height += data->bottommargin;
			if (text_height > newheight)
			 {
				newheight = text_height;
			 }
		 }
		else
		 {
			LineTableExtra	extra;
			LineNum			l;
    
			for (l=0; l<data->number_lines; l++)
			 {
				_XmTextLineInfo(tw, l, &start, &extra);
				if (extra && newheight < extra->width)
				 {
					newheight = extra->width;
				 }
			 }
		 }
		*heightRtn = newheight;
	 }
	else
	 {
		*heightRtn = tw->text.total_lines * data->lineheight +
							data->topmargin + data->bottommargin;
  		_XmTextLineInfo(tw, (LineNum) 0, &start, &extra);
		if (start > 0)
		 {
			first_position = (*tw->text.source->Scan)(tw->text.source, start,
																	XmSELECT_ALL, XmsdLeft, 1, True);
			if (start > first_position)
			 {
				_XmTextSetTopCharacter((Widget)tw, start);
				return;
			 }
		 }
	 }
}


static void 
CheckForNewSize(XmTextWidget tw, XmTextPosition position)
{
	OutputData	data=tw->text.output->data;
	Dimension	newwidth, newheight;
  
	if(XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
	 {
		if (data->scrollhorizontal &&
			 XmIsScrolledWindow(XtParent(tw)) &&
			 !tw->text.hsbar_scrolling)
		 {
			_XmChangeHSB(tw);
		 }
	 }
	else
	 {
		if (data->scrollvertical &&
			 XmIsScrolledWindow(XtParent(tw)) &&
			 !tw->text.vsbar_scrolling)
		 {
			_XmChangeVSB(tw);
		 }
	 }
  
	if (tw->text.in_resize || tw->text.in_expose)
	 {
		if(XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
		 {
			if (data->scrollvertical &&
				 XmIsScrolledWindow(XtParent(tw)))
			 {
				TextFindNewHeight(tw, position, &newheight);
				newheight -= (data->bottommargin + data->topmargin);
				if ((newheight != data->scrollheight) &&
					 !data->suspend_voffset)
				 {
					if (newheight)
					 {
						data->scrollheight = newheight;
					 }
				 }
				else
				 {
					data->scrollheight = 1;
				 }
				_XmRedisplayVBar(tw);
			 }
		 }
		else
		 {
			if (data->scrollhorizontal &&
				 XmIsScrolledWindow(XtParent(tw)))
			 {
				TextFindNewWidth(tw, &newwidth);
				newwidth -= (data->rightmargin + data->leftmargin);
				if (newwidth != data->scrollwidth &&
					 !data->suspend_hoffset)
				 {
					if (newwidth)
					 {
						data->scrollwidth = newwidth;
					 }
					else
					 {
						data->scrollwidth = 1;
					 }
					_XmRedisplayHBar(tw);
				 }
			 }
		 }
	 }
	else
	 {
		if(XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
		 {
			if (data->resizeheight || 
				 (data->scrollvertical &&
				  XmIsScrolledWindow(XtParent(tw))))
			 {
				TextFindNewHeight(tw, position, &newheight);
				if (data->scrollvertical &&
					 XmIsScrolledWindow(XtParent(tw)))
				 {
					newheight -= (data->bottommargin + data->topmargin);
					if (newheight != data->scrollheight &&
						 !data->suspend_voffset)
					 {
						if (newheight)
						 {
							data->scrollheight = newheight;
						 }
						else
						 {
							data->scrollheight = 1;
						 }
						_XmRedisplayVBar(tw);
					 }
					newheight = tw->text.inner_widget->core.height;
				 }
				else if ((newheight < data->minheight))
				 {
					newheight = data->minheight;
				 }
			 }
			else
			 {
				newheight = tw->text.inner_widget->core.height;
			 }
    
			newwidth = tw->text.inner_widget->core.width;
    
			if (data->resizewidth)
			 {
				TextFindNewWidth(tw, &newwidth);
				if (newwidth < data->minwidth)
				 {
					newwidth = data->minwidth;
				 }
			 }
		 }
		else
		 {
			if (data->resizewidth  || 
				 (data->scrollhorizontal &&
				  XmIsScrolledWindow(XtParent(tw))))
			 {
				TextFindNewWidth(tw, &newwidth);
				if (data->scrollhorizontal &&
					 XmIsScrolledWindow(XtParent(tw)))
				 {
					newwidth -= (data->rightmargin + data->leftmargin);
					if ((newwidth != data->scrollwidth) &&
						 !data->suspend_hoffset)
					 {
						if (newwidth)
						 {
							data->scrollwidth = newwidth;
						 }
						else
						 {
							data->scrollwidth = 1;
						 }
						_XmRedisplayHBar(tw);
					 }
					newwidth = tw->text.inner_widget->core.width;
				 }
				else if (newwidth < data->minwidth)
				 {
					newwidth = data->minwidth;
				 }
			 }
			else
			 {
				newwidth = tw->text.inner_widget->core.width;
			 }

			newheight = tw->text.inner_widget->core.height;
			if (data->resizeheight && !(data->scrollvertical &&
				 XmIsScrolledWindow(XtParent((Widget)tw))))
			 {
				TextFindNewHeight(tw, position, &newheight);
				if (newheight < data->minheight)
				 {
					newheight = data->minheight;
				 }
			 }
		 }

		if ((newwidth != tw->text.inner_widget->core.width) ||
			 (newheight != tw->text.inner_widget->core.height))
		 {
			if (tw->text.in_setvalues)
			 {
				tw->core.width = newwidth;
				tw->core.height = newheight;
			 }
			else
			 {
				if (TryResize(tw, newwidth, newheight) == XtGeometryYes)
				 {
					NotifyResized((Widget) tw, False);
				 }
				else
				 {
					tw->text.needs_refigure_lines = False;
				 }
			 }
		 }
	 }
}


static XtPointer
OutputBaseProc(Widget widget, XtPointer client_data)
{
	XmTextWidget	tw=(XmTextWidget)widget;
	XtPointer		ret_val;

	_XmProcessLock();
	ret_val = (XtPointer)tw->text.output;
	_XmProcessUnlock();
	return ret_val;
}


void
_XmTextOutputGetSecResData(XmSecondaryResourceData *secResDataRtn)
{
	XmSecondaryResourceData	secResData=XtNew(XmSecondaryResourceDataRec);
  

	_XmTransformSubResources(output_resources, XtNumber(output_resources),
									 &(secResData->resources), &(secResData->num_resources));
  
	secResData->name = NULL;
	secResData->res_class = NULL;
	secResData->client_data = NULL;
	secResData->base_proc = OutputBaseProc;
	*secResDataRtn = secResData;
}


/*****************************************************************************
 * To make TextOut a true "Object" this function should be a class function. *
 *****************************************************************************/
int
_XmTextGetNumberLines(XmTextWidget tw)
{
	OutputData	data=tw->text.output->data;

	return (data->number_lines);
}


/*****************************************************************************
 * To make TextOut a true "Object" this function should be a class function. *
 *****************************************************************************/
/* This routine is used to control foreground vs. background when moving
 * cursor position.  It ensures that when cursor position is changed
 * between "inside the selection" and "outside the selection", that the
 * correct foreground and background are used when "painting" the cursor
 * through the IBeam stencil.
 */
void
_XmTextMovingCursorPosition(XmTextWidget tw, XmTextPosition	position)
{
	OutputData			data=tw->text.output->data;
	_XmHighlightRec *	hl_list=tw->text.highlight.list;
	int					i;
  
	for (i=tw->text.highlight.number - 1; i>=0; i--)
	 {
		if (position >= hl_list[i].position)
		 {
			break;
		 }
	 }

	if (position == hl_list[i].position)
	 {
		if (data->have_inverted_image_gc)
		 {
			InvertImageGC(tw);
		 }
	 }
	else if (hl_list[i].mode != XmHIGHLIGHT_SELECTED)
	 {
		if (data->have_inverted_image_gc)
		 {
			InvertImageGC(tw);
		 }
	 }
	else if (!data->have_inverted_image_gc)
	 {
		InvertImageGC(tw);
	 }
}


static Boolean 
MeasureLine(XmTextWidget		tw,
				LineNum				line,
				XmTextPosition		position,
				XmTextPosition *	nextpos,
				LineTableExtra *	extra)
{
	OutputData		data=tw->text.output->data;
	XmTextPosition	temp, last_position;
	XmTextBlockRec	block;
	Dimension		width=0;
	Dimension		height=0;
  

	_XmProcessLock();
	posToXYCachedWidget = NULL;
	_XmProcessUnlock();
	if (extra)
	 {
		*extra = NULL;
	 }
	if (line >= data->number_lines)
	 {
		if (data->resizewidth || data->resizeheight ||
			 ((data->scrollvertical || data->scrollhorizontal) &&
			  XmIsScrolledWindow(XtParent(tw))))
		 {
			CheckForNewSize(tw, position);
		 }
		return False;
	 }

	if (nextpos)
	 {
		if (position == PASTENDPOS)
		 {
			*nextpos = last_position = PASTENDPOS;
		 }
		else
		 {
			if (ShouldWordWrap(data, tw))
			 {
				*nextpos = _XmTextFindLineEnd(tw, position, extra);
			 }
			else
			 {
				last_position = (*tw->text.source->Scan)(tw->text.source,
																	  position, XmSELECT_LINE,
																	  XmsdRight, 1, False);
				*nextpos = (*tw->text.source->Scan)(tw->text.source,
																last_position, XmSELECT_LINE,
																XmsdRight, 1, True);
				if (*nextpos == last_position)
				 {
					*nextpos = PASTENDPOS;
				 }
				if(XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
				 {
					if (extra && (data->resizeheight || (data->scrollvertical &&
																	 XmIsScrolledWindow(XtParent(tw)))))
					 {
						(*extra) = (LineTableExtra)XtMalloc(sizeof(LineTableExtraRec));
						(*extra)->wrappedbychar = False;
						height = data->topmargin;
						temp = position;
						while (temp < last_position)
						 {
							temp = (*tw->text.source->ReadSource)(tw->text.source, temp, last_position, &block);
							height += FindHeight(tw, (Position)width, &block,
														0, block.length);
						 }
						(*extra)->width = height + data->bottommargin;
					 }
				 }
				else
				 {
					if (extra && (data->resizewidth || (data->scrollhorizontal &&
																	XmIsScrolledWindow(XtParent(tw)))))
					 {
						(*extra) = (LineTableExtra)XtMalloc(sizeof(LineTableExtraRec));
						(*extra)->wrappedbychar = False;
						width = data->leftmargin;
						temp = position;
						while (temp < last_position)
						 {
							temp = (*tw->text.source->ReadSource)(tw->text.source, temp, last_position, &block);
							width += FindWidth(tw, (Position) width, &block,
													 0, block.length);
						 }
						(*extra)->width = width + data->rightmargin;
					 }
				 }
			 }
			if (*nextpos == position)
			 {
				*nextpos = (*tw->text.source->Scan)(tw->text.source, position,
																XmSELECT_POSITION, XmsdRight, 1, True);
			 }
		 }
	 }

	return True;
}


static int
color_contrast(XColor *c1, XColor *c2)
{
	int	contrast=0, tmp;


	tmp = abs((int)(c1->red / 256) - (int)(c2->red / 256));
   if (tmp > 25)
	 {
		contrast += tmp - 25;
	 }
   tmp = abs((int)(c1->green / 256) - (int)(c2->green / 256));
   if (tmp > 25)
	 {
		contrast += tmp - 25;
	 }
   tmp = abs((int)(c1->blue / 256) - (int)(c2->blue / 256));
   if (tmp > 25)
	 {
      contrast += tmp - 25;
	 }
   return contrast;
}


static int
too_bright(XColor *c)
{
   if (((c->red / 256 > 230) && (c->green / 256 > 230))
       || ((c->red / 256 > 230) && (c->blue / 256 > 230))
       || ((c->green / 256 > 230) && (c->blue / 256 > 230)))
	 {
      return 1;
    }
   else
	 {
      return 0;
    }
}


static void
change_color(XColor *old, XColor *new, int change)
{
   int	colormax=65535;


   new->red = old->red + change * 256;
   if (old->red + change * 256 > colormax)
	 {
       new->red = colormax;
	 }
   if (old->red + change * 256 < 0)
	 {
       new->red = 0;
	 }
   new->green = old->green + change * 256;
   if (old->green + change * 256 > colormax)
	 {
       new->green = colormax;
	 }
   if (old->green + change * 256 < 0)
	 {
       new->green = 0;
	 }
   new->blue = old->blue + change * 256;
   if (old->blue + change * 256 > colormax)
	 {
       new->blue = colormax;
	 }
   if (old->blue + change * 256 < 0)
	 {
       new->blue = 0;
	 }
}


static int
colormatch(XColor color, int red, int green, int blue)
{
   if ((abs(color.red / 256 - red) < 4) &&
      (abs(color.green / 256 - green) < 4) &&
      (abs(color.blue / 256 - blue) < 4))
      return 1;
   else
      return 0;
}


static int
palettecolors(XColor *colors, XColor *high1, XColor **high2)
{
   int i,j, match = 0;
   unsigned char pcolors[][18] = {
/* The first four RGB values are for the four color chips in each palette.
   The last two RGB values give the URL colors, or tell you which color
   chip to use.  */
/* Alpine */    {175,191,251, 120,170,184, 122,185,216, 191,192,197,
	102,102,255, 0,0,204},
/* Arizona */   {255,171,120, 168,107,121, 222,179,148, 136,87 ,112,
	0,0,0, 255,102,0},
/* BeigeRose */ {97 ,136,136, 209,192,174, 183,137,137, 240,236,235,
	0,0,0, 0,91,91},
/* Broica */    {237,168,112, 153,153,153, 137,152,170, 104,111,130,
	0,0,0, 255,102,0},
/* Cabernet */  {216,135,117, 99 ,138,122, 69 ,98 ,86,  75 ,94 ,97,
	0,0,0, 255,51,51},
/* Camouflag */ {172,190,180, 110,125,118, 144,123,123, 130,130,155,
	0,0,0, 0,102,102},
/* Charcoal */  {135,180,176, 118,114,121, 105,99 ,99,  137,109,109,
	204,255,255, 0,0,0},
/* Chocolate */ {169,169,255, 112,140,141, 135,135,175, 150,120,120,
	0,0,0, 102,0,204},
/* Cinnamon */  {177,75 ,80,  146,104,102, 121,110,109, 131,105,96,
	255,153,0, 255,102,0},
/* Clay */      {255,170,170, 150,130,130, 90 ,139,141, 140,100,100,
	0,0,0, 255,51,51},
/* Crimson */   {178,77 ,122, 174,178,195, 113,139,165, 255,247,233,
	0,0,2, 91,91,122},
/* DarkGold */  {184,150,99,  122,124,127, 83 ,85 ,112, 255,255,226,
	0,0,0, 102,0,0},
/* Delphiniu */ {133,196,237, 200,200,200, 133,140,217, 255,255,230,
	0,0,2, 51,0,153},
/* Desert */    {229,183,147, 194,187,167, 140,143,152, 253,253,250,
	0,0,0, 130,0,0},
/* Golden */    {250,173,73,  77 ,100,141, 190,130,111, 71 ,99 ,79,
	0,0,0, 204,102,0},
/* Grass */     {212,199,125, 190,198,183, 94 ,156,123, 226,243,226,
	0,0,2, 0,102,51},
/* Grayscale */ {195,195,195, 120,119,119, 189,189,189, 148,148,147,
	0,0,0, 102,102,102},
/* Lilac */     {211,209,120, 165,165,172, 166,146,183, 195,195,201,
	153,51,204, 51,51,153},
/* Mustard */   {213,128,46,  183,95 ,95,  77 ,100,141, 92 ,106,88,
	255,204,102, 0,0,0},
/* Neptune */   {162,229,198, 63 ,147,141, 45 ,78 ,118, 120,137,165,
	0,0,0, 0,102,102},
/* NorthernS */ {164,117,145, 27 ,93 ,108, 12 ,90 ,135, 0  ,87 ,122,
	255,153,153, 255,102,102},
/* Nutmeg */    {206,194,187, 37 ,101,147, 159,136,129, 205,178,166,
	0,0,1, 51,51,102},
/* Olive */     {131,131,90,  195,195,185, 135,121,135, 206,194,184,
	0,0,0, 51,102,0},
/* Orchid */    {185,185,255, 155,155,185, 168,168,205, 130,130,171,
	204,204,255, 0,0,2},
/* PBNJ */      {182,92 ,97,  217,193,178, 222,161,127, 241,227,210,
	0,0,2, 0,0,0},
/* Sand */      {234,190,147, 199,175,175, 142,162,193, 212,212,218,
	245,153,0, 204,51,0},
/* SantaFe */   {239,157,157, 126,171,171, 189,162,141, 206,175,154,
	255,102,102, 215,0,0},
/* Savannah */  {218,154,104, 180,170,148, 160,178,200, 210,191,168,
	102,102,255, 51,51,153},
/* SeaFoam */   {170,203,219, 167,168,167, 125,183,187, 151,176,178,
	102,102,255, 51,51,153},
/* SkyRed */    {204,104,111, 175,192,194, 109,167,180, 225,235,225,
	0,0,2, 0,51,140},
/* SoftBlue */  {127,169,235, 118,169,156, 91 ,133,208, 108,108,108,
	204,255,204, 0,0,1},
/* SouthWest */ {255,125,121, 169,117,97,  192,155,128, 177,136,133,
	0,0,0, 153,0,51},
/* Summer */    {255,86 ,100, 255,214,144, 130,159,255, 255,255,255,
	0,0,2, 51,51,153},
/* Tundra */    {241,188,127, 165,150,128, 124,146,185, 118,126,99,
	255,255,204, 0,0,0},
/* Urchin */    {213,165,145, 15 ,124,122, 112,104,137, 12 ,104,100,
	0,0,0, 204,102,51},
/* Wheat */     {209,165,122, 219,194,145, 72, 109,127, 191,192,197,
	51,153,255, 0,0,2},
/* END */       {0,0,0, 0,0,0, 0,0,0, 0,0,0}};

   for (i=0; !(match || ((pcolors[i][0] == 0) && (pcolors[i][1] == 0))); i++)
	 {
      for (j=0; j<12; j+=3)
		 {
			match += colormatch(colors[j/3], pcolors[i][j],
									  pcolors[i][j+1], pcolors[i][j+2]);
		 }
		if (match != 4)
		 {
			match = 0;
		 }
	 }

	i--;

	if (match)
	 {
		match = 4;
		if ((pcolors[i][12] == 0) && (pcolors[i][13] == 0) &&
			 (pcolors[i][14] < 8))
		 {
			*high1 = colors[pcolors[i][14]];
		 }
		else
		 {
			high1->red = pcolors[i][12] * 256;
			high1->green = pcolors[i][13] * 256;
			high1->blue = pcolors[i][14] * 256;
			match = 1; /* Allocate url color 1 */
		 }
		if (high2)
		 {
			*high2 = (XColor *) XtMalloc(sizeof(XColor));
			if ((pcolors[i][15] == 0) && (pcolors[i][16] == 0)
				 && (pcolors[i][17] < 8))
			 {
				**high2 = colors[pcolors[i][17]];
			 }
			else
			 {
				(*high2)->red = pcolors[i][15] * 256;
				(*high2)->green = pcolors[i][16] * 256;
				(*high2)->blue = pcolors[i][17] * 256;
				if (match == 1)
				 {
					match = 3; /* Allocate both url colors */
				 }
				else
				 {
					match = 2; /* Allocate url color 2 */
				 }
			 }
		 }
	 }

   return match;
}
	 

static Boolean
_FontStructPerCharExtents(XmTextWidget		tw,
								  char *				str,
								  int					length,
								  XCharStruct *	overall)
{
	OutputData		data=tw->text.output->data;
	XFontStruct *	font=data->font;
	unsigned char	c;
	int				dummy;


	memset((char *)overall, 0x00, sizeof(XCharStruct));

	if(data->use_fontset)
	 {
		return False;
	 }

	if ((length <= 0) || (str == (char *)NULL))
	 {
		return True;
	 }

	if (tw->text.char_size != 1)
	 {
		if (length == 1)
		 {
			c = (unsigned char) *str;
			if (c == '\t')
			 {
				return True;
			 }
			else
			 {
				if (font->per_char && ((c >= font->min_char_or_byte2) &&
											  (c <= font->max_char_or_byte2)))
				 {
					overall->lbearing = font->per_char[c - font->min_char_or_byte2].lbearing;
					overall->rbearing = font->per_char[c - font->min_char_or_byte2].rbearing;
					overall->width = font->per_char[c - font->min_char_or_byte2].width;
				 }
				else
				 {
					overall->lbearing = font->min_bounds.lbearing;
					overall->rbearing = font->min_bounds.rbearing;
					overall->width = font->min_bounds.width;
				 }
				overall->ascent = font->max_bounds.ascent;
				overall->descent = font->max_bounds.descent;
			 }
		 }
		else
		 {
			XTextExtents(data->font, str, length, &dummy, &dummy, &dummy, overall);
		 }
	 }
	else
	 {
		c = (unsigned char) *str;
		if (c == '\t')
		 {
			return True;
		 }
		else
		 {
			if (font->per_char)
			 {
			 	if ((c >= font->min_char_or_byte2) &&
					 (c <= font->max_char_or_byte2))
				 {
					overall->lbearing = font->per_char[c - font->min_char_or_byte2].lbearing;
					overall->rbearing = font->per_char[c - font->min_char_or_byte2].rbearing;
					overall->width = font->per_char[c - font->min_char_or_byte2].width;
				 }
				else if ((font->default_char >= font->min_char_or_byte2) &&
							(font->default_char <= font->max_char_or_byte2))
				 {
					overall->lbearing = font->per_char[font->default_char -
													font->min_char_or_byte2].lbearing;
					overall->rbearing = font->per_char[font->default_char -
													font->min_char_or_byte2].rbearing;
					overall->width = font->per_char[font->default_char -
													font->min_char_or_byte2].width;
				 }
				else
				 {
					overall->lbearing = font->min_bounds.lbearing;
					overall->rbearing = font->min_bounds.rbearing;
					overall->width = font->min_bounds.width;
				 }
			 }
			else
			 {
				overall->lbearing = font->min_bounds.lbearing;
				overall->rbearing = font->min_bounds.rbearing;
				overall->width = font->min_bounds.width;
			 }
			overall->ascent = font->max_bounds.ascent;
			overall->descent = font->max_bounds.descent;
		 }
	 }

	return True;;
}


#ifdef SUN_CTL
static void _ClearLineArea(XmTextWidget		tw, 
									XmHighlightMode	hl_mode, 
									Position				x, 
									Position				y,
									size_t				erase_width)
{
	OutputData	data=tw->text.output->data;
	size_t		drawable_height=(size_t)(tw->text.inner_widget->core.height - data->bottommargin);
	size_t		erase_height;
    
	if ((erase_width > 0) && (data->lineheight > 0))
	 {
		if (hl_mode == XmHIGHLIGHT_SELECTED)
		 {
			SetNormGC(tw, data->gc, False, False);
		 }
		else
		 {
			SetInvGC(tw, data->gc);
		 }
	
		SetFullGC(tw, data->gc);
	
		if ((int) (y + data->font_descent) > drawable_height)
		 {
			erase_height = drawable_height - (y - data->font_ascent);
		 }
		else
		 {
			erase_height = data->font_ascent + data->font_descent;
		 }
	
		XFillRectangle(XtDisplay(tw), XtWindow(tw->text.inner_widget), data->gc,
							x, y - data->font_ascent, erase_width, erase_height);
		SetMarginGC(tw, data->gc);
	 }
}


/*********************************************************************/
/*                                                                   */
/* Function CTLDraw                                                  */
/*                                                                   */
/* Note that the assumption is that the data is a lineful, and no    */
/* more.                                                             */
/* Note also that caller must warrant that start and end are in      */
/* fact the start positions of line and line+1, respectively.        */
/*                                                                   */
/*********************************************************************/

static void
CTLDraw( XmTextWidget         tw,
         LineNum              line,
         XmTextPosition       start,
         XmTextPosition       end,
         _XmHighlightData *   hl_data
       )
{
   OutputData        data=tw->text.output->data;
   XmTextPosition    linestart=start;
   XmTextPosition    nextlinestart=end;
   XmHighlightMode   firsthighlight=hl_data->list[0].mode;
   XmHighlightMode   endhighlight=hl_data->list[hl_data->number-1].mode;
   int               maxy=(((int)tw->text.inner_widget->core.height) - data->bottommargin);

   if (linestart <= nextlinestart)        /* ignore start > end -- something's wrong   */
    {
      XmTextBlockRec block;
      int            x, y, length, newx, i;
      int            drawable_width=(int)(tw->text.inner_widget->core.width - data->rightmargin);
      int            rightedge=drawable_width + data->hoffset;
      Boolean        stipple=False;
      int            text_border = tw->primitive.shadow_thickness + tw->primitive.highlight_thickness;

      if (!XtIsRealized((Widget) tw))
         return;

      _XmTextAdjustGC(tw);

      if (!XtIsSensitive((Widget)tw))
         stipple = True;

      x = data->leftmargin;
      y = data->topmargin + line * data->lineheight + data->font_ascent - data->voffset;

      /* Get the text of the whole line, to calculate the alignment offset. */
      if ((y >= 0) && (y <= maxy))
       {
         if (linestart < nextlinestart)
          {
            (void)(*tw->text.source->ReadSource)(tw->text.source, linestart,
                                                 nextlinestart, &block);
          }
         else
          {
            block.length = 0;
          }

         if (block.length > 0)
          {
            int         line_width=0;
            Boolean     istext_rightaligned=ISTEXT_RIGHTALIGNED(tw);
            Boolean     is_wchar;
            char *      draw_text;
            int         count;
            wchar_t     tmp_cache[200];
            wchar_t *   tmp_wc;

            SetFullGC(tw, data->gc);

            if (istext_rightaligned)
             {
               line_width = FindWidth(tw, 0, &block, 0, block.length);
               x = data->hoffset + drawable_width - line_width;
               if (data->wordwrap)
                  x = (int)(tw->text.inner_widget->core.width - data->rightmargin) - line_width;

               if (x > data->leftmargin)        /* Erase any beginning (i.e. left side)   */
                {                               /* space.                                 */
                  _ClearLineArea(tw,  XmHIGHLIGHT_NORMAL, data->leftmargin, y, x - data->leftmargin);
                }
             }
            else                                /* Else let clipping deal with scrolling. */
             {
	        x -= data->hoffset;	
	       _ClearLineArea(tw, XmHIGHLIGHT_NORMAL , text_border, y, x -text_border); /* fix for bug 4280825 - moatazm*/
             }

            /* ReadSource() fills block with mbs.   But for our purposes we   */
            /* prefer wcs when char_size > 1 and rendition is XmFONT_IS_XOC,  */
            /* since layout transformation operates most efficiently on flat  */
            /* arrays.                                                        */

            is_wchar = (tw->text.char_size > 1);
            draw_text = block.ptr;
            count = block.length;

            if (is_wchar && _XmRendLayoutIsCTL(data->rendition))
             {
               if (nextlinestart == PASTENDPOS)
                {
                  XmSourceData srcData = tw->text.source->data;
                  nextlinestart = srcData->length;
                }
               count = nextlinestart - linestart;
               tmp_wc = (wchar_t*)XmStackAlloc(((count + 1) * sizeof(wchar_t)), tmp_cache);
               count = mbstowcs(tmp_wc, block.ptr, count);     /* block.ptr is not null-terminated */

               if (count == -1)
                  count = _Xm_mbs_invalid(tmp_wc, block.ptr, nextlinestart - linestart);  /* fix for bug 4277497 - leob */
               tmp_wc[count] = (wchar_t)0;

               draw_text = (char*)tmp_wc;
             }

            x = _XmRenditionDraw(data->rendition, (Widget)tw, data->gc, &tw->text, x, y,
                                 draw_text, count, is_wchar, tw->text.editable, False,
                                 hl_data, data->tabwidth, istext_rightaligned);

            if (is_wchar)
               XmStackFree((char*)tmp_wc, tmp_cache);
          }
         else                             /* redraw the line      */
	   {                              /* clear left margin, moatazm fix for bugID 4260422 */
            _ClearLineArea(tw,  XmHIGHLIGHT_NORMAL,text_border, y,
                           tw->text.inner_widget->core.width);
          }

         if (x < rightedge)
          {                               /* Erase any leftover space if there is any  */
            _ClearLineArea(tw, endhighlight, x, y, rightedge - x);
          }
       }
    }
   else
    {
      XmeWarning((Widget) tw, "TextOut.c:Draw(): linestart > nextlinestart");
    }
}
#endif /* CTL */


static void
#ifdef SUN_CTL
NONCTLDraw(XmTextWidget tw, /* another name for CTL version */
#else /* CTL */
Draw(XmTextWidget tw,
#endif /* CTL */
       LineNum line,
       XmTextPosition start,
       XmTextPosition end,
       XmHighlightMode highlight)
{
   OutputData        data=tw->text.output->data;
   XmTextPosition    linestart, nextlinestart;
   LineTableExtra    extra;
   XmTextBlockRec    block;
   int               x, y, length, newx, i;
   int               num_bytes = 0;
   int               text_border;
   int               rightedge=(((int)tw->text.inner_widget->core.width) -
                               data->rightmargin) + data->hoffset;
   int               width, height;
   int               rec_width=0;
   int               rec_height=0;
   Boolean           cleartoend, cleartobottom;
   Boolean           stipple=False;
   XmHighlightMode   endhighlight=highlight;
   int               bottomedge=(((int)tw->text.inner_widget->core.height) -
                                data->bottommargin) + data->voffset;
   int               win_width=0;
   int               newy=0;
   int               charheight=data->font_ascent + data->font_descent;

   /* Sun highlight callbacks */
   Arg               args[4];
   XColor            exactcolor;
   int               tmp, url_color, url_contrast, nocolors=0;
   unsigned long     valueMask = (GCForeground | GCBackground);
   XGCValues         values;

   if (!XtIsRealized((Widget)tw))
    {
      return;
    }
   _XmTextLineInfo(tw, line+1, &nextlinestart, &extra);
   _XmTextLineInfo(tw, line, &linestart, &extra);

   _XmTextAdjustGC(tw);

   if (!XtIsSensitive((Widget)tw))
    {
      stipple = True;
    }

   if (linestart == PASTENDPOS)
    {
      start = end = nextlinestart = PASTENDPOS;
      cleartoend = cleartobottom = True;
    }
   else if (nextlinestart == PASTENDPOS)
    {
      nextlinestart = (*tw->text.source->Scan)(tw->text.source, 0,
                                               XmSELECT_ALL, XmsdRight, 1,
                                               False);
      cleartoend = cleartobottom = (end >= nextlinestart);
      if (start >= nextlinestart)
       {
         endhighlight = highlight = XmHIGHLIGHT_NORMAL;
       }
      else if (cleartoend)
       {
         endhighlight = XmHIGHLIGHT_NORMAL;
       }
    }
   else
    {
      cleartobottom = False;
      cleartoend = (end >= nextlinestart);
      if (cleartoend && (!extra || !extra->wrappedbychar))
       {
         end = (*tw->text.source->Scan)(tw->text.source, nextlinestart,
                                        XmSELECT_POSITION, XmsdLeft, 1, True);
       }
    }

   if (XmDirectionMatch(XmPrim_layout_direction(tw),
                        XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
    {
      y = data->topmargin;
      x = tw->text.inner_widget->core.width -
            (data->rightmargin + line * data->linewidth +
            (int)(data->linewidth * 0.5));
      while ((linestart < start) && (x <= rightedge))
       {
         linestart = (*tw->text.source->ReadSource)(tw->text.source,
                                                    linestart, start, &block);
         y += FindHeight(tw, y, &block, 0, block.length);
       }
    }
   else
    {
      y = data->topmargin + line * data->lineheight + data->font_ascent;
      x = data->leftmargin;
      while ((linestart < start) && (x <= rightedge))
       {
         linestart = (*tw->text.source->ReadSource)(tw->text.source,
                                                    linestart, start, &block);
         x += FindWidth(tw, x, &block, 0, block.length);
       }
    }

   if(XmDirectionMatch(XmPrim_layout_direction(tw),
                       XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
    {
      XOrientation   orient=0;

      newy = y;
      if(data->use_fontset == True)
       {
         XGetOCValues((XOC)data->font, XNOrientation, &orient, NULL);
         SetXOCOrientation(tw, (XOC)data->font, XOMOrientation_TTB_RTL);
       }
      while ((start < end) && (y <= bottomedge))
       {
         start = (*tw->text.source->ReadSource)(tw->text.source, start,
                                                end, &block);
         if ((int)tw->text.char_size == 1)
          {
            num_bytes = 1;
          }
         else
          {
            num_bytes = mblen(block.ptr, (int)tw->text.char_size);
            if (num_bytes < 1)
             {
               num_bytes = 1;
             }
          }
         while (block.length != 0)
          {
            while ((num_bytes == 1) && (block.ptr[0] == '\t'))
             {
               newy = y;
               while ((block.length != 0) && (num_bytes == 1) &&
                      ((newy - data->voffset) < data->topmargin))
                {
                  height = FindHeight(tw, newy, &block, 0, 1);
                  newy += height;

                  if ((newy - data->voffset) < data->topmargin)
                   {
                     block.length--;
                     block.ptr++;
                     y = newy;
                     if ((int)tw->text.char_size != 1)   /* check if we've got   */
                      {                                  /* mbyte char           */
                        num_bytes = mblen(block.ptr,
                                          (int)tw->text.char_size);
                        if (num_bytes < 1)
                         {
                           num_bytes = 1;
                         }
                      }
                   }
                }
               if ((block.length == 0) || (block.length == (size_t) -1) ||
                   (num_bytes != 1) || (block.ptr[0] != '\t'))
                {
                  break;
                }

               height = FindHeight(tw, y, &block, 0, 1);

               if (highlight == XmHIGHLIGHT_SELECTED)
                {
                  SetNormGC(tw, data->gc, False, False);
                }
               else
                {
                  SetInvGC(tw, data->gc);
                }
               SetFullGC(tw, data->gc);

               if (((y - data->voffset) + height) >
                     (int)(tw->text.inner_widget->core.height - data->bottommargin))
                {
                  rec_height = (tw->text.inner_widget->core.height -
                              data->bottommargin) - (y - data->voffset);
                }
               else
                {
                  rec_height = height;
                }

               if (x - (int)(data->linewidth * 0.5) < data->leftmargin)
                {
                  rec_width = (tw->text.inner_widget->core.width -
                              data->rightmargin) - x;
                }
               else
                {
                  rec_width = data->linewidth;
                }

               XFillRectangle(XtDisplay(tw), XtWindow(tw->text.inner_widget),
                              data->gc, x - (data->linewidth * 0.5),
                              y - data->voffset, rec_width, rec_height);

               SetMarginGC(tw, data->gc);
               if (highlight == XmHIGHLIGHT_SECONDARY_SELECTED)
                {
                  if (highlight == XmHIGHLIGHT_SELECTED)
                   {
                     SetInvGC(tw, data->gc);
                   }
                  else
                   {
                     SetNormGC(tw, data->gc, False, False);
                   }

                  XDrawLine(XtDisplay(tw), XtWindow(tw->text.inner_widget),
                            data->gc, x + (int)(data->linewidth * 0.5) - 1,
                            y - data->voffset,
                            (int)(x + data->linewidth * 0.5) - 1,
                            ((y - data->voffset) + height) - 1);
                }
               y += height;

               block.length--;
               block.ptr++;
               if ((int)tw->text.char_size != 1)
                {
                  num_bytes = mblen(block.ptr, (int)tw->text.char_size);
                  if (num_bytes < 0)
                   {
                     num_bytes = 1;
                   }
                }
               if ((block.length == 0) ||
                   (block.length == (unsigned long)-1))
                {
                  break;
                }
             }
            if ((int)tw->text.char_size == 1)
             {
               for (length=0; length<block.length; length++)
                {
                  if (block.ptr[length] == '\t')
                   {
                     break;
                   }
                }
             }
            else
             {
               num_bytes = mblen(block.ptr, (int)tw->text.char_size);
               if (num_bytes == -1)
                {
                  num_bytes = 1;
                }
               for (length=0; length<block.length; )
                {
                  if ((num_bytes == 1) && (block.ptr[length] == '\t'))
                   {
                     break;
                   }
                  if (num_bytes == 0)
                   {
                     break;
                   }
                  if (num_bytes < 0)
                   {
                     num_bytes = 1;
                   }
                  length += num_bytes;
                  num_bytes = mblen(&block.ptr[length], (int)tw->text.char_size);
                  if (num_bytes == -1)
                   {
                     num_bytes = 1;
                   }
                }
             }
            if (length <= 0)
             {
               break;
             }
            newy = y;
            while ((length > 0) && (newy + charheight < data->topmargin))
             {
               newy += FindHeight(tw, newy, &block, 0, 1);
               if ((int)tw->text.char_size == 1)
                {
                  if (newy + charheight < data->topmargin)
                   {
                     length--;
                     block.length--;
                     block.ptr++;
                     y = newy;
                   }
                }
               else
                {
                  if (newy + charheight < data->topmargin)
                   {
                     num_bytes = mblen(block.ptr, (int)tw->text.char_size);
                     if (num_bytes < 0)
                      {
                        num_bytes = 1;
                      }
                     length -= num_bytes;
                     block.length -= num_bytes;
                     block.ptr += num_bytes;
                     y = newy;
                   }
                }
             }
            if (length == 0)
             {
               continue;
             }
            newy = y + FindHeight(tw, y, &block, 0, length);
            if (newy > bottomedge)
             {
               newy = y;
               if ((int)tw->text.char_size == 1)
                {
                  for (i=0; i<length && (newy - data->voffset) <= bottomedge; i++)
                   {
                     newy += FindHeight(tw, newy, &block, i, i+1);
                   }
                }
               else
                {
                  num_bytes = mblen(block.ptr, (int)tw->text.char_size);
                  if (num_bytes == -1)
                   {
                     num_bytes = 1;
                   }
                  for (i=0; i<length && (newy - data->voffset) <= bottomedge &&
                            num_bytes > 0; )
                   {
                     newy += FindHeight(tw, newy, &block, i, i + num_bytes);
                     i += num_bytes;
                     num_bytes = mblen(&block.ptr[i], (int)tw->text.char_size);
                     if (num_bytes == -1)
                      {
                        num_bytes = 1;
                      }
                   }
                }
               length = i;
               start = end; /* Force a break out of the outer loop. */
               block.length = length; /* ... and out of the inner loop. */
             }
            if (highlight == XmHIGHLIGHT_SELECTED)
             {                         /* Draw the inverse background,  */
                                       /* then draw the text over it    */
               SetNormGC(tw, data->gc, False, False);
               SetFullGC(tw, data->gc);

               if (((y - data->voffset) + (newy - y)) >
                   (int) tw->text.inner_widget->core.height - data->bottommargin)
                {
                  rec_height = tw->text.inner_widget->core.height -
                                 (y - data->voffset) - data->bottommargin;
                }
               else
                {
                  rec_height = newy - y;
                }

               if (x + (int)(data->linewidth * 0.5) < data->leftmargin)
                {
                  rec_width = x + (tw->text.inner_widget->core.width -
                              data->rightmargin);
                }
               else
                {
                  rec_width = data->linewidth;
                }

               XFillRectangle(XtDisplay(tw), XtWindow(tw->text.inner_widget),
                              data->gc, x - (int)(data->linewidth * 0.5),
                              y - data->voffset, rec_width, rec_height);

               SetInvGC(tw, data->gc);
               SetMarginGC(tw, data->gc);
               if (data->use_fontset)
                {
                  XmbDrawString(XtDisplay(tw), XtWindow(tw->text.inner_widget),
                                (XFontSet) data->font, data->gc, x,
                                y - data->voffset, block.ptr, length);
                }
               else
                {
                  int         len=0, csize=0, wx=0, orig_x=0, orig_y=0;
                  char *      p=NULL;
                  XCharStruct overall;

                  wx = x - data->hoffset;
                  orig_y = y - data->voffset;
                  for (len=length, p=block.ptr; len>0 && p;
                       len-= csize, p += csize)
                   {
                     csize = mblen(p, (int)tw->text.char_size);
                     if (csize == -1)
                      {
                        csize = 1;
                      }

                     _FontStructPerCharExtents(tw, p, csize, &overall);

                     orig_x = wx - (int)((overall.rbearing - overall.lbearing) >> 1) -
                                 overall.lbearing;
                     orig_y += overall.ascent;
                     XDrawString(XtDisplay(tw),
                     XtWindow(tw->text.inner_widget), data->gc,
                     orig_x, orig_y, p, csize);

                     orig_y += overall.descent;
                   }
                }
             }
            else
             {
               SetInvGC(tw, data->gc);
               if (newy > y)
                {
                  if (x + (int)(data->linewidth * 0.5) < data->leftmargin)
                   {
                     rec_width = x + (tw->text.inner_widget->core.width -
                                 data->rightmargin);
                   }
                  else
                   {
                     rec_width = data->linewidth;
                   }

                  XFillRectangle(XtDisplay(tw), XtWindow(tw->text.inner_widget),
                                 data->gc, x - (int)(data->linewidth * 0.5),
                                 y - data->voffset, rec_width, newy - y);
                }
               SetNormGC(tw, data->gc, True, stipple);
               if (data->use_fontset)
                {
                  int   wx, wy;

                  wx = x - data->hoffset;
                  XmbDrawString(XtDisplay(tw), XtWindow(tw->text.inner_widget),
                                (XFontSet) data->font, data->gc,
                                wx, y - data->voffset, block.ptr, length);
                }
               else
                {
                  int         len=0, csize=0, wx=0, orig_x=0, orig_y=0;
                  char *      p=NULL;
                  XCharStruct overall;

                  wx = x - data->hoffset;
                  orig_y = y - data->voffset;
                  for (len=length, p=block.ptr; len>0 && p;
                       len-= csize, p += csize)
                   {
                     csize = mblen(p, (int)tw->text.char_size);
                     if (csize == -1)
                      {
                        csize = 1;
                      }

                     _FontStructPerCharExtents(tw, p, csize, &overall);
                     orig_x = wx - (int)((overall.rbearing - overall.lbearing) >> 1) -
                              overall.lbearing;
                     orig_y += overall.ascent;
                     XDrawString(XtDisplay(tw),
                     XtWindow(tw->text.inner_widget), data->gc,
                     orig_x, orig_y, p, csize);
                     orig_y += overall.descent;
                   }
                }
               if (stipple)
                {
                  SetNormGC(tw, data->gc, True, !stipple);
                }
             }
            if (highlight == XmHIGHLIGHT_SECONDARY_SELECTED)
             {
               XDrawLine(XtDisplay(tw), XtWindow(tw->text.inner_widget),
               data->gc, x + (int)(data->linewidth * 0.5) - 1,
               y - data->voffset,
               x + (int)(data->linewidth * 0.5) - 1,
               (newy - data->voffset) - 1);
             }
            y = newy;
            block.length -= length;
            block.ptr += length;
            if ((int)tw->text.char_size != 1)
             {
               num_bytes = mblen(block.ptr, (int)tw->text.char_size);
               if (num_bytes < 1)
                {
                  num_bytes = 1;
                }
             }
          }
       }
      if(data->use_fontset == True)
       {
         SetXOCOrientation(tw, (XOC)data->font, orient);
       }
    }
   else
    {
      newx = x;

      while ((start < end) && (x <= rightedge))
       {
         start = (*tw->text.source->ReadSource)(tw->text.source, start,
                                                end, &block);
         if ((int)tw->text.char_size == 1)
          {
            num_bytes = 1;
          }
         else
          {
            num_bytes = mblen(block.ptr, (int)tw->text.char_size);
            if (num_bytes < 1) num_bytes = 1;
          }
         while (block.length > 0)
          {
            while ((num_bytes == 1) && (block.ptr[0] == '\t'))
             {
               newx = x;
               while ((block.length > 0) && (num_bytes == 1) &&
                      (newx - data->hoffset < data->leftmargin))
                {
                  width = FindWidth(tw, newx, &block, 0, 1);
                  newx += width;

                  if (newx - data->hoffset < data->leftmargin)
                   {
                     block.length--;
                     block.ptr++;
                     x = newx;
                     if ((int)tw->text.char_size != 1)
                      {
                        /* check if we've got mbyte char */
                        num_bytes = mblen(block.ptr,
                                          (int)tw->text.char_size);
                        if (num_bytes < 1) num_bytes = 1;
                      }
                   }
                }
               if ((block.length <= 0) || (num_bytes != 1) ||
                   (block.ptr[0] != '\t'))
                {
                  break;
                }

               width = FindWidth(tw, x, &block, 0, 1);

               if (highlight == XmHIGHLIGHT_SELECTED)
                {
                  SetNormGC(tw, data->gc, False, False);
                }
               else
                {
                  SetInvGC(tw, data->gc);
                }
               SetFullGC(tw, data->gc);

               if ((int)((x - data->hoffset) + width) >
                   (int)(tw->text.inner_widget->core.width - data->rightmargin))
                {
                  rec_width = (tw->text.inner_widget->core.width -
                              data->rightmargin) - (x - data->hoffset);
                }
               else
                {
                  rec_width = width;
                }

               if ((int)(y + data->font_descent) >
                   (int)(tw->text.inner_widget->core.height - data->bottommargin))
                {
                  rec_height = (tw->text.inner_widget->core.height -
                                 data->bottommargin) - y;
                }
               else
                {
                  rec_height = data->font_ascent + data->font_descent;
                }

               XFillRectangle(XtDisplay(tw), XtWindow(tw->text.inner_widget),
                              data->gc, x - data->hoffset, y - data->font_ascent,
                              rec_width, rec_height);

               SetMarginGC(tw, data->gc);
               if (highlight == XmHIGHLIGHT_SECONDARY_SELECTED)
                {
                  if (highlight == XmHIGHLIGHT_SELECTED)
                   {
                     SetInvGC(tw, data->gc);
                   }
                  else
                   {
                     SetNormGC(tw, data->gc, False, False);
                   }

                  XDrawLine(XtDisplay(tw), XtWindow(tw->text.inner_widget),
                            data->gc, x - data->hoffset, y,
                            ((x - data->hoffset) + width) - 1, y);
                }
               x += width;


               block.length--;
               block.ptr++;
               if ((int)tw->text.char_size != 1)
                {
                  num_bytes = mblen(block.ptr, (int)tw->text.char_size);
                  if (num_bytes < 0) num_bytes = 1;
                }
               if (block.length <= 0)
                {
                  break;
                }
             }
            if ((int)tw->text.char_size == 1)
             {
               for (length=0; length<block.length; length++)
                {
                  if (block.ptr[length] == '\t')
                   {
                     break;
                   }
                }
             }
            else
             {
               for (length=0, num_bytes=mblen(block.ptr, (int)tw->text.char_size);
                    length < block.length;
                    num_bytes = mblen(&block.ptr[length],
                    (int)tw->text.char_size))
                {
                  if ((num_bytes == 1) && (block.ptr[length] == '\t'))
                   {
                     break;
                   }
                  if (num_bytes == 0)
                   {
                     break;
                   }
                  if (num_bytes < 0)
                   {
                     num_bytes = 1;
                   }
                  length += num_bytes;
                }
             }
            if (length <= 0)
             {
               break;
             }
            newx = x;
            while ((length > 0) && (newx - data->hoffset < data->leftmargin))
             {
               newx += FindWidth(tw, newx, &block, 0, 1);
               if ((int)tw->text.char_size == 1)
                {
                  if (newx - data->hoffset < data->leftmargin)
                   {
                     length--;
                     block.length--;
                     block.ptr++;
                     x = newx;
                   }
                }
               else
                {
                  if (newx - data->hoffset < data->leftmargin)
                   {
                     num_bytes = mblen(block.ptr, (int)tw->text.char_size);
                     if (num_bytes < 0) num_bytes = 1;
                     length -= num_bytes;
                     block.length -= num_bytes;
                     block.ptr += num_bytes;
                     x = newx;
                   }
                }
             }

            if (length == 0)
             {
               continue;
             }
            newx = x + FindWidth(tw, x, &block, 0, length);
            if (newx > rightedge)
             {
               newx = x;
               if ((int)tw->text.char_size == 1)
                {
                  for (i=0; i<length && newx<=rightedge; i++)
                   {
                     newx += FindWidth(tw, newx, &block, i, i+1);
                   }
                }
               else
                {
                  for (i=0, num_bytes=mblen(block.ptr, (int)tw->text.char_size);
                       (i<length) && (newx<=rightedge) && (num_bytes!=0);
                       i+=num_bytes, num_bytes=mblen(&block.ptr[i],
                                                     (int)tw->text.char_size))
                  {
                     if (num_bytes == -1) num_bytes = 1;
                     newx += FindWidth(tw, newx, &block, i, i + num_bytes);
                  }
                }
               length = i;
               start = end; /* Force a break out of the outer loop. */
               block.length = length; /* ... and out of the inner loop. */
             }
            if (highlight == XmHIGHLIGHT_SELECTED)
             {                         /* Draw the inverse background,  */
                                       /* then draw the text over it    */
               SetNormGC(tw, data->gc, False, False);
               SetFullGC(tw, data->gc);
   
               if ((int) ((x - data->hoffset) + (newx - x)) >
                   (int) (tw->text.inner_widget->core.width - data->rightmargin))
                {
                  rec_width = tw->text.inner_widget->core.width -
                              (x - data->hoffset) - data->rightmargin;
                }
               else
                {
                  rec_width = newx - x;
                }

               if ((int) (y + data->font_descent) >
                   (int) (tw->text.inner_widget->core.height - data->bottommargin))
                {
                  rec_height = (tw->text.inner_widget->core.height -
                                 data->bottommargin) - (y - data->font_ascent);
                }
               else
                {
                  rec_height = data->font_ascent + data->font_descent;
                }

               XFillRectangle(XtDisplay(tw), XtWindow(tw->text.inner_widget),
                              data->gc, x - data->hoffset,
                              y - data->font_ascent, rec_width, rec_height);

               SetInvGC(tw, data->gc);
               SetMarginGC(tw, data->gc);
               if (data->use_fontset)
                {
                  XmbDrawString(XtDisplay(tw), XtWindow(tw->text.inner_widget),
                                (XFontSet) data->font, data->gc,
                                x - data->hoffset, y, block.ptr, length);
                }
               else
                {
                  XDrawString(XtDisplay(tw), XtWindow(tw->text.inner_widget),
                              data->gc, x - data->hoffset, y, block.ptr, length);
                }

#ifdef ENABLE_URLS
          /* Sun highlight callbacks */
             }
            else if ((!tw->text.editable) &&
                     (highlight == XmHIGHLIGHT_COLOR_1))
             {
               SetInvGC(tw, data->gc);
      
               if (newx > x)
                {
                  if (y + data->font_descent >
                       tw->text.inner_widget->core.height-data->bottommargin)
                   {
                     rec_height = (tw->text.inner_widget->core.height -
                                    data->bottommargin) - (y-data->font_ascent);
                   }
                  else
                   {
                     rec_height = data->font_ascent + data->font_descent;
                   }

                  XFillRectangle(XtDisplay(tw), XtWindow(tw->text.inner_widget),
                                 data->gc, x - data->hoffset,
                                 y - data->font_ascent, newx - x, rec_height);
                }
               _Setup_hl1((Widget)tw, &tw->text, XtDisplay(tw), XtScreen(tw));

               values.foreground = tw->text.highlightColor1->pixel;
               values.background = tw->text.inner_widget->core.background_pixel;
               XChangeGC(XtDisplay(tw), data->gc, valueMask, &values);
               if (data->use_fontset)
                {
                  XmbDrawString(XtDisplay(tw), XtWindow(tw->text.inner_widget),
                                (XFontSet) data->font, data->gc,
                                x - data->hoffset, y, block.ptr, length);
                }
               else
                {
                  XDrawString(XtDisplay(tw), XtWindow(tw->text.inner_widget),
                              data->gc, x - data->hoffset, y, block.ptr, length);
                }
               XDrawLine(XtDisplay(tw), XtWindow(tw->text.inner_widget),
                         data->gc, x - data->hoffset, y,
                         (newx - data->hoffset) - 1, y);
             }
            else if ((!tw->text.editable) && (highlight == XmHIGHLIGHT_COLOR_2))
             {
               SetInvGC(tw, data->gc);

               if (newx > x)
                {
                  if (y + data->font_descent >
                      tw->text.inner_widget->core.height - data->bottommargin)
                   {
                     rec_height = (tw->text.inner_widget->core.height -
                                    data->bottommargin) - (y - data->font_ascent);
                   }
                  else
                   {
                     rec_height = data->font_ascent + data->font_descent;
                   }
   
                  XFillRectangle(XtDisplay(tw), XtWindow(tw->text.inner_widget),
                                 data->gc, x - data->hoffset,
                                 y - data->font_ascent, newx - x, rec_height);
                }
               _Setup_hl2((Widget)tw, &tw->text, XtDisplay(tw), XtScreen(tw));

               values.foreground = tw->text.highlightColor2->pixel;
               values.background = tw->text.inner_widget->core.background_pixel;
               XChangeGC(XtDisplay(tw), data->gc, valueMask, &values);
               if (data->use_fontset)
                {
                  XmbDrawString(XtDisplay(tw), XtWindow(tw->text.inner_widget),
                                (XFontSet) data->font, data->gc,
                                x - data->hoffset, y, block.ptr, length);
                }
               else
                {
                  XDrawString(XtDisplay(tw), XtWindow(tw->text.inner_widget),
                              data->gc, x - data->hoffset, y, block.ptr, length);
                }
               XDrawLine(XtDisplay(tw), XtWindow(tw->text.inner_widget),
                         data->gc, x - data->hoffset, y,
                         (newx - data->hoffset) - 1, y);
            /* end Sun highlight callbacks */
#endif
             }
            else
             {
               SetInvGC(tw, data->gc);
               if (newx > x)
                {
                  if ((int) (y + data->font_descent) >
                      (int) (tw->text.inner_widget->core.height - data->bottommargin))
                   {
                     rec_height = (tw->text.inner_widget->core.height -
                                    data->bottommargin) - (y - data->font_ascent);
                   }
                  else
                   {
                     rec_height = data->font_ascent + data->font_descent;
                   }

                  XFillRectangle(XtDisplay(tw), XtWindow(tw->text.inner_widget),
                                 data->gc, x - data->hoffset,
                                 y - data->font_ascent, newx - x, rec_height);
                }
               SetNormGC(tw, data->gc, True, stipple);
               if (data->use_fontset)
                {
                  XmbDrawString(XtDisplay(tw), XtWindow(tw->text.inner_widget),
                                (XFontSet) data->font, data->gc,
                                x - data->hoffset, y, block.ptr, length);
                }
               else
                {
                  XDrawString(XtDisplay(tw), XtWindow(tw->text.inner_widget),
                              data->gc, x - data->hoffset, y, block.ptr, length);
                }
               if (stipple)
                {
                  SetNormGC(tw, data->gc, True, !stipple);
                }
             }
            if (highlight == XmHIGHLIGHT_SECONDARY_SELECTED)
             {
               XDrawLine(XtDisplay(tw), XtWindow(tw->text.inner_widget),
                         data->gc, x - data->hoffset, y,
                         (newx - data->hoffset) - 1, y);
             }
            x = newx;
            block.length -= length;
            block.ptr += length;
            if ((int)tw->text.char_size != 1)
             {
               num_bytes = mblen(block.ptr, (int)tw->text.char_size);
               if (num_bytes < 1) num_bytes = 1;
             }
          }
       }
    }

   if(XmDirectionMatch(XmPrim_layout_direction(tw),
                       XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
    {
      /* clear top margin */

      text_border = tw->primitive.shadow_thickness +
                     tw->primitive.highlight_thickness;
      if (((data->topmargin - text_border) > 0) &&
          (x < (rightedge + text_border)))
       {
         XClearArea(XtDisplay(tw), XtWindow(tw->text.inner_widget),
                    x - (int) (data->linewidth * 0.5)-1, text_border,
                    rightedge - x + (int) (data->linewidth * 0.5),
                    data->topmargin - text_border, FALSE);
       }

      if (cleartoend)
       {
         y -= data->voffset;
         if (y > ((int)tw->text.inner_widget->core.height)- data->bottommargin)
          {
            y = ((int)tw->text.inner_widget->core.height) -
                  data->bottommargin;
          }
         if (y < data->topmargin)
          {
            y = data->topmargin;
          }
         height = ((int)tw->text.inner_widget->core.height) - y -
                     data->bottommargin;
         if ((height > 0) && (data->linewidth > 0))
          {
            if (endhighlight == XmHIGHLIGHT_SELECTED)
             {
               SetNormGC(tw, data->gc, False, False);
             }
            else
             {
               SetInvGC(tw, data->gc);
             }
            SetFullGC(tw, data->gc);
            if ((int) (x - (data->linewidth * 0.5)) < data->leftmargin)
             {
               rec_width = x + (data->linewidth * 0.5) - data->leftmargin;
             }
            else
             {
               rec_width = data->linewidth;
             }

            XFillRectangle(XtDisplay(tw), XtWindow(tw->text.inner_widget),
                           data->gc, x - (int)(data->linewidth * 0.5),
                           y, rec_width, height);
            SetMarginGC(tw, data->gc);
          }
       }
      if (cleartobottom)
       {
         y = data->topmargin;
         width = x - (data->linewidth * 0.5) - data->leftmargin;
         height = tw->text.inner_widget->core.height -
                  (data->topmargin + data->bottommargin);
         if ((width > 0) && (height > 0))
          {
            XClearArea(XtDisplay(tw), XtWindow(tw->text.inner_widget),
                       data->leftmargin, y, width, height, False);
          }
       }
    }
   else
    {
      /* clear left margin */

      text_border = tw->primitive.shadow_thickness +
                     tw->primitive.highlight_thickness;
      if (((data->leftmargin - text_border) > 0) &&
          (int)(y + data->font_descent) > 0)
         XClearArea(XtDisplay(tw), XtWindow(tw->text.inner_widget),
                    text_border, text_border,
                    data->leftmargin - text_border,
                    y + data->font_descent - text_border, False);

      if (cleartoend)
      {
            x -= data->hoffset;
            if (x > ((int)tw->text.inner_widget->core.width)- data->rightmargin)
             {
               x = ((int)tw->text.inner_widget->core.width)- data->rightmargin;
             }
            if (x < data->leftmargin)
             {
               x = data->leftmargin;
             }
            width = ((int)tw->text.inner_widget->core.width) - x -
                     data->rightmargin;
            if ((width > 0) && (data->lineheight > 0))
             {
               if (endhighlight == XmHIGHLIGHT_SELECTED)
                {
                  SetNormGC(tw, data->gc, False, False);
                }
               else
                {
                  SetInvGC(tw, data->gc);
                }
               SetFullGC(tw, data->gc);
               if ((int)(y + data->font_descent) >
                   (int)(tw->text.inner_widget->core.height - data->bottommargin))
                {
                  rec_height = (tw->text.inner_widget->core.height -
                                 data->bottommargin) - (y - data->font_ascent);
                }
               else
                {
                  rec_height = data->font_ascent + data->font_descent;
                }

               XFillRectangle(XtDisplay(tw),
                              XtWindow(tw->text.inner_widget), data->gc,
                              x, y - data->font_ascent, width, rec_height);
               SetMarginGC(tw, data->gc);
             }
      }
      if (cleartobottom)
      {
            x = data->leftmargin;
            width = tw->text.inner_widget->core.width -
                     (data->rightmargin + data->leftmargin);
            height = tw->text.inner_widget->core.height -
                     ((y + data->font_descent) + data->bottommargin);
            if ((width > 0) && (height > 0))
             {
               XClearArea(XtDisplay(tw), XtWindow(tw->text.inner_widget),
                          x, y + data->font_descent, width, height, False);
             }
      }
    }

   /* Before exiting, force PaintCursor to refresh its save area */
   data->refresh_ibeam_off = True;
}


#ifdef SUN_CTL
static void 
Draw(XmTextWidget 		tw,
     LineNum				line,
     XmTextPosition		start,
     XmTextPosition		end,
     _XmHighlightData *	hl_data)
{
	if (TextW_LayoutActive(tw))
	 {
		CTLDraw(tw, line, start, end, hl_data);
	 }
	else
	 {
		NONCTLDraw (tw, line, start, end,(XmHighlightMode) hl_data);
	 }
}
#endif /* CTL */


static OnOrOff 
CurrentCursorState(XmTextWidget tw)
{
	OutputData	data=tw->text.output->data;

	if (data->cursor_on < 0)
	 {
		return off;
	 }
	if ((data->blinkstate == on) || !XtIsSensitive((Widget)tw))
	 {
		return on;
	 }

	return off;
}


#ifdef SUN_CTL
int
_XmTextPosSegment(XmTextWidget tw, XmTextPosition pos, XSegment* char_segment)
{
	LineNum			line=0;
	XmTextPosition	line_start, next_line_start;
	XmTextBlockRec	block;
	int				status;
	Boolean			is_wchar;
	OutputData		data;
	char *			text;
	int				text_len;
	char				tmp_cache[200];
	char *			tmp_ch;
	wchar_t			tmp_wcache[200];
	wchar_t *		tmp_wc;
	

   line = PosToAbsLine(tw, pos);						/*	Get the line text			*/
	if (line == NOLINE)
	 {
		char_segment->x1 = char_segment->x2 = 0;
		char_segment->y1 = char_segment->y2 = 0;
		return 1;
	 }

	CTLLineInfo(tw, line, &line_start);
	CTLLineInfo(tw, line + 1, &next_line_start);

	if (next_line_start == PASTENDPOS)
	 {
		next_line_start = tw->text.last_position;
	 }

	if (next_line_start > line_start)
	 {
		(*tw->text.source->ReadSource)(tw->text.source, line_start,
												 next_line_start, &block);
	 }
	else
	 {
		block.length = 0;
	 }
    
	is_wchar = (tw->text.char_size > 1);
   data = tw->text.output->data;
   text     = block.ptr;
   text_len = block.length;
   
   if (is_wchar && _XmRendLayoutIsCTL(data->rendition))
	 {
		tmp_ch = XmStackAlloc((text_len + 1), tmp_cache);
		memcpy(tmp_ch, text, text_len);
		tmp_ch[text_len] = 0;
		tmp_wc = (wchar_t*)XmStackAlloc(((text_len + 1) * sizeof(wchar_t)), tmp_wcache);
		text_len = mbstowcs(tmp_wc, tmp_ch, text_len);
	
		if (text_len == -1)
		 {
			text_len = _Xm_mbs_invalid(tmp_wc, tmp_ch, block.length);  /* fix for bug 4277497 - leob */
		 }
		tmp_wc[text_len] = (wchar_t)0;
		text = (char*)tmp_wc;
	 }
    
	status = _XmRenditionPosCharExtents(data->rendition, pos - line_start, text,
													text_len, is_wchar, data->tabwidth,
													ISTEXT_RIGHTALIGNED(tw), char_segment);
	if (!status)
	 {
		XmeWarning((Widget) tw, "Error in _XmRenditionPosCharExtents\n");
	 }

	if (is_wchar)
	 {
		XmStackFree(tmp_ch, tmp_cache);
		XmStackFree((char*)tmp_wc, tmp_wcache);
    }

	return status;
}
#endif /* CTL */


/*
 * All the info about the cursor has been figured; draw or erase it.
 */
static void 
PaintCursor(XmTextWidget tw)
{
	OutputData	data=tw->text.output->data;
	Position		x, y;
    
	if (!data->cursor_position_visible)
	 {
		return;
	 }
    
	_XmTextToggleCursorGC((Widget)tw);
   
   if (!tw->text.input->data->overstrike)
	 {
		x = data->insertx - (data->cursorwidth >> 1) - 1;
	 }
	else
	 {
		int				pxlen;
		XmTextBlockRec	block;
		XmTextPosition cursor_pos=tw->text.cursor_position;

		x = data->insertx;
#ifdef SUN_CTL
		if (TextW_LayoutActive(tw))
		 {
			int		status;
			XSegment	char_segment;
			Boolean	r_to_l;
	    
			status = _XmTextPosSegment(tw, tw->text.cursor_position, &char_segment);
			if (!status)
			 {
				XmeWarning((Widget) tw, "Error in _XmTextPosSegment. TextOut.c\n");
			 }
#ifdef SUN_CTL_NYI
			if (ISVISUAL_EDITPOLICY(tw))
			 {
				r_to_l = False;
			 }
			else
#endif
				r_to_l = char_segment.x2 <= char_segment.x1;
			pxlen = abs(char_segment.x2 - char_segment.x1);
			data->cursorwidth = pxlen;
			if (r_to_l)
			 {
				x -= data->cursorwidth;
			 }
		 }
		else
		 {
#endif /* CTL */
		(void)(*tw->text.source->ReadSource)(tw->text.source, cursor_pos, cursor_pos+1, &block);
		pxlen = FindWidth(tw, x, &block, 0, block.length);
#ifdef SUN_CTL
		 }
#endif /* CTL */
		if (pxlen > (int) data->cursorwidth)
		 {
			x += (short) (pxlen - data->cursorwidth) >> 1;
		 }
	 }
	y =  data->inserty + data->font_descent - data->cursorheight;
    

	/*	If time to paint the I Beam, first capture the IBeamOffArea,	*/
	/*	then draw the I Beam.														*/

	if ((tw->text.top_character <= tw->text.cursor_position) &&
		 (tw->text.cursor_position <= tw->text.bottom_position))
	 {
		int	cursor_width=data->cursorwidth;
		int	cursor_height=data->cursorheight;

		if (data->refresh_ibeam_off == True)
		 {
			XFillRectangle(XtDisplay((Widget)tw), XtWindow((Widget)tw), data->save_gc, 0, 0, 0, 0);
														/*	(Needed to realign clip rectangle with gc)	*/
			XCopyArea(XtDisplay((Widget)tw), XtWindow((Widget)tw), data->ibeam_off,
						 data->save_gc, x, y, data->cursorwidth, data->cursorheight, 0, 0);
			data->refresh_ibeam_off = False;
		 }
	
		/* redraw cursor, being very sure to keep it within the bounds */
		/*	of the text area, not spilling into the highlight area.		*/

		if ((data->cursor_on >= 0) && (data->blinkstate == on))
		 {
			if (x + data->cursorwidth > (tw->text.inner_widget->core.width -     
												  tw->primitive.shadow_thickness -   
												  tw->primitive.highlight_thickness))
			 {
				cursor_width = (tw->text.inner_widget->core.width -              
										(tw->primitive.shadow_thickness +          
										 tw->primitive.highlight_thickness)) - x;
   		 }
			if ((cursor_width > 0) && (cursor_height > 0))
			 {
				XFillRectangle(XtDisplay((Widget)tw), XtWindow((Widget)tw),  
									data->imagegc, x, y, cursor_width, cursor_height);                        
			 }
		 } 
		else
		 {
			Position		src_x=0;                                                     
			if (x + data->cursorwidth > (tw->text.inner_widget->core.width -    
												  tw->primitive.shadow_thickness -                 
												  tw->primitive.highlight_thickness))
			 {            
				cursor_width = (tw->text.inner_widget->core.width -
									 (tw->primitive.shadow_thickness +
									  tw->primitive.highlight_thickness)) - x;   
			 }
			else if (x < tw->primitive.highlight_thickness + tw->primitive.shadow_thickness)
			 {          
				cursor_width = data->cursorwidth - (tw->primitive.highlight_thickness +       
																tw->primitive.shadow_thickness - x);        
				src_x = data->cursorwidth - cursor_width;                          
				x = tw->primitive.highlight_thickness + tw->primitive.shadow_thickness;             
			 }

			if (y + data->cursorheight > (tw->text.inner_widget->core.height -    
													(tw->primitive.highlight_thickness +        
													 tw->primitive.shadow_thickness)))
			 {         
				cursor_height = data->cursorheight - ((y + data->cursorheight) -                    
																  (tw->text.inner_widget->core.height -       
																	(tw->primitive.highlight_thickness +        
																	 tw->primitive.shadow_thickness)));          
			 }

			if ((cursor_width > 0) && (cursor_height > 0))
			 {
				XCopyArea(XtDisplay((Widget)tw), data->ibeam_off, XtWindow((Widget)tw),
							 data->save_gc, src_x, 0, cursor_width, cursor_height, x, y);
			 }
		 }
	 }
}


static void 
ChangeHOffset(XmTextWidget tw, int newhoffset, Boolean redisplay_hbar)
{
	OutputData	data=tw->text.output->data;
	int			delta;
	int			width=tw->text.inner_widget->core.width;
	int			height=tw->text.inner_widget->core.height;
	int			innerwidth=width - (data->leftmargin + data->rightmargin);
	int			innerheight=height - (data->topmargin + data->bottommargin);

	if (ShouldWordWrap(data, tw) || data->suspend_hoffset)
	 {
		return;
	 }

	if ((data->scrollhorizontal && XmIsScrolledWindow(XtParent(tw))) &&
		 (data->scrollwidth - innerwidth < newhoffset))
	 {
		newhoffset = data->scrollwidth - innerwidth;
	 }

	if (newhoffset < 0)
	 {
		newhoffset = 0;
	 }
	if (newhoffset == data->hoffset)
	 {
		return;
	 }

	delta = newhoffset - data->hoffset;
	data->hoffset = newhoffset;
#ifdef SUN_CTL 
	/* The following code is advisable even when the text is left	*/
	/*	aligned as for CTL XCopyArea on the line is not required.	*/

	if (TextW_LayoutActive(tw))
	 {
		if (ISTEXT_RIGHTALIGNED(tw))
		 {
			_XmProcessLock();
			posToXYCachedWidget = NULL;
			_XmProcessUnlock();
			if (XtIsRealized((Widget)tw))
			 {
				_XmTextAdjustGC(tw);
				SetNormGC(tw, data->gc, False, False);
				RedrawRegion(tw, width - data->rightmargin, 0, -width, height);
				data->exposehscroll++;
			 }
 			if (redisplay_hbar) 
			 {
				_XmRedisplayHBar(tw);	
			 }
			return;
		 }
	 }
#endif

	_XmProcessLock();
	posToXYCachedWidget = NULL;
	_XmProcessUnlock();
	if (XtIsRealized((Widget)tw))
	 {
		_XmTextAdjustGC(tw);
		SetNormGC(tw, data->gc, False, False);
		if (delta < 0)
		 {
			if ((width > 0) && (innerheight > 0))
			 {
				XCopyArea(XtDisplay(tw), XtWindow(tw->text.inner_widget),
							 XtWindow(tw->text.inner_widget), data->gc,
							 data->leftmargin, data->topmargin, width, innerheight,
							 data->leftmargin - delta, data->topmargin);
				if ((int)(data->leftmargin - delta - (tw->primitive.shadow_thickness +
							 tw->primitive.highlight_thickness)) < innerwidth)
				 {
																/* clear left margin + delta change	*/
					XClearArea(XtDisplay(tw), XtWindow(tw),
								  tw->primitive.shadow_thickness + tw->primitive.highlight_thickness,
								  data->topmargin,
								  data->leftmargin - (tw->primitive.shadow_thickness + tw->primitive.highlight_thickness) - delta,
								  innerheight, False);
				 }

				if ((int)(data->rightmargin - (tw->primitive.shadow_thickness +
														 tw->primitive.highlight_thickness)) > 0)
				 {
																/*	clear right margin					*/
					XClearArea(XtDisplay(tw), XtWindow(tw),
								  data->leftmargin + innerwidth, data->topmargin,
								  data->rightmargin - (tw->primitive.shadow_thickness + tw->primitive.highlight_thickness),
								  innerheight, False);
				 }

				data->exposehscroll++;
			 }
			RedrawRegion(tw, data->leftmargin, 0, -delta, height);
		 }
		else
		 {
			if ((innerwidth - delta > 0) && (innerheight > 0))
			 {
				XCopyArea(XtDisplay(tw), XtWindow(tw->text.inner_widget),
							 XtWindow(tw->text.inner_widget), data->gc,
							 data->leftmargin + delta, data->topmargin,
							 innerwidth - delta, innerheight, data->leftmargin, data->topmargin);

				XClearArea(XtDisplay(tw), XtWindow(tw),
							  data->leftmargin + innerwidth - delta, data->topmargin,
							  delta + data->rightmargin - (tw->primitive.shadow_thickness + tw->primitive.highlight_thickness),
							  innerheight, False);
																/*	clear right margin + delta change	*/
				if (data->leftmargin - (int)(tw->primitive.shadow_thickness +
													  tw->primitive.highlight_thickness) > 0)
				 {
					XClearArea(XtDisplay(tw), XtWindow(tw),
								  tw->primitive.shadow_thickness + tw->primitive.highlight_thickness,
								  data->topmargin,
								  data->leftmargin - (tw->primitive.shadow_thickness + tw->primitive.highlight_thickness),
								  innerheight, False);
																/*	clear left margin							*/
				 }
				data->exposehscroll++;
			 }
			else										/*	Else clear all text						*/
			 {
				XClearArea(XtDisplay(tw), XtWindow(tw),
							  tw->primitive.shadow_thickness + tw->primitive.highlight_thickness,
							  data->topmargin,
							  width - 2 *(tw->primitive.shadow_thickness + tw->primitive.highlight_thickness),
							  innerheight, False);
				data->exposehscroll++;
			 }
			RedrawRegion(tw, width - data->rightmargin - delta, 0, delta, height);
		 }
	 }

	if (redisplay_hbar)
	 {
		_XmRedisplayHBar(tw);
	 }
}


static void 
ChangeVOffset(XmTextWidget tw, int newvoffset, Boolean redisplay_vbar)
{
	OutputData	data=tw->text.output->data;
	int			delta=0;
	int			width=tw->text.inner_widget->core.width;
	int			height=tw->text.inner_widget->core.height;
	int			innerwidth=width - (data->leftmargin + data->rightmargin);
	int			innerheight=height - (data->topmargin + data->bottommargin);


	if (ShouldWordWrap(data, tw) || data->suspend_voffset)
	 {
		return;
	 }

	if ((data->scrollvertical && XmIsScrolledWindow(XtParent(tw))) &&
		 data->scrollheight - innerheight < newvoffset)
	 {
		newvoffset = data->scrollheight - innerheight;
	 }

	if (newvoffset < 0)
	 {
		newvoffset = 0;
	 }
	if (newvoffset == data->voffset)
	 {
		return;
	 }

	delta = newvoffset - data->voffset;
	data->voffset = newvoffset;

	_XmProcessLock();
	posToXYCachedWidget = NULL;
	_XmProcessUnlock();
	if (XtIsRealized((Widget)tw))
	 {
		_XmTextAdjustGC(tw);
		SetNormGC(tw, data->gc, False, False);
		if (delta < 0)
		 {
			if (height > 0 && innerwidth > 0)
			 {
				XCopyArea(XtDisplay(tw), XtWindow(tw->text.inner_widget),
							 XtWindow(tw->text.inner_widget), data->gc,
							 data->leftmargin, data->topmargin, innerwidth, height,
							 data->leftmargin, data->topmargin - delta);
				if ((int)(data->topmargin - (tw->primitive.shadow_thickness +
													  tw->primitive.highlight_thickness) - delta)
										< innerheight)
				 {												/*	clear top margin + delta change	*/
					XClearArea(XtDisplay(tw), XtWindow(tw), data->leftmargin,
								  tw->primitive.shadow_thickness + tw->primitive.highlight_thickness,
								  innerwidth,
								  data->topmargin - (tw->primitive.shadow_thickness + tw->primitive.highlight_thickness) - delta, False);
				 }

				if ((int) data->topmargin - (int)(tw->primitive.shadow_thickness +
															 tw->primitive.highlight_thickness) > 0)
				 {
																/*	clear right margin					*/
					XClearArea(XtDisplay(tw), XtWindow(tw), data->leftmargin,
								  data->topmargin + innerheight, innerwidth,
								  data->bottommargin - (tw->primitive.shadow_thickness + tw->primitive.highlight_thickness),
								  False);
				 }

				data->exposevscroll++;
			 }
			RedrawRegion(tw, 0, data->topmargin, width, -delta);
		 }
		else
		 {
			if ((innerheight - delta > 0) && (innerwidth > 0))
			 {
				XCopyArea(XtDisplay(tw), XtWindow(tw->text.inner_widget),
							 XtWindow(tw->text.inner_widget), data->gc, data->leftmargin,
							 data->topmargin + delta, innerwidth, innerheight - delta,
							 data->leftmargin, data->topmargin);

				XClearArea(XtDisplay(tw), XtWindow(tw), data->leftmargin,
							  data->topmargin + innerheight - delta, innerwidth,
							  delta + data->bottommargin - (tw->primitive.shadow_thickness + tw->primitive.highlight_thickness),
							  False);						/* clear bottom margin + delta change	*/

				if (data->topmargin - (int)(tw->primitive.shadow_thickness +
													 tw->primitive.highlight_thickness) > 0)
				 {
																/*	clear top margin							*/
					XClearArea(XtDisplay(tw), XtWindow(tw), data->leftmargin,
								  tw->primitive.shadow_thickness + tw->primitive.highlight_thickness,
								  innerwidth,
								  data->topmargin - (tw->primitive.shadow_thickness + tw->primitive.highlight_thickness),
								  False);
				 }
				data->exposevscroll++;
			 }
			else												/*	Else clear all text						*/
			 {
				XClearArea(XtDisplay(tw), XtWindow(tw), data->leftmargin,
							  tw->primitive.shadow_thickness + tw->primitive.highlight_thickness,
							  innerwidth,
							  height - 2 *(tw->primitive.shadow_thickness + tw->primitive.highlight_thickness),
							  False);
				data->exposevscroll++;
			 }
			RedrawRegion(tw, 0, height - data->bottommargin - delta, width, delta);
		 }
	 }
  
	if (redisplay_vbar)
	 {
		_XmRedisplayVBar(tw);
	 }
}


static void 
DrawInsertionPoint(XmTextWidget tw, XmTextPosition position, OnOrOff onoroff)
{
	OutputData	data=tw->text.output->data;
  
	if (onoroff == on)
	 {
		data->cursor_on += 1;
		if ((data->blinkrate == 0) || !data->hasfocus)
		 {
			data->blinkstate = on;
		 }
	 }
	else
	 {
		if ((data->blinkstate == on) && (data->cursor_on == 0))
		 {
			if ((data->blinkstate == CurrentCursorState(tw)) && XtIsRealized((Widget)tw))
			 {
				data->blinkstate = off;
				data->cursor_on -= 1;
				PaintCursor(tw);
			 }
			else
			 {
				data->cursor_on -= 1;
			 }
		 }
		else
		 {
			data->cursor_on -= 1;
		 }
	 }
  
	if ((data->cursor_on < 0) || !XtIsRealized((Widget)tw))
	 {
		return;
	 }

	if (PosToXY(tw, position, &data->insertx, &data->inserty))
	 {
		PaintCursor(tw);
	 }
}


static void 
MakePositionVisible(XmTextWidget tw, XmTextPosition position)
{
	OutputData	data=tw->text.output->data;
	Position		x, y;
	LineNum		line_num;

	if (!ShouldWordWrap(data, tw) && PosToXY(tw, position, &x, &y))
	 {
		if(XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
		 {
			if (y <= data->topmargin)
			 {
				if (tw->text.edit_mode == XmSINGLE_LINE_EDIT)
				 {
					if (position == tw->text.bottom_position)
					 {
						position = MAX(position - data->rows/2, 0);
					 }
				 }
				else
				 {
					line_num = _XmTextGetTableIndex(tw, position);
					if ((position == tw->text.bottom_position) ||
						 ((line_num < tw->text.total_lines) &&
						  (position == tw->text.line_table[line_num+1].start_pos - 1)))
					 {
						position = MAX(position - (int) data->rows/2,
											line_num ? (int) (tw->text.line_table[line_num].start_pos) : 0);
					 }
				 }
				PosToXY(tw, position, &x, &y);
			 }

			y += data->voffset;
			if (y - data->voffset < data->topmargin)
			 {
				ChangeVOffset(tw, y - data->topmargin, True);
			 }
			else if (y - data->voffset > ((Position)(tw->text.inner_widget->core.height -
																  data->bottommargin)))
			 {
				ChangeVOffset(tw, (int)(y) - (int)(tw->text.inner_widget->core.height) +
										(int)(data->bottommargin),
								  True);
			 }
		 }
		else
		 {
			if (x <= data->leftmargin)
			 {
#ifdef SUN_CTL
				/* The changing of position based on the # of columns		*/
				/*	doesn't mean anything in CTL and hence disabled.		*/

				if (!TextW_LayoutActive(tw)) 
#endif /* SUN_CTL */
				if (tw->text.edit_mode == XmSINGLE_LINE_EDIT)
				 {
					if (position == tw->text.bottom_position)
					 {
						position = MAX(position - data->columns/2, 0);
					 }
				 }
				else
				 {
					line_num = _XmTextGetTableIndex(tw, position);
					if ((position == tw->text.bottom_position) ||
						 ((line_num < tw->text.total_lines) &&
						  (position == tw->text.line_table[line_num+1].start_pos - 1)))
					 {
						position =  MAX(position - data->columns/2, line_num ? (int) (tw->text.line_table[line_num].start_pos): 0);
					 }
				 }
				PosToXY(tw, position, &x, &y);
			 }
#ifdef SUN_CTL
			/* Note that the interpretation of data->hoffset in CTL is based	*/
			/*	on the invariant edge, i.e., if the text is aligned with right	*/
			/*	edge then hoffset is the pixel length of amount of text which	*/
			/*	is beyond the right edge and for left edge aligned text, it is	*/
			/*	the pixel length of the text which falls left of the left edge.*/

			if (TextW_LayoutActive(tw))
			 {
				if (ISTEXT_RIGHTALIGNED(tw))	/*	Cursor beyond left edge so scroll to right.	*/
				 {										/*	Note that HRE applies here (see above).		*/
					if (x < data->leftmargin) 
					 {
						ChangeHOffset(tw, data->hoffset + data->leftmargin - x, True);
					 }
					else
					 {
						int	right_edge=tw->text.inner_widget->core.width - data->rightmargin;

						if (x > right_edge)
						 {								/*	(Cursor beyond right edge so scroll to left)	*/
							int	drawable_width=tw->text.inner_widget->core.width - data->rightmargin;
							int	extra=x - drawable_width;

							ChangeHOffset (tw, data->hoffset - extra, True);
						 }
					 }
					return;
				 }
			 }
#endif /* CTL */
			x += data->hoffset;
			if (x - data->hoffset < data->leftmargin)
			 {
				ChangeHOffset(tw, x - data->leftmargin, True);
			 }
			else if (x - data->hoffset > ((Position) (tw->text.inner_widget->core.width - data->rightmargin)))
			 {
				ChangeHOffset(tw, (int) (x) - (int) (tw->text.inner_widget->core.width) + (int) (data->rightmargin), True);
			 }
		 }
	 }
}


static void 
BlinkInsertionPoint(XmTextWidget tw)
{
	OutputData	data=tw->text.output->data;
  
	if ((data->cursor_on >=0) && 
		 (data->blinkstate == CurrentCursorState(tw)) &&
		 XtIsRealized((Widget)tw))
	 {											/*	Toggle blink state	*/
		if (data->blinkstate == on)
		 {
			data->blinkstate = off;
		 }
		else
		 {
			data->blinkstate = on;
		 }
		PaintCursor(tw);
	 }
}


static Boolean 
MoveLines(XmTextWidget tw, LineNum fromline, LineNum toline, LineNum destline)
{
	OutputData	data=tw->text.output->data;


	if (!XtIsRealized((Widget) tw))
	 {
		return False;
	 }

	_XmTextAdjustGC(tw);
	SetNormGC(tw, data->gc, False, False);
	SetFullGC(tw, data->gc);
	if(XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
	 {
		XCopyArea(XtDisplay(tw), XtWindow(tw->text.inner_widget),
					 XtWindow(tw->text.inner_widget), data->gc,
					 tw->text.inner_widget->core.width - data->rightmargin - (Position)data->linewidth * (toline + 1),
					 tw->primitive.shadow_thickness + tw->primitive.highlight_thickness,
					 (Position)data->linewidth * (toline - fromline + 1),
					 tw->text.inner_widget->core.height - 2 * (tw->primitive.shadow_thickness + tw->primitive.highlight_thickness),
					 tw->text.inner_widget->core.width - data->rightmargin - (Position)data->linewidth * (destline + toline - fromline + 1),
					 tw->primitive.shadow_thickness + tw->primitive.highlight_thickness);
	 }
	else
	 {
		XCopyArea(XtDisplay(tw), XtWindow(tw->text.inner_widget), XtWindow(tw->text.inner_widget),
					 data->gc, tw->primitive.shadow_thickness + tw->primitive.highlight_thickness,
					 (Position) data->lineheight * fromline + data->topmargin,
					 tw->text.inner_widget->core.width - 2 * (tw->primitive.shadow_thickness + tw->primitive.highlight_thickness),
					 data->lineheight * (toline - fromline + 1),
					 tw->primitive.shadow_thickness + tw->primitive.highlight_thickness,
					 (Position) data->lineheight * destline + data->topmargin);
	 }
	SetMarginGC(tw, data->gc);
	if(XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
	 {
		data->exposehscroll++;
	 }
	else
	 {
		data->exposevscroll++;
	 }
	return True;
}


static void 
OutputInvalidate(XmTextWidget tw, XmTextPosition position, XmTextPosition topos, long delta)
{
	_XmProcessLock();
	posToXYCachedWidget = NULL;
	_XmProcessUnlock();
}


static void 
RefigureDependentInfo(XmTextWidget tw)
{
	OutputData	data=tw->text.output->data;
  
	data->columns = data->number_lines;
	if(XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
	 {
		data->rows = (short)(((int) tw->core.height - (data->topmargin + data->bottommargin)) /
							(int) (data->font_ascent + data->font_descent));

		if (data->rows <= 0)
		 {
			data->rows = 1;
		 }
	 }
	else
	 {
		data->rows = data->number_lines;
		data->columns = (short)(((int) tw->core.width - (data->leftmargin + data->rightmargin)) /
								(data->averagecharwidth));
		if (data->columns <= 0)
		 {
			data->columns = 1;
		 }
	 }
}


static void 
SizeFromRowsCols(XmTextWidget tw, Dimension* width, Dimension* height)
{
	OutputData	data=tw->text.output->data;
	short			lines=0;
  

	if (tw->text.edit_mode == XmSINGLE_LINE_EDIT)
	 {
		lines = 1;
	 }
	else
	 {
		if(XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
		 {
			lines = data->columns_set;
		 }
		else
		 {
			lines = data->rows_set;
		 }
	 }
  
	if(XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
	 {
		*width = (Dimension)((lines * data->linewidth) + data->leftmargin + data->rightmargin);
		*height = (Dimension)((data->rows_set * (data->font_ascent + data->font_descent)) +
							data->topmargin + data->bottommargin);
		if ((tw->text.source->data->length > 0) && data->resizeheight)
		 {
			XmTextPosition		nextpos;
			LineTableExtra		extra=NULL;
			Boolean				past_end=False;
			int					i;

     		for (i=0; i<tw->text.number_lines && !past_end; i++)
			 {
				past_end = !MeasureLine(tw, i, tw->text.line[i].start, &nextpos, &extra);
				if (extra)
				 {
					if (extra->width > *height) 
					 {
						*height = extra->width;
					 }
					XtFree((char *)extra);
				 }
			 }
		 }
	 }
	else
	 {
		*width = (Dimension) ((data->columns_set * data->averagecharwidth) +
						data->leftmargin + data->rightmargin);
		if ((tw->text.source->data->length > 0) && data->resizewidth)
		 {
			XmTextPosition	nextpos;
			LineTableExtra	extra=NULL;
			Boolean			past_end=False;
			int				i;
    
			for (i=0; (i < tw->text.number_lines) && !past_end; i++)
			 {
				past_end = !MeasureLine(tw, i, tw->text.line[i].start, &nextpos, &extra);
				if (extra)
				 {
					if (extra->width > *width) 
					 {
						*width = extra->width;
					 }
					XtFree((char *)extra);
				 }
			 }
		 }
		*height = (Dimension)((lines * data->lineheight) + data->topmargin + data->bottommargin);
	 }
}


static void 
LoadFontMetrics(XmTextWidget tw)
{
	OutputData			data=tw->text.output->data;
	XmFontContext		context;
	XmFontListEntry	next_entry;
	XmFontType			type_return=XmFONT_IS_FONT;
	XtPointer			tmp_font;
	Boolean				have_font_struct=False;
	Boolean				have_font_set=False;
	XFontSetExtents *	fs_extents;
	XFontStruct *		font;
	unsigned long		width=0;
	char *				font_tag=NULL;
    
	if (!XmFontListInitFontContext(&context, data->fontlist))
	 {
		XmeWarning((Widget) tw, MSG3);
	 }

	do
	 {
		next_entry = XmFontListNextEntry(context);
		if (next_entry)
		 {
			tmp_font = XmFontListEntryGetFont(next_entry, &type_return);
	    
#ifdef SUN_CTL
			if ((type_return == XmFONT_IS_FONTSET) || (type_return == XmFONT_IS_XOC))
#else
			if (type_return == XmFONT_IS_FONTSET)
#endif
			 {
				font_tag = XmFontListEntryGetTag(next_entry);
				if (!have_font_set)
				 {
					data->use_fontset = True;					/* Save the first fontset found,	*/
					data->font = (XFontStruct *)tmp_font;	/*	just in case we don't find a	*/
																		/*	default tag set.					*/
#ifdef SUN_CTL
					data->rendition = (XmRendition)next_entry;
#endif
					have_font_struct = True;					/* we have a font set, so no		*/
																		/*	need to consider future ones	*/
					have_font_set = True;
		    
			 		if (!strcmp(XmFONTLIST_DEFAULT_TAG, font_tag))
					 {
						if (font_tag)
						 {
							XtFree(font_tag);
						 }
						break;
					 }
				 } 
				else if (!strcmp(XmFONTLIST_DEFAULT_TAG, font_tag))
				 {
					data->font = (XFontStruct *)tmp_font;
					have_font_set = True;
					if (font_tag)
					 {
						XtFree(font_tag);
					 }
					break;
				 }
			 } 
			else if (!have_font_struct)
			 {														/*	(Return_type must be XmFONT_IS_FONT)	*/
				data->use_fontset = False;
				data->font = (XFontStruct*)tmp_font;	/* save the first one in case no font set	*/
																	/*	is found.										*/
				data->use_fontset = False;
				/* Bug 4403332 -
				/* Implement fix for 4348643, which was implemented for XmTextField and not for XmText */
				/* Using a font instead of a fontset but still need to move the rendition pointer along */
#ifdef SUN_CTL
				data->rendition = (XmRendition)next_entry;
#endif /* SUN_CTL */
				have_font_struct = True;
			 }
		 }
	 }
	while (next_entry != NULL);							/*	(This is the end of a do...while)		*/

	if (!have_font_struct && !have_font_set)
	 {
		XmeWarning ((Widget)tw, MSG4);
	 }
    
	XmFontListFreeFontContext(context);
    
	if (data->use_fontset)
	 {
		fs_extents = XExtentsOfFontSet((XFontSet)data->font);
		if (XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
		 {
			width = (unsigned long)fs_extents->max_ink_extent.width;
		 }
		else
		 {
			width = (unsigned long)fs_extents->max_logical_extent.width;
		 }

		/*	max_logical_extent.y is number of pixels from origin	*/
		/*	to top of rectangle (i.e. y is negative)					*/

		data->font_ascent = -fs_extents->max_logical_extent.y;
		data->font_descent = fs_extents->max_logical_extent.height +
									fs_extents->max_logical_extent.y;
	 } 
	else
	 {
		font = data->font;
		data->font_ascent = font->max_bounds.ascent;
		data->font_descent = font->max_bounds.descent;
		if (XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
		 {
			width = font->max_bounds.rbearing - font->max_bounds.lbearing;
		 }
		else
		 {
			if ((!XGetFontProperty(font, XA_QUAD_WIDTH, &width)) || (width == 0))
			 {
				if (font->per_char && (font->min_char_or_byte2 <= '0') &&
					 (font->max_char_or_byte2 >= '0'))
				 {
					width = font->per_char['0' - font->min_char_or_byte2].width;
				 }
				else
				 {
					width = font->max_bounds.width;
				 }
			 }
		 }
	 }

	if (XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
	 {
		if (width == 0)
		 {
			width = 1;
		 }
		data->linewidth = width;
		data->averagecharwidth = (int) width;
		data->tabheight = (int)(8 * (data->font_ascent + data->font_descent));
    } 
	else
	 {
		data->lineheight = data->font_descent + data->font_ascent;
		if ((width == 0) || (width == (unsigned long)-1))
		 {
			width = 1;
		 }
		data->averagecharwidth = (int)width;		/* (This assumes there will be no truncation)	*/
		if (data->use_fontset)
		 {
			data->tabwidth = 8 * XmbTextEscapement((XFontSet)data->font, "0", 1);
			if (data->tabwidth == 0)					/*	Check if tabwidth was calculated correctly	*/
			 {
				data->tabwidth = (int)(8 * width);
			 }
		 } 
		else
		 {
			data->tabwidth = (int)(8 * width);
		 }
	 }
}


static Boolean 
SetXOCOrientation(XmTextWidget tw, XOC oc, XOrientation orientation)
{
	XOM				om;
	XOMOrientation orients;
	int				i=0;


	if (!oc)
	 {
		return False;
	 }

	om = XOMOfOC(oc);
	if (om)
	 {
		if (!XGetOMValues(om, XNQueryOrientation, &orients, NULL))
		 {
			for (i=0; i<orients.num_orientation; i++)
			 {
				if (orients.orientation[i] == orientation)
				 {
					if (XSetOCValues(oc, XNOrientation, orientation, NULL)!=NULL)
					 {
						return False;
					 }
				 }
			 }
		 }
		else
		 {
			return False;
		 }
	 }
	else
	 {
		return False;
	 }

	return True;
}


static void 
LoadGCs(XmTextWidget tw, Pixel background, Pixel foreground)
{
	OutputData		data=tw->text.output->data;
	XGCValues		values;
	unsigned long	valueMask=(GCFunction | GCForeground | GCBackground | GCGraphicsExposures),
						dynamicMask=GCClipMask,
						unusedMask=GCClipXOrigin | GCClipYOrigin | GCFont;


	/*	Get GC for saving area under cursor.	*/

	values.function = GXcopy;
	values.foreground = tw->primitive.foreground;
	values.background = tw->core.background_pixel;
	values.graphics_exposures = (Bool) False;
	if (data->save_gc != NULL)
	 {
		XtReleaseGC((Widget) tw, data->save_gc);
	 }
	data->save_gc = XtAllocateGC((Widget) tw, tw->core.depth, valueMask,
										  &values, dynamicMask, unusedMask);


	/* Get GC for drawing text.					*/

	if (!data->use_fontset)
	 {
		valueMask |= GCFont;
		values.font = data->font->fid;
	 } 
  
	values.graphics_exposures = (Bool) True;
	values.foreground = foreground ^ background;
	values.background = 0;
	if (data->gc != NULL)
	 {
		XtReleaseGC((Widget) tw, data->gc);
	 }
	dynamicMask |= GCForeground | GCBackground | GCFillStyle | GCStipple;
	data->gc = XtAllocateGC((Widget) tw, tw->core.depth, valueMask,
									&values, dynamicMask, 0);
  

	/*	Create a temporary GC - change it later in MakeIBeamStencil	*/

	valueMask |= GCStipple | GCFillStyle;
	values.graphics_exposures = (Bool) False;
	values.stipple = data->stipple_tile;
	values.fill_style = FillStippled;
	if (data->imagegc != NULL)
	 {
		XtReleaseGC((Widget) tw, data->imagegc);
	 }
	dynamicMask |= (GCTileStipXOrigin | GCTileStipYOrigin | GCFunction);
	data->imagegc = XtAllocateGC((Widget) tw, tw->core.depth,
										  valueMask, &values, dynamicMask, 0);
}


static void
MakeIBeamOffArea(XmTextWidget tw, Dimension width, Dimension height)
{
	OutputData	data=tw->text.output->data;
	Display *	dpy=XtDisplay(tw);
	Screen  *	screen=XtScreen((Widget)tw);
  

	/* Create a pixmap for storing the screen data where	*/
	/*	the I-Beam will be painted									*/

	data->ibeam_off = XCreatePixmap(dpy, RootWindowOfScreen(screen), width,
											  height, tw->core.depth);
	data->refresh_ibeam_off = True;
}


static Pixmap 
FindPixmap(Screen *screen, char *image_name, Pixel foreground, Pixel background, int depth)
{    
	XmAccessColorDataRec	acc_color_rec;

	acc_color_rec.foreground = foreground;
	acc_color_rec.background = background;
	acc_color_rec.top_shadow_color = XmUNSPECIFIED_PIXEL;
	acc_color_rec.bottom_shadow_color = XmUNSPECIFIED_PIXEL;
	acc_color_rec.select_color = XmUNSPECIFIED_PIXEL;
	acc_color_rec.highlight_color = XmUNSPECIFIED_PIXEL;
	return  _XmGetColoredPixmap(screen, image_name, &acc_color_rec, depth, True);
}


static void
MakeIBeamStencil(XmTextWidget tw, int line_width)
{
	Screen *			screen=XtScreen((Widget)tw);
	char				pixmap_name[17];
	XGCValues		values;
	unsigned long	valueMask;
	OutputData		data=tw->text.output->data;
  

	sprintf(pixmap_name, "_XmText_%d_%d", data->cursorheight, line_width);
	data->cursor = FindPixmap(screen, pixmap_name, 1, 0, 1);

	if (data->cursor == XmUNSPECIFIED_PIXMAP)
	 {
		Display *	dpy=XtDisplay(tw);
		XSegment		segments[3];


		/* Create a pixmap for the I-Beam stencil */
		data->cursor = XCreatePixmap(dpy, XtWindow(tw), data->cursorwidth,
											  data->cursorheight, 1);
		values.foreground = 0;
		values.line_width = 0;
		values.fill_style = FillSolid;
		values.function = GXcopy;
		valueMask = GCForeground | GCLineWidth | GCFillStyle | GCFunction;
		XChangeGC(dpy, data->cursor_gc, valueMask, &values);

		XFillRectangle(dpy, data->cursor, data->cursor_gc, 0, 0, data->cursorwidth,
							data->cursorheight);

		/* Change the GC for use in "cutting out" the I-Beam shape */
		values.foreground = 1;
		values.line_width = line_width;
		XChangeGC(dpy, data->cursor_gc, GCForeground | GCLineWidth, &values);

		/* Draw the segments of the I-Beam */
		if(XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
		 {
			/* 1st segment is the left vertical line of the cursor */
			segments[0].x1 = 1;
			segments[0].y1 = line_width - 1;
			segments[0].x2 = 1;
			segments[0].y2 = data->cursorheight - 1;

			/* 2nd segment is the right vertical line of the cursor */
			segments[1].x1 = data->cursorwidth - 1;
			segments[1].y1 = line_width - 1;
			segments[1].x2 = data->cursorwidth - 1;
			segments[1].y2 = data->cursorheight - 1;

			/* 3rd segment is the horizontal line of the cursor */
			segments[2].x1 = 0;
			segments[2].y1 = data->cursorheight >> 1;
			segments[2].x2 = data->cursorwidth;
			segments[2].y2 = data->cursorheight >> 1;
		 }
		else
		 {
			/* 1st segment is the top horizontal line of the 'I' */
			segments[0].x1 = 0;
			segments[0].y1 = line_width - 1;
			segments[0].x2 = data->cursorwidth;
			segments[0].y2 = line_width - 1;

			/* 2nd segment is the bottom horizontal line of the 'I' */
			segments[1].x1 = 0;
			segments[1].y1 = data->cursorheight - 1;
			segments[1].x2 = data->cursorwidth;
			segments[1].y2 = data->cursorheight - 1;

			/* 3rd segment is the vertical line of the 'I' */
			segments[2].x1 = data->cursorwidth >> 1;
			segments[2].y1 = line_width;
			segments[2].x2 = data->cursorwidth >> 1;
			segments[2].y2 = data->cursorheight - 1;
		 }

		 /* Draw the segments onto the cursor */
		 XDrawSegments(dpy, data->cursor, data->cursor_gc, segments, 3);
    
		/* Install the cursor for pixmap caching */
		_XmCachePixmap(data->cursor, screen, pixmap_name, 1, 0,
							1, data->cursorwidth, data->cursorheight);
	 }

	valueMask = (GCForeground | GCBackground | GCStipple | GCFillStyle);
	if (tw->text.input->data->overstrike)
	 {
		values.background =
		values.foreground = tw->core.background_pixel ^ tw->primitive.foreground;
	 }
	else
	 {
		values.foreground = tw->primitive.foreground;
		values.background = tw->core.background_pixel;
	 }
	values.stipple = data->cursor;
	values.fill_style = FillStippled;
	XChangeGC(XtDisplay(tw), data->imagegc, valueMask, &values);
}


 /* The IBeam Stencil must have already been created before this routine
  * is called.
  */
static void 
MakeAddModeCursor(XmTextWidget tw, int line_width)
{
	Screen *		screen=XtScreen((Widget)tw);
	char			pixmap_name[25];
	OutputData	data=tw->text.output->data;
  
	sprintf(pixmap_name, "_XmText_AddMode_%d_%d", data->cursorheight,line_width);

	data->add_mode_cursor = FindPixmap(screen, pixmap_name, 1, 0, 1);
  
	if (data->add_mode_cursor == XmUNSPECIFIED_PIXMAP)
	 {
		XtGCMask		valueMask;
		XGCValues	values;
		Display *	dpy=XtDisplay((Widget)tw);
    
		data->add_mode_cursor = XCreatePixmap(dpy, XtWindow((Widget)tw),
														  data->cursorwidth, data->cursorheight, 1);
		values.function = GXcopy;
		valueMask = GCFunction;
		XChangeGC(dpy, data->cursor_gc, valueMask, &values);

		XCopyArea(dpy, data->cursor, data->add_mode_cursor, data->cursor_gc, 0, 0,
					 data->cursorwidth, data->cursorheight, 0, 0);
    
		valueMask = (GCTile | GCFillStyle | GCForeground |
						 GCBackground | GCFunction | GCTileStipXOrigin);
		values.function = GXand;
		values.tile = data->stipple_tile;
		values.fill_style = FillTiled;
		values.ts_x_origin = -1;
		values.foreground = tw->primitive.foreground;
		values.background = tw->core.background_pixel;
    
		XChangeGC(XtDisplay((Widget)tw), data->cursor_gc, valueMask, &values);
    
		XFillRectangle(dpy, data->add_mode_cursor, data->cursor_gc,
							0, 0, data->cursorwidth, data->cursorheight);
    
		_XmCachePixmap(data->add_mode_cursor, screen, pixmap_name, 1, 0,
							1, data->cursorwidth, data->cursorheight);
	 }
}


static void 
MakeCursors(XmTextWidget tw)
{
	OutputData	data=tw->text.output->data;
	Screen *		screen=XtScreen(tw);
	int			line_width=1;
	int			oldwidth=data->cursorwidth; 
	int			oldheight=data->cursorheight;
  

	if (!XtIsRealized((Widget) tw))
	 {
		return;
	 }
  
	if(XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
	 {
		data->cursorwidth = data->averagecharwidth;
		data->cursorheight = 5;

		if (data->cursorwidth > 19)			/*	set up parameters to make a	*/
														/*	thicker I-Beam.					*/
		 {
			data->cursorheight++;
			line_width = 2;
		 }
	 }
	else
	 {
		data->cursorwidth = 5;
		data->cursorheight = data->font_ascent + data->font_descent;
  
		if (data->cursorheight > 19)
		 {
			data->cursorwidth++;
			line_width = 2;
		 }
	 }

	if ((data->cursor == XmUNSPECIFIED_PIXMAP) ||
		 (data->add_mode_cursor == XmUNSPECIFIED_PIXMAP) ||
		 (data->ibeam_off == XmUNSPECIFIED_PIXMAP) ||
		 (oldheight != data->cursorheight) ||
		 (oldwidth != data->cursorwidth))
	 {
		if (data->cursor_gc == NULL)
		 {
			unsigned long	valueMask=0;
			XGCValues		values;
			unsigned long	dynamicMask=GCForeground | GCLineWidth | GCTile | GCFillStyle |
												GCBackground | GCFunction | GCTileStipXOrigin;

			data->cursor_gc = XtAllocateGC((Widget)tw, 1, valueMask, &values, dynamicMask, 0);
		 }

		/* Remove old ibeam_off pixmap */
		if (data->ibeam_off != XmUNSPECIFIED_PIXMAP)
		 {
			XFreePixmap(XtDisplay((Widget)tw), data->ibeam_off);
		 }
	    
		if (data->cursor != XmUNSPECIFIED_PIXMAP)
		 {
			Xm21DestroyPixmap(screen, data->cursor);				/* Remove old insert stencil		*/
		 }
	    
		if (data->add_mode_cursor != XmUNSPECIFIED_PIXMAP)
		 {
			Xm21DestroyPixmap(screen, data->add_mode_cursor);	/*	Remove old add mode cursor		*/
		 }
	    
		/* Create area in which to save text located underneath I beam */
		MakeIBeamOffArea(tw, MAX((int) data->cursorheight,
							  (int)data->cursorheight >> 1), data->cursorheight);
	    
		MakeIBeamStencil(tw, line_width);							/* Create a new i-beam stencil	*/
		MakeAddModeCursor(tw, line_width);							/* Create a new add_mode cursor	*/
	 }

	_XmTextResetClipOrigin(tw, XmTextGetCursorPosition((Widget)tw), False);

	if (tw->text.input->data->overstrike)
	 {
		data->cursorwidth = data->cursorheight >> 1;
	 }
}


static void 
OutputGetValues(Widget w, ArgList args, Cardinal num_args)
{
	XmTextWidget	tw=(XmTextWidget)w;

	RefigureDependentInfo(tw);
	XtGetSubvalues((XtPointer) tw->text.output->data, output_resources,
						XtNumber(output_resources), args, num_args);
}


static Boolean
CKCols(ArgList args, Cardinal num_args)
{
	ArgList	arg;

	for (arg=args; num_args!=0; num_args--, arg++)
	 {
		if (strcmp(arg->name, XmNcolumns) == 0)
		 {
			return True;
		 }
	 }
	return False;
}


static Boolean
CKRows(ArgList args, Cardinal num_args)
{
	ArgList	arg;

	for (arg=args; num_args!=0; num_args--, arg++)
	 {
		if (strcmp(arg->name, XmNrows) == 0)
		 {
			return True;
		 }
	 }
	return False;
}


/* ARGSUSED */
static Boolean 
OutputSetValues(Widget			oldw,
					 Widget			reqw,
					 Widget			new_w,
					 ArgList			args,
					 Cardinal *		num_args
					)
{
#define CK(fld) (newdata->fld != data->fld)
#define CP(fld) (data->fld = newdata->fld)

	XmTextWidget	oldtw=(XmTextWidget)oldw;
	XmTextWidget	newtw=(XmTextWidget)new_w;
	OutputData		data=newtw->text.output->data;
	OutputDataRec	newdatarec;
	OutputData		newdata=&newdatarec;
	Boolean			needgcs=False;
	Boolean			newsize=False;
	Boolean			newcols=False;
	Boolean			o_redisplay=False;
	Dimension		new_width=newtw->core.width;		/* save in case something changes	*/
	Dimension		new_height=newtw->core.height;	/* these values during SetValues		*/
	XPoint			xmim_point;
	XRectangle		xmim_area;
	Arg				im_args[17];
	Cardinal			n=0;

	*newdata = *data;
	XtSetSubvalues((XtPointer) newdata, output_resources,
						XtNumber(output_resources), args, *num_args);

	if (newtw->core.background_pixel != oldtw->core.background_pixel)
	 {
		needgcs = True;
	 }

	if (newtw->primitive.foreground != oldtw->primitive.foreground)
	 {
		needgcs = True;
	 }

#ifdef SUN_CTL
	if (CK(alignment))
	 {
		CP(alignment);
		o_redisplay = True;
	 }
#endif /* CTL */

	if (CK(fontlist) || CK(rendertable))
	 {
		XmRenderTableFree(data->fontlist);
		if (CK(rendertable))
		 {
			if (newdata->rendertable == NULL)
			 {
				newdata->fontlist = XmeGetDefaultRenderTable(new_w, XmTEXT_FONTLIST);
			 }
			else
			 {
				newdata->fontlist = XmRenderTableCopy(newdata->rendertable, NULL, 0);
			 }
		 }
		else if (CK(fontlist))
		 {
			if (newdata->fontlist == NULL)
			 {
				newdata->fontlist = XmeGetDefaultRenderTable(new_w, XmTEXT_FONTLIST);
			 }
			else
			 {
				newdata->fontlist = XmRenderTableCopy(newdata->fontlist, NULL, 0);
			 }
		 }
		newdata->rendertable = newdata->fontlist;
		CP(fontlist);
		CP(rendertable);
		LoadFontMetrics(newtw);

		/* We want to be able to connect to an IM if XmNfontList has changed. */
		if (newtw->text.editable)
		 {
			newtw->text.editable = False;
			XmTextSetEditable(new_w, True);
		 }

		if(XmDirectionMatch(XmPrim_layout_direction(newtw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
		 {
			if (data->vbar)
			 {
				XmNavigatorDataRec	nav_data;
				int						new_sliderSize=0;

				data->ignorevbar = True;

				new_sliderSize = newtw->text.inner_widget->core.height -
										(data->topmargin + data->bottommargin);

				if (new_sliderSize < 1) new_sliderSize = 1;
				if (new_sliderSize > data->scrollheight)
					new_sliderSize = data->scrollheight;

				nav_data.value.y = data->voffset;
				nav_data.minimum.y = 0;
				nav_data.maximum.y = data->scrollheight;
				nav_data.slider_size.y = new_sliderSize;
				nav_data.increment.y = data->font_ascent + data->font_descent;
				nav_data.page_increment.y = 0;

				nav_data.dimMask = NavigDimensionY;
				nav_data.valueMask = NavValue|NavMinimum|NavMaximum|NavSliderSize|NavIncrement;
				_XmSFUpdateNavigatorsValue(XtParent(new_w), &nav_data, True);

				data->ignorevbar = False;

			 }
		 }
		else
		 {
			if (data->hbar)
			 {
				XmNavigatorDataRec nav_data;
				int new_sliderSize;

				data->ignorehbar = True;

				new_sliderSize = newtw->text.inner_widget->core.width -
										(data->leftmargin + data->rightmargin);

				if (new_sliderSize < 1) new_sliderSize = 1;
				if (new_sliderSize > data->scrollwidth)
					new_sliderSize = data->scrollwidth;

				nav_data.value.x = data->hoffset;
				nav_data.minimum.x = 0;
				nav_data.maximum.x = data->scrollwidth;
				nav_data.slider_size.x = new_sliderSize;
				nav_data.increment.x = data->averagecharwidth;
				nav_data.page_increment.x = 0;

				nav_data.dimMask = NavigDimensionX;
				nav_data.valueMask = NavValue|NavMinimum|NavMaximum|NavSliderSize|NavIncrement;
				_XmSFUpdateNavigatorsValue(XtParent(new_w), &nav_data, True);

				data->ignorehbar = False;
			 }
		 }
		o_redisplay = True;
		needgcs = True;
		newsize = True;
	 }

	if ((data->fontlist != oldtw->text.output->data->fontlist) ||
		 (newtw->core.background_pixel != oldtw->core.background_pixel) ||
		 (newtw->primitive.foreground != oldtw->primitive.foreground))
	 {
		XtSetArg(im_args[n], XmNbackground, newtw->core.background_pixel); n++;
		XtSetArg(im_args[n], XmNforeground, newtw->primitive.foreground); n++;
		XtSetArg(im_args[n], XmNfontList, data->fontlist); n++;
	 }


	/* Don't word wrap, have multiple row or have vertical	*/
	/*	scrollbars if editMode is single_line						*/

	if (newtw->text.edit_mode != oldtw->text.edit_mode)
	 {
		if(XmDirectionMatch(XmPrim_layout_direction(newtw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
		 {
			if (newtw->text.edit_mode == XmSINGLE_LINE_EDIT)
			 {
				newdata->rows = 1;
				o_redisplay = True;
				if (data->hbar) XtUnmanageChild(data->hbar);
			 }
			else
			 {
				if (data->hbar) XtManageChild(data->hbar);
			 }
			if (newtw->text.edit_mode == XmSINGLE_LINE_EDIT)
			 {
				newdata->rows = 1;
				o_redisplay = True;
				if (data->vbar)	XtUnmanageChild(data->vbar);
			 }
			else
			 {
				if (data->vbar) XtManageChild(data->vbar);
			 }
		 }
	 }

	/*	what is called margin, in this code, is composed of margin, shadow, and	*/
	/*	highlight. Previously, only margin was accomodated. This addition may	*/
	/*	not be very clever, but it blends in with the rest of the way this code	*/
	/*	works.																						*/

	if ((newtw->text.margin_width != oldtw->text.margin_width) ||
		 (newtw->text.margin_height != oldtw->text.margin_height) ||
		 (newtw->primitive.shadow_thickness != oldtw->primitive.shadow_thickness) ||
		 (newtw->primitive.highlight_thickness != oldtw->primitive.highlight_thickness)
		)
	 {
		data->leftmargin = data->rightmargin = newtw->text.margin_width +
									newtw->primitive.shadow_thickness +
									newtw->primitive.highlight_thickness;
		data->topmargin = data->bottommargin = newtw->text.margin_height +
									newtw->primitive.shadow_thickness +
									newtw->primitive.highlight_thickness;
		o_redisplay = True;
		newsize = True;
	 }

	if (CK(wordwrap))
	 {
		if(XmDirectionMatch(XmPrim_layout_direction(newtw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
		 {
			/* If we are turning on wrapping, we don't want any horiz. offset */
			if (!data->wordwrap)
				ChangeVOffset(newtw, 0, True);

			if (data->vbar)
			 {
				if (newdata->wordwrap)
				 {
					XmNavigatorDataRec nav_data;

					data->ignorevbar = True;

					nav_data.value.y = 0;
					nav_data.minimum.y = 0;
					nav_data.maximum.y = 1;
					nav_data.slider_size.y = 1;
					nav_data.increment.y = 0;
					nav_data.page_increment.y = 0;

					nav_data.dimMask = NavigDimensionY;
					nav_data.valueMask = NavValue|NavMinimum|NavMaximum|
						NavSliderSize;
					_XmSFUpdateNavigatorsValue(XtParent(new_w), &nav_data, True);

					data->ignorevbar = False;

					data->voffset = 0;
				 }
				else
				 {
					_XmRedisplayVBar(newtw);
				 }
			 }
		 }
		else
		 {
			/* If we are turning on wrapping, we don't want any horiz. offset */
			if (!data->wordwrap)
				ChangeHOffset(newtw, 0, True);

			if (data->hbar)
			 {
				if (newdata->wordwrap)
				 {
					XmNavigatorDataRec nav_data;

					data->ignorehbar = True;

					nav_data.value.x = 0;
					nav_data.minimum.x = 0;
					nav_data.maximum.x = 1;
					nav_data.slider_size.x = 1;
					nav_data.increment.x = 0;
					nav_data.page_increment.x = 0;

					nav_data.dimMask = NavigDimensionX;
					nav_data.valueMask = NavValue|NavMinimum|NavMaximum|
						NavSliderSize;
					_XmSFUpdateNavigatorsValue(XtParent(new_w), &nav_data, True);

					data->ignorehbar = False;

					data->hoffset = 0;
				 }
				else
				 {
					_XmRedisplayHBar(newtw);
				 }
			 }
		 }

		CP(wordwrap);

		_XmTextRealignLineTable(newtw, NULL, 0, 0, 0, PASTENDPOS);

		/* If we've just turned off wrapping, get new top_character by scanning */
		/* left from the current top character until we find a new line.			*/

		if (!oldtw->text.output->data->wordwrap)
		 {
			if (data->resizeheight)
			 {
				newtw->text.top_character = newtw->text.new_top = 0;
			 }
			else
			 {
				newtw->text.top_character =
					(*newtw->text.source->Scan)(newtw->text.source, newtw->text.top_character,
														 XmSELECT_LINE, XmsdLeft, 1, False);
				newtw->text.new_top = newtw->text.top_character;
			 }
		 }

		if (newtw->text.top_character)
			newtw->text.top_line = CountLines(newtw, 0, newtw->text.top_character);

		o_redisplay = True;
	 }

	if (data->hasfocus && XtIsSensitive((Widget)newtw) && CK(blinkrate))
	 {
		if (newdata->blinkrate == 0)
		 {
			data->blinkstate = on;
			if (data->timerid)
			 {
				XtRemoveTimeOut(data->timerid);
				/* Fix for bug 1254749	*/
				data->timerid = (XtIntervalId)NULL;
			 }
		 }
		else if (data->timerid == (XtIntervalId)0)
		 {
			data->timerid = XtAppAddTimeOut(XtWidgetToApplicationContext(new_w),
													  (unsigned long) newdata->blinkrate,
													  HandleTimer, (XtPointer) newtw);
		 }
	 }
	CP(blinkrate);

	CP(resizewidth);
	CP(resizeheight);

	CP(cursor_position_visible);

	if (needgcs)
	 {
		EraseInsertionPoint(newtw);
		LoadGCs(newtw, newtw->core.background_pixel, newtw->primitive.foreground);
		if (XtIsRealized(new_w))
		 {
			MakeCursors(newtw);
		 }
		TextDrawInsertionPoint(newtw);
		o_redisplay = True;
	 }

	if (newdata->rows <= 0)
	 {
		XmeWarning(new_w, MSG1);
		newdata->rows = data->rows;
	 }

	if (newdata->columns <= 0)
	 {
		XmeWarning(new_w, MSG2);
		newdata->columns = data->columns;
	 }


	/* Process arglist to verify that a value is being set.	*/

	if (CKCols(args, *num_args))
	 {
		data->columns_set = newdata->columns_set = newdata->columns;
		newcols = True;
	 }
	if (CKRows(args, *num_args))
	 {
		data->rows_set = newdata->rows_set = newdata->rows;
	 }

	if ((new_width == oldtw->core.width) || (new_height == oldtw->core.height))
	 {
		if (CK(columns) || CK(rows) || newsize)
		 {
			Dimension	width, height;

			SizeFromRowsCols(newtw, &width, &height);
			if(XmDirectionMatch(XmPrim_layout_direction(newtw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
			 {
				if (new_width == oldtw->core.width)
				 {
					newtw->core.width = width;
				 }
				data->minwidth = newtw->core.width;
				if (new_height == oldtw->core.height)
				 {
					newtw->core.height = height;
					data->minheight = (Dimension)((data->rows_set *
															(data->font_ascent + data->font_descent)) +
																data->topmargin + data->bottommargin);
				 }
				else
				 {
					data->minheight = new_height;
				 }
			 }
			else
			 {
			   if ((new_width == oldtw->core.width) && newcols)
				 {
					newtw->core.width = width;
					data->minwidth = (Dimension)((data->columns_set * data->averagecharwidth) +
															data->leftmargin + data->rightmargin);
				 }
				else
				 {
					data->minwidth = new_width;
				 }
				if (new_height == oldtw->core.height)
				 {
					newtw->core.height = height;
				 }
				data->minheight = newtw->core.height;
			 }
			CP(columns);
			CP(rows);
			o_redisplay = True;
		 }
	 }
	else
	 {
		newtw->core.width = new_width;
		data->minwidth = new_width;
		newtw->core.height = new_height;
		data->minheight = new_height;
	 }

	PosToXY(newtw, newtw->text.cursor_position, &xmim_point.x, &xmim_point.y);
	(void)_XmTextGetDisplayRect((Widget)newtw, &xmim_area);
	XtSetArg(im_args[n], XmNbackgroundPixmap, newtw->core.background_pixmap); n++;
	XtSetArg(im_args[n], XmNspotLocation, &xmim_point); n++;
	XtSetArg(im_args[n], XmNarea, &xmim_area); n++;
	XtSetArg(im_args[n], XmNlineSpace, newdata->lineheight); n++;
	XmImSetValues(new_w, im_args, n);

	return (o_redisplay);
#undef CK
#undef CP
}


static void 
NotifyResized(Widget w, Boolean o_create)
{
	XmTextWidget	tw=(XmTextWidget)w;
	OutputData		data=tw->text.output->data;
	Boolean			resizewidth=data->resizewidth;
	Boolean			resizeheight=data->resizeheight;
	XmTextPosition	linestart=0;
	XmTextPosition	position;
	XPoint			xmim_point;
	XRectangle		xmim_area;
	int				n;
	XmTextBlockRec	block;
	Arg				args[10];

	data->resizewidth = data->resizeheight = False;
	if(XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
	 {
		data->number_lines = tw->text.inner_widget->core.width -
									data->leftmargin - data->rightmargin;
		if (data->number_lines < (int) data->linewidth || !data->linewidth)
			data->number_lines = 1;
		else
			data->number_lines /= (int) data->linewidth;

		if (tw->text.top_character)
		 {
			tw->text.top_line = CountLines(tw, 0, tw->text.top_character);
		 }
	 }
	else
	 {
		data->number_lines = tw->text.inner_widget->core.height -
									data->topmargin - data->bottommargin;
		if (data->number_lines < (int) data->lineheight || !data->lineheight)
			data->number_lines = 1;
		else
			data->number_lines /= (int) data->lineheight;

		if (tw->text.top_character)
		 {
			tw->text.top_line = CountLines(tw, 0, tw->text.top_character);
		 }
	 }

	if (data->vbar)
	 {
		int						local_total, new_size;
		XmNavigatorDataRec	nav_data;

		if(XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
		 {
			new_size = tw->text.inner_widget->core.height
							- (data->topmargin + data->bottommargin);
			if (new_size < 1)							new_size = 1;
			if (new_size > data->scrollheight)	new_size = data->scrollheight;

			data->ignorevbar = True;

			nav_data.value.y = data->voffset;
			nav_data.minimum.y = 0;
			nav_data.maximum.y = data->scrollheight;
			nav_data.slider_size.y = new_size;
			nav_data.page_increment.y = new_size;
		 }
		else
		 {
			data->ignorevbar = True;

			if (tw->text.top_line + tw->text.number_lines > tw->text.total_lines)
				local_total = tw->text.top_line + tw->text.number_lines;
			else
				local_total = tw->text.total_lines;

			if (local_total >= tw->text.number_lines)
				new_size = tw->text.number_lines;
			else
				new_size = local_total;
			if (new_size + tw->text.top_line > local_total)
				new_size = local_total - tw->text.top_line;

			nav_data.value.y = tw->text.top_line;
			nav_data.minimum.y = 0;
			nav_data.maximum.y = local_total;
			nav_data.slider_size.y = new_size;
			nav_data.page_increment.y = (data->number_lines > 1) ?
													(data->number_lines - 1) : 1;
		 }

		nav_data.dimMask = NavigDimensionY;
		nav_data.valueMask = NavValue|NavMinimum|NavMaximum|NavSliderSize|NavPageIncrement;
		_XmSFUpdateNavigatorsValue(XtParent((Widget)tw), &nav_data, True);

		data->ignorevbar = False;
	 }

	if (data->hbar)
	 {
		XmNavigatorDataRec	nav_data;
		int						new_size = 0;
		int						local_total = 0;

		if(XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
		 {
			data->ignorehbar = True;

			if (tw->text.top_line + tw->text.number_lines > tw->text.total_lines)
				local_total = tw->text.top_line + tw->text.number_lines;
			else
				local_total = tw->text.total_lines;

			if (local_total >= tw->text.number_lines)
				new_size = tw->text.number_lines;
			else
				new_size = local_total;
			if (new_size + tw->text.top_line > local_total)
				new_size = local_total - tw->text.top_line;

			nav_data.value.x = tw->text.top_line;
			nav_data.minimum.x = 0;
			nav_data.maximum.x = local_total;
			nav_data.slider_size.x = new_size;
			nav_data.page_increment.x = (data->number_lines > 1) ?
													(data->number_lines - 1) : 1;
		 }
		else
		 {
			new_size = tw->text.inner_widget->core.width - (data->leftmargin + data->rightmargin);
			if (new_size < 1)							new_size = 1;
			if (new_size > data->scrollwidth)	new_size = data->scrollwidth;

			data->ignorehbar = True;

			nav_data.value.x = data->hoffset;
			nav_data.minimum.x = 0;
			nav_data.maximum.x = data->scrollwidth;
			nav_data.slider_size.x = new_size;
			nav_data.page_increment.x = new_size;
		 }

		nav_data.dimMask = NavigDimensionX;
		nav_data.valueMask = NavValue|NavMinimum|NavMaximum|
		NavSliderSize|NavPageIncrement;
		_XmSFUpdateNavigatorsValue(XtParent((Widget)tw), &nav_data, True);

		data->ignorehbar = False;
	 }

	RefigureDependentInfo(tw);
	if (resizewidth)
		data->columns_set = data->columns;
	if (resizeheight)
		data->rows_set = data->rows;

	if (XtIsRealized(w))
	 {
		XClearWindow(XtDisplay(tw), XtWindow(tw->text.inner_widget));
		data->refresh_ibeam_off = True;
	 }

	if (!o_create)					/* False only if called from OutputCreate */
		_XmTextInvalidate(tw, (XmTextPosition) 0, (XmTextPosition) 0, NODELTA);

	/* the new size grew enough to include new text */

	if (XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
	 {
		if (tw->text.edit_mode == XmSINGLE_LINE_EDIT)
		 {								/* this assumes only one line of text! (linestart = 0) */
#ifdef AS_TEXTFIELD
			ChangeVOffset(tw, 0, True);
#else
			int	text_height = data->topmargin; /* to make tab calculation correct */
			int	new_height = tw->core.height - (data->topmargin + data->bottommargin);

			position = (*tw->text.source->Scan)(tw->text.source, linestart,
															XmSELECT_LINE, XmsdRight, 1, False);
			while (linestart < position)
			 {
				linestart = (*tw->text.source->ReadSource)(tw->text.source,
																		 linestart, position, &block);
				text_height += FindHeight(tw, text_height, &block, 0, block.length);
			 }
			text_height -= data->topmargin;
			if (text_height - new_height < data->voffset)
			 {													/* space top over		*/
				if (text_height - new_height >= 0)
					ChangeVOffset(tw, text_height - new_height, True);
				else
					ChangeVOffset(tw, 0, True);
			 }
#endif
			if (tw->text.auto_show_cursor_position)
			 {
				MakePositionVisible(tw, tw->text.cursor_position);
			 }
		 }
		else
		 {
			_XmRedisplayVBar(tw);
		 }
	 }
	else
	 {
		if (tw->text.edit_mode == XmSINGLE_LINE_EDIT)
		 {										/* this assumes only one line of text! (linestart = 0) */
#ifdef AS_TEXTFIELD
			ChangeHOffset(tw, 0, True);
#else
			int	text_width = data->leftmargin; /* to make tab calculation correct */
			int	new_width = tw->core.width - (data->leftmargin + data->rightmargin);

			position = (*tw->text.source->Scan)(tw->text.source, linestart,
															XmSELECT_LINE, XmsdRight, 1, False);
			while (linestart < position)
			 {
				linestart = (*tw->text.source->ReadSource)(tw->text.source,
																		 linestart, position, &block);
				text_width += FindWidth(tw, text_width, &block, 0, block.length);
		 	 }
			text_width -= data->leftmargin;
			if (text_width - new_width < data->hoffset)
			 {															/*	space left over	*/
				if (text_width - new_width >= 0)
					ChangeHOffset(tw, text_width - new_width, True);
				else
					ChangeHOffset(tw, 0, True);
			 }
#endif
			if (tw->text.auto_show_cursor_position)
			 {
				MakePositionVisible(tw, tw->text.cursor_position);
			 }
		 }
		else
		 {
			_XmRedisplayHBar(tw);
		 }
	 }

	data->resizewidth = resizewidth;
	data->resizeheight = resizeheight;

	if (XtIsRealized(w))
		_XmTextDrawShadow(tw);

	/* Text is now rediplayed at the correct location, so force the tw to
	* refresh the putback area.
	*/

	data->refresh_ibeam_off = True;

	/* Somehow we need to let the input method know that the window has
	* changed size (for case of over-the-spot).	Try telling it that
	* the cursor position has changed and hopefully it will re-evaluate
	* the position/visibility/... of the pre-edit window.
	*/

	PosToXY(tw, tw->text.cursor_position, &xmim_point.x, &xmim_point.y);
	(void)_XmTextGetDisplayRect((Widget)tw, &xmim_area);
	n = 0;
	XtSetArg(args[n], XmNspotLocation, &xmim_point); n++;
	XtSetArg(args[n], XmNarea, &xmim_area); n++;
	XmImSetValues(w, args, n);
}


static void 
HandleTimer(XtPointer closure, XtIntervalId* id)
{
	XmTextWidget	tw=(XmTextWidget)closure;
	OutputData		data=tw->text.output->data;

	if (data->blinkrate != 0)
	 {
		data->timerid = XtAppAddTimeOut(XtWidgetToApplicationContext((Widget) tw),
												  (unsigned long)data->blinkrate,
												  HandleTimer, (XtPointer)closure);
	 }
	if (data->hasfocus && XtIsSensitive((Widget)tw))
	 {
		BlinkInsertionPoint(tw);
	 }
}


/*****************************************************************************
 * To make TextOut a True "Object" this function should be a class function. *
 *****************************************************************************/
void 
_XmTextChangeBlinkBehavior(XmTextWidget tw, Boolean newvalue)
{
	OutputData	data=tw->text.output->data;
  
	if (newvalue)
	 {
		if ((data->blinkrate != 0) && (data->timerid == (XtIntervalId)0))
		 {
			data->timerid = XtAppAddTimeOut(XtWidgetToApplicationContext((Widget) tw),
													  (unsigned long)data->blinkrate,
													  HandleTimer, (XtPointer)tw);
		 }
		data->blinkstate = on;
	 }
	else
	 {
		if (data->timerid)
		 {
			XtRemoveTimeOut(data->timerid);
			data->timerid = (XtIntervalId)NULL;
		 }
	 }
}


static void
HandleFocusEvents(Widget w, XtPointer closure, XEvent* event, Boolean* cont)
{
	XmTextWidget			tw=(XmTextWidget)w;
	OutputData				data=tw->text.output->data;
	Boolean					newhasfocus=data->hasfocus;
	XmAnyCallbackStruct	cb;
	XPoint					xmim_point;
	XRectangle				xmim_area;
	Arg						args[10];
	int						n=0;
  

	PosToXY(tw, tw->text.cursor_position, &xmim_point.x, &xmim_point.y);
	_XmTextGetDisplayRect((Widget)tw, &xmim_area);
  
	switch (event->type)
	 {
		case FocusIn:
			if (event->xfocus.send_event && !(newhasfocus))
			 {
				cb.reason = XmCR_FOCUS;
				cb.event = event;
				XtCallCallbackList (w, tw->text.focus_callback, (Opaque) &cb);
				newhasfocus = True;

				n = 0;
				XtSetArg(args[n], XmNspotLocation, &xmim_point); n++;
				XtSetArg(args[n], XmNarea, &xmim_area); n++;
				XmImSetFocusValues(w, args, n);
			 }
			break;

		case FocusOut:
			if (event->xfocus.send_event && newhasfocus)
			 {
				newhasfocus = False;
				XmImUnsetFocus(w);
			 }
			break;

		case EnterNotify:
			if ((_XmGetFocusPolicy(w) != XmEXPLICIT) && !(newhasfocus) &&
				 event->xcrossing.focus && (event->xcrossing.detail != NotifyInferior))
			 {
				cb.reason = XmCR_FOCUS;
				cb.event = event;
				XtCallCallbackList (w, tw->text.focus_callback, (Opaque) &cb);
				newhasfocus = True;
				n = 0;
				XtSetArg(args[n], XmNspotLocation, &xmim_point); n++;
				XtSetArg(args[n], XmNarea, &xmim_area); n++;
				XmImSetFocusValues(w, args, n);
			 }
			break;

		case LeaveNotify:
			if ((_XmGetFocusPolicy(w) != XmEXPLICIT) && newhasfocus &&
				 event->xcrossing.focus && (event->xcrossing.detail != NotifyInferior))
			 {
				newhasfocus = False;
				XmImUnsetFocus(w);
			 }
			break;
	 }

	if (newhasfocus != data->hasfocus)
	 {
		if (newhasfocus && XtIsSensitive((Widget)tw))
		 {
			EraseInsertionPoint(tw);
			data->hasfocus = newhasfocus;
			data->blinkstate = off;
			_XmTextChangeBlinkBehavior(tw, True);
			TextDrawInsertionPoint(tw);
		 }
		else
		 {
			_XmTextChangeBlinkBehavior(tw, False);
			EraseInsertionPoint(tw);
			data->hasfocus = newhasfocus;
			data->blinkstate = on;
			TextDrawInsertionPoint(tw);
		 }
	 }
}


static void 
HandleGraphicsExposure(Widget w, XtPointer closure, XEvent* event, Boolean* cont)
{
	XmTextWidget	tw=(XmTextWidget)w;
	OutputData		data=tw->text.output->data;

	if (event->xany.type == GraphicsExpose)
	 {
		XGraphicsExposeEvent *	xe=(XGraphicsExposeEvent *)event;

		if (data->exposehscroll != 0)
		 {
			xe->x = 0;
			xe->width = tw->core.width;
		 }
		if (data->exposevscroll != 0)
		 {
			xe->y = 0;
			xe->height = tw->core.height;
		 }
		RedrawRegion(tw, xe->x, xe->y, xe->width, xe->height);
		if (xe->count == 0)
		 {
			if (data->exposehscroll)
			 {
				data->exposehscroll--;
			 }
			if (data->exposevscroll)
			 {
				data->exposevscroll--;
			 }
		 }
	 }

	if (event->xany.type == NoExpose)
	 {
		if (data->exposehscroll)
		 {
			data->exposehscroll--;
		 }
		if (data->exposevscroll)
		 {
			data->exposevscroll--;
		 }
	 }
}


static void 
OutputRealize(Widget w, XtValueMask* valueMask, XSetWindowAttributes* attributes)
{
	XmTextWidget	tw=(XmTextWidget)w;
  
	XtCreateWindow(w, (unsigned int)InputOutput, (Visual *)CopyFromParent,
						*valueMask, attributes);
	posToXYCachedWidget = NULL;
	tw->text.needs_refigure_lines = TRUE;
	MakeCursors(tw);
}


static void 
OutputDestroy(Widget w)
{
	XmTextWidget	tw=(XmTextWidget)w;
	OutputData		data=tw->text.output->data;
  
	if (data->timerid)
	 {
		XtRemoveTimeOut(data->timerid);
		data->timerid = (XtIntervalId) NULL;
	 }

	XtRemoveEventHandler((Widget) tw->text.inner_widget,
								FocusChangeMask|EnterWindowMask|LeaveWindowMask,
								False, HandleFocusEvents, NULL);
  
	XtRemoveEventHandler((Widget) tw->text.inner_widget, (EventMask)0, True,
								HandleGraphicsExposure, NULL);
  
	XtReleaseGC(w, data->imagegc);
	XtReleaseGC(w, data->gc);
	XtReleaseGC(w, data->save_gc);
	XtReleaseGC(w, data->cursor_gc);
  
	XmFontListFree(data->fontlist);
  
	if (data->add_mode_cursor != XmUNSPECIFIED_PIXMAP)
	 {
		Xm21DestroyPixmap(XtScreen(tw), data->add_mode_cursor);
	 }
  
	if (data->cursor != XmUNSPECIFIED_PIXMAP)
	 {
		Xm21DestroyPixmap(XtScreen(tw), data->cursor);
	 }
  
	if (data->ibeam_off != XmUNSPECIFIED_PIXMAP)
	 {
		XFreePixmap(XtDisplay((Widget)tw), data->ibeam_off);
	 }
  
	XtFree((char *)data);
	XtFree((char *)tw->text.output);
	_XmProcessLock();
	posToXYCachedWidget = NULL;
	_XmProcessUnlock();
}


static void 
RedrawRegion(XmTextWidget tw, int x, int y, int width, int height)
{
	OutputData		data=tw->text.output->data;
	int				i;
	XmTextPosition	first, last;


	if(XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
	 {
		for (i=x; i<(int)(x + width + data->linewidth); i+=data->linewidth)
		 {
			first = XYToPos(tw, i, y);
			last = XYToPos(tw, i, y + height);
			first = (*tw->text.source->Scan)(tw->text.source, first, XmSELECT_POSITION,
														XmsdLeft, 1, True);
			last = (*tw->text.source->Scan)(tw->text.source, last, XmSELECT_POSITION,
													  XmsdRight, 1, True);
			_XmTextMarkRedraw(tw, first, last);
		 }
	 }
	else
	 {
		for (i=y; i<(int)(y + height + data->lineheight); i+=data->lineheight)
		 {
			first = XYToPos(tw, x, i);
			last = XYToPos(tw, x + width, i);
			first = (*tw->text.source->Scan)(tw->text.source, first, XmSELECT_POSITION,
														XmsdLeft, 1, True);
			last = (*tw->text.source->Scan)(tw->text.source, last, XmSELECT_POSITION,
													  XmsdRight, 1, True);
#ifdef SUN_CTL
			/* Note that swapping is required as the _XmTextMarkRedraw	*/
			/*	is based on the assumtion that left < right, but for		*/
			/*	bidirectional text this doesn't hold good.					*/

			if (TextW_LayoutActive(tw))
			 {
				if (last < first)
				 {										/* then swap them */
					XmTextPosition	temp=first;

					first = last;
					last = temp;
				 }
				else if (last == first)
				 {
					LineNum			line;
					XmTextPosition	line_start, next_line_start;

					line = _XmTextPosToLine(tw, first);			/*	Get the line text	*/
					if (line != NOLINE)
					 {
						_XmTextLineInfo(tw, line, &line_start, NULL);
						_XmTextLineInfo(tw, line + 1, &next_line_start, NULL);

						if (next_line_start == PASTENDPOS)
						 {
							next_line_start = tw->text.last_position;
						 }
			
						first = line_start;
						last = next_line_start;
					 }
				 }
			 }
#endif /* CTL */
			_XmTextMarkRedraw(tw, first, last);
		 }
	 }
} 


static void 
OutputExpose(Widget w, XEvent *event, Region region)
{
	XmTextWidget	tw=(XmTextWidget)w;
	XExposeEvent *	xe=(XExposeEvent *)event;
	OutputData		data=tw->text.output->data;
	Boolean			erased_cursor=False;
	int				old_number_lines=data->number_lines;
	Arg				im_args[10];
	XRectangle		xmim_area;
	XPoint			xmim_point;
	int				n=0;
	Boolean			font_may_have_changed=False;
	int				offset=0;
  

	if (tw->text.in_setvalues)					/*	If here via SetValues, force (x,y) of	*/
	 {													/*	IM and clip origin for I-beam in case	*/
														/*	font changed.									*/
		tw->text.in_setvalues = False;
		font_may_have_changed = True;
	 }
  
	if (event->xany.type != Expose)
	 {
		return;
	 }
  
	if (XtIsSensitive(w) && data->hasfocus)
	 {
		_XmTextChangeBlinkBehavior(tw, False);
	 }

	EraseInsertionPoint(tw);
  
	if(XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
	 {
		data->number_lines = tw->text.inner_widget->core.width -
									data->leftmargin - data->rightmargin;
		if ((data->number_lines < (int)data->linewidth) || !data->linewidth)
		 {
			data->number_lines = 1;
		 }
		else
		 {
			data->number_lines /= (int) data->linewidth;
		 }
  
		if (data->hbar && (old_number_lines != data->number_lines))
		 {
			int						local_total, new_size;
			XmNavigatorDataRec	nav_data;
      
			data->ignorehbar = True;
			if (tw->text.top_line + tw->text.number_lines > tw->text.total_lines)
			 {
				local_total = tw->text.top_line + tw->text.number_lines;
			 }
			else
			 {
				local_total = tw->text.total_lines;
			 }
      
			if (local_total >= tw->text.number_lines)
			 {
				new_size = tw->text.number_lines;
			 }
			else
			 {
				new_size = local_total;
			 }

			if (new_size + tw->text.top_line > local_total)
			 {
				new_size = local_total - tw->text.top_line;
			 }

			offset = local_total - (tw->text.number_lines + tw->text.top_line);
			nav_data.value.x = offset;
			nav_data.minimum.x = 0;
			nav_data.maximum.x = local_total;
			nav_data.slider_size.x = new_size;
			nav_data.page_increment.x = (data->number_lines > 1) ? (data->number_lines - 1) : 1;
      
			nav_data.dimMask = NavigDimensionX;
			nav_data.valueMask = NavValue|NavMinimum|NavMaximum|
										NavSliderSize|NavPageIncrement;
			_XmSFUpdateNavigatorsValue(XtParent((Widget)tw), &nav_data, True);
      
			data->ignorehbar = False;
		 }
	 }
	else
	 {
		data->number_lines = tw->text.inner_widget->core.height -
									data->topmargin - data->bottommargin;
		if ((data->number_lines < (int)data->lineheight) || !data->lineheight)
		 {
			data->number_lines = 1;
		 }
		else
		 {
			data->number_lines /= (int) data->lineheight;
		 }
  
		if (data->vbar && (old_number_lines != data->number_lines))
		 {
			int						local_total, new_size;
			XmNavigatorDataRec	nav_data;

			data->ignorevbar = True;

			if (tw->text.top_line + tw->text.number_lines > tw->text.total_lines)
			 {
				local_total = tw->text.top_line + tw->text.number_lines;
			 }
			else
			 {
				local_total = tw->text.total_lines;
			 }
      
			if (local_total >= tw->text.number_lines)
			 {
				new_size = tw->text.number_lines;
			 }
			else
			 {
				new_size = local_total;
			 }

			if (new_size + tw->text.top_line > local_total)
			 {
				new_size = local_total - tw->text.top_line;
			 }

			nav_data.value.y = tw->text.top_line;
			nav_data.minimum.y = 0;
			nav_data.maximum.y = local_total;
			nav_data.slider_size.y = new_size;
			nav_data.page_increment.y = (data->number_lines > 1) ? (data->number_lines - 1) : 1;
      
			nav_data.dimMask = NavigDimensionY;
			nav_data.valueMask = NavValue | NavMinimum | NavMaximum |
										NavSliderSize | NavPageIncrement;
			_XmSFUpdateNavigatorsValue(XtParent((Widget)tw), &nav_data, True);

			data->ignorevbar = False;
		 }
	 }
  
	if (!data->handlingexposures)
	 {
		_XmTextDisableRedisplay(tw, False);
		data->handlingexposures = True;
	 }
	if (data->exposehscroll != 0)
	 {
		xe->x = 0;
		xe->width = tw->core.width;
	 }
	if (data->exposevscroll != 0)
	 {
		xe->y = 0;
		xe->height = tw->core.height;
	 }
	if ((xe->x == 0) && (xe->y == 0) && (xe->width == tw->core.width) &&
		 (xe->height == tw->core.height))
	 {
		_XmTextMarkRedraw(tw, (XmTextPosition)0, 9999999);
	 }
	else
	 {
		if (!erased_cursor)
		 {
			RedrawRegion(tw, xe->x, xe->y, xe->width, xe->height);
		 }
	 }
  
	_XmTextInvalidate(tw, (XmTextPosition) tw->text.top_character,
							(XmTextPosition) tw->text.top_character, NODELTA);
  
	_XmTextEnableRedisplay(tw);

	data->handlingexposures = False;

	_XmTextDrawShadow(tw);
  
	/* If the expose happened because of SetValues, the font may have changed.	*/
   /* At this point, RefigureLines has run and the tw is relayed out.			*/
   /* So it is safe to calculate the x,y position of the cursor to pass			*/
   /* to the IM.  And we can reset the clip origin so that the I-Beam will		*/
   /* be drawn correctly.																		*/

	if (font_may_have_changed)
	 {
		EraseInsertionPoint(tw);
		TextDrawInsertionPoint(tw);
		PosToXY(tw, tw->text.cursor_position, &xmim_point.x, &xmim_point.y);
		_XmTextGetDisplayRect((Widget)tw, &xmim_area);
		XtSetArg(im_args[n], XmNspotLocation, &xmim_point); n++;
		XtSetArg(im_args[n], XmNarea, &xmim_area); n++;
		XmImSetValues(w, im_args, n);
	 }

	if ((data->cursor_on < 0) || (data->blinkstate == off))
	 {
		data->refresh_ibeam_off = True;
	 }
  
	if (XtIsSensitive((Widget)tw) && data->hasfocus)
	 {
		_XmTextChangeBlinkBehavior(tw, True);
	 }
	TextDrawInsertionPoint(tw);
}


static void 
GetPreferredSize(Widget w, Dimension *width, Dimension *height)
{
	XmTextWidget	tw=(XmTextWidget)w;
	OutputData		data=tw->text.output->data;
  

	SizeFromRowsCols(tw, width, height);
  
	if (data->resizewidth)
	 {
		TextFindNewWidth(tw, width);
		if (*width < data->minwidth)
		 {
			*width = data->minwidth;
		 }
	 }
  
	if (data->resizeheight)
	 {
		TextFindNewHeight(tw, PASTENDPOS, height);
		if (*height < data->minheight)
		 {
			*height = data->minheight;
		 }
	 }

	if (*width == 0)
	 {
		*width = 1;
	 }
	if (*height == 0)
	 {
		*height = 1;
	 }
}


static void 
HandleVBarButtonRelease(Widget w, XtPointer closure, XEvent* event, Boolean* cont)
{
	XmTextWidget	tw=(XmTextWidget)closure;
	OutputData		data=tw->text.output->data;
  
	data->suspend_hoffset = False;

	EraseInsertionPoint(tw);
	XmTextScroll((Widget) tw, 0);
	TextDrawInsertionPoint(tw);
}


static void 
HandleHBarButtonRelease(Widget w, XtPointer closure, XEvent* event, Boolean* cont)
{
	XmTextWidget	tw=(XmTextWidget)closure;
	OutputData		data=tw->text.output->data;

	data->suspend_voffset = False;

	EraseInsertionPoint(tw);
	XmTextScroll((Widget) tw, 0);
	TextDrawInsertionPoint(tw);
}


/************************************************************************
 *
 *  SliderMove	
 *  Callback for the value changes of navigators.
 * 
 ************************************************************************/
static void
SliderMove(Widget w, XtPointer closure, XtPointer cd)
{
	/* w is a navigator tw */

	XmTextWidget			tw=(XmTextWidget)closure;
	XmNavigatorDataRec	nav_data;
	int						offset, n;
	XPoint					xmim_point;
	XRectangle				xmim_area;
	Arg						args[10];
	OutputData				data=tw->text.output->data;
	int						local_total=0, new_top=0;
  

	/* Get the navigator information using the trait getValue since	*/
	/*	a callback struct cannot be used.										*/
  
	nav_data.valueMask = NavValue;
	((XmNavigatorTrait)XmeTraitGet((XtPointer)XtClass(w), XmQTnavigator))->getValue(w, &nav_data);


	/* look at the kind of navigator and make the appropriate update */
  
	if (!data->ignorehbar && (nav_data.dimMask & NavigDimensionX))
	 {
		if(XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
		 {
			data->suspend_voffset = True;
			tw->text.hsbar_scrolling = True;
			if (tw->text.top_line + tw->text.number_lines > tw->text.total_lines)
			 {
				local_total = tw->text.top_line + tw->text.number_lines;
			 }
			else
			 {
				local_total = tw->text.total_lines;
			 }
  
			new_top = local_total - nav_data.value.x - tw->text.number_lines;
			offset = nav_data.value.x - tw->text.top_line;
			tw->text.top_line = nav_data.value.x;
			EraseInsertionPoint(tw);
			XmTextScroll((Widget)tw, offset);
			TextDrawInsertionPoint(tw);
			tw->text.hsbar_scrolling = False;
		 }
		else
		 {
			offset = nav_data.value.x;
			EraseInsertionPoint(tw);
			ChangeHOffset(tw, offset, False);
			TextDrawInsertionPoint(tw);
		 }
    
		PosToXY(tw, tw->text.cursor_position, &xmim_point.x, &xmim_point.y);
		_XmTextGetDisplayRect((Widget)tw, &xmim_area);
		n = 0;
		XtSetArg(args[n], XmNarea, &xmim_area); n++;
		XtSetArg(args[n], XmNspotLocation, &xmim_point); n++;
		XmImSetValues(w, args, n);
		data->suspend_voffset = False;
	 }
  
	if (!data->ignorevbar && (nav_data.dimMask & NavigDimensionY))
	 {
		if(XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
		 {
			offset = nav_data.value.y;
			EraseInsertionPoint(tw);
			ChangeVOffset(tw, offset, False);
			TextDrawInsertionPoint(tw);
		 }
		else
		 {
			data->suspend_hoffset = True;

			tw->text.vsbar_scrolling = True;
			offset = nav_data.value.y - tw->text.top_line;
			tw->text.top_line = nav_data.value.y;
			EraseInsertionPoint(tw);
			XmTextScroll((Widget)tw, offset);
			TextDrawInsertionPoint(tw);

			tw->text.vsbar_scrolling = False;
		 }
		PosToXY(tw, tw->text.cursor_position, &xmim_point.x, &xmim_point.y);
		_XmTextGetDisplayRect((Widget)tw, &xmim_area);
		n = 0;
		XtSetArg(args[n], XmNarea, &xmim_area); n++;
		XtSetArg(args[n], XmNspotLocation, &xmim_point); n++;
		XmImSetValues(w, args, n);
		data->suspend_hoffset = False;
	 } 

	/* now update the other navigator value */
	_XmSFUpdateNavigatorsValue(XtParent((Widget)tw), &nav_data, False);
}


/* Public routines. */

/*****************************************************************************
 * To make TextOut a True "Object" this function should be a class function. *
 *****************************************************************************/
void 
_XmTextOutputCreate(Widget wid, ArgList args, Cardinal num_args)
{
	XmTextWidget			tw=(XmTextWidget)wid;
	Output					output;
	OutputData				data;
	Dimension				width, height;
	XmScrollFrameTrait	scrollFrameTrait;
  

	tw->text.output =
	output = (Output)XtMalloc(sizeof(OutputRec));
	output->data =
	data = (OutputData)XtMalloc(sizeof(OutputDataRec));
  
	XtGetSubresources(wid, (XtPointer)data, NULL, NULL, output_resources,
							XtNumber(output_resources), args, num_args);
  
	if (output->data->scrollleftside == XmDYNAMIC_BOOL)
	 {
		if (XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
		 {
			output->data->scrollleftside = True;
		 }
		else
		 {
			output->data->scrollleftside = False;
		 }
	 }

	output->XYToPos = XYToPos;
	output->PosToXY = PosToXY;
	output->MeasureLine = MeasureLine;
	output->Draw = Draw;
	output->DrawInsertionPoint = DrawInsertionPoint;
	output->MakePositionVisible = MakePositionVisible;
	output->MoveLines = MoveLines;
	output->Invalidate = OutputInvalidate;
	output->GetPreferredSize = GetPreferredSize;
	output->GetValues = OutputGetValues;
	output->SetValues = OutputSetValues;
	output->realize = OutputRealize;
	output->destroy = OutputDestroy;
	output->resize = NotifyResized;
	output->expose = OutputExpose;

	data->insertx = data->inserty = -99;
	data->suspend_hoffset = False;
	data->hoffset = 0;
	data->scrollwidth = 1;
	data->exposehscroll = data->exposevscroll = False;
	data->stipple_tile = _XmGetInsensitiveStippleBitmap((Widget) tw);
	data->add_mode_cursor = XmUNSPECIFIED_PIXMAP;
	data->ibeam_off = XmUNSPECIFIED_PIXMAP;
	data->cursor = XmUNSPECIFIED_PIXMAP;
	data->timerid = (XtIntervalId)0;
	data->font = NULL;
	data->scrollheight = 1;
	data->voffset = 0;
	data->suspend_voffset = False;

#ifdef SUN_CTL
	data->rendition = NULL;
	data->ctl_direction = IS_DEF_TEXT_RIGHTALIGNED(tw);
#endif

	/* Copy over the fontlist/rendertable.			Final result stored in	*/
	/*	fontlist since that's what the rest of the code is expecting, but	*/
	/*	rendertable takes precedence since that's the model for 2.0.		*/

	if ((data->fontlist == NULL) && (data->rendertable == NULL))
	 {
		data->fontlist = XmRenderTableCopy(XmeGetDefaultRenderTable(wid, XmTEXT_FONTLIST),
													  NULL, 0);
	 }
	else if (data->rendertable != NULL)
	 {
		data->fontlist = XmRenderTableCopy(data->rendertable, NULL, 0);
	 }
	else
	 {
		data->fontlist = XmRenderTableCopy(data->fontlist, NULL, 0);
	 }
	data->rendertable = data->fontlist;
	LoadFontMetrics(tw);

	data->cursorwidth = 5;
	data->cursorheight = data->font_ascent + data->font_descent;
	tw->text.inner_widget = wid;
	data->leftmargin = data->rightmargin = tw->text.margin_width +
							 tw->primitive.shadow_thickness +
							 tw->primitive.highlight_thickness;
	data->topmargin = data->bottommargin = tw->text.margin_height +
							tw->primitive.shadow_thickness +
							tw->primitive.highlight_thickness;
  
	if (tw->text.edit_mode == XmSINGLE_LINE_EDIT)
	 {
		if(XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
		 {
			data->columns = 1;
		 }
		else
		 {
			data->rows = 1;
		 }
	 }
  
	if ((tw->text.edit_mode != XmSINGLE_LINE_EDIT) && data->wordwrap)
	 {
		/* Don't grow in width if word wrap is	on */

		if(XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
		 {
			data->resizeheight = False;
		 }
		else
		 {
			data->resizewidth = False;
		 }
	 }
  
	if (data->rows <= 0)
	 {
		if (data->rows < 0)
		 {
			XmeWarning(wid, MSG1);
		 }

		if(XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
		 {
			data->rows = 20;
		 }
		else
		 {
			data->rows = 1;
		 }
	 }
  
	if (data->columns <= 0)
	 {
		if (data->columns < 0)
		 {
			XmeWarning(wid, MSG2);
		 }

		if(XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
		 {
			data->columns = 1;
		 }
		else
		 {
			data->columns = 20;
		 }
	 }
  
	data->columns_set = data->columns;			/*	(These 2 needed				*/
	data->rows_set = data->rows;					/*	 by Query Geometry.)			*/

	SizeFromRowsCols(tw, &width, &height);

	if (tw->core.width == 0)
	 {
		tw->core.width = width;
	 }
	if (tw->core.height == 0)
	 {
		tw->core.height = height;
	 }
  
	if(XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
	 {
		data->number_lines = tw->text.inner_widget->core.width -
									data->leftmargin - data->rightmargin;
		if ((data->number_lines < (int)data->linewidth) || !data->linewidth)
		 {
			data->number_lines = 1;
		 }
		else
		 {
			data->number_lines /= (int) data->linewidth;
		 }
	 }
	else
	 {
		data->number_lines = tw->text.inner_widget->core.height -
									data->topmargin - data->bottommargin;
		if ((data->number_lines < (int)data->lineheight) || !data->lineheight)
		 {
			data->number_lines = 1;
		 }
		else
		 {
			data->number_lines /= (int) data->lineheight;
		 }
	 }
  
	if ((tw->core.height != height) || (tw->core.width != width))
	 {
		RefigureDependentInfo(tw);
	 }
  
	/* reset columns_set and rows_set after RefigureDependentInfo() */
	data->columns_set = data->columns;
	data->rows_set = data->rows;
	data->prevW = tw->core.width;
	data->prevH = tw->core.height;
	data->minwidth = tw->core.width;
	data->minheight = tw->core.height;

	data->imagegc = NULL;
	data->gc = NULL;
	data->save_gc = NULL;
	data->cursor_gc = NULL;
	data->has_rect = False;

	LoadGCs(tw, tw->core.background_pixel, tw->primitive.foreground);
  
  
	/****************
	 *
	 * Now look at our parent and see if it's a non inited ScrollFrame. 
	 * If it is, create the navigators 
	 * and set up all the scrolling stuff using the trait.
	 */

	scrollFrameTrait = (XmScrollFrameTrait)XmeTraitGet((XtPointer)XtClass(wid->core.parent),
																		XmQTscrollFrame);
  
	if ((scrollFrameTrait != NULL) &&
		 !(scrollFrameTrait->getInfo(wid->core.parent, NULL, NULL, NULL)))
	 {
		int	n;
		Arg	arglist[30];
		Arg	swarglist[1];

      /*	set up the default move callback, so that our navigator	*/
		/*	gets associated nicely by the scrollFrame						*/

		scrollFrameTrait->init(wid->core.parent, SliderMove, wid);
    
		if (data->scrollhorizontal)
		 {
			data->resizewidth = False;
			data->ignorehbar = False;

			n = 0;
			XtSetArg(arglist[n], XmNunitType, XmPIXELS); n++;
			XtSetArg(arglist[n], XmNshadowThickness, tw->primitive.shadow_thickness); n++;

			XtSetArg(arglist[n], XmNorientation, XmHORIZONTAL); n++;

			XtSetArg(arglist[n], XmNtraversalOn, False); n++;
			XtSetArg(arglist[n], XmNhighlightThickness, 0); n++;
			XtSetArg(arglist[n], XmNincrement, data->averagecharwidth); n++;

			data->hbar = XmCreateScrollBar(XtParent(tw), "HorScrollBar", arglist, n);

			if(XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
			 {
				if (tw->text.edit_mode != XmSINGLE_LINE_EDIT)
				 {
					XtManageChild(data->hbar);
				 }

				XtAddEventHandler(data->hbar, (EventMask)ButtonReleaseMask,
										False, HandleHBarButtonRelease, (Opaque)tw);
			 }
			else
			 {
				XtManageChild(data->hbar);
			 }
		 }
		else
		 {
			data->hbar = NULL;
		 }
    
		if (data->scrollvertical)
		 {
			data->resizeheight = False;
			data->ignorevbar = False;

			n = 0;
			XtSetArg(arglist[n], XmNunitType, XmPIXELS); n++;
			XtSetArg(arglist[n], XmNshadowThickness, tw->primitive.shadow_thickness); n++;

			XtSetArg(arglist[n], XmNorientation, XmVERTICAL); n++;

			XtSetArg(arglist[n], XmNtraversalOn, False); n++;
			XtSetArg(arglist[n], XmNhighlightThickness, 0); n++;

			data->vbar = XmCreateScrollBar(XtParent(tw), "VertScrollBar", arglist, n);

			if(XmDirectionMatch(XmPrim_layout_direction(tw), XmTOP_TO_BOTTOM_RIGHT_TO_LEFT))
			 {
				XtManageChild(data->vbar);
			 }
			else
			 {
				if (tw->text.edit_mode != XmSINGLE_LINE_EDIT)
				 {
					XtManageChild(data->vbar);
				 }
				XtAddEventHandler(data->vbar, (EventMask)ButtonReleaseMask,
										False, HandleVBarButtonRelease, (Opaque)tw);
			 }
		 }
		else
		 {
			data->vbar = NULL;
		 }
    
    
		/* Tell scrolled window parent where to put the scrollbars */
    
		if (data->scrollleftside)
		 {
			if (data->scrolltopside)
			 {
				XtSetArg(swarglist[0], XmNscrollBarPlacement, XmTOP_LEFT);
			 }
			else
			 {
				XtSetArg(swarglist[0], XmNscrollBarPlacement, XmBOTTOM_LEFT);
			 }
		 }
		else
		 {
			if (data->scrolltopside)
			 {
				XtSetArg(swarglist[0], XmNscrollBarPlacement, XmTOP_RIGHT);
			 }
			else
			 {
				XtSetArg(swarglist[0], XmNscrollBarPlacement, XmBOTTOM_RIGHT);
			 }
		 }
    
		XtSetValues(tw->core.parent, swarglist, 1);
	 }
	else
	 {
		data->vbar = NULL;
		data->hbar = NULL;
		if (XmIsScrolledWindow(XtParent(tw)) &&
			 (((XmScrolledWindowWidget)tw->core.parent)->swindow.VisualPolicy == XmCONSTANT))
		 {
			data->scrollhorizontal = False;
			data->scrollvertical = False;
			data->resizewidth = True;
			data->resizeheight = True;
		 }
	 }
  
	data->hasfocus = False;
	data->blinkstate = on;
	data->cursor_on = 0;
	data->refresh_ibeam_off = True;
	data->have_inverted_image_gc = False;
	data->handlingexposures = False;
	XtAddEventHandler((Widget) tw->text.inner_widget,
							(EventMask) FocusChangeMask|EnterWindowMask|LeaveWindowMask,
							False, HandleFocusEvents, (Opaque)NULL);
	XtAddEventHandler((Widget) tw->text.inner_widget, (EventMask) 0, True,
							HandleGraphicsExposure, (Opaque)NULL);
}


/*****************************************************************************
 * To make TextOut a True "Object" this function should be a class function. *
 *****************************************************************************/
Boolean 
_XmTextGetBaselines(Widget w, Dimension ** baselines, int *line_count)
{
	XmTextWidget	tw=(XmTextWidget)w;
	OutputData		data=tw->text.output->data;
	Dimension *		base_array;
	int				i;
  

	*line_count = data->number_lines;
  
	base_array = (Dimension *)XtMalloc((sizeof(Dimension) * (*line_count)));
  
	for (i=0; i<*line_count; i++)
	 {
		base_array[i] = data->topmargin + i * data->lineheight + data->font_ascent;
	 }
  
	*baselines = base_array;
  
	return True;
}


/*****************************************************************************
 * To make TextOut a True "Object" this function should be a class function. *
 *****************************************************************************/
Boolean 
_XmTextGetDisplayRect(Widget w, XRectangle* display_rect)
{
	XmTextWidget	tw=(XmTextWidget)w;
	OutputData		data=tw->text.output->data;
  
	(*display_rect).x = data->leftmargin;
	(*display_rect).y = data->topmargin;
	(*display_rect).width = tw->core.width - (data->leftmargin + data->rightmargin);
	(*display_rect).height = data->number_lines * data->lineheight;

	return True;
}


/*****************************************************************************
 * To make TextOut a true "Object" this function should be a class function. *
 *****************************************************************************/
/* ARGSUSED */
void 
_XmTextMarginsProc(Widget w, XmBaselineMargins* margins_rec)
{
	XmTextWidget	tw=(XmTextWidget)w;
	OutputData		data=tw->text.output->data;
  
	if (margins_rec->get_or_set == XmBASELINE_SET)
	 {
		data->topmargin = margins_rec->margin_top +
								tw->primitive.shadow_thickness +
								tw->primitive.highlight_thickness;
		_XmProcessLock();
		posToXYCachedWidget = NULL;
		_XmProcessUnlock();
	 }
	else
	 {
		margins_rec->margin_top = data->topmargin -
										  (tw->primitive.shadow_thickness +
											tw->primitive.highlight_thickness);
		margins_rec->margin_bottom = data->bottommargin -
											  (tw->primitive.shadow_thickness +
												tw->primitive.highlight_thickness);
		margins_rec->text_height =  data->font_ascent + data->font_descent;
		margins_rec->shadow = tw->primitive.shadow_thickness;
		margins_rec->highlight = tw->primitive.highlight_thickness;
		margins_rec->margin_height = 0;
	 }
}


/*****************************************************************************
 * To make TextOut a True "Object" this function should be a class function. *
 *****************************************************************************/
void 
_XmTextChangeHOffset(XmTextWidget tw, int length)
{
	OutputData		data=tw->text.output->data;
	Dimension		margin_width=tw->text.margin_width +
										 tw->primitive.shadow_thickness +
										 tw->primitive.highlight_thickness;
	int				new_offset=data->hoffset;
	XmTextPosition	nextpos;
	XmTextPosition	last_position;
	XmTextPosition	temp;
	int				inner_width, width, i;
	int				text_width=0;
	int				new_text_width;
	XmTextBlockRec	block;


	length += (length < 0 ? (2 * margin_width) : - (2 * margin_width));
	new_offset += length;
  
	for (i=0; i<tw->text.number_lines; i++)
	 {
		last_position = (*tw->text.source->Scan)(tw->text.source, tw->text.line[i].start,
															  XmSELECT_LINE, XmsdRight, 1, False);
		nextpos = (*tw->text.source->Scan)(tw->text.source, last_position, XmSELECT_LINE,
													  XmsdRight, 1, True);
		if (nextpos == last_position)
		 {
			nextpos = PASTENDPOS;
		 }

		width = data->leftmargin;
		temp = tw->text.line[i].start;
		while (temp < last_position)
		 {
      	temp = (*tw->text.source->ReadSource)(tw->text.source, temp, last_position,
															  &block);
			width += FindWidth(tw, (Position) width, &block, 0, block.length);
		 }
		new_text_width = width - data->leftmargin;
		if (new_text_width > text_width)
		 {
			text_width = new_text_width;
		 }
	 }
  
	inner_width = tw->core.width - (2 * margin_width);
	if (new_offset >= text_width - inner_width)
	 {
		new_offset = text_width - inner_width;
	 }
  
	ChangeHOffset(tw, new_offset, True);
}


/*****************************************************************************
 * To make TextOut a True "Object" this function should be a class function. *
 *****************************************************************************/
void 
_XmTextChangeVOffset(XmTextWidget tw, int length)
{
	OutputData		data=tw->text.output->data;
	Dimension		margin_height=tw->text.margin_height +
										  tw->primitive.shadow_thickness +
										  tw->primitive.highlight_thickness;
	int				new_offset=data->voffset;
	XmTextPosition	nextpos=0,
						last_position=0,
						temp=0;
	int				inner_height=0, height=0, i=0;
	int				text_height=0;
	int				new_text_height=0;
	XmTextBlockRec	block;


	length += (length < 0 ? (2 * margin_height) : - (2 * margin_height));
	new_offset += length;

	for (i=0; i<tw->text.number_lines; i++)
	 {
		last_position = (*tw->text.source->Scan)(tw->text.source, tw->text.line[i].start,
															  XmSELECT_LINE, XmsdRight, 1, False);
		nextpos = (*tw->text.source->Scan)(tw->text.source, last_position, XmSELECT_LINE,
													  XmsdRight, 1, True);
		if (nextpos == last_position)
		 {
			nextpos = PASTENDPOS;
		 }
		height = data->topmargin;
		temp = tw->text.line[i].start;
		while (temp < last_position)
		 {
			temp = (*tw->text.source->ReadSource)(tw->text.source, temp, last_position,
															  &block);
			height += FindHeight(tw, (Position) height, &block, 0, block.length);
		 }
		new_text_height = height - data->topmargin;
		if (new_text_height > text_height)
		 {
			text_height = new_text_height;
		 }
	 }
  
	inner_height = tw->core.height - (2 * margin_height);
	if (new_offset >= text_height - inner_height)
	 {
		new_offset = text_height - inner_height;
	 }
  
	ChangeVOffset(tw, new_offset, True);
}


/*****************************************************************************
 * To make TextOut a true "Object" this function should be a class function. *
 *****************************************************************************/
void 
_XmTextToggleCursorGC(Widget w)
{
	XmTextWidget	tw=(XmTextWidget)w;
	OutputData		data=tw->text.output->data;
	InputData		i_data=tw->text.input->data;
	XGCValues		values;
	unsigned long	valueMask;
	Pixmap			stipple=XmUNSPECIFIED_PIXMAP;


	if (!XtIsRealized((Widget)tw))
	 {
		return;
	 }
  
	SetFullGC(tw, data->imagegc);
  
	_XmTextResetClipOrigin(tw, tw->text.cursor_position, False);
  
	if (i_data->overstrike)
	 {
		valueMask = GCFillStyle|GCFunction|GCForeground|GCBackground;
		if (XtIsSensitive(w) && !tw->text.add_mode &&
			 (data->hasfocus || _XmTextHasDestination(w)))
		 {
			values.fill_style = FillSolid;
		 }
		else
		 {
			valueMask |= GCStipple;
			values.fill_style = FillStippled;
			values.stipple = data->stipple_tile;
		 }
		values.foreground =
		values.background = tw->primitive.foreground ^ tw->core.background_pixel;
		values.function = GXxor;
	 }
	else
	 {
		valueMask = GCStipple;
		if (XGetGCValues(XtDisplay(tw), data->imagegc, valueMask, &values))
		 {
			stipple = values.stipple;
		 }
		valueMask = GCFillStyle|GCFunction|GCForeground|GCBackground;
		if (XtIsSensitive(w) && !tw->text.add_mode &&
			 (data->hasfocus || _XmTextHasDestination(w)))
		 {
			if (stipple != data->cursor)
			 {
				values.stipple = data->cursor;
				valueMask |= GCStipple;
			 }
		 }
		else
		 {
			if (stipple != data->add_mode_cursor)
			 {
				values.stipple = data->add_mode_cursor;
				valueMask |= GCStipple;
			 }
		 }
		if (tw->text.input->data->overstrike)
		 {
			values.background =
			values.foreground = tw->core.background_pixel ^ tw->primitive.foreground;
		 }
		else if (data->have_inverted_image_gc)
		 {
			values.background = tw->primitive.foreground;
			values.foreground = tw->core.background_pixel;
		 }
		else
		 {
			values.foreground = tw->primitive.foreground;
			values.background = tw->core.background_pixel;
		 }
		values.fill_style = FillStippled;
		values.function = GXcopy;
	 }
	XSetClipMask(XtDisplay(tw), data->save_gc, None);
	XChangeGC(XtDisplay(tw), data->imagegc, valueMask, &values);
}


void _Setup_hl1(Widget w, XmTextPart* tpp, Display* dp, Screen* sp)
{
   if (tpp->highlightColor1 == NULL)
    {

      XmPixelSet  pixels[XmCO_MAX_NUM_COLORS];
      Arg         args[4];
      XColor      palette[5];
      XColor      colors[4];
      XColor      exactcolor;
      int         coloruse, match=1;
      int         tmp, url_color, url_contrast, nocolors=0;
      short       i2, i3, i4, i5;

      tpp->highlightColor1 = (XColor *) XtMalloc(sizeof(XColor));
      if ((tpp->highlightColorname1 != NULL) &&
          XAllocNamedColor(dp,
                           DefaultColormapOfScreen(sp),
                           tpp->highlightColorname1,
                           tpp->highlightColor1, &exactcolor));

      else if (_XmGetPixelData(XScreenNumberOfScreen(sp),
                               &coloruse, pixels, &i2, &i3, &i4, &i5))
       {
         match = 0;
         palette[4].pixel = pixels[2].bg;
         XQueryColor(dp, DefaultColormapOfScreen(sp), &palette[4]);

         if ((coloruse == HIGH_COLOR) || (coloruse == MEDIUM_COLOR))
          {
            colors[0].pixel = pixels[0].bg;
            colors[1].pixel = pixels[1].bg;
            colors[2].pixel = pixels[2].bg;
            colors[3].pixel = pixels[3].bg;
            XQueryColors(dp,
                         DefaultColormapOfScreen(sp),
                         colors, 4);
            if (tpp->highlightColorname2 != NULL)
             {
               match = palettecolors(colors, tpp->highlightColor1, NULL);
             }
            else
             {
               match = palettecolors(colors, tpp->highlightColor1,
                                     &(tpp->highlightColor2));
             }
            if ((match == 1) || (match == 3))
             {
               if (XAllocColor(dp, DefaultColormapOfScreen(sp),
                               tpp->highlightColor1));
               else
                {
                  match = 0;
                }
             }
            if ((match == 2) || (match == 3))
             {
               if (XAllocColor(dp, DefaultColormapOfScreen(sp),
                               tpp->highlightColor2));
               else
                {
                  match = 0;
                }
             }
          }
       }
      else
       {
         match = -1;                   /* Ensure no contrast */
       }

      if (match <= 0)
       {
         XtSetArg(args[0], XmNhighlightColor, &(palette[0].pixel));
         XtSetArg(args[1], XmNbackground, &(palette[1].pixel));
         XtSetArg(args[2], XmNforeground, &(palette[2].pixel));
         XtGetValues(w, args, 3);
         XtSetArg(args[0], XmNbackground, &(palette[3].pixel));
         XtGetValues(XtParent(w), args, 1);
         XQueryColors(dp, DefaultColormapOfScreen(sp), palette, 4);
         if (match == -1)
          {
            palette[4] = palette[1];   /* Ensure no contrast */
          }

         /* Find the best contrast color to the text background */
         *(tpp->highlightColor1) = palette[0];
         url_color = 0;
         url_contrast = color_contrast(&palette[1], &palette[0]);

         if ((tmp = color_contrast(&palette[1], &palette[3]))
                               > url_contrast)
          {
            url_contrast = tmp;
            url_color = 3;
          }
         if ((tmp = color_contrast(&palette[1], &palette[4]))
                            > url_contrast)
          {
            url_contrast = tmp;
            url_color = 4;
          }
         if (url_contrast < 25)
          {                      /* ALL color chips have no contrast! */
            nocolors = 1;
            *(tpp->highlightColor1) = palette[2];
          }


         /* Do not allocate a new color if only two color chips */

         if ((nocolors) || (palette[1].red == palette[3].red &&
                            palette[1].green == palette[3].green &&
                            palette[1].blue == palette[3].blue))
            ;
         else if (tpp->highlightColorname2 != NULL)
          {
            *(tpp->highlightColor1) = palette[url_color];
          }
         else
          {
            tpp->highlightColor2 = (XColor *) XtMalloc(sizeof(XColor));

            /* Calculate the new url color. If foreground   */
            /* is white make brighter, if foreground is     */
            /* black make darker.                           */

            if ((palette[2].red + palette[2].green + palette[2].blue) > 30)
             {
               if (!too_bright(&palette[url_color]))
                {
                  change_color(&palette[url_color], &exactcolor, 50);
                  if (XAllocColor(dp,
                                  DefaultColormapOfScreen(sp),
                                  &exactcolor))
                   {
                     *(tpp->highlightColor1) = exactcolor;
                     *(tpp->highlightColor2) = palette[url_color];
                   }
                  else
                   {
                     *(tpp->highlightColor1) = palette[url_color];
                     *(tpp->highlightColor2) = palette[2];
                   }
                }
               else           /* url_color is too bright */
                {
                  change_color(&palette[url_color], &exactcolor, -50);
                  if (XAllocColor(dp, DefaultColormapOfScreen(sp), &exactcolor))
                   {
                     *(tpp->highlightColor1) = palette[url_color];
                     *(tpp->highlightColor2) = exactcolor;
                   }
                  else
                   {
                     *(tpp->highlightColor1) = palette[url_color];
                     *(tpp->highlightColor2) = palette[2];
                   }
                }
             }
            else           /* foreground color is black */
             {
               change_color(&palette[url_color], &exactcolor, -50);
               if (XAllocColor(dp, DefaultColormapOfScreen(sp), &exactcolor))
                {
                  *(tpp->highlightColor1) = palette[url_color];
                  *(tpp->highlightColor2) = exactcolor;
                }
               else
                {
                  *(tpp->highlightColor1) = palette[url_color];
                  *(tpp->highlightColor2) = palette[2];
                }
             }
          }
       }
    }
}


void _Setup_hl2(Widget w, XmTextPart* tpp, Display* dp, Screen* sp)
{
   if (tpp->highlightColor2 == NULL)
    {
      Arg         args[4];
      XColor      palette[5];
      XColor      exactcolor;


      tpp->highlightColor2 = (XColor *)XtMalloc(sizeof(XColor));

      if ((tpp->highlightColorname2 != NULL) &&
          XAllocNamedColor(dp, DefaultColormapOfScreen(sp),
                           tpp->highlightColorname2, tpp->highlightColor2,
                           &exactcolor)
         );
      else
       {
         XtSetArg(args[0], XmNforeground, &(palette[0].pixel));
         XtGetValues(w, args, 1);
         tpp->highlightColor2->pixel = palette[0].pixel;
       }
    }
}
