/* $XConsortium: DrPoly.c /main/6 1995/10/25 20:00:09 cde-sun $ */
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

#include <Xm/DrawP.h>
#include "RegionI.h"
#include "XmI.h"

/****************************XmeDrawPolygonShadow***************************/
void 
XmeDrawPolygonShadow (
		      Display *dpy,
		      Drawable d,
		      GC topGC,
		      GC bottomGC,
		      XPoint *points,
		      int n_points,
#if NeedWidePrototypes
		      int shadowThickness,
		      unsigned int shadowType)
#else
		      Dimension shadowThickness,
		      unsigned char shadowType)
#endif /* NeedWidePrototypes */
{
    Region  xregion;
    XtAppContext app;

    app = XtDisplayToApplicationContext(dpy);

    _XmAppLock(app);
    xregion = XPolygonRegion(points, n_points, /* FillRule */ WindingRule);

    _XmRegionDrawShadow (dpy, d,
			 topGC, bottomGC,
			 (XmRegion)xregion,
			 /* border_width */ 0, shadowThickness,
			 shadowType);

    XDestroyRegion(xregion);
    _XmAppUnlock(app);
}
