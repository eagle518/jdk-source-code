/* $XConsortium: CareVisualT.h /main/5 1995/07/15 20:48:21 drk $ */
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
#ifndef _XmCareVisualT_H
#define _XmCareVisualT_H

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref XrmQuark XmQTcareParentVisual;

/* Trait structures and typedefs, place typedefs first */

typedef Boolean (*XmCareVisualRedrawProc)(Widget kid, 
					  Widget cur_parent,
					  Widget new_parent,
					  Mask visual_flag);

/* Version 0: initial release. */

typedef struct _XmCareVisualTraitRec {
  int 			 version;	/* 0 */
  XmCareVisualRedrawProc redraw;
} XmCareVisualTraitRec, *XmCareVisualTrait;


#define NoVisualChange                    0L
#define VisualForeground                  (1L<<0)  
#define VisualHighlightPixmap             (1L<<1)                              
#define VisualHighlightColor              (1L<<2)   
#define VisualBottomShadowPixmap          (1L<<3)   
#define VisualBottomShadowColor           (1L<<4)   
#define VisualTopShadowPixmap             (1L<<5)   
#define VisualTopShadowColor              (1L<<6)   
#define VisualBackgroundPixel             (1L<<7)   
#define VisualBackgroundPixmap            (1L<<8)   
#define VisualSelectColor                 (1L<<9)   


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmCareVisualT_H */
