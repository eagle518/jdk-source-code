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
/* $XConsortium: TextOutP.h /main/16 1996/03/28 15:16:23 daniel $ */
/* (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
 *  (c) Copyright 1995 FUJITSU LIMITED
 *  This is source code modified by FUJITSU LIMITED under the Joint
 *  Development Agreement for the CDEnext PST.
 *  This is unpublished proprietary source code of FUJITSU LIMITED
 */
#ifndef _XmTextOutP_h
#define _XmTextOutP_h


#ifndef MOTIF12_HEADERS

#include <Xm/XmP.h>
#include <Xm/Text.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Definitions for modules implementing and using text output routines.
 *
 ****************************************************************/

#define ShouldWordWrap(data, widget)	(data->wordwrap && \
       (!(((XmDirectionMatch(XmPrim_layout_direction(widget), \
			     XmTOP_TO_BOTTOM_RIGHT_TO_LEFT)) ? \
            data->scrollvertical : data->scrollhorizontal) \
       && XmIsScrolledWindow(widget->core.parent))) \
       && widget->text.edit_mode != XmSINGLE_LINE_EDIT \
       && !((XmDirectionMatch(XmPrim_layout_direction(widget), \
			      XmTOP_TO_BOTTOM_RIGHT_TO_LEFT)) ? \
             data->resizeheight : data->resizewidth))

typedef struct _LineTableExtraRec {
    Dimension width;
    Boolean wrappedbychar;
} LineTableExtraRec, *LineTableExtra ;

/*         
 * output.c  (part of stext)
 */

typedef unsigned int LineNum;
typedef enum {on, off} OnOrOff;	/* For when Booleans aren't obvious enough. */

/*
 * Return the line number containing the given position.  If text currently
 * knows of no line containing that position, returns NOLINE.
 */

#define NOLINE	30000

/*
 * These next define the types of the routines that output is required
 * to export for use by text and by input.
 */

typedef struct _OutputDataRec {
    XmFontList fontlist;	/* Fontlist for text. */
    unsigned int blinkrate;
    Boolean wordwrap;		/* Whether to wordwrap. */
    Boolean cursor_position_visible;
    Boolean autoshowinsertpoint;
    Boolean hasfocus;
    Boolean has_rect;
    Boolean handlingexposures;	/* TRUE if in the midst of expose events. */
    Boolean exposevscroll;	/* Non-zero if we expect expose events to be
				   off vertically. */
    Boolean exposehscroll;	/* Non-zero if we expect expose events to be
				   off horizontally. */
    Boolean resizewidth, resizeheight;
    Boolean scrollvertical, scrollhorizontal;
    Boolean scrollleftside, scrolltopside;
    Boolean ignorevbar;		/* Whether to ignore callbacks from vbar. */
    Boolean ignorehbar;		/* Whether to ignore callbacks from hbar. */
    short int cursor_on;		/* Whether IBeam cursor is visible. */
    Boolean refresh_ibeam_off;	/* Indicates whether area under IBeam needs
				 * to be re-captured */
    Boolean suspend_hoffset;	/* temporarily suspend horizontal scrolling */
    Boolean use_fontset;        /* True if font to be used is fontset (and
				 * thus need X11R5 Xmb* routines to draw */
    Boolean have_inverted_image_gc; /* fg/bg of image gc have been swapped;
				     * on == True, off == False */
    OnOrOff blinkstate;
    Position insertx, inserty;
    int number_lines;		/* Number of lines that fit in the window. */
    int leftmargin, rightmargin;
    int topmargin, bottommargin;
    int scrollwidth;		/* Total width of text we have to display. */
    int vsliderSize;		/* How big the thumb is in the vbar. */
    int hoffset;		/* How much we've scrolled off the left. */
    int averagecharwidth;	/* Number of pixels for an "average" char. */
    int tabwidth;		/* Number of pixels for a tab. */
    short columns, rows;
    Dimension lineheight;	/* Number of pixels per line. */
    Dimension minwidth, minheight;
    Dimension prevW;
    Dimension prevH;
    Dimension cursorwidth, cursorheight;
    Dimension font_ascent;      /* ascent of the font[set] */
    Dimension font_descent;     /* descent of the font[set] */
    XtIntervalId timerid;
    Pixmap cursor;		/* Pixmap for IBeam cursor stencil. */
    Pixmap add_mode_cursor;	/* Pixmap to use for add mode cursor. */
    Pixmap ibeam_off;		/* Pixmap for area under the IBeam. */
    Pixmap stipple_tile;	/* stipple for add mode cursor. */
    GC gc, imagegc;
    Widget vbar, hbar;
    XFontStruct *font;		/* font used when NULL font is set. */
/* New for 1.2 */
    GC save_gc;                 /* GC for saving/resotring under IBeam */
    short columns_set, rows_set; /* history of previously set dimensions */
/* New for 2.0 */
    XmRenderTable rendertable;  /* Alias for fontlist - only for resource 
				   setting */
    GC cursor_gc;               /* 1-bit depth GC for creating the I-beam 
				   stipples (normal & add mode) */

    int scrollheight;		/* Total height of text we have to display. */
    int voffset;		/* How much we've scrolled off the top. */
    int tabheight;		/* Number of pixels for a tab. */
    Dimension linewidth;	/* Number of pixels per line (when vw). */
    Boolean suspend_voffset;	/* temporarily suspend horizontal scrolling */
#ifdef SUN_CTL
    XmStringTag rendition_tag;	/* This one is resourced */
    XmRendition rendition;	/* This is a cached pointer */
    unsigned char alignment;	/* This one is resourced */
    XmDirection  ctl_direction; /* for CTL dynamic direction */
#endif /* CTL */
} OutputDataRec, *OutputData;

