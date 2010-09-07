/* $XConsortium: IconGP.h /main/9 1995/10/25 20:06:59 cde-sun $ */
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
#ifndef _XmIconGP_h
#define _XmIconGP_h

#ifndef MOTIF12_HEADERS

#include <Xm/XmP.h>
#include <Xm/ManagerP.h>
#include <Xm/GadgetP.h>
#include <Xm/IconG.h>
#include <Xm/ExtObjectP.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef Widget (*XmGetContainerParentProc)(Widget) ;

#define XmInheritGetContainerParentProc ((XmGetContainerParentProc) _XtInherit)


/* IconGadget class record */
typedef struct _XmIconGadgetClassPart
	{
	XmGetContainerParentProc	get_container_parent;
	XtPointer extension ;
	} 	XmIconGadgetClassPart;


/* Full class record declaration */
typedef struct _XmIconGadgetClassRec
	{
	RectObjClassPart	rect_class;
	XmGadgetClassPart	gadget_class;
	XmIconGadgetClassPart	icong_class;
	} 	XmIconGadgetClassRec;

extern	XmIconGadgetClassRec 	xmIconGadgetClassRec;

/*****************************************************************/
/* The Icon Gadget Cache Object's class and instance records*/
/*****************************************************************/

typedef struct _XmIconGCacheObjClassPart
{
    XtPointer extension;
} XmIconGCacheObjClassPart;


typedef struct _XmIconGCacheObjClassRec  /* Icon cache class record */
{
    ObjectClassPart                     object_class;
    XmExtClassPart                      ext_class;
    XmIconGCacheObjClassPart            icon_class_cache;
} XmIconGCacheObjClassRec;

externalref XmIconGCacheObjClassRec xmIconGCacheObjClassRec;

/*  The Icon Gadget Cache instance record  */

typedef struct _XmIconGCacheObjPart
{
   XmRenderTable    render_table;		/* XmNrenderTable */
   GC               selected_GC;
   GC               inverse_GC;
 	
   Pixel            background;
   Pixel            foreground;
   Pixel            top_shadow_color;
   Pixel            bottom_shadow_color;
   Pixel            highlight_color;

   Pixmap           background_pixmap;
   Pixmap           top_shadow_pixmap;
   Pixmap           bottom_shadow_pixmap;
   Pixmap           highlight_pixmap;

   GC               normal_GC;
   GC               background_GC;
   GC               insensitive_GC;
   GC               top_shadow_GC;
   GC               bottom_shadow_GC;
   GC               highlight_GC;
  
   unsigned char    alignment;
   Dimension        spacing;
   Dimension        margin_width;
   Dimension        margin_height;
} XmIconGCacheObjPart;

typedef struct _XmIconGCacheObjRec
{
  ObjectPart                object;
  XmExtPart		    ext;
  XmIconGCacheObjPart       icon_cache;
} XmIconGCacheObjRec;

typedef struct _XmIconGCacheObjRec   * XmIconGCacheObject;

/* IconGadget instance record */
typedef struct _XmIconGadgetPart
	{
	XmString	label_string;		/* XmNlabelString */
	Pixmap		large_icon_mask;	/* XmNlargeIconMask */
	Pixmap		large_icon_pixmap;	/* XmNlargeIconPixmap */
	Pixmap		small_icon_mask;	/* XmNsmallIconMask */
	Pixmap		small_icon_pixmap;	/* XmNsmallIconPixmap */
	unsigned char	viewtype;		/* XmNviewType */
	unsigned char	visual_emphasis;	/* XmNvisualEmphasis */
	XmString *	detail;	                /* XmNdetail */
	Cardinal	detail_count;	        /* XmNdetailCount */
	/* Private variables */
	Dimension	label_rect_width ;
	Dimension	label_rect_height ;
	Dimension	large_icon_rect_width;
	Dimension	large_icon_rect_height;
	Dimension	small_icon_rect_width;
	Dimension	small_icon_rect_height;
	String          large_pixmap_name ;
	String          small_pixmap_name ;

	XmIconGCacheObjPart  *cache;
   	Boolean	        check_set_render_table;
} XmIconGadgetPart;

