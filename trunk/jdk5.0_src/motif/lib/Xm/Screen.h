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
/*   $XConsortium: Screen.h /main/9 1995/07/13 17:53:35 drk $ */
/*
*  (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#ifndef _XmScreen_h
#define _XmScreen_h

#include <Xm/Xm.h>
#ifdef __cplusplus
extern "C" {
#endif

#ifndef XmIsScreen
#define XmIsScreen(w) (XtIsSubclass(w, xmScreenClass))
#endif /* XmIsScreen */

/* Class record constants */

typedef struct _XmScreenRec *XmScreen;
typedef struct _XmScreenClassRec *XmScreenClass;
externalref 	WidgetClass xmScreenClass;


/********    Public Function Declarations    ********/

typedef void (*XmScreenColorProc) (Screen * screen, 
				   XColor *bg_color, XColor *fg_color,
				   XColor *sel_color, XColor *ts_color, 
				   XColor *bs_color);
typedef Status (*XmAllocColorProc) (Display *display,
				    Colormap colormap,
				    XColor *screen_in_out);
extern Widget XmGetXmScreen( 
                        Screen *screen) ;
/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmScreen_h */

