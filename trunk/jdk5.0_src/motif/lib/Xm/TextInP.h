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
/* $XConsortium: TextInP.h /main/13 1995/07/13 18:07:54 drk $ */
/* (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmTextInP_h
#define _XmTextInP_h

#ifndef MOTIF12_HEADERS

#include <Xm/Text.h>
#include <Xm/TextStrSoP.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Definitions for modules implementing text input modules.
 *
 ****************************************************************/

typedef struct {
    int x;
    int y;
} SelectionHint;

typedef struct _InputDataRec {
    XmTextWidget widget;		/* Back-pointer to widget record. */
    XmTextScanType *sarray;	/* Description of what to cycle through on */
				/* selections. */
    int sarraycount;		/* Number of elements in above. */
    int new_sel_length;		/* New selection length for selection moves. */
    int threshold;		/* number of pixels crossed -> drag */
    SelectionHint selectionHint; /* saved coords of button down */
    SelectionHint Sel2Hint;	/* saved the coords of button down */
    XtIntervalId select_id;
    XmTextScanType stype;	/* Current selection type. */
    XmTextScanDirection extendDir;
    XmTextScanDirection Sel2ExtendDir;
    XmTextPosition origLeft, origRight;
    XmTextPosition Sel2OrigLeft, Sel2OrigRight;
    XmTextPosition stuffpos;
    XmTextPosition sel2Left, sel2Right; /* secondary selection */
    XmTextPosition anchor;	/* anchor point of the primary selection */
    Position select_pos_x;	/* x position for timer-based scrolling */
    Position select_pos_y;	/* y position for timer-based scrolling */
    Boolean pendingdelete;	/* TRUE if we're implementing pending delete */
    Boolean syncing;		/* If TRUE, then we've multiple keystrokes */
    Boolean extending;      /* true if we are extending */
    Boolean Sel2Extending;	/* true if we are extending */
    Boolean hasSel2;   		/* has secondary selection */
    Boolean has_destination;  	/* has destination selection */
    Boolean selectionMove;	/* delete selection after stuff */
    Boolean cancel;		/* indicates that cancel was pressed */
    Boolean overstrike;     	/* overstrike */
    Boolean sel_start;		/* indicates that a btn2 was pressed */
    Time dest_time;		/* time of destination selection ownership */
    Time sec_time;		/* time of secondary selection ownership */
    Time lasttime;		/* Time of last event. */
    Boolean selectionLink;	/* This is a link vs. a copy operation */
    XtIntervalId drag_id;       /* timer to start btn1 drag */
    _XmTextActionRec *transfer_action;  /* to keep track of delayed action */
#ifdef SUN_CTL
    unsigned char edit_policy;  /* Resourced as XmNeditPolicy */
#endif /* CTL */
} InputDataRec, *InputData;

/* 
 * Create a new instance of an input object.  By the time this is called,
 * the widget context has been saved.
 */

typedef void (*InputCreateProc)(
			Widget,
			ArgList,
			Cardinal) ;

/*
 * Get values out of the input object.
 */
typedef void (*InputGetValuesProc)(
			Widget,
			ArgList,
			Cardinal) ;

/*
 * Set values in the input object.
 */

typedef void (*InputSetValuesProc)(
			Widget,
			Widget,
			Widget,
			ArgList,
			Cardinal *) ;

/*
 * Inform input of invalidated positions.
 */
typedef void (*InputInvalidateProc)(
			XmTextWidget,
			XmTextPosition,
			XmTextPosition,
			long) ;

/*
 * Get secondary resources.
 */
typedef void (*InputGetSecResProc)(
			XmSecondaryResourceData *) ;


typedef struct _InputRec {
    struct _InputDataRec *data;	/* Input-specific data; opaque type. */
    InputInvalidateProc Invalidate;
    InputGetValuesProc  GetValues;
    InputSetValuesProc	SetValues;
    XtWidgetProc	destroy;
    InputGetSecResProc  GetSecResData;
} InputRec;


externalref XtPointer _XmdefaultTextActionsTable;
externalref Cardinal  _XmdefaultTextActionsTableSize;


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#else /* MOTIF12_HEADERS */


/* 
 * @OSF_COPYRIGHT@
 * (c) Copyright 1990, 1991, 1992, 1993, 1994 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *  
*/ 
/*
 * HISTORY
 * Motif Release 1.2.5
*/
/*   $XConsortium: TextInP.h /main/cde1_maint/2 1995/08/18 19:27:38 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#include <Xm/Text.h>
#include <Xm/TextStrSoP.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Definitions for modules implementing text input modules.
 *
 ****************************************************************/

typedef struct {
    int x;
    int y;
} SelectionHint;

