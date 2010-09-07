/* $XConsortium: Container.h /main/5 1995/07/15 20:50:08 drk $ */
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
#ifndef	_XmContainer_h
#define _XmContainer_h

#include <Xm/Xm.h>
#include <Xm/Manager.h>

#ifdef __cplusplus
extern "C" {
#endif



/* Class record constants */
externalref	WidgetClass	xmContainerWidgetClass;

typedef	struct	_XmContainerClassRec	*XmContainerWidgetClass;
typedef struct  _XmContainerRec	*XmContainerWidget;

#ifndef XmIsContainer
#define XmIsContainer(w) XtIsSubclass(w, xmContainerWidgetClass)
#endif /* XmIsContainer */


/********    Public Function Declarations    ********/
extern	Widget	XmCreateContainer(
			Widget parent,
			String name,
			ArgList arglist,
			Cardinal argcount);
extern	int	XmContainerGetItemChildren(
			Widget		wid,
			Widget		item,
			WidgetList	*item_children);
extern	void	XmContainerRelayout(
			Widget	wid);
extern	void	XmContainerReorder(
			Widget		wid,
			WidgetList	cwid_list,
			int		cwid_count);	
extern  Boolean XmContainerCut(
                        Widget  wid,
                        Time    timestamp);
extern  Boolean XmContainerCopy(
                        Widget  wid,
                        Time    timestamp);
extern  Boolean XmContainerPaste(
                        Widget  wid);
extern  Boolean XmContainerCopyLink(
                        Widget  wid,
                        Time    timestamp);
extern  Boolean XmContainerPasteLink(
                        Widget  wid);
/********    End Public Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmContainer_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
