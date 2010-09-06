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
/*   $XConsortium: ArrowBGP.h /main/13 1995/07/14 10:09:51 drk $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmArrowGadgetP_h
#define _XmArrowGadgetP_h

#ifndef MOTIF12_HEADERS

#include <Xm/ArrowBG.h>
#include <Xm/GadgetP.h>

#ifdef __cplusplus
extern "C" {
#endif

/*  Arrow class structure  */

typedef struct _XmArrowButtonGadgetClassPart
{
   XtPointer extension;
} XmArrowButtonGadgetClassPart;


/*  Full class record declaration for Arrow class  */

typedef struct _XmArrowButtonGadgetClassRec
{
  RectObjClassPart             rect_class;
  XmGadgetClassPart            gadget_class;
  XmArrowButtonGadgetClassPart arrow_button_class;
} XmArrowButtonGadgetClassRec;

externalref XmArrowButtonGadgetClassRec xmArrowButtonGadgetClassRec;

/*  The Arrow instance record  */

typedef struct _XmArrowButtonGadgetPart
{
  XtCallbackList activate_callback;
  XtCallbackList arm_callback;
  XtCallbackList disarm_callback;
  unsigned char  direction;	/* The direction the arrow is pointing. */

  Boolean	 selected;

  short		 top_count;
  short		 cent_count;
  short		 bot_count;
  XRectangle	*top;
  XRectangle	*cent;
  XRectangle	*bot;

  Position	 old_x;
  Position	 old_y;

  GC		 arrow_GC;
  XtIntervalId	 timer;	
  unsigned char	 multiClick;	/* KEEP/DISCARD resource */
  int		 click_count;
  GC		 insensitive_GC;

   
  GC		 background_GC;
  GC		 top_shadow_GC;
  GC		 bottom_shadow_GC;
  GC		 highlight_GC;
   
  Pixel		 foreground;
  Pixel		 background;
   
  Pixel		 top_shadow_color;
  Pixmap	 top_shadow_pixmap;
   
  Pixel		 bottom_shadow_color;
  Pixmap	 bottom_shadow_pixmap;
   
  Pixel		 highlight_color;
  Pixmap	 highlight_pixmap;

  Boolean	 fill_bg_box;
  Dimension detail_shadow_thickness ;
  Boolean    colors_inherited;
} XmArrowButtonGadgetPart;


/*  Full instance record declaration  */

typedef struct _XmArrowButtonGadgetRec
{
   ObjectPart              object;
   RectObjPart             rectangle;
   XmGadgetPart            gadget;
   XmArrowButtonGadgetPart arrowbutton;
} XmArrowButtonGadgetRec;

#define ARROWBG_INHERIT_NONE			0
#define ARROWBG_INHERIT_BACKGROUND		(1 << 0)
#define ARROWBG_INHERIT_FOREGROUND		(1 << 1)
#define ARROWBG_INHERIT_TOP_SHADOW		(1 << 2)
#define ARROWBG_INHERIT_BOTTOM_SHADOW	(1 << 3)
#define ARROWBG_INHERIT_HIGHLIGHT		(1 << 4)

#define ArrowBG_SetColorsInherited(byte, bit, v)	byte = (byte & (~bit)) | (v ? bit : 0)
#define ArrowBG_InheritBackground(w) (((XmArrowButtonGadget)(w))-> \
									arrowbutton.colors_inherited & ARROWBG_INHERIT_BACKGROUND)
#define ArrowBG_InheritForeground(w) (((XmArrowButtonGadget)(w))-> \
									arrowbutton.colors_inherited & ARROWBG_INHERIT_FOREGROUND)
#define ArrowBG_InheritTopShadow(w) (((XmArrowButtonGadget)(w))-> \
									arrowbutton.colors_inherited & ARROWBG_INHERIT_TOP_SHADOW)
#define ArrowBG_InheritBottomShadow(w) (((XmArrowButtonGadget)(w))-> \
									arrowbutton.colors_inherited & ARROWBG_INHERIT_BOTTOM_SHADOW)
#define ArrowBG_InheritHighlight(w) (((XmArrowButtonGadget)(w))-> \
									arrowbutton.colors_inherited & ARROWBG_INHERIT_HIGHLIGHT)

#define ArrowBG_BackgroundGC(w)		(((XmArrowButtonGadget)(w)) -> \
                                           arrowbutton. background_GC)
