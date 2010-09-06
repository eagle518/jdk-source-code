/* $XConsortium: AccColorT.h /main/5 1995/07/15 20:47:59 drk $ */
/*
 * COPYRIGHT NOTICE
 * Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 * ALL RIGHTS RESERVED (MOTIF).  See the file named COPYRIGHT.MOTIF
 * for the full copyright text.
 * 
 */
/*
 * HISTORY
 */

#ifndef _XmAccessColorsT_H
#define _XmAccessColorsT_H

#include <Xm/Xm.h>
#include <X11/Xresource.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref XrmQuark XmQTaccessColors;

/* this one can be expanded in the future */
typedef struct _XmAccessColorDataRec {
    Mask  valueMask ;
    Pixel foreground  ;
    Pixel background  ;
    Pixel highlight_color  ;
    Pixel top_shadow_color  ;
    Pixel bottom_shadow_color ;
    Pixel select_color ;
} XmAccessColorDataRec, *XmAccessColorData;

typedef void (*XmAccessColorsGetProc)(Widget widget, 
				      XmAccessColorData color_data);
typedef void (*XmAccessColorsSetProc)(Widget widget, 
				      XmAccessColorData color_data);

/* Trait structures and typedefs, place typedefs first */

/* Version 0: initial release. */

typedef struct _XmAccessColorsTraitRec {
  int			version;	/* 0 */
  XmAccessColorsGetProc getColors;
  XmAccessColorsGetProc setColors;
} XmAccessColorsTraitRec, *XmAccessColorsTrait;

#define AccessColorInvalid         0L
#define AccessForeground           (1L<<0)  
#define AccessBackgroundPixel      (1L<<1)   
#define AccessHighlightColor       (1L<<2)   
#define AccessTopShadowColor       (1L<<3)   
#define AccessBottomShadowColor    (1L<<4)   
#define AccessSelectColor          (1L<<5)   

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

/* to do:

 add it to PushB/G and ToggleB/G so that they can report their
   select color
 implement the setValues ?

*/

#endif /* _XmAccessColorsT_H */