/* Full instance record declaration */
typedef struct _XmIconGadgetRec
	{
	ObjectPart	object;
	RectObjPart	rectangle;
	XmGadgetPart	gadget;
	XmIconGadgetPart icong;
	} 	XmIconGadgetRec;


/* Useful macros */
#define	IG_LabelString(w)	(((XmIconGadget)(w))->icong.label_string)
#define	IG_LargeIconMask(w)	(((XmIconGadget)(w))->icong.large_icon_mask)
#define	IG_LargeIconPixmap(w)	(((XmIconGadget)(w))->icong.large_icon_pixmap)
#define	IG_SmallIconMask(w)	(((XmIconGadget)(w))->icong.small_icon_mask)
#define IG_SmallIconPixmap(w)	(((XmIconGadget)(w))->icong.small_icon_pixmap)
#define	IG_ViewType(w)		(((XmIconGadget)(w))->icong.viewtype)
#define	IG_VisualEmphasis(w)	(((XmIconGadget)(w))->icong.visual_emphasis)
#define	IG_Detail(w)	        (((XmIconGadget)(w))->icong.detail)
#define	IG_DetailCount(w)	(((XmIconGadget)(w))->icong.detail_count)
#define	IG_LabelRectWidth(w)	(((XmIconGadget)(w))->icong.label_rect_width)
#define	IG_LabelRectHeight(w)	(((XmIconGadget)(w))->icong.label_rect_height)
#define	IG_LargeIconRectWidth(w) \
                (((XmIconGadget)(w))->icong.large_icon_rect_width)
#define	IG_LargeIconRectHeight(w) \
		(((XmIconGadget)(w))->icong.large_icon_rect_height)
#define	IG_SmallIconRectWidth(w) \
		(((XmIconGadget)(w))->icong.small_icon_rect_width)
#define	IG_SmallIconRectHeight(w) \
		(((XmIconGadget)(w))->icong.small_icon_rect_height)
#define	IG_LargePixmapName(w) (((XmIconGadget)(w))->icong.large_pixmap_name)
#define	IG_SmallPixmapName(w) (((XmIconGadget)(w))->icong.small_pixmap_name)

/* XmNrecomputeSize didn't make it as a resource, but since the
   code is already written, I'll keep it and force its value here.
   If it's ever wanted back, just replace that macro by:
 #define IG_RecomputeSize(w)	(((XmIconGadget)(w))->icong.recompute_size) */
#define	IG_RecomputeSize(w)	(True) 

#define	IG_LayoutDirection(w)	(((XmIconGadget)(w))->gadget.layout_direction)
#define	IG_Highlighted(w)	(((XmIconGadget)(w))->gadget.highlighted)
#define	IG_HLThickness(w)     (((XmIconGadget)(w))->gadget.highlight_thickness)
#define	IG_ShadowThickness(w)	(((XmIconGadget)(w))->gadget.shadow_thickness)
#define	IG_Depth(w)		(((XmManagerWidget) \
			      (((XmGadget)(w))->object.parent))->core.depth)

/* cached resources for IconGadget */
#define	IG_RenderTable(w)	(((XmIconGadget)(w))-> \
				 icong.cache->render_table)
#define	IG_SelectedGC(w)	(((XmIconGadget)(w))-> \
				 icong.cache->selected_GC)
#define	IG_InverseGC(w)	        (((XmIconGadget)(w))-> \
				 icong.cache->inverse_GC)

/** These are gadget resources really. hopefully in 2.1,
    that will be replaced by stuff like:
    #define	IG_Background(w)    Gad_Background(w)
    #define	IG_BackgroundGC(w)  Gad_BackgroundGC(w)
    etc, etc ***/