/*
 * Create a new instance of an output object.  This is expected to fill in
 * info about innerwidget and output in the widget record.
 */

typedef void (*OutputCreateProc)(
			Widget,
			ArgList,
			Cardinal) ;
/*
 * Given an (x,y) coordinate, return the closest corresponding position. (For
 * use by input; text shouldn't ever need to know.)
 */

typedef XmTextPosition (*XYToPosProc)(
			XmTextWidget,
#if NeedWidePrototypes
			int,
			int) ;
#else
			Position,	/* These are relative to the */
			Position) ;	/* innerwindow returned above. */
#endif

/*
 * Return the (x,y) coordinate corresponing to the given position.  If this
 * returns FALSE, then the given position isn't being displayed.
 */

typedef Boolean (*PosToXYProc)(
			XmTextWidget,
			XmTextPosition,
			Position *,	/* These are relative to the */
			Position *) ;	/* innerwindow returned above. */

/*
 * Measures the extent of a line.  Given the line number and starting position
 * of a line, returns the starting position of the next line.  Also returns
 * any extra information that the output module may want to associate with
 * this line for future reference.  (In particular, it will want to associate
 * a vertical coordinate for this line.)  This routine should return FALSE if
 * it decides that this line will not fit in the window.  FALSE should never
 * be returned if the line number is zero.  Output may assume that the line
 * table for all preceeding lines have already been set.  In particular, when
 * this routine will return FALSE, then output knows that the entire linetable
 * has been calculated; that is a good time for it to look over the linetable
 * and decide if it wants to do something obnoxious like resize the window.
 *
 * A possible value to put in nextpos is PASTENDPOS.  This indicates that the
 * current line contains the end of the text in the source.
 *
 * nextpos may be NULL.  If it is, then this indicates that we only want to
 * know if the line will fit on the window.  The caller already has its own
 * idea where the next line will start if it does fit.  (If nextpos is NULL,
 * then no extra information should be generated, and the 'extra' parameter
 * should be ignored.)
 */

#define PASTENDPOS	2147483647  /* Biggest number that can fit in long */

typedef Boolean (*MeasureLineProc)(
			XmTextWidget,
			LineNum,
			XmTextPosition,
			XmTextPosition *,
			LineTableExtraRec **) ;

