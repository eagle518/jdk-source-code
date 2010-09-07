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
/* $XConsortium: DrawP.h /main/10 1995/07/14 10:27:48 drk $ */
/* (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmDrawP_h
#define _XmDrawP_h

#ifndef MOTIF12_HEADERS

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------*/
/*   Functions used by Xm widgets for the Motif visual drawing   */
/*---------------------------------------------------------------*/
/* All these functions have an Xlib draw like API: 
      a Display*, a Drawable, then GCs, Positions and Dimensions 
      and finally some specific paramaters */

/******** The Draw.c file has been split in several module for
          a better link profile *********/

/*---------------------------------------------------------------
  XmeDrawShadows, 
       use in place of the 1.1 _XmDrawShadow and _XmDrawShadowType
       with changes to the interface (widget vs window, offsets, new order)
       and in the implementation (uses XSegments instead of XRectangles).
       Both etched and regular shadows use now a single private routine
       xmDrawSimpleShadow.
    XmeDrawHighlight.
       Implementation using FillRectangles, for solid highlight only. 
    _XmDrawHighlight.
       Highlight using wide lines, so that dash mode works. 
    XmeClearBorder,    
       new name for _XmEraseShadow  (_XmClearShadowType, which clear half a 
       shadow with a 'widget' API stays in Manager.c ) 
       XmClearBorder is only usable on window, not on drawable.
    XmeDrawSeparator, 
       use in place of the duplicate redisplay method of both separator and 
       separatorgadget (highlight_thickness not used, must be incorporated
       in the function call parameters). use xmDrawSimpleShadow.
       Has 2 new separator types for dash shadowed lines.
    XmeDrawDiamond, 
       new interface for _XmDrawDiamondButton (_XmDrawSquareButton is
       really a simple draw shadow and will be in the widget file as is).
    XmeDrawArrow, 
       same algorithm as before but in one function that re-uses the malloced
       rects and does not store anything in the wigdet instance.
    XmeDrawPolygonShadow,
       new one that use the RegionDrawShadow API to implement an Xme call 
    XmeDrawCircle,
       new one for toggle visual
    XmeDrawIndicator
       new one for toggle drawing
---------------------------------------------------------------------------*/


/********    Private Function Declarations    ********/

extern void XmeDrawShadows( 
                        Display *display,
                        Drawable d,
                        GC top_gc,
                        GC bottom_gc,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int width,
                        int height,
                        int shad_thick,
#else
                        Position x,
                        Position y,
                        Dimension width,
                        Dimension height,
                        Dimension shad_thick,
#endif /* NeedWidePrototypes */
                        unsigned int shad_type);
extern void XmeClearBorder( 
                        Display *display,
                        Window w,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int width,
                        int height,
                        int shadow_thick);
#else
                        Position x,
                        Position y,
                        Dimension width,
                        Dimension height,
                        Dimension shadow_thick);
#endif /* NeedWidePrototypes */
extern void XmeDrawSeparator( 
                        Display *display,
                        Drawable d,
                        GC top_gc,
                        GC bottom_gc,
                        GC separator_gc,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int width,
                        int height,
                        int shadow_thick,
                        int margin,
                        unsigned int orientation,
                        unsigned int separator_type);
#else
                        Position x,
                        Position y,
                        Dimension width,
                        Dimension height,
                        Dimension shadow_thick,
                        Dimension margin,
                        unsigned char orientation,
                        unsigned char separator_type);
#endif /* NeedWidePrototypes */
extern void XmeDrawDiamond( 
                        Display *display,
                        Drawable d,
                        GC top_gc,
                        GC bottom_gc,
                        GC center_gc,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int width,
                        int height,
                        int shadow_thick,
                        int margin);
#else
                        Position x,
                        Position y,
                        Dimension width,
                        Dimension height,
                        Dimension shadow_thick,
                        Dimension margin);
#endif /* NeedWidePrototypes */

extern void XmeDrawCircle( 
                        Display *display,
                        Drawable d,
                        GC top_gc,
                        GC bottom_gc,
                        GC center_gc,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int width,
                        int height,
                        int shadow_thick,
                        int margin);
#else
                        Position x,
                        Position y,
                        Dimension width,
                        Dimension height,
                        Dimension shadow_thick,
                        Dimension margin);
#endif /* NeedWidePrototypes */

extern void XmeDrawHighlight( 
                        Display *display,
                        Drawable d,
                        GC gc,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int width,
                        int height,
                        int highlight_thick
#else
                        Position x,
                        Position y,
                        Dimension width,
                        Dimension height,
                        Dimension highlight_thick
#endif /* NeedWidePrototypes */
                        );
