/* $XConsortium: IconG.h /main/5 1995/07/15 20:52:04 drk $ */
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
#ifndef _XmIconG_h
#define _XmIconG_h

#ifndef MOTIF12_HEADERS

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif



/* Class record constants */
extern	WidgetClass	xmIconGadgetClass;

typedef struct _XmIconGadgetClassRec * XmIconGadgetClass;
typedef struct _XmIconGadgetRec      * XmIconGadget;

#ifndef XmIsIconGadget
#define XmIsIconGadget(w) XtIsSubclass(w, xmIconGadgetClass)
#endif /* XmIsIconGadget */

/********    Public Function Declarations    ********/
extern Widget XmCreateIconGadget(
                        Widget parent,
                        String name,
                        ArgList arglist,
                        Cardinal argcount) ;

/********    End Public Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#else /* MOTIF12_HEADERS */


/* $XConsortium: IconG.h /main/cde1_maint/2 1995/08/18 19:07:18 drk $ */
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

#include <Xm/Xm.h>

#define XmNborderType "borderType"
#define XmCBorderType "BorderType"
#define XmRBorderType "BorderType"
#define XmNimageName "imageName"
#define XmNpixmapForeground "pixmapForeground"
#define XmNpixmapBackground "pixmapBackground"
#define XmNunderline "underline"
#define XmCUnderline "Underline"
#define XmNbehavior "behavior"
#define XmCBehavior "Behavior"
#define XmRBehavior "Behavior"
#define XmNpixmapPosition "pixmapPosition"
#define XmCPixmapPosition "PixmapPosition"
#define XmRPixmapPosition "PixmapPosition"
#define XmNstringPosition "stringPosition"
#define XmCStringPosition "StringPosition"
#define XmRStringPosition "StringPosition"
#define XmNfillMode "fillMode"
#define XmCFillMode "FillMode"
#define XmRFillMode "FillMode"


#ifndef XmIsIconGadget
#define XmIsIconGadget(w) XtIsSubclass(w, xmIconGadgetClass)
#endif /* XmIsIconGadget */

typedef struct _XmIconGadgetClassRec * XmIconGadgetClass;
typedef struct _XmIconGadgetRec      * XmIconGadget;
typedef struct _XmIconGCacheObjRec   * XmIconCacheObject;


extern WidgetClass xmIconGadgetClass;


#define XmCR_SELECT	XmCR_SINGLE_SELECT
#define XmCR_DROP	50
#define XmCR_POPUP	51
#define XmCR_HIGHLIGHT	52
#define XmCR_UNHIGHLIGHT	53
#define XmCR_SHADOW	54

typedef struct
{
	int		reason;
	XEvent *	event;
	Boolean		set;
} XmIconGadgetCallbackStruct;


enum { XmPIXMAP_TOP, XmPIXMAP_BOTTOM, XmPIXMAP_LEFT, XmPIXMAP_RIGHT } ;

enum { XmSTRING_BOTTOM, XmSTRING_TOP, XmSTRING_RIGHT, XmSTRING_LEFT } ;

enum { XmICON_LABEL, XmICON_BUTTON, XmICON_TOGGLE, XmICON_DRAG } ;

enum { XmFILL_NONE, XmFILL_PARENT, XmFILL_SELF, XmFILL_TRANSPARENT } ;

enum { XmRECTANGLE, XmNON_RECTANGLE } ;

#define XmPIXMAP_RECT  0x01
#define XmLABEL_RECT   0x02


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Widget XmCreateIconGadget() ;

#else

extern Widget XmCreateIconGadget( 
                        Widget parent,
                        String name,
                        ArgList arglist,
                        Cardinal argcount) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/

#endif /* MOTIF12_HEADERS */

#endif /* _XmIconG_h */

/* DON'T ADD ANYTHING AFTER THIS #endif */
