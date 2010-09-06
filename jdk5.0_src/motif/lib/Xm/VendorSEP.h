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
/* $XConsortium: VendorSEP.h /main/14 1996/05/21 12:11:50 pascale $ */
/* (c) Copyright 1989, 1990  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/* (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
/* (c) Copyright 1988 MICROSOFT CORPORATION */
/*
 *  (c) Copyright 1995 FUJITSU LIMITED
 *  This is source code modified by FUJITSU LIMITED under the Joint
 *  Development Agreement for the CDEnext PST.
 *  This is unpublished proprietary source code of FUJITSU LIMITED
 */
#ifndef _XmVendorSEP_h
#define _XmVendorSEP_h

#ifndef MOTIF12_HEADERS

#include <Xm/ShellEP.h>
#include <Xm/MwmUtil.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef XmIsVendorShellExt
#define XmIsVendorShellExt(w)	XtIsSubclass(w, xmVendorShellExtObjectClass)
#endif /* XmIsVendorShellExt */

typedef struct _XmVendorShellExtRec *XmVendorShellExtObject;
typedef struct _XmVendorShellExtClassRec *XmVendorShellExtObjectClass;
externalref WidgetClass xmVendorShellExtObjectClass;


#define XmInheritProtocolHandler	((XtCallbackProc)_XtInherit)

typedef struct _XmVendorShellExtClassPart{
    XtCallbackProc	delete_window_handler;
    XtCallbackProc	offset_handler;
    XtPointer		extension;
}XmVendorShellExtClassPart, *XmVendorShellExtClassPartPtr;

typedef struct _XmVendorShellExtClassRec{
    ObjectClassPart		object_class;
    XmExtClassPart		ext_class;
    XmDesktopClassPart 		desktop_class;
    XmShellExtClassPart		shell_class;
    XmVendorShellExtClassPart 	vendor_class;
}XmVendorShellExtClassRec;

typedef struct {
 XmFontList		default_font_list;
 unsigned char		focus_policy;
 XmFocusData		focus_data;
 unsigned char		delete_response;
 unsigned char		unit_type;
 MwmHints		mwm_hints;
 MwmInfo		mwm_info;
 String			mwm_menu;
 XtCallbackList		focus_moved_callback;
 /*
  * internal fields
  */
 Widget			old_managed;
 Position		xAtMap, yAtMap, xOffset, yOffset;
 unsigned long		lastOffsetSerial;
 unsigned long		lastMapRequest;
 Boolean		externalReposition;

 /* mapStyle is an unused field. I'm using this field to keep
  * track of the *font_list resource values. Refer 
  * CheckSetRenderTable in VendorSE.c
  */
 unsigned char		mapStyle;

 XtCallbackList		realize_callback;
 XtGrabKind		grab_kind;
 Boolean		audible_warning;
 XmFontList             button_font_list;
 XmFontList             label_font_list;
 XmFontList             text_font_list;
 String			input_method_string;
 String			preedit_type_string;
 unsigned int           light_threshold;
 unsigned int           dark_threshold;
 unsigned int           foreground_threshold;
 unsigned int		im_height;
 XtPointer		im_info;
 Boolean		im_vs_height_set;

 /* New public resources for Motif 2.0 */
 XmDirection            layout_direction;
 XmInputPolicy		input_policy;

 Boolean 		verify_preedit;
} XmVendorShellExtPart, *XmVendorShellExtPartPtr;

externalref XmVendorShellExtClassRec 	xmVendorShellExtClassRec;

typedef struct _XmVendorShellExtRec{
    ObjectPart			object;
    XmExtPart			ext;
    XmDesktopPart		desktop;
    XmShellExtPart		shell;
    XmVendorShellExtPart 	vendor;
} XmVendorShellExtRec;


/******** Xme Functions ********/

void XmeAddFocusChangeCallback(Widget, XtCallbackProc, XtPointer);
void XmeRemoveFocusChangeCallback(Widget, XtCallbackProc, XtPointer);

/******** End Xme Functions ********/

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
/*   $XConsortium: VendorSEP.h /main/cde1_maint/2 1995/08/18 19:33:12 drk $ */
/*
*  (c) Copyright 1989, 1990  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
*  (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
/*
*  (c) Copyright 1988 MICROSOFT CORPORATION */

#include <Xm/ShellEP.h>
#include <Xm/MwmUtil.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef XmIsVendorShellExt
#define XmIsVendorShellExt(w)	XtIsSubclass(w, xmVendorShellExtObjectClass)
#endif /* XmIsVendorShellExt */

typedef struct _XmVendorShellExtRec *XmVendorShellExtObject;
typedef struct _XmVendorShellExtClassRec *XmVendorShellExtObjectClass;
externalref WidgetClass xmVendorShellExtObjectClass;


#define XmInheritProtocolHandler	((XtCallbackProc)_XtInherit)

typedef struct _XmVendorShellExtClassPart{
    XtCallbackProc	delete_window_handler;
    XtCallbackProc	offset_handler;
    XtPointer		extension;
}XmVendorShellExtClassPart, *XmVendorShellExtClassPartPtr;

typedef struct _XmVendorShellExtClassRec{
    ObjectClassPart		object_class;
    XmExtClassPart		ext_class;
    XmDesktopClassPart 		desktop_class;
    XmShellExtClassPart		shell_class;
    XmVendorShellExtClassPart 	vendor_class;
}XmVendorShellExtClassRec;

typedef struct {
 XmFontList		default_font_list;
 unsigned char		focus_policy;
 XmFocusData		focus_data;
 unsigned char		delete_response;
 unsigned char		unit_type;
 MwmHints		mwm_hints;
 MwmInfo		mwm_info;
 String			mwm_menu;
 XtCallbackList		focus_moved_callback;
 /*
  * internal fields
  */
 Widget			old_managed;
 Position		xAtMap, yAtMap, xOffset, yOffset;
 unsigned long		lastOffsetSerial;
 unsigned long		lastMapRequest;
 Boolean		externalReposition;
 unsigned char		mapStyle;
 XtCallbackList		realize_callback;
 XtGrabKind		grab_kind;
 Boolean		audible_warning;
 XmFontList             button_font_list;
 XmFontList             label_font_list;
 XmFontList             text_font_list;
 String			input_method_string;
 String			preedit_type_string;
 unsigned int           light_threshold;
 unsigned int           dark_threshold;
 unsigned int           foreground_threshold;
 unsigned int		im_height;
 XtPointer		im_info;
 Boolean		im_vs_height_set;
} XmVendorShellExtPart, *XmVendorShellExtPartPtr;

externalref XmVendorShellExtClassRec 	xmVendorShellExtClassRec;

typedef struct _XmVendorShellExtRec{
    ObjectPart			object;
    XmExtPart			ext;
    XmDesktopPart		desktop;
    XmShellExtPart		shell;
    XmVendorShellExtPart 	vendor;
}XmVendorShellExtRec;


/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern unsigned char _XmGetAudibleWarning() ;
extern char * _XmGetIconPixmapName() ;
extern void   _XmClearIconPixmapName() ;
#else

extern unsigned char _XmGetAudibleWarning( 
                        Widget w) ;
extern char * _XmGetIconPixmapName( void ) ;
extern void   _XmClearIconPixmapName( void ) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* MOTIF12_HEADERS */

#endif  /* _XmVendorSEP_h */
