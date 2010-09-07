/* $XConsortium: Notebook.h /main/5 1995/07/15 20:53:41 drk $ */
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

#ifndef _XmNotebook_h
#define _XmNotebook_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref WidgetClass xmNotebookWidgetClass;

typedef struct _XmNotebookClassRec *XmNotebookWidgetClass;
typedef struct _XmNotebookRec *XmNotebookWidget;


/************************************************************************
 *  Notebook Defines
 ************************************************************************/

/* XmNotebookPageStatus */
typedef enum
{
    XmPAGE_FOUND,		/* page widget found */
    XmPAGE_INVALID,		/* page number out of the range */
    XmPAGE_EMPTY,		/* no page widget found */
    XmPAGE_DUPLICATED		/* there are more than one page widgets */
} XmNotebookPageStatus;

/* Notebook page information structure */
typedef struct
{
    int         page_number;
    Widget      page_widget;
    Widget	status_area_widget;
    Widget      major_tab_widget;
    Widget	minor_tab_widget;
} XmNotebookPageInfo;


/************************************************************************
 *  Public Functions
 ************************************************************************/

#ifndef XmIsNotebook
#define XmIsNotebook(w) XtIsSubclass((w), xmNotebookWidgetClass)
#endif

extern Widget XmCreateNotebook(
			Widget		parent,
			String		name,
			ArgList		arglist,
			Cardinal	argcount);

extern XmNotebookPageStatus XmNotebookGetPageInfo(
			Widget notebook,
			int page_number,
			XmNotebookPageInfo *page_info) ;


#ifdef __cplusplus
}
#endif

#endif /* _XmNotebook_h  */