/*
 * Draw some text within a line.  The output finds out information about the
 * line by calling the line table routines.
 *
 * %%% Document special cases (like lines containing PASTENDPOS).
 */

typedef void (*DrawProc)(
			XmTextWidget,
			LineNum,
			XmTextPosition,
			XmTextPosition,
#ifdef SUN_CTL
			_XmHighlightData*) ;
#else  /* CTL */
			XmHighlightMode) ;
#endif /* CTL */

/*
 * Output should draw or erase an insertion point at the given position.
 */

typedef void (*DrawInsertionPointProc)(
			XmTextWidget,
			XmTextPosition,
			OnOrOff) ;

/*
 * Output should ensure that the given position is visible (e.g., not scrolled
 * off horizontally).
 */
typedef void (*MakePositionVisibleProc)(
			XmTextWidget,
			XmTextPosition) ;

/* Text would like to move some lines around on the screen.  It would like to
 * move lines fromline through toline (inclusive) to now start at line
 * destline.  If output can perform this move by means of a XCopyArea or
 * similar simple call, it does so and returns TRUE; otherwise, it will return
 * FALSE.  If TRUE, output should modify affected values in the
 * "extra" entries in the linetable, because after returning text will go ahead
 * and move linetable entries around.
 */

typedef Boolean (*MoveLinesProc)(
			XmTextWidget,
			LineNum,
			LineNum,
			LineNum) ;

/*
 * Inform output of invalidated positions.
 */

typedef void (*InvalidateProc)(
			XmTextWidget,
			XmTextPosition,
			XmTextPosition,
			long) ;

/*
 * Get preferred size of text widget based on the row and column
 * resources multiplied by font attributes as well as adding the
 * margin resource values to the computed size.
 */
typedef void (*GetPreferredSizeProc)(
			Widget,
			Dimension *,
			Dimension *) ;

/*
 * Get values out of the output object.
 */

typedef void (*GetValuesProc)(
			Widget,
			ArgList,
			Cardinal) ;

/*
 * Set values in the output object.
 */

typedef Boolean (*SetValuesProc)(
			Widget,
			Widget,
			Widget,
			ArgList,
			Cardinal *) ;


typedef struct _OutputRec {
    struct _OutputDataRec	*data; /* Output-specific data; opaque type. */
    XYToPosProc			XYToPos;
    PosToXYProc			PosToXY;
    MeasureLineProc		MeasureLine;
    DrawProc			Draw;
    DrawInsertionPointProc	DrawInsertionPoint;
    MakePositionVisibleProc	MakePositionVisible;
    MoveLinesProc		MoveLines;
    InvalidateProc		Invalidate;
    GetPreferredSizeProc	GetPreferredSize;
    GetValuesProc		GetValues;
    SetValuesProc		SetValues;
    XmRealizeOutProc		realize;
    XtWidgetProc		destroy;
    XmResizeFlagProc		resize;
    XtExposeProc		expose;
} OutputRec;


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#else /* MOTIF12_HEADERS */


/* 
 * @OSF_COPYRIGHT@
 * (c) Copyright 1990, 1991, 1992, 1993, 1994 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *  
*/ 
/*
 * HISTORY
 * Motif Release 1.2.5
*/
/*   $XConsortium: TextOutP.h /main/cde1_maint/2 1995/08/18 19:28:06 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#include <Xm/XmP.h>
#include <Xm/Text.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Definitions for modules implementing and using text output routines.
 *
 ****************************************************************/

#define ShouldWordWrap(data, widget)	(data->wordwrap && \
       (!(data->scrollhorizontal && \
       (XtClass(widget->core.parent) == xmScrolledWindowWidgetClass))) \
       && widget->text.edit_mode != XmSINGLE_LINE_EDIT && !data->resizewidth)

typedef struct _LineTableExtraRec {
    Dimension width;
    Boolean wrappedbychar;
} LineTableExtraRec, *LineTableExtra ;

/*         
 * output.c  (part of stext)
 */