#define	IG_Background(w)	(((XmIconGadget)(w))-> \
				 icong.cache->background)
#define	IG_Foreground(w)	(((XmIconGadget)(w))-> \
				 icong.cache->foreground)
#define	IG_TopShadowColor(w)	(((XmIconGadget)(w))-> \
				 icong.cache->top_shadow_color)
#define	IG_BottomShadowColor(w)	(((XmIconGadget)(w))-> \
				 icong.cache->bottom_shadow_color)
#define	IG_HighlightColor(w)	(((XmIconGadget)(w))-> \
				 icong.cache->highlight_color)

#define	IG_BackgroundPixmap(w)	(((XmIconGadget)(w))-> \
				 icong.cache->background_pixmap)
#define	IG_TopShadowPixmap(w)	(((XmIconGadget)(w))-> \
				 icong.cache->top_shadow_pixmap)
#define	IG_BottomShadowPixmap(w)	(((XmIconGadget)(w))-> \
				 icong.cache->bottom_shadow_pixmap)
#define	IG_HighlightPixmap(w)	(((XmIconGadget)(w))-> \
				 icong.cache->highlight_pixmap)

#define	IG_NormalGC(w)	        (((XmIconGadget)(w))-> \
				 icong.cache->normal_GC)
#define	IG_BackgroundGC(w)	(((XmIconGadget)(w))-> \
				 icong.cache->background_GC)
#define	IG_InsensitiveGC(w)	(((XmIconGadget)(w))-> \
				 icong.cache->insensitive_GC)
#define	IG_TopShadowGC(w)	(((XmIconGadget)(w))-> \
				 icong.cache->top_shadow_GC)
#define	IG_BottomShadowGC(w)	(((XmIconGadget)(w))-> \
				 icong.cache->bottom_shadow_GC)
#define	IG_HighlightGC(w)	(((XmIconGadget)(w))-> \
				 icong.cache->highlight_GC)
#define	IG_Alignment(w) 	(((XmIconGadget)(w))-> \
				 icong.cache->alignment)
#define	IG_Spacing(w) 	 	(((XmIconGadget)(w))-> \
				 icong.cache->spacing)
#define	IG_MarginWidth(w) 	(((XmIconGadget)(w))-> \
				 icong.cache->margin_width)
#define	IG_MarginHeight(w) 	(((XmIconGadget)(w))-> \
				 icong.cache->margin_height)


/* Convenience Macros */
#define IG_Cache(w)            (((XmIconGadget)(w))->icong.cache)
#define IG_ClassCachePart(w)   (((XmIconGadgetClass)xmIconGadgetClass)->\
				gadget_class.cache_part)

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#else /* MOTIF12_HEADERS */


/* $XConsortium: IconGP.h /main/cde1_maint/2 1995/08/18 19:07:27 drk $ */
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
/* (c) Copyright 1990, 1991, 1992, 1993 HEWLETT-PACKARD COMPANY */


#include <Xm/IconG.h>
#include <Xm/XmP.h>
#include <Xm/ExtObjectP.h>
#include <Xm/CacheP.h>


/*-------------------------------------------------------------
**	Cache Class Structure
*/

/*	Cache Class Part
*/
typedef struct _XmIconGCacheObjClassPart
{
    int foo;
} XmIconGCacheObjClassPart;

/*	Cache Full Class Record
*/
typedef struct _XmIconGCacheObjClassRec     /* label cache class record */
{
    ObjectClassPart                     object_class;
    XmExtClassPart                      ext_class;
    XmIconGCacheObjClassPart            icon_class_cache;
} XmIconGCacheObjClassRec;

/*	Cache Actual Class
*/
externalref XmIconGCacheObjClassRec xmIconGCacheObjClassRec;


/*-------------------------------------------------------------
**	Cache Instance Structure
*/

