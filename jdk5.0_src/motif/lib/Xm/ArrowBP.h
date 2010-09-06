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
/*   $XConsortium: ArrowBP.h /main/13 1995/07/14 10:10:05 drk $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmArrowButtonP_h
#define _XmArrowButtonP_h


#ifndef MOTIF12_HEADERS

#include <Xm/ArrowB.h>
#include <Xm/PrimitiveP.h>

#ifdef __cplusplus
extern "C" {
#endif

/*  Arrow class structure  */

typedef struct _XmArrowButtonClassPart
{
  XtPointer extension;
} XmArrowButtonClassPart;


/*  Full class record declaration for Arrow class  */

typedef struct _XmArrowButtonClassRec
{
  CoreClassPart	  	 core_class;
  XmPrimitiveClassPart	 primitive_class;
  XmArrowButtonClassPart arrowbutton_class;
} XmArrowButtonClassRec;

externalref XmArrowButtonClassRec xmArrowButtonClassRec;


/*  The ArrowButton instance record  */

typedef struct _XmArrowButtonPart
{
  XtCallbackList activate_callback;
  XtCallbackList arm_callback;
  XtCallbackList disarm_callback;
  unsigned char  direction;	/* The direction the arrow is pointing. */

  Boolean 	 selected;
  short          top_count;
  short          cent_count;
  short          bot_count;
  XRectangle    *top;
  XRectangle    *cent;
  XRectangle    *bot;

  GC		 arrow_GC;
  XtIntervalId   timer;	
  unsigned char  multiClick;	/* KEEP/DISCARD resource */
  int            click_count;
  Time		 armTimeStamp;
  GC		 insensitive_GC;
  Dimension detail_shadow_thickness ;
} XmArrowButtonPart;


/*  Full instance record declaration  */

typedef struct _XmArrowButtonRec
{
  CorePart	   	core;
  XmPrimitivePart	primitive;
  XmArrowButtonPart	arrowbutton;
} XmArrowButtonRec;


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
/*   $XConsortium: ArrowBP.h /main/cde1_maint/2 1995/08/18 18:50:15 drk $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#include <Xm/ArrowB.h>
#include <Xm/PrimitiveP.h>

#ifdef __cplusplus
extern "C" {
#endif

/*  Arrow class structure  */

typedef struct _XmArrowButtonClassPart
{
   XtPointer extension;
} XmArrowButtonClassPart;


/*  Full class record declaration for Arrow class  */

typedef struct _XmArrowButtonClassRec
{
   CoreClassPart        	core_class;
   XmPrimitiveClassPart 	primitive_class;
   XmArrowButtonClassPart     	arrowbutton_class;
} XmArrowButtonClassRec;

externalref XmArrowButtonClassRec xmArrowButtonClassRec;


/*  The ArrowButton instance record  */

typedef struct _XmArrowButtonPart
{
   XtCallbackList activate_callback;
   XtCallbackList arm_callback;
   XtCallbackList disarm_callback;
   unsigned char  direction;	  /*  the direction the arrow is pointing  */

   Boolean selected;
   short        top_count;
   short        cent_count;
   short        bot_count;
   XRectangle * top;
   XRectangle * cent;
   XRectangle * bot;

   GC      arrow_GC;	  /*  graphics context for arrow drawing   */
   XtIntervalId     timer;	
   unsigned char    multiClick;         /* KEEP/DISCARD resource */
   int              click_count;
   Time		    armTimeStamp;
   GC		    insensitive_GC; /* graphics context for insensitive arrow drawing */
} XmArrowButtonPart;


/*  Full instance record declaration  */

typedef struct _XmArrowButtonRec
{
   CorePart	   	core;
   XmPrimitivePart	primitive;
   XmArrowButtonPart    arrowbutton;
} XmArrowButtonRec;


/********    Private Function Declarations    ********/
#ifdef _NO_PROTO


#else


#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif  /* MOTIF12_HEADERS */

#endif /* _XmArrowButtonP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
