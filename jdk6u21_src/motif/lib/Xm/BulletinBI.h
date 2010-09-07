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
/* $XConsortium: BulletinBI.h /main/7 1996/06/14 23:09:13 pascale $ */
#ifndef _XmBulletinBI_h
#define _XmBulletinBI_h

#include <Xm/BulletinBP.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    XmOkStringLoc,
    XmCancelStringLoc,
    XmSelectionStringLoc,
    XmApplyStringLoc, 
    XmHelpStringLoc,
    XmFilterStringLoc,
    XmDirListStringLoc,
    XmItemsStringLoc,
    XmDirTextStringLoc,
    XmPromptStringLoc
} XmLabelStringLoc;

/********    Private Function Declarations    ********/

extern Widget _XmBB_CreateButtonG( 
                        Widget bb,
                        XmString l_string,
			char *name,
                        XmLabelStringLoc l_loc) ;
extern Widget _XmBB_CreateLabelG( 
                        Widget bb,
                        XmString l_string,
                        char *name,
                        XmLabelStringLoc l_loc) ;
extern void _XmBulletinBoardSizeUpdate( 
                        Widget wid) ;
extern void _XmBulletinBoardFocusMoved( 
                        Widget wid,
                        XtPointer client_data,
                        XtPointer data) ;
extern void _XmBulletinBoardReturn( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *numParams) ;
extern void _XmBulletinBoardCancel( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *numParams) ;
extern void _XmBulletinBoardMap( 
                         Widget wid,
                         XEvent *event,
                         String *params,
                         Cardinal *numParams) ;
extern void _XmBulletinBoardSetDynDefaultButton( 
                        Widget wid,
                        Widget newDefaultButton) ;

/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmBulletinBI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
