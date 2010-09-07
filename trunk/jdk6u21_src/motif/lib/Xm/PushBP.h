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
/*   $XConsortium: PushBP.h /main/12 1995/07/13 17:44:19 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
*  (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
/*
*  (c) Copyright 1988 MICROSOFT CORPORATION */
#ifndef _XmPButtonP_h
#define _XmPButtonP_h

#include <Xm/PushB.h>
#include <Xm/LabelP.h>

#ifdef __cplusplus
extern "C" {
#endif

/* PushButton class structure */

typedef struct _XmPushButtonClassPart
{
   XtPointer extension;   /* Pointer to extension record */
} XmPushButtonClassPart;


/* Full class record declaration for PushButton class */

typedef struct _XmPushButtonClassRec {
    CoreClassPart	  core_class;
    XmPrimitiveClassPart  primitive_class;
    XmLabelClassPart      label_class;
    XmPushButtonClassPart pushbutton_class;
} XmPushButtonClassRec;


externalref XmPushButtonClassRec xmPushButtonClassRec;

/* PushButton instance record */

typedef struct _XmPushButtonPart
{
   Boolean 	    fill_on_arm;
   Dimension        show_as_default;
   Pixel	    arm_color;
   Pixmap	    arm_pixmap;
   XtCallbackList   activate_callback;
   XtCallbackList   arm_callback;
   XtCallbackList   disarm_callback;

   Boolean 	    armed;
   Pixmap	    unarm_pixmap;
   GC               fill_gc;
   GC               background_gc;
   XtIntervalId     timer;	
   unsigned char    multiClick;		/* KEEP/DISCARD resource */
   int		    click_count;
   Time		    armTimeStamp;
   Boolean      compatible;   /* if false it is Motif 1.1 else Motif 1.0  */
   Dimension    default_button_shadow_thickness;  
		/* New resource - always add it
                    to widgets dimension. */

} XmPushButtonPart;


/* Full instance record declaration */

typedef struct _XmPushButtonRec {
    CorePart	     core;
    XmPrimitivePart  primitive;
    XmLabelPart      label;
    XmPushButtonPart pushbutton;
} XmPushButtonRec;


/********    Private Function Declarations    ********/

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmPButtonP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
