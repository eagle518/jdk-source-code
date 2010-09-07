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
/* $XConsortium: DragIconP.h /main/11 1995/07/14 10:25:42 drk $ */
/* (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#ifndef _XmDragIconP_h
#define _XmDragIconP_h

#include <Xm/VendorSEP.h>
#include <Xm/DragIcon.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef void (*XmCloneVisualProc) (XmDragIconObject, Widget, Widget);
typedef void (*XmMovePixmapProc)  (XmDragIconObject, 
				   XmDragIconObject, 
				   XmDragIconObject,
#if NeedWidePrototypes
				   int, int);
#else
				   Position, Position);
#endif /* NeedWidePrototypes */

typedef struct {
  XtPointer		extension;
} XmDragIconClassPart;

typedef struct _XmDragIconClassRec{
  RectObjClassPart		rectangle_class;
  XmDragIconClassPart		dragIcon_class;
} XmDragIconClassRec;

typedef struct {
  Cardinal	depth;
  Pixmap	pixmap;
  Dimension	width, height;
  Pixmap	mask;
  Position	hot_x, hot_y;
  Position	offset_x, offset_y;
  unsigned char	attachment;
  Boolean	isDirty;
  Region        region;
  Region        restore_region;
  Position	x_offset, y_offset;
} XmDragIconPart, *XmDragIconPartPtr;

externalref XmDragIconClassRec 	xmDragIconClassRec;

typedef struct _XmDragIconRec{
  ObjectPart		object;
  RectObjPart		rectangle;
  XmDragIconPart	drag;
} XmDragIconRec;

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDragIconP_h */