/*	Cache Instance Part
*/
typedef struct _XmIconGCacheObjPart
{
	Dimension	margin_width;
	Dimension	margin_height;
	Dimension	string_height;
	Dimension	spacing;
	Pixel		foreground;
	Pixel		background;
	Pixel		arm_color;
	Boolean		fill_on_arm;
	Boolean		recompute_size;
	unsigned char	pixmap_position;
	unsigned char	string_position;
	unsigned char	alignment;
	unsigned char	behavior;
	unsigned char	fill_mode;
} XmIconGCacheObjPart;

typedef struct _XmIconGCacheObjRec
{
    ObjectPart               object;
    XmExtPart                ext;
    XmIconGCacheObjPart      icon_cache;
} XmIconGCacheObjRec;


typedef void (*XmGetPositionProc)(
#ifndef _NO_PROTO
	Widget,
#if NeedWidePrototypes
	int,
	int,
	int,
	int,
#else /* NeedWidePrototypes */
	Position,
	Position,
	Dimension,
	Dimension,
#endif /* NeedWidePrototypes */
	Position *,
	Position *,
	Position *,
	Position *
#endif
);
typedef void (*XmGetSizeProc)(
#ifndef _NO_PROTO
	Widget,
	Dimension *,
	Dimension *
#endif
);
typedef void (*XmDrawProc)(
#ifndef _NO_PROTO
	Widget,
	Drawable,
#if NeedWidePrototypes
	int,
	int,
	int,
	int,
	int,
	int,
	unsigned int,
	unsigned int
#else /* NeedWidePrototypes */
	Position,
	Position,
	Dimension,
	Dimension,
	Dimension,
	Dimension,
	unsigned char,
	unsigned char
#endif /* NeedWidePrototypes */
#endif
);
typedef void (*XmCallCallbackProc)(
#ifndef _NO_PROTO
	Widget,
	XtCallbackList,
	int,
	XEvent *
#endif
);
typedef void (*XmUpdateGCsProc)(
#ifndef _NO_PROTO
	Widget
#endif
);
/*-------------------------------------------------------------
**	Class Structure
*/

/*	Class Part
*/
typedef struct _XmIconGadgetClassPart
{
	XmGetSizeProc		get_size;
	XmGetPositionProc		get_positions;
	XmDrawProc		draw;
	XmCallCallbackProc	call_callback;
	XmUpdateGCsProc		update_gcs;
	Boolean			optimize_redraw;
	XmCacheClassPartPtr	cache_part;
	XtPointer			extension;
} XmIconGadgetClassPart;

/*	Full Class Record
*/
typedef struct _XmIconGadgetClassRec
{
	RectObjClassPart	rect_class;
	XmGadgetClassPart	gadget_class;
	XmIconGadgetClassPart	icon_class;
} XmIconGadgetClassRec;

/*	Actual Class
*/
externalref XmIconGadgetClassRec xmIconGadgetClassRec;



/*-------------------------------------------------------------
**	Instance Structure
*/

/*	Instance Part
*/
typedef struct _XmIconGadgetPart
{
	Boolean		set;
	Boolean		armed;
	Boolean		sync;
	Boolean		underline;
	unsigned char	shadow_type;
	unsigned char	border_type;
	XtCallbackList	callback;
	XtIntervalId	click_timer_id;
	XButtonEvent *	click_event;
	String		image_name;
	Pixmap		pixmap;
	Pixmap		mask;
	Pixel		pixmap_foreground;
	Pixel		pixmap_background;
	XmFontList	font_list;
	_XmString	string;
	Dimension	string_width;
	Dimension	pixmap_width;
	Dimension	pixmap_height;
	GC		clip_gc;
	GC		normal_gc;
	GC		background_gc;
	GC		armed_gc;
	GC		armed_background_gc;
	GC		parent_background_gc;
        Pixel		saved_parent_background;
	XmIconGCacheObjPart *cache;
} XmIconGadgetPart;