#define ArrowBG_TopShadowGC(w)		(((XmArrowButtonGadget)(w)) -> \
                                           arrowbutton. top_shadow_GC)
#define ArrowBG_BottomShadowGC(w)	(((XmArrowButtonGadget)(w)) -> \
                                           arrowbutton. bottom_shadow_GC)
#define ArrowBG_HighlightGC(w)		(((XmArrowButtonGadget)(w)) -> \
                                           arrowbutton. highlight_GC)
#define ArrowBG_Foreground(w)		(((XmArrowButtonGadget)(w)) -> \
                                           arrowbutton. foreground)
#define ArrowBG_Background(w)		(((XmArrowButtonGadget)(w)) -> \
                                           arrowbutton. background)
#define ArrowBG_TopShadowColor(w)	(((XmArrowButtonGadget)(w)) -> \
                                           arrowbutton. top_shadow_color)
#define ArrowBG_TopShadowPixmap(w)	(((XmArrowButtonGadget)(w)) -> \
                                           arrowbutton. top_shadow_pixmap)
#define ArrowBG_BottomShadowColor(w)	(((XmArrowButtonGadget)(w)) -> \
                                           arrowbutton. bottom_shadow_color)
#define ArrowBG_BottomShadowPixmap(w)	(((XmArrowButtonGadget)(w)) -> \
                                           arrowbutton. bottom_shadow_pixmap)
#define ArrowBG_HighlightColor(w)	(((XmArrowButtonGadget)(w)) -> \
                                           arrowbutton. highlight_color)
#define ArrowBG_HighlightPixmap(w)	(((XmArrowButtonGadget)(w)) -> \
                                           arrowbutton. highlight_pixmap)
#define ArrowBG_ColorsInherited(w)	(((XmArrowButtonGadget)(w)) -> \
                                           arrowbutton. colors_inherited)

/********    Private Function Declarations    ********/


/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#else  /* MOTIF12_HEADERS */

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
/*   $XConsortium: ArrowBGP.h /main/cde1_maint/2 1995/08/18 18:50:03 drk $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#include <Xm/ArrowBG.h>
#include <Xm/GadgetP.h>

#ifdef __cplusplus
extern "C" {
#endif

/*  Arrow class structure  */

typedef struct _XmArrowButtonGadgetClassPart
{
   XtPointer extension;
} XmArrowButtonGadgetClassPart;


/*  Full class record declaration for Arrow class  */

typedef struct _XmArrowButtonGadgetClassRec
{
   RectObjClassPart             rect_class;
   XmGadgetClassPart            gadget_class;
   XmArrowButtonGadgetClassPart arrow_button_class;
} XmArrowButtonGadgetClassRec;

externalref XmArrowButtonGadgetClassRec xmArrowButtonGadgetClassRec;

/* "Gadget caching" is currently under investigation for ArrowBG.  It would
 * be very desirable to cache the XRectangles, requiring immediate reference
 * instead of indirectly through pointers.  ArrowBG will be cached by Beta.
 */
/*  The Arrow instance record  */

typedef struct _XmArrowButtonGadgetPart
{
   XtCallbackList activate_callback;
   XtCallbackList arm_callback;
   XtCallbackList disarm_callback;
   unsigned char direction;	  /*  the direction the arrow is pointing  */

   Boolean selected;

   short        top_count;
   short        cent_count;
   short        bot_count;
   XRectangle * top;
   XRectangle * cent;
   XRectangle * bot;

   Position old_x;
   Position old_y;

   GC               arrow_GC;	    /* graphics context for arrow drawing */
   XtIntervalId     timer;	
   unsigned char    multiClick;     /* KEEP/DISCARD resource */
   int              click_count;
   GC       	    insensitive_GC; /* graphics context for insensitive arrow drawing */

} XmArrowButtonGadgetPart;


/*  Full instance record declaration  */

typedef struct _XmArrowButtonGadgetRec
{
   ObjectPart              object;
   RectObjPart             rectangle;
   XmGadgetPart            gadget;
   XmArrowButtonGadgetPart arrowbutton;
} XmArrowButtonGadgetRec;


/********    Private Function Declarations    ********/
#ifdef _NO_PROTO


#else


#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif   /* MOTIF12_HEADERS */

#endif /* _XmArrowGadgetP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