typedef unsigned int LineNum;
typedef enum {on, off} OnOrOff;	/* For when Booleans aren't obvious enough. */

/*
 * Return the line number containing the given position.  If text currently
 * knows of no line containing that position, returns NOLINE.
 */

#define NOLINE	30000

/*
 * These next define the types of the routines that output is required
 * to export for use by text and by input.
 */

typedef struct _OutputDataRec {
    XmFontList fontlist;	/* Fontlist for text. */
    unsigned int blinkrate;
    Boolean wordwrap;		/* Whether to wordwrap. */
    Boolean cursor_position_visible;
    Boolean autoshowinsertpoint;
    Boolean hasfocus;
    Boolean has_rect;
    Boolean handlingexposures;	/* TRUE if in the midst of expose events. */
    Boolean exposevscroll;	/* Non-zero if we expect expose events to be
				   off vertically. */
    Boolean exposehscroll;	/* Non-zero if we expect expose events to be
				   off horizontally. */
    Boolean resizewidth, resizeheight;
    Boolean scrollvertical, scrollhorizontal;
    Boolean scrollleftside, scrolltopside;
    Boolean ignorevbar;		/* Whether to ignore callbacks from vbar. */
    Boolean ignorehbar;		/* Whether to ignore callbacks from hbar. */
    short int cursor_on;		/* Whether IBeam cursor is visible. */
    Boolean refresh_ibeam_off;	/* Indicates whether area under IBeam needs
				 * to be re-captured */
    Boolean suspend_hoffset;	/* temporarily suspend horizontal scrolling */
    Boolean use_fontset;        /* True if font to be used is fontset (and
				 * thus need X11R5 Xmb* routines to draw */
    Boolean have_inverted_image_gc; /* fg/bg of image gc have been swapped;
				     * on == True, off == False */
    OnOrOff blinkstate;
    Position insertx, inserty;
    int number_lines;		/* Number of lines that fit in the window. */
    int leftmargin, rightmargin;
    int topmargin, bottommargin;
    int scrollwidth;		/* Total width of text we have to display. */
    int vsliderSize;		/* How big the thumb is in the vbar. */
    int hoffset;		/* How much we've scrolled off the left. */
    int averagecharwidth;	/* Number of pixels for an "average" char. */
    int tabwidth;		/* Number of pixels for a tab. */
    short columns, rows;
    Dimension lineheight;	/* Number of pixels per line. */
    Dimension minwidth, minheight;
    Dimension prevW;
    Dimension prevH;
    Dimension cursorwidth, cursorheight;
    Dimension font_ascent;      /* ascent of the font[set] */
    Dimension font_descent;     /* descent of the font[set] */
    XtIntervalId timerid;
    Pixmap cursor;		/* Pixmap for IBeam cursor stencil. */
    Pixmap add_mode_cursor;	/* Pixmap to use for add mode cursor. */
    Pixmap ibeam_off;		/* Pixmap for area under the IBeam. */
    Pixmap stipple_tile;	/* stiiple for add mode cursor. */
    GC gc, imagegc;
    Widget vbar, hbar;
    XFontStruct *font;		/* font used when NULL font is set. */
/* New for 1.2 */
    GC save_gc;                 /* GC for saving/resotring under IBeam */
    short columns_set, rows_set; /* history of previously set dimensions */
} OutputDataRec, *OutputData;


/*
 * Create a new instance of an output object.  This is expected to fill in
 * info about innerwidget and output in the widget record.
 */

#ifdef _NO_PROTO
typedef void (*OutputCreateProc)();
#else
typedef void (*OutputCreateProc)(
			Widget,
			ArgList,
			Cardinal) ;
#endif
/*
 * Given an (x,y) coordinate, return the closest corresponding position. (For
 * use by input; text shouldn't ever need to know.)
 */

#ifdef _NO_PROTO
typedef XmTextPosition (*XYToPosProc)(); /* ctx, x, y */
#else
typedef XmTextPosition (*XYToPosProc)(
			XmTextWidget,
#if NeedWidePrototypes
			int,
			int) ;
