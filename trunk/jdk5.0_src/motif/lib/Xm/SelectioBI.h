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
/* $XConsortium: SelectioBI.h /main/5 1995/07/13 17:57:50 drk $ */
#ifndef _XmSelectioBI_h
#define _XmSelectioBI_h

#include <Xm/SelectioBP.h>

#ifdef __cplusplus
extern "C" {
#endif


/********    Private Function Declarations    ********/

extern void _XmSelectionBoxCreateListLabel( 
                        XmSelectionBoxWidget sel) ;
extern void _XmSelectionBoxCreateSelectionLabel( 
                        XmSelectionBoxWidget sel) ;
extern void _XmSelectionBoxCreateList( 
                        XmSelectionBoxWidget sel) ;
extern void _XmSelectionBoxCreateText( 
                        XmSelectionBoxWidget sel) ;
extern void _XmSelectionBoxCreateSeparator( 
                        XmSelectionBoxWidget sel) ;
extern void _XmSelectionBoxCreateOkButton( 
                        XmSelectionBoxWidget sel) ;
extern void _XmSelectionBoxCreateApplyButton( 
                        XmSelectionBoxWidget sel) ;
extern void _XmSelectionBoxCreateCancelButton( 
                        XmSelectionBoxWidget sel) ;
extern void _XmSelectionBoxCreateHelpButton( 
                        XmSelectionBoxWidget sel) ;
extern XmGeoMatrix _XmSelectionBoxGeoMatrixCreate( 
                        Widget wid,
                        Widget instigator,
                        XtWidgetGeometry *desired) ;
extern Boolean _XmSelectionBoxNoGeoRequest( 
                        XmGeoMatrix geoSpec) ;
extern void _XmSelectionBoxGetSelectionLabelString( 
                        Widget wid,
                        int resource_offset,
                        XtArgVal *value) ;
extern void _XmSelectionBoxGetListLabelString( 
                        Widget wid,
                        int resource_offset,
                        XtArgVal *value) ;
extern void _XmSelectionBoxGetTextColumns( 
                        Widget wid,
                        int resource_offset,
                        XtArgVal *value) ;
extern void _XmSelectionBoxGetTextString( 
                        Widget wid,
                        int resource_offset,
                        XtArgVal *value) ;
extern void _XmSelectionBoxGetListItems( 
                        Widget wid,
                        int resource_offset,
                        XtArgVal *value) ;
extern void _XmSelectionBoxGetListItemCount( 
                        Widget wid,
                        int resource_offset,
                        XtArgVal *value) ;
extern void _XmSelectionBoxGetListVisibleItemCount( 
                        Widget wid,
                        int resource_offset,
                        XtArgVal *value) ;
extern void _XmSelectionBoxGetOkLabelString( 
                        Widget wid,
                        int resource_offset,
                        XtArgVal *value) ;
extern void _XmSelectionBoxGetApplyLabelString( 
                        Widget wid,
                        int resource_offset,
                        XtArgVal *value) ;
extern void _XmSelectionBoxGetCancelLabelString( 
                        Widget wid,
                        int resource_offset,
                        XtArgVal *value) ;
extern void _XmSelectionBoxGetHelpLabelString( 
                        Widget wid,
                        int resource_offset,
                        XtArgVal *value) ;
extern void _XmSelectionBoxUpOrDown( 
                        Widget wid,
                        XEvent *event,
                        String *argv,
                        Cardinal *argc) ;
extern void _XmSelectionBoxRestore( 
                        Widget wid,
                        XEvent *event,
                        String *argv,
                        Cardinal *argc) ;

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmSelectioBI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
