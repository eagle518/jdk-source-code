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
/* $XConsortium: ToggleBP.h /main/14 1996/03/29 18:56:59 pascale $ */
/* (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/********************************************
 *
 *   No new fields need to be defined
 *   for the Toggle widget class record
 *
 ********************************************/

#ifndef _XmToggleButtonP_h
#define _XmToggleButtonP_h

#ifndef MOTIF12_HEADERS 

#include <Xm/ToggleB.h>
#include <Xm/LabelP.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _XmToggleButtonClassPart
 {
   XtPointer extension;   /* Pointer to extension record */
 } XmToggleButtonClassPart;


/****************************************************
 *
 * Full class record declaration for Toggle class
 *
 ****************************************************/
typedef struct _XmToggleButtonClassRec {
    CoreClassPart	  	core_class;
    XmPrimitiveClassPart  	primitive_class;
    XmLabelClassPart      	label_class;
    XmToggleButtonClassPart	toggle_class;
} XmToggleButtonClassRec;


externalref XmToggleButtonClassRec xmToggleButtonClassRec;


/********************************************
 *
 * No new fields needed for instance record
 *
 ********************************************/

typedef struct _XmToggleButtonPart
{ 
   unsigned char	ind_type;
   Boolean		visible;
   Dimension		spacing;
   Dimension		indicator_dim;
   Boolean		indicator_set;
   Pixmap		on_pixmap; 
   Pixmap		insen_pixmap; 
   unsigned char	set;
   unsigned char	visual_set; /* used for visuals and does not reflect
                                        the true state of the button */
   unsigned char	ind_on;
   Boolean		fill_on_select;
   Pixel		select_color;
   GC			select_GC;
   GC			background_gc;
   GC                   arm_GC;    /* used in menus when enableEtchedInMenu 
				      is set. */
   XtCallbackList 	value_changed_CB,
                        arm_CB,
                        disarm_CB;
   Boolean      	Armed;
   unsigned char        toggle_mode;
   Boolean		reversed_select;
   Pixmap               indeterminate_pixmap;
   Pixmap               indeterminate_insensitive_pixmap;
   Pixel                unselect_color;
   GC                   unselect_GC;
   GC                   indeterminate_GC;
   GC                   indeterminate_box_GC;
   Dimension		ind_left_delta;
   Dimension		ind_right_delta;
   Dimension		ind_top_delta;
   Dimension		ind_bottom_delta;
   Dimension detail_shadow_thickness ;
} XmToggleButtonPart;



/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _XmToggleButtonRec {
    CorePart	        core;
    XmPrimitivePart     primitive;
    XmLabelPart		label;
    XmToggleButtonPart  toggle;
} XmToggleButtonRec;


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
/*   $XConsortium: ToggleBP.h /main/cde1_maint/2 1995/08/18 19:30:20 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/********************************************
 *
 *   No new fields need to be defined
 *   for the Toggle widget class record
 *
 ********************************************/

#include <Xm/ToggleB.h>
#include <Xm/LabelP.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _XmToggleButtonClassPart
 {
    XtPointer extension;   /* Pointer to extension record */
 } XmToggleButtonClassPart;


/****************************************************
 *
 * Full class record declaration for Toggle class
 *
 ****************************************************/
typedef struct _XmToggleButtonClassRec {
    CoreClassPart	  	core_class;
    XmPrimitiveClassPart  	primitive_class;
    XmLabelClassPart      	label_class;
    XmToggleButtonClassPart	toggle_class;
} XmToggleButtonClassRec;


externalref XmToggleButtonClassRec xmToggleButtonClassRec;


/********************************************
 *
 * No new fields needed for instance record
 *
 ********************************************/

typedef struct _XmToggleButtonPart
{ 
   unsigned char	ind_type;
   Boolean		visible;
   Dimension		spacing;
   Dimension		indicator_dim;
   Boolean		indicator_set;
   Pixmap		on_pixmap; 
   Pixmap		insen_pixmap; 
   Boolean		set;
   Boolean     		visual_set; /* used for visuals and does not reflect
                                        the true state of the button */
   Boolean		ind_on;
   Boolean		fill_on_select;
   Pixel		select_color;
   GC			select_GC;
   GC			background_gc;
   XtCallbackList 	value_changed_CB,
                        arm_CB,
                        disarm_CB;
   Boolean      	Armed;
} XmToggleButtonPart;



/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _XmToggleButtonRec {
    CorePart	        core;
    XmPrimitivePart     primitive;
    XmLabelPart		label;
    XmToggleButtonPart  toggle;
} XmToggleButtonRec;


/********    Private Function Declarations    ********/
#ifdef _NO_PROTO


#else


#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* MOTIF12_HEADERS */

#endif /* _XmToggleButtonP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