#else
			Position,	/* These are relative to the */
			Position) ;	/* innerwindow returned above. */
#endif
#endif

/*
 * Return the (x,y) coordinate corresponing to the given position.  If this
 * returns FALSE, then the given position isn't being displayed.
 */

#ifdef _NO_PROTO
typedef Boolean (*PosToXYProc)(); /* widget, position, x, y */
#else
typedef Boolean (*PosToXYProc)(
			XmTextWidget,
			XmTextPosition,
			Position *,	/* These are relative to the */
			Position *) ;	/* innerwindow returned above. */
#endif

/*
 * Measures the extent of a line.  Given the line number and starting position
 * of a line, returns the starting position of the next line.  Also returns
 * any extra information that the output module may want to associate with
 * this line for future reference.  (In particular, it will want to associate
 * a vertical coordinate for this line.)  This routine should return FALSE if
 * it decides that this line will not fit in the window.  FALSE should never
 * be returned if the line number is zero.  Output may assume that the line
 * table for all preceeding lines have already been set.  In particular, when
 * this routine will return FALSE, then output knows that the entire linetable
 * has been calculated; that is a good time for it to look over the linetable
 * and decide if it wants to do something obnoxious like resize the window.
 *
 * A possible value to put in nextpos is PASTENDPOS.  This indicates that the
 * current line contains the end of the text in the source.
 *
 * nextpos may be NULL.  If it is, then this indicates that we only want to
 * know if the line will fit on the window.  The caller already has its own
 * idea where the next line will start if it does fit.  (If nextpos is NULL,
 * then no extra information should be generated, and the 'extra' parameter
 * should be ignored.)
 */

#define PASTENDPOS	2147483647  /* Biggest number that can fit in long */

#ifdef _NO_PROTO
typedef Boolean (*MeasureLineProc)(); /* ctx, line, pos, nextpos, extra */
#else
typedef Boolean (*MeasureLineProc)(
			XmTextWidget,
			LineNum,
			XmTextPosition,
			XmTextPosition *,
			LineTableExtraRec **) ;
#endif

/*
 * Draw some text within a line.  The output finds out information about the
 * line by calling the line table routines.
 *
 * %%% Document special cases (like lines containing PASTENDPOS).
 */

#ifdef _NO_PROTO
typedef void (*DrawProc)();	/* ctx, line, start, end, highlight */
#else
typedef void (*DrawProc)(
			XmTextWidget,
			LineNum,
			XmTextPosition,
			XmTextPosition,
			XmHighlightMode) ;
#endif

/*
 * Output should draw or erase an insertion point at the given position.
 */

#ifdef _NO_PROTO
typedef void (*DrawInsertionPointProc)(); /* ctx, position, onoroff */
#else
typedef void (*DrawInsertionPointProc)(
			XmTextWidget,
			XmTextPosition,
			OnOrOff) ;
#endif

/*
 * Output should ensure that the given position is visible (e.g., not scrolled
 * off horizontally).
 */
#ifdef _NO_PROTO
typedef void (*MakePositionVisibleProc)();	/* widget, position */
#else
typedef void (*MakePositionVisibleProc)(
			XmTextWidget,
			XmTextPosition) ;
#endif

/* Text would like to move some lines around on the screen.  It would like to
 * move lines fromline through toline (inclusive) to now start at line
 * destline.  If output can perform this move by means of a XCopyArea or
 * similar simple call, it does so and returns TRUE; otherwise, it will return
 * FALSE.  If TRUE, output should modify affected values in the
 * "extra" entries in the linetable, because after returning text will go ahead
 * and move linetable entries around.
 */

#ifdef _NO_PROTO
typedef Boolean (*MoveLinesProc)(); /* ctx, fromline, toline, destline */
#else
typedef Boolean (*MoveLinesProc)(
			XmTextWidget,
			LineNum,
			LineNum,
			LineNum) ;
#endif

/*
 * Inform output of invalidated positions.
 */