/*	Full Instance Record
*/
typedef struct _XmIconGadgetRec
{
	ObjectPart	object;
	RectObjPart	rectangle;
	XmGadgetPart	gadget;
	XmIconGadgetPart icon;
} XmIconGadgetRec;


/*-------------------------------------------------------------
**	Class and Instance Macros
*/


/*	XmIconGadget Macros
*/
#define IG_GetSize(g,w,h) \
  (((XmIconGadgetClassRec *)((XmIconGadget) g) -> object.widget_class) -> icon_class.get_size) \
	(g,w,h)
#define IG_GetPositions(g,w,h,h_t,s_t,p_x,p_y,s_x,s_y) \
  (((XmIconGadgetClassRec *)((XmIconGadget) g) -> object.widget_class) -> icon_class.get_positions) \
	(g,w,h,h_t,s_t,p_x,p_y,s_x,s_y)
#define IG_Draw(g,d,x,y,w,h,h_t,s_t,s_type,fill) \
  (((XmIconGadgetClassRec *)((XmIconGadget) g) -> object.widget_class) -> icon_class.draw) \
	(g,d,x,y,w,h,h_t,s_t,s_type,fill)
#define IG_CallCallback(g,cb,r,e) \
  (((XmIconGadgetClassRec *)((XmIconGadget) g) -> object.widget_class) -> icon_class.call_callback) \
	(g,cb,r,e)
#define IG_UpdateGCs(g) \
  (((XmIconGadgetClassRec *)((XmIconGadget) g) -> object.widget_class) -> icon_class.update_gcs) \
	(g)

/*	Cached Instance Field Macros
*/
#define IG_FillOnArm(g)		(((XmIconGadget)(g)) -> \
				  icon.cache -> fill_on_arm)
#define IG_RecomputeSize(g)	(((XmIconGadget)(g)) -> \
				  icon.cache -> recompute_size)
#define IG_PixmapPosition(g)	(((XmIconGadget)(g)) -> \
				  icon.cache -> pixmap_position)
#define IG_StringPosition(g)	(((XmIconGadget)(g)) -> \
				  icon.cache -> string_position)
#define IG_Alignment(g)		(((XmIconGadget)(g)) -> \
				  icon.cache -> alignment)
#define IG_Behavior(g)		(((XmIconGadget)(g)) -> \
				  icon.cache -> behavior)
#define IG_FillMode(g)		(((XmIconGadget)(g)) -> \
				  icon.cache -> fill_mode)
#define IG_MarginWidth(g)	(((XmIconGadget)(g)) -> \
				  icon.cache -> margin_width)
#define IG_MarginHeight(g)	(((XmIconGadget)(g)) -> \
				  icon.cache -> margin_height)
#define IG_StringHeight(g)	(((XmIconGadget)(g)) -> \
				  icon.cache -> string_height)
#define IG_Spacing(g)		(((XmIconGadget)(g)) -> \
				  icon.cache -> spacing)
#define IG_Foreground(g)		(((XmIconGadget)(g)) -> \
				  icon.cache -> foreground)
#define IG_Background(g)		(((XmIconGadget)(g)) -> \
				  icon.cache -> background)
#define IG_ArmColor(g)		(((XmIconGadget)(g)) -> \
				  icon.cache -> arm_color)

