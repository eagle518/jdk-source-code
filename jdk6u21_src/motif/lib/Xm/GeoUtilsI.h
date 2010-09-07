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
/* $XConsortium: GeoUtilsI.h /main/5 1995/07/13 17:28:18 drk $ */
#ifndef _XmGeoUtilsI_h
#define _XmGeoUtilsI_h

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations for GeoUtils.c    ********/

extern XtGeometryResult _XmHandleQueryGeometry( 
                        Widget wid,
                        XtWidgetGeometry *intended,
                        XtWidgetGeometry *desired,
#if NeedWidePrototypes
                        unsigned int policy,
#else
                        unsigned char policy,
#endif /* NeedWidePrototypes */
                        XmGeoCreateProc createMatrix) ;
extern XtGeometryResult _XmHandleGeometryManager( 
                        Widget wid,
                        Widget instigator,
                        XtWidgetGeometry *desired,
                        XtWidgetGeometry *allowed,
#if NeedWidePrototypes
                        unsigned int policy,
#else
                        unsigned char policy,
#endif /* NeedWidePrototypes */
                        XmGeoMatrix *cachePtr,
                        XmGeoCreateProc createMatrix) ;
extern void _XmHandleSizeUpdate( 
                        Widget wid,
#if NeedWidePrototypes
                        unsigned int policy,
#else
                        unsigned char policy,
#endif /* NeedWidePrototypes */
                        XmGeoCreateProc createMatrix) ;
extern XmGeoMatrix _XmGeoMatrixAlloc( 
                        unsigned int numRows,
                        unsigned int numBoxes,
                        unsigned int extSize) ;
extern void _XmGeoMatrixFree( 
                        XmGeoMatrix geo_spec) ;
extern Boolean _XmGeoSetupKid( 
                        XmKidGeometry geo,
                        Widget kidWid) ;
extern void _XmGeoMatrixGet( 
                        XmGeoMatrix geoSpec,
                        int geoType) ;
extern void _XmGeoMatrixSet( 
                        XmGeoMatrix geoSpec) ;
extern void _XmGeoAdjustBoxes( 
                        XmGeoMatrix geoSpec) ;
extern void _XmGeoGetDimensions( 
                        XmGeoMatrix geoSpec) ;
extern void _XmGeoArrangeBoxes( 
                        XmGeoMatrix geoSpec,
#if NeedWidePrototypes
                        int x,
                        int y,
#else
                        Position x,
                        Position y,
#endif /* NeedWidePrototypes */
                        Dimension *pW,
                        Dimension *pH) ;
extern Dimension _XmGeoBoxesSameWidth( 
                        XmKidGeometry rowPtr,
#if NeedWidePrototypes
                        int width) ;
#else
                        Dimension width) ;
#endif /* NeedWidePrototypes */
extern Dimension _XmGeoBoxesSameHeight( 
                        XmKidGeometry rowPtr,
#if NeedWidePrototypes
                        int height) ;
#else
                        Dimension height) ;
#endif /* NeedWidePrototypes */
extern void _XmSeparatorFix( 
                        XmGeoMatrix geoSpec,
                        int action,
                        XmGeoMajorLayout layoutPtr,
                        XmKidGeometry rowPtr) ;
extern void _XmMenuBarFix( 
                        XmGeoMatrix geoSpec,
                        int action,
                        XmGeoMajorLayout layoutPtr,
                        XmKidGeometry rowPtr) ;
extern void _XmGeoLoadValues( 
                        Widget wid,
                        int geoType,
                        Widget instigator,
                        XtWidgetGeometry *request,
                        XtWidgetGeometry *geoResult) ;
extern int _XmGeoCount_kids( 
                        register CompositeWidget c) ;
extern XmKidGeometry _XmGetKidGeo( 
                        Widget wid,
                        Widget instigator,
                        XtWidgetGeometry *request,
                        int uniform_border,
#if NeedWidePrototypes
                        int border,
#else
                        Dimension border,
#endif /* NeedWidePrototypes */
                        int uniform_width_margins,
                        int uniform_height_margins,
                        Widget help,
                        int geo_type) ;
extern void _XmGeoClearRectObjAreas( 
                        RectObj r,
                        XWindowChanges *old) ;
extern void _XmSetKidGeo( 
                        XmKidGeometry kg,
                        Widget instigator) ;
extern Boolean _XmGeometryEqual( 
                        Widget wid,
                        XtWidgetGeometry *geoA,
                        XtWidgetGeometry *geoB) ;
extern Boolean _XmGeoReplyYes( 
                        Widget wid,
                        XtWidgetGeometry *desired,
                        XtWidgetGeometry *response) ;
extern XtGeometryResult _XmMakeGeometryRequest( 
                        Widget w,
                        XtWidgetGeometry *geom) ;

/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmGeoUtilsI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
