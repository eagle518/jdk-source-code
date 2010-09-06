/* $XConsortium: ScrollFrameT.h /main/5 1995/07/15 20:55:24 drk $ */
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
#ifndef _XmScrollFrameT_H
#define _XmScrollFrameT_H

#include <Xm/Xm.h>
#include <Xm/NavigatorT.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref XrmQuark XmQTscrollFrame;

/* Trait structures and typedefs, place typedefs first */

typedef void (*XmScrollFrameInitProc)(Widget sf, 
				      XtCallbackProc moveCB,
				      Widget scrollable);
typedef Boolean  (*XmScrollFrameGetInfoProc)(Widget sf,
					     Cardinal * dimension,
					     Widget ** nav_list,
					     Cardinal * num_nav_list);
typedef void (*XmScrollFrameAddNavigatorProc)(Widget sf, 
					      Widget nav,
					      Mask dimMask);
typedef void (*XmScrollFrameRemoveNavigatorProc)(Widget sf, 
						 Widget nav);


/* Version 0: initial release. */

typedef struct _XmScrollFrameTraitRec {
  int				    version;		/* 0 */
  XmScrollFrameInitProc		    init;       
  XmScrollFrameGetInfoProc	    getInfo; 
  XmScrollFrameAddNavigatorProc     addNavigator;
  XmScrollFrameRemoveNavigatorProc  removeNavigator;
} XmScrollFrameTraitRec, *XmScrollFrameTrait;


/* This one gets allocated per instance by the scrollFrame
   class. It is just a convenient structure reusable by other scrollFrame
   and it needs not to be part of the public trait API */

typedef struct _XmScrollFrameDataRec {
   XtCallbackProc move_cb ;
   Widget         scrollable ;
   Widget *       nav_list;
   Cardinal       num_nav_list ;
   Cardinal       num_nav_slots;
} XmScrollFrameDataRec, *XmScrollFrameData;


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmScrollFrameT_H */
