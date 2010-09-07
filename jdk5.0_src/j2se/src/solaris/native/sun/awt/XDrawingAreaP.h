/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)XDrawingAreaP.h	1.5 03/12/19
 */

#ifndef _XDrawingAreaP_h_
#define _XDrawingAreaP_h_

#include <Xm/DrawingAP.h>
#include "XDrawingArea.h"


/***************************************************************
 * VDrawingArea Widget Data Structures
 *
 *
 **************************************************************/

/* Define part class structure */
typedef struct _XDrawingAreaClass {
	XtPointer			extension;
} XDrawingAreaClassPart;

/* Define the full class record */
typedef struct _XDrawingAreaClassRec {
	CoreClassPart		core_class;
	CompositeClassPart	composite_class;
	ConstraintClassPart	constraint_class;
	XmManagerClassPart	manager_class;
	XmDrawingAreaClassPart	drawing_area_class;
	XDrawingAreaClassPart	xdrawingarea_class;
} XDrawingAreaClassRec;

/* External definition for class record */
extern XDrawingAreaClassRec xDrawingAreaClassRec;

/****************************************************************
 *
* Full instance record declaration
 *
 ****************************************************************/

typedef struct _XDrawingAreaRec
{
	CorePart		core;
	CompositePart		composite;
	ConstraintPart		constraint;
	XmManagerPart		manager;
	XmDrawingAreaPart	drawing_area;
} XDrawingAreaRec;



#endif /* !_VDrawingAreaP_h_ */

