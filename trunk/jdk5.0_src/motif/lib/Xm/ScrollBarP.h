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
/*   $XConsortium: ScrollBarP.h /main/13 1995/07/13 17:55:17 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmScrollBarP_h
#define _XmScrollBarP_h

#ifndef MOTIF12_HEADERS


#include <Xm/ScrollBar.h>
#include <Xm/PrimitiveP.h>

#ifdef __cplusplus
extern "C" {
#endif

/*  Minimum slider width or height  */

#define MIN_SLIDER_THICKNESS	1
#define MIN_SLIDER_LENGTH	6
#define DEFAULT_ROUND_MARK_RADIUS 3
#define THERMO_MARK_OFFSET 10

/*  ScrollBar class structure  */

typedef struct _XmScrollBarClassPart
{
   XtPointer extension;   /* Pointer to extension record */
} XmScrollBarClassPart;


/*  Full class record declaration for CheckBox class  */

typedef struct _XmScrollBarClassRec
{
   CoreClassPart        core_class;
   XmPrimitiveClassPart primitive_class;
   XmScrollBarClassPart scrollBar_class;
} XmScrollBarClassRec;

externalref XmScrollBarClassRec xmScrollBarClassRec;


/*  The ScrollBar instance record  */

typedef struct _XmScrollBarPart
{
   int value;
   int minimum;
   int maximum;
   int slider_size;

   unsigned char orientation;
   unsigned char processing_direction;
   XtEnum show_arrows;

   int increment;
   int page_increment;

   int initial_delay;
   int repeat_delay;

   XtCallbackList value_changed_callback;
   XtCallbackList increment_callback;
   XtCallbackList decrement_callback;
   XtCallbackList page_increment_callback;
   XtCallbackList page_decrement_callback;
   XtCallbackList to_top_callback;
   XtCallbackList to_bottom_callback;
   XtCallbackList drag_callback;

   /* obsolete */
   GC unhighlight_GC;
   /* Change the logical name of this one in 2.0 */

   GC foreground_GC;
   Pixel trough_color;

   Drawable pixmap;
   Boolean  sliding_on;
   int saved_value;

   XtEnum  etched_slider;
   /* we used a #define slider_visual to referenced this field in 
      the c file, etched_slider is not a good name anymore, */
   XtEnum slider_mark;
 
   unsigned char flags;
/* Values for the XmScrollBarPart flags field */

#define FIRST_SCROLL_FLAG (1<<0)
#define VALUE_SET_FLAG    (1<<1)
#define END_TIMER         (1<<2)
#define ARROW1_AVAILABLE  (1<<3)
#define ARROW2_AVAILABLE  (1<<4)
#define SLIDER_AVAILABLE  (1<<5)
#define KEYBOARD_GRABBED  (1<<6)
#define OPERATION_CANCELLED  (1<<7)  /* last field */

   unsigned char add_flags;
/* Values for the additionnal flags field */
#define SNAPPED_OUT         (1<<0)


   unsigned char change_type;
   XtIntervalId timer;

   short initial_x;
   short initial_y;
   short separation_x;
   short separation_y;

   short slider_x;
   short slider_y;
   short slider_width;
   short slider_height;

   short slider_area_x;
   short slider_area_y;
   short slider_area_width;
   short slider_area_height;

   short arrow1_x;
   short arrow1_y;
   unsigned char arrow1_orientation;
   Boolean arrow1_selected;

   short arrow2_x;
   short arrow2_y;
   unsigned char arrow2_orientation;
   Boolean arrow2_selected;

   short arrow_width;
   short arrow_height;

   /*  Obsolete fields as 1.2.0 */
   short arrow1_top_count;
   short arrow1_cent_count;
   short arrow1_bot_count;

   XRectangle * arrow1_top;
   XRectangle * arrow1_cent;
   XRectangle * arrow1_bot;

   short arrow2_top_count;
   short arrow2_cent_count;
   short arrow2_bot_count;

   XRectangle * arrow2_top;
   XRectangle * arrow2_cent;
   XRectangle * arrow2_bot;
   /***********/

   /* new for 1.2 */
   GC	unavailable_GC;

   /* new for 2.0 */
   unsigned short snap_back_multiple;
   XtEnum sliding_mode;
   Boolean editable;

   Mask dimMask ;  /* for the navigator trait */
} XmScrollBarPart;




