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
/*   $XConsortium: PanedWP.h /main/12 1995/07/13 17:40:37 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
*  (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
/*********************************************************************
 *
 * XmPanedWindowWidget Private Data
 *
 *********************************************************************/

#ifndef _XmPanedWP_h
#define _XmPanedWP_h

#ifndef MOTIF12_HEADERS

#include <Xm/PanedW.h>
#include <Xm/ManagerP.h>

#ifdef __cplusplus
extern "C" {
#endif

/* New fields for the PanedWindow widget class record */

typedef struct _XmPanedWindowClassPart
{
    XtPointer extension;
} XmPanedWindowClassPart;


/* Full Class record declaration */

typedef struct _XmPanedWindowClassRec {
    CoreClassPart       core_class;
    CompositeClassPart  composite_class;
    ConstraintClassPart constraint_class;
    XmManagerClassPart  manager_class;
    XmPanedWindowClassPart     paned_window_class;
} XmPanedWindowClassRec;

externalref XmPanedWindowClassRec xmPanedWindowClassRec;


/* PanedWindow constraint record */

typedef struct _XmPanedWindowConstraintPart {
    int		position;	/* position location in PanedWindow */
    int		dheight;	/* Desired size */
    Position	dy;		/* Desired Location */
    Position	olddy;		/* The last value of dy. */
    Dimension	min;		/* Minimum height */
    Dimension	max;		/* Maximum height */
    Boolean     isPane;         /* true if constraint of pane, false if
				   constraint of sash */
    Boolean	allow_resize;	/* TRUE iff child resize requests are ok */
    Boolean	skip_adjust;	/* TRUE iff child's height should not be */
				/* changed without explicit user action. */
    Widget	sash;		/* The sash for this child */
    Widget	separator;	/* The separator for this child */
    short       position_index; /* new 1.2 positionIndex resource */
} XmPanedWindowConstraintPart;

typedef struct _XmPanedWindowConstraintRec 
{
    XmManagerConstraintPart manager;
    XmPanedWindowConstraintPart  panedw;
} XmPanedWindowConstraintRec, * XmPanedWindowConstraintPtr;


/* New Fields for the PanedWindow widget record */

typedef struct {
    /* resources */
    Boolean     refiguremode;        /* Whether to refigure changes right now */
    Boolean	separator_on;	     /* make separator visible */

    Dimension  	margin_width;	     /* space between right and left edges of
					PanedWindow window and it's children */
    Dimension  	margin_height;	     /* space between top and bottom edges of
					PanedWindow window and it's children */
    Dimension   spacing;             /* whitespace between panes
				        around window, else leave none */
    /* sash modifying resources */
    Dimension	sash_width;	       /* Modify sash width */
    Dimension	sash_height;	       /* Modify sash height */
    Dimension   sash_shadow_thickness; /* Modify sash shadow_thickness */

    Position    sash_indent;           /* Location of sashs (offset
                                          from right margin)	*/
    /* private */
    int         starty;               /* mouse origin when adjusting */

    short	increment_count;      /* Sash increment count */
    short       pane_count;           /* number of managed panes */
    short       num_slots;	      /* number of avail. slots for children*/
    short       num_managed_children; /* holds number of managed children */

    Boolean     recursively_called;    /* For change_managed AND creation of
					* private sash and separator
					* children
					*/
    Boolean     resize_at_realize;    /* For realize if GeometryNo condition */

    XmPanedWindowConstraintPtr top_pane;    /* pane closest to 0 index */
    XmPanedWindowConstraintPtr bottom_pane; /* pane farthest away from 0 index*/

    GC          flipgc;               /* GC to use when animating borders */
    WidgetList  managed_children;     /* keep track of managed children */

    unsigned char orientation ;     /* horizontal or vertical panedw */
    
    XtIntervalId timer ;            /* handle key event on sash */
} XmPanedWindowPart;

/**************************************************************************
 *
 * Full instance record declaration
 *
 **************************************************************************/

typedef struct _XmPanedWindowRec {
    CorePart       core;
    CompositePart  composite;
    ConstraintPart constraint;
    XmManagerPart  manager;
    XmPanedWindowPart   paned_window;
} XmPanedWindowRec;


/********    Private Function Declarations    ********/


/********    End Private Function Declarations    ********/


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
/*   $XConsortium: PanedWP.h /main/cde1_maint/2 1995/08/18 19:14:34 drk $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
*  (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
/*********************************************************************
 *
 * XmPanedWindowWidget Private Data
 *
 *********************************************************************/

#include <Xm/PanedW.h>
#include <Xm/ManagerP.h>

#ifdef __cplusplus
extern "C" {
#endif

/* New fields for the PanedWindow widget class record */

typedef struct _XmPanedWindowClassPart
{
    XtPointer extension;
} XmPanedWindowClassPart;


/* Full Class record declaration */

typedef struct _XmPanedWindowClassRec {
    CoreClassPart       core_class;
    CompositeClassPart  composite_class;
    ConstraintClassPart constraint_class;
    XmManagerClassPart  manager_class;
    XmPanedWindowClassPart     paned_window_class;
} XmPanedWindowClassRec;

externalref XmPanedWindowClassRec xmPanedWindowClassRec;


/* PanedWindow constraint record */

typedef struct _XmPanedWindowConstraintPart {
    int		position;	/* position location in PanedWindow */
    int		dheight;	/* Desired size */
    Position	dy;		/* Desired Location */
    Position	olddy;		/* The last value of dy. */
    Dimension	min;		/* Minimum height */
    Dimension	max;		/* Maximum height */
    Boolean     isPane;         /* true if constraint of pane, false if
				   constraint of sash */
    Boolean	allow_resize;	/* TRUE iff child resize requests are ok */
    Boolean	skip_adjust;	/* TRUE iff child's height should not be */
				/* changed without explicit user action. */
    Widget	sash;		/* The sash for this child */
    Widget	separator;	/* The separator for this child */
    short       position_index; /* new 1.2 positionIndex resource */
#ifdef ORIENTED_PANEDW
    unsigned char orientation ; /* new pane orientation support */
#endif

 
} XmPanedWindowConstraintPart;

typedef struct _XmPanedWindowConstraintRec 
{
    XmManagerConstraintPart manager;
    XmPanedWindowConstraintPart  panedw;
} XmPanedWindowConstraintRec, * XmPanedWindowConstraintPtr;


/* New Fields for the PanedWindow widget record */

typedef struct {
    /* resources */
    Boolean     refiguremode;        /* Whether to refigure changes right now */
    Boolean	separator_on;	     /* make separator visible */

    Dimension  	margin_width;	     /* space between right and left edges of
					PanedWindow window and it's children */
    Dimension  	margin_height;	     /* space between top and bottom edges of
					PanedWindow window and it's children */
    Dimension   spacing;             /* whitespace between panes
				        around window, else leave none */
    /* sash modifying resources */
    Dimension	sash_width;	       /* Modify sash width */
    Dimension	sash_height;	       /* Modify sash height */
    Dimension   sash_shadow_thickness; /* Modify sash shadow_thickness */

    Position    sash_indent;           /* Location of sashs (offset
                                          from right margin)	*/
    /* private */
    int         starty;               /* mouse origin when adjusting */

    short	increment_count;      /* Sash increment count */
    short       pane_count;           /* number of managed panes */
    short       num_slots;	      /* number of avail. slots for children*/
    short       num_managed_children; /* holds number of managed children */

    Boolean     recursively_called;    /* For change_managed AND creation of
					* private sash and separator
					* children
					*/
    Boolean     resize_at_realize;    /* For realize if GeometryNo condition */

    XmPanedWindowConstraintPtr top_pane;    /* pane closest to 0 index */
    XmPanedWindowConstraintPtr bottom_pane; /* pane farthest away from 0 index*/

    GC          flipgc;               /* GC to use when animating borders */
    WidgetList  managed_children;     /* keep track of managed children */
} XmPanedWindowPart;

/**************************************************************************
 *
 * Full instance record declaration
 *
 **************************************************************************/

typedef struct _XmPanedWindowRec {
    CorePart       core;
    CompositePart  composite;
    ConstraintPart constraint;
    XmManagerPart  manager;
    XmPanedWindowPart   paned_window;
} XmPanedWindowRec;


/********    Private Function Declarations    ********/
#ifdef _NO_PROTO


#else


#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif  /* MOTIF12_HEADERS */

#endif /* _XmPanedWP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