extern void XmeDrawArrow( 
                        Display *display,
                        Drawable d,
                        GC top_gc,
                        GC bot_gc,
                        GC cent_gc,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int width,
                        int height,
                        int shadow_thick,
                        unsigned int direction);
#else
                        Position x,
                        Position y,
                        Dimension width,
                        Dimension height,
                        Dimension shadow_thick,
                        unsigned char direction);
#endif /* NeedWidePrototypes */

extern void XmeDrawPolygonShadow(
		      Display *dpy,
		      Drawable d,
		      GC topGC,
		      GC bottomGC,
		      XPoint *points,
		      int n_points,
#if NeedWidePrototypes
		      int shadowThickness,
		      unsigned int shadowType);
#else
		      Dimension shadowThickness,
		      unsigned char shadowType);
#endif /* NeedWidePrototypes */

extern void XmeDrawIndicator(Display *display, 
		 Drawable d, 
		 GC gc, 
#if NeedWidePrototypes
		 int x, int y, 
		 int width, int height, 
		 int margin,
		 int type);
#else
                 Position x, Position y, 
                 Dimension width, Dimension height,
		 Dimension margin, 
                 XtEnum type);
#endif /* NeedWidePrototypes */

/********    End Private Function Declarations    ********/


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
/*   $XConsortium: DrawP.h /main/cde1_maint/2 1995/08/18 19:00:28 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif


/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern void _XmDrawShadows() ;
extern void _XmClearBorder() ;
extern void _XmDrawSeparator() ;
extern void _XmDrawDiamond() ;
extern void _XmDrawHighlight() ;
extern void _XmDrawSimpleHighlight() ;
extern void _XmDrawArrow() ;

#else

extern void _XmDrawShadows( 
                        Display *display,
                        Drawable d,
                        GC top_gc,
                        GC bottom_gc,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int width,
                        int height,
                        int shad_thick,
#else
                        Position x,
                        Position y,
                        Dimension width,
                        Dimension height,
                        Dimension shad_thick,
#endif /* NeedWidePrototypes */
                        unsigned int shad_type) ;
extern void _XmClearBorder( 
                        Display *display,
                        Window w,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int width,
                        int height,
                        int shadow_thick) ;
#else
                        Position x,
                        Position y,
                        Dimension width,
                        Dimension height,
                        Dimension shadow_thick) ;
#endif /* NeedWidePrototypes */
extern void _XmDrawSeparator( 
                        Display *display,
                        Drawable d,
                        GC top_gc,
                        GC bottom_gc,
                        GC separator_gc,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int width,
                        int height,
                        int shadow_thick,
                        int margin,
                        unsigned int orientation,
                        unsigned int separator_type) ;
#else
                        Position x,
                        Position y,
                        Dimension width,
                        Dimension height,
                        Dimension shadow_thick,
                        Dimension margin,
                        unsigned char orientation,
                        unsigned char separator_type) ;
#endif /* NeedWidePrototypes */
extern void _XmDrawDiamond( 
                        Display *display,
                        Drawable d,
                        GC top_gc,
                        GC bottom_gc,
                        GC center_gc,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int width,
                        int height,
                        int shadow_thick,
                        int fill) ;
#else
                        Position x,
                        Position y,
                        Dimension width,
                        Dimension height,
                        Dimension shadow_thick,
                        Dimension fill) ;
#endif /* NeedWidePrototypes */
extern void _XmDrawSimpleHighlight( 
                        Display *display,
                        Drawable d,
                        GC gc,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int width,
                        int height,
                        int highlight_thick);
#else
                        Position x,
                        Position y,
                        Dimension width,
                        Dimension height,
                        Dimension highlight_thick);
#endif /* NeedWidePrototypes */
extern void _XmDrawHighlight( 
                        Display *display,
                        Drawable d,
                        GC gc,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int width,
                        int height,
                        int highlight_thick,
#else
                        Position x,
                        Position y,
                        Dimension width,
                        Dimension height,
                        Dimension highlight_thick,
#endif /* NeedWidePrototypes */
                        int line_style) ;
extern void _XmDrawArrow( 
                        Display *display,
                        Drawable d,
                        GC top_gc,
                        GC bot_gc,
                        GC cent_gc,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int width,
                        int height,
                        int shadow_thick,
                        unsigned int direction) ;
#else
                        Position x,
                        Position y,
                        Dimension width,
                        Dimension height,
                        Dimension shadow_thick,
                        unsigned char direction) ;
#endif /* NeedWidePrototypes */

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#if defined(__cplusplus) || defined(c_plusplus)
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* MOTIF12_HEADERS */

#endif /* _XmDrawP_h */