typedef struct _InputDataRec {
    XmTextWidget widget;		/* Back-pointer to widget record. */
    XmTextScanType *sarray;	/* Description of what to cycle through on */
				/* selections. */
    int sarraycount;		/* Number of elements in above. */
    int new_sel_length;		/* New selection length for selection moves. */
    int threshold;		/* number of pixels crossed -> drag */
    SelectionHint selectionHint; /* saved coords of button down */
    SelectionHint Sel2Hint;	/* saved the coords of button down */
    XtIntervalId select_id;
    XmTextScanType stype;	/* Current selection type. */
    XmTextScanDirection extendDir;
    XmTextScanDirection Sel2ExtendDir;
    XmTextPosition origLeft, origRight;
    XmTextPosition Sel2OrigLeft, Sel2OrigRight;
    XmTextPosition stuffpos;
    XmTextPosition sel2Left, sel2Right; /* secondary selection */
    XmTextPosition anchor;	/* anchor point of the primary selection */
    Position select_pos_x;	/* x position for timer-based scrolling */
    Position select_pos_y;	/* y position for timer-based scrolling */
    Boolean pendingdelete;	/* TRUE if we're implementing pending delete */
    Boolean syncing;		/* If TRUE, then we've multiple keystrokes */
    Boolean extending;      /* true if we are extending */
    Boolean Sel2Extending;	/* true if we are extending */
    Boolean hasSel2;   		/* has secondary selection */
    Boolean has_destination;  	/* has destination selection */
    Boolean selectionMove;	/* delete selection after stuff */
    Boolean cancel;		/* indicates that cancel was pressed */
    Boolean overstrike;     	/* overstrike */
    Boolean sel_start;		/* indicates that a btn2 was pressed */
    Time dest_time;		/* time of destination selection ownership */
    Time sec_time;		/* time of secondary selection ownership */
    Time lasttime;		/* Time of last event. */
} InputDataRec, *InputData;


/* 
 * Create a new instance of an input object.  By the time this is called,
 * the widget context has been saved.
 */

#ifdef _NO_PROTO
typedef void (*InputCreateProc)(); /* widget, args, num_args */
#else
typedef void (*InputCreateProc)(
			Widget,
			ArgList,
			Cardinal) ;
#endif

/*
 * Get values out of the input object.
 */
#ifdef _NO_PROTO
typedef void (*InputGetValuesProc)(); /* widget, args, num_args */
#else
typedef void (*InputGetValuesProc)(
			Widget,
			ArgList,
			Cardinal) ;
#endif

/*
 * Set values in the input object.
 */

#ifdef _NO_PROTO
typedef void (*InputSetValuesProc)(); /* oldw, reqw, new_w, args, num_args */
#else
typedef void (*InputSetValuesProc)(
			Widget,
			Widget,
			Widget,
			ArgList,
			Cardinal *) ;
#endif

/*
 * Inform input of invalidated positions.
 */
#ifdef _NO_PROTO
typedef void (*InputInvalidateProc)(); /* ctx, position, topos, delta */
#else
typedef void (*InputInvalidateProc)(
			XmTextWidget,
			XmTextPosition,
			XmTextPosition,
			long) ;
#endif

/*
 * Get secondary resources.
 */
#ifdef _NO_PROTO
typedef void (*InputGetSecResProc)(); /* secResDataRtn */
#else
typedef void (*InputGetSecResProc)(
			XmSecondaryResourceData *) ;
#endif


typedef struct _InputRec {
    struct _InputDataRec *data;	/* Input-specific data; opaque type. */
    InputInvalidateProc Invalidate;
    InputGetValuesProc  GetValues;
    InputSetValuesProc	SetValues;
    XtWidgetProc	destroy;
    InputGetSecResProc  GetSecResData;
} InputRec;


externalref XtPointer _XmdefaultTextActionsTable;
externalref Cardinal  _XmdefaultTextActionsTableSize;



/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern Widget _XmTextGetDropReciever() ;
extern Boolean _XmTextHasDestination() ;
extern Boolean _XmTextSetDestinationSelection() ;
extern Boolean _XmTextSetSel2() ;
extern Boolean _XmTextGetSel2() ;
extern void _XmTextInputGetSecResData() ;
extern XmTextPosition _XmTextGetAnchor() ;
extern void _XmTextInputCreate() ;

#else

extern Widget _XmTextGetDropReciever( 
                        Widget w) ;
extern Boolean _XmTextHasDestination( 
                        Widget w) ;
extern Boolean _XmTextSetDestinationSelection( 
                        Widget w,
                        XmTextPosition position,
#if NeedWidePrototypes
                        int disown,
#else
                        Boolean disown,
#endif /* NeedWidePrototypes */
                        Time set_time) ;
extern Boolean _XmTextSetSel2( 
                        XmTextWidget tw,
                        XmTextPosition left,
                        XmTextPosition right,
                        Time set_time) ;
extern Boolean _XmTextGetSel2( 
                        XmTextWidget tw,
                        XmTextPosition *left,
                        XmTextPosition *right) ;
extern void _XmTextInputGetSecResData( 
                        XmSecondaryResourceData *secResDataRtn) ;
extern XmTextPosition _XmTextGetAnchor( 
                        XmTextWidget tw) ;
extern void _XmTextInputCreate( 
                        Widget wid,
                        ArgList args,
                        Cardinal num_args) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* MOTIF12_HEADERS */

#endif /* _XmTextInP_h */
/*DON'T ADD ANYTHING AFTER THIS #endif */
