/* $XConsortium: IconHP.h /main/4 1995/07/15 20:52:25 drk $ */
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
#ifndef _XmIconHP_h
#define _XmIconHP_h

#include <Xm/XmP.h>
#include <Xm/IconGP.h>
#include <Xm/IconH.h>

#ifdef __cplusplus
extern "C" {
#endif

/* IconHeader class record */
typedef struct _XmIconHeaderClassPart
	{
	XtPointer extension ;
	} 	XmIconHeaderClassPart;


/* Full class record declaration */
typedef struct _XmIconHeaderClassRec
	{
	RectObjClassPart	rect_class;
	XmGadgetClassPart	gadget_class;
	XmIconGadgetClassPart	icong_class;
	XmIconHeaderClassPart	iconh_class;
	} 	XmIconHeaderClassRec;

extern	XmIconHeaderClassRec 	xmIconHeaderClassRec;

/* IconHeader instance record */
typedef struct _XmIconHeaderPart
	{
	Widget container_ID;	                /* XmNcontainerID */
	} 	XmIconHeaderPart;

/* Full instance record declaration */
typedef struct _XmIconHeaderRec
	{
	ObjectPart	object;
	RectObjPart	rectangle;
	XmGadgetPart	gadget;
	XmIconGadgetPart icong;
	XmIconHeaderPart iconh;
	} 	XmIconHeaderRec;

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmIconHP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