#ifdef _NO_PROTO
typedef void (*InvalidateProc)(); /* ctx, position, topos, delta */
#else
typedef void (*InvalidateProc)(
			XmTextWidget,
			XmTextPosition,
			XmTextPosition,
			long) ;
#endif

/*
 * Get preferred size of text widget based on the row and column
 * resources multiplied by font attributes as well as adding the
 * margin resource values to the computed size.
 */
#ifdef _NO_PROTO
typedef void (*GetPreferredSizeProc)(); /* widget, width, height */
#else
typedef void (*GetPreferredSizeProc)(
			Widget,
			Dimension *,
			Dimension *) ;
#endif

/*
 * Get values out of the output object.
 */

#ifdef _NO_PROTO
typedef void (*GetValuesProc)(); /* widget, args, num_args */
#else
typedef void (*GetValuesProc)(
			Widget,
			ArgList,
			Cardinal) ;
#endif

/*
 * Set values in the output object.
 */

#ifdef _NO_PROTO
typedef Boolean (*SetValuesProc)(); /* oldw, reqw, new_w, args, num_args */
#else
typedef Boolean (*SetValuesProc)(
			Widget,
			Widget,
			Widget,
			ArgList,
			Cardinal *) ;
#endif


typedef struct _OutputRec {
    struct _OutputDataRec	*data; /* Output-specific data; opaque type. */
    XYToPosProc			XYToPos;
    PosToXYProc			PosToXY;
    MeasureLineProc		MeasureLine;
    DrawProc			Draw;
    DrawInsertionPointProc	DrawInsertionPoint;
    MakePositionVisibleProc	MakePositionVisible;
    MoveLinesProc		MoveLines;
    InvalidateProc		Invalidate;
    GetPreferredSizeProc	GetPreferredSize;
    GetValuesProc		GetValues;
    SetValuesProc		SetValues;
    XmRealizeOutProc		realize;
    XtWidgetProc		destroy;
    XmResizeFlagProc		resize;
    XtExposeProc		expose;
} OutputRec;


/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern void _XmTextFreeContextData() ;
extern void _XmTextResetClipOrigin() ;
extern void _XmTextAdjustGC() ;
extern Boolean _XmTextShouldWordWrap() ;
extern Boolean _XmTextScrollable() ;
extern XmTextPosition _XmTextFindLineEnd() ;
extern void _XmTextOutputGetSecResData() ;
extern int _XmTextGetNumberLines() ;
extern void _XmTextMovingCursorPosition() ;
extern void _XmTextDrawDestination() ;
extern void _XmTextClearDestination() ;
extern void _XmTextDestinationVisible() ;
extern void _XmTextChangeBlinkBehavior() ;
extern void _XmTextOutputCreate() ;
extern Boolean _XmTextGetBaselines() ;
extern Boolean _XmTextGetDisplayRect() ;
extern void _XmTextMarginsProc() ;
extern void _XmTextChangeHOffset() ;
extern void _XmTextToggleCursorGC() ;

#else

extern void _XmTextFreeContextData( 
                        Widget w,
                        XtPointer clientData,
                        XtPointer callData) ;
extern void _XmTextResetClipOrigin( 
                        XmTextWidget tw,
                        XmTextPosition position,
#if NeedWidePrototypes
                        int clip_mask_reset) ;
#else
                        Boolean clip_mask_reset) ;
#endif /* NeedWidePrototypes */
extern void _XmTextAdjustGC( 
                        XmTextWidget tw) ;
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
extern void _XmTextDrawDestination( 
                        XmTextWidget widget) ;
extern void _XmTextClearDestination( 
                        XmTextWidget widget,
#if NeedWidePrototypes
                        int ignore_sens) ;
#else
                        Boolean ignore_sens) ;
#endif /* NeedWidePrototypes */
extern void _XmTextDestinationVisible(Widget, Boolean);
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
extern void _XmTextToggleCursorGC( 
                        Widget widget) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/



#if defined(__cplusplus) || defined(c_plusplus)
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* MOTIF12_HEADERS */

#endif /* _XmTextOutP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
