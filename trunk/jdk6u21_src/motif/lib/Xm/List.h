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
/*   $XConsortium: List.h /main/12 1995/07/13 17:33:36 drk $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmList_h
#define _XmList_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref WidgetClass xmListWidgetClass;

#define XmINITIAL 	0
#define XmADDITION	1
#define XmMODIFICATION	2

#ifndef XmIsList
#define XmIsList(w)	XtIsSubclass(w, xmListWidgetClass)
#endif /* XmIsList */

typedef struct _XmListClassRec * XmListWidgetClass;
typedef struct _XmListRec      * XmListWidget;

/********    Public Function Declarations    ********/

extern void XmListAddItem( 
                        Widget w,
                        XmString item,
                        int pos) ;
extern void XmListAddItems( 
                        Widget w,
                        XmString *items,
                        int item_count,
                        int pos) ;
extern void XmListAddItemsUnselected( 
                        Widget w,
                        XmString *items,
                        int item_count,
                        int pos) ;
extern void XmListAddItemUnselected( 
                        Widget w,
                        XmString item,
                        int pos) ;
extern void XmListDeleteItem( 
                        Widget w,
                        XmString item) ;
extern void XmListDeleteItems( 
                        Widget w,
                        XmString *items,
                        int item_count) ;
extern void XmListDeletePositions(
                        Widget    w,
                        int      *position_list,
                        int       position_count ) ;
extern void XmListDeletePos( 
                        Widget w,
                        int pos) ;
extern void XmListDeleteItemsPos( 
                        Widget w,
                        int item_count,
                        int pos) ;
extern void XmListDeleteAllItems( 
                        Widget w) ;
extern void XmListReplaceItems( 
                        Widget w,
                        XmString *old_items,
                        int item_count,
                        XmString *new_items) ;
extern void XmListReplaceItemsPos( 
                        Widget w,
                        XmString *new_items,
                        int item_count,
                        int position) ;
extern void XmListReplaceItemsUnselected( 
                        Widget w,
                        XmString *old_items,
                        int item_count,
                        XmString *new_items) ;
extern void XmListReplaceItemsPosUnselected( 
                        Widget w,
                        XmString *new_items,
                        int item_count,
                        int position) ;
extern void XmListReplacePositions(
                        Widget    w,
                        int      *position_list,
                        XmString *item_list,
                        int       item_count ) ;
extern void XmListSelectItem( 
                        Widget w,
                        XmString item,
#if NeedWidePrototypes
                        int notify) ;
#else
                        Boolean notify) ;
#endif /* NeedWidePrototypes */
extern void XmListSelectPos( 
                        Widget w,
                        int pos,
#if NeedWidePrototypes
                        int notify) ;
#else
                        Boolean notify) ;
#endif /* NeedWidePrototypes */
extern void XmListDeselectItem( 
                        Widget w,
                        XmString item) ;
extern void XmListDeselectPos( 
                        Widget w,
                        int pos) ;
extern void XmListDeselectAllItems( 
                        Widget w) ;
extern void XmListSetPos( 
                        Widget w,
                        int pos) ;
extern void XmListSetBottomPos( 
                        Widget w,
                        int pos) ;
extern void XmListSetItem( 
                        Widget w,
                        XmString item) ;
extern void XmListSetBottomItem( 
                        Widget w,
                        XmString item) ;
extern void XmListSetAddMode( 
                        Widget w,
#if NeedWidePrototypes
                        int add_mode) ;
#else
                        Boolean add_mode) ;
#endif /* NeedWidePrototypes */
extern Boolean XmListItemExists( 
                        Widget w,
                        XmString item) ;
extern int XmListItemPos( 
                        Widget w,
                        XmString item) ;
extern int XmListGetKbdItemPos(
                        Widget w) ;
extern Boolean XmListSetKbdItemPos(
                        Widget w,
                        int    pos ) ;
extern int XmListYToPos( 
                        Widget w,
                        Position y) ; /* NeedWidePrototypes ????? */
extern Boolean XmListPosToBounds(
                        Widget w,
                        int         position,
                        Position   *x,
                        Position   *y,
                        Dimension  *width,
                        Dimension  *height) ;
extern Boolean XmListGetMatchPos( 
                        Widget w,
                        XmString item,
                        int **pos_list,
                        int *pos_count) ;
extern Boolean XmListGetSelectedPos( 
                        Widget w,
                        int **pos_list,
                        int *pos_count) ;
extern void XmListSetHorizPos( 
                        Widget w,
                        int position) ;
extern void XmListUpdateSelectedList( 
                        Widget w) ;
extern Boolean XmListPosSelected(
			Widget 	w,
			int 	pos);
extern Widget XmCreateList( 
                        Widget parent,
                        char *name,
                        ArgList args,
                        Cardinal argCount) ;
extern Widget XmCreateScrolledList( 
                        Widget parent,
                        char *name,
                        ArgList args,
                        Cardinal argCount) ;

/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmList_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