/*  Full instance record declaration  */

typedef struct _XmScrollBarRec
{
   CorePart	   core;
   XmPrimitivePart primitive;
   XmScrollBarPart scrollBar;
} XmScrollBarRec;


/********    Private Function Declarations    ********/

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
/*   $XConsortium: ScrollBarP.h /main/cde1_maint/2 1995/08/18 19:20:57 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */


#include <Xm/ScrollBar.h>
#include <Xm/PrimitiveP.h>

#ifdef __cplusplus
extern "C" {
#endif

/*  Minimum slider width or height  */

#define MIN_SLIDER_THICKNESS	1
#define MIN_SLIDER_LENGTH	6


/*  ScrollBar class structure  */

typedef struct _XmScrollBarClassPart
{
   XtPointer extension;   /* Pointer to extension record */
} XmScrollBarClassPart;


/*  Full class record declaration for CheckBox class  */

typedef struct _XmScrollBarClassRec
{
   CoreClassPart        core_class;
   XmPrimitiveClassPart primitive_class;
   XmScrollBarClassPart scrollBar_class;
} XmScrollBarClassRec;

externalref XmScrollBarClassRec xmScrollBarClassRec;


/*  The ScrollBar instance record  */

typedef struct _XmScrollBarPart
{
   int value;
   int minimum;
   int maximum;
   int slider_size;

   unsigned char orientation;
   unsigned char processing_direction;
   Boolean show_arrows;

   int increment;
   int page_increment;

   int initial_delay;
   int repeat_delay;

   XtCallbackList value_changed_callback;
   XtCallbackList increment_callback;
   XtCallbackList decrement_callback;
   XtCallbackList page_increment_callback;
   XtCallbackList page_decrement_callback;
   XtCallbackList to_top_callback;
   XtCallbackList to_bottom_callback;
   XtCallbackList drag_callback;

   /* obsolete */
   GC unhighlight_GC;
   /***********/

   GC foreground_GC;
   Pixel trough_color;

   Drawable pixmap;
   Boolean  sliding_on;
   Boolean  etched_slider;
   int saved_value;

   unsigned char flags;

   unsigned char change_type;
   XtIntervalId timer;

   short initial_x;
   short initial_y;
   short separation_x;
   short separation_y;

   short slider_x;
   short slider_y;
   short slider_width;
   short slider_height;

   short slider_area_x;
   short slider_area_y;
   short slider_area_width;
   short slider_area_height;

   short arrow1_x;
   short arrow1_y;
   unsigned char arrow1_orientation;
   Boolean arrow1_selected;

   short arrow2_x;
   short arrow2_y;
   unsigned char arrow2_orientation;
   Boolean arrow2_selected;

   short arrow_width;
   short arrow_height;

   /*  Obsolete fields */
   short arrow1_top_count;
   short arrow1_cent_count;
   short arrow1_bot_count;

   XRectangle * arrow1_top;
   XRectangle * arrow1_cent;
   XRectangle * arrow1_bot;

   short arrow2_top_count;
   short arrow2_cent_count;
   short arrow2_bot_count;

   XRectangle * arrow2_top;
   XRectangle * arrow2_cent;
   XRectangle * arrow2_bot;
   /***********/


   /* new for 1.2 */
   GC	unavailable_GC;
} XmScrollBarPart;


/*  Full instance record declaration  */

typedef struct _XmScrollBarRec
{
   CorePart	   core;
   XmPrimitivePart primitive;
   XmScrollBarPart scrollBar;
} XmScrollBarRec;


/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern void _XmSetEtchedSlider() ;

#else

extern void _XmSetEtchedSlider( 
                        XmScrollBarWidget sbw) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* MOTIF12_HEADERS */

#endif /* _XmScrollBarP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