/*	Non-Cached Instance Field Macros
*/
#define IG_Armed(g)		(g -> icon.armed)
#define IG_Set(g)		(g -> icon.set)
#define IG_Sync(g)		(g -> icon.sync)
#define IG_Callback(g)		(g -> icon.callback)
#define IG_ClickTimerID(g)	(g -> icon.click_timer_id)
#define IG_ClickInterval(g)	(g -> icon.click_interval)
#define IG_ClickEvent(g)		(g -> icon.click_event)
#define IG_ShadowType(g)		(g -> icon.shadow_type)
#define IG_BorderType(g)		(g -> icon.border_type)
#define IG_Pixmap(g)		(g -> icon.pixmap)
#define IG_Mask(g)		(g -> icon.mask)
#define IG_PixmapForeground(g)	(g -> icon.pixmap_foreground)
#define IG_PixmapBackground(g)	(g -> icon.pixmap_background)
#define IG_String(g)		(g -> icon.string)
#define IG_FontList(g)		(g -> icon.font_list)
#define IG_ImageName(g)		(g -> icon.image_name)
#define IG_StringWidth(g)	(g -> icon.string_width)
#define IG_PixmapWidth(g)	(g -> icon.pixmap_width)
#define IG_PixmapHeight(g)	(g -> icon.pixmap_height)
#define IG_BackgroundGC(g)	(g -> icon.background_gc)
#define IG_ArmedGC(g)		(g -> icon.armed_gc)
#define IG_ArmedBackgroundGC(g)	(g -> icon.armed_background_gc)
#define IG_NormalGC(g)		(g -> icon.normal_gc)
#define IG_ClipGC(g)		(g -> icon.clip_gc)
#define IG_Underline(g)		(g -> icon.underline)
#define IG_ParentBackgroundGC(g) (g -> icon.parent_background_gc)
#define IG_SavedParentBG(g)      (g -> icon.saved_parent_background)


/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern int _XmIconGCacheCompare() ;
extern void _XmReCacheIconG() ;
extern void _XmAssignIconG_StringHeight() ;
extern void _XmAssignIconG_Foreground() ;
extern void _XmAssignIconG_Background() ;
extern Boolean _XmIconGadgetGetState() ;
extern void _XmIconGadgetSetState() ;
extern Drawable _XmIconGadgetDraw() ;
extern Widget _XmDuplicateIconG() ;
extern Boolean _XmIconGSelectInTitle() ;
extern XRectangle * _XmIconGGetTextExtent() ;
extern void _XmIconGGetIconRects() ;

#else

extern int _XmIconGCacheCompare( 
                        XtPointer ii,
                        XtPointer ici) ;
extern void _XmReCacheIconG( 
                        XmIconGadget g) ;
extern void _XmAssignIconG_StringHeight( 
                        XmIconGadget g,
#if NeedWidePrototypes
                        int value) ;
#else
                        Dimension value) ;
#endif /* NeedWidePrototypes */
extern void _XmAssignIconG_Foreground( 
                        XmIconGadget g,
                        Pixel value) ;
extern void _XmAssignIconG_Background( 
                        XmIconGadget g,
                        Pixel value) ;
extern Boolean _XmIconGadgetGetState( 
                        Widget w) ;
extern void _XmIconGadgetSetState( 
                        Widget w,
#if NeedWidePrototypes
                        int state,
                        int notify) ;
#else
                        Boolean state,
                        Boolean notify) ;
#endif /* NeedWidePrototypes */
extern Drawable _XmIconGadgetDraw( 
                        Widget widget,
                        Drawable drawable,
#if NeedWidePrototypes
                        int x,
                        int y,
                        int fill) ;
#else
                        Position x,
                        Position y,
                        Boolean fill) ;
#endif /* NeedWidePrototypes */
extern Widget _XmDuplicateIconG( 
                        Widget parent,
                        Widget widget,
                        XmString string,
                        String pixmap,
                        XtPointer user_data,
#if NeedWidePrototypes
                        int underline) ;
#else
                        Boolean underline) ;
#endif /* NeedWidePrototypes */
extern Boolean _XmIconGSelectInTitle( 
                        Widget widget,
#if NeedWidePrototypes
                        int pt_x,
                        int pt_y) ;
#else
                        Position pt_x,
                        Position pt_y) ;
#endif /* NeedWidePrototypes */
extern XRectangle * _XmIconGGetTextExtent( 
                        Widget widget) ;
extern void _XmIconGGetIconRects( 
                        Widget gw,
                        unsigned char *flags,
                        XRectangle *rect1,
                        XRectangle *rect2) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/

#endif /* MOTIF12_HEADERS */

#endif /* _XmIconP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
