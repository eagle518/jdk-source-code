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
/* $XConsortium: BaseClassI.h /main/6 1995/07/14 10:10:58 drk $ */
#ifndef _XmBaseClassI_h
#define _XmBaseClassI_h

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations    ********/

extern void _XmPushWidgetExtData( 
                        Widget widget,
                        XmWidgetExtData data,
#if NeedWidePrototypes
                        unsigned int extType) ;
#else
                        unsigned char extType) ;
#endif /* NeedWidePrototypes */
extern void _XmPopWidgetExtData( 
                        Widget widget,
                        XmWidgetExtData *dataRtn,
#if NeedWidePrototypes
                        unsigned int extType) ;
#else
                        unsigned char extType) ;
#endif /* NeedWidePrototypes */
extern XmWidgetExtData _XmGetWidgetExtData( 
                        Widget widget,
#if NeedWidePrototypes
                        unsigned int extType) ;
#else
                        unsigned char extType) ;
#endif /* NeedWidePrototypes */
extern void _XmInitializeExtensions( void ) ;
extern void _XmTransformSubResources( 
                        XtResourceList comp_resources,
                        Cardinal num_comp_resources,
                        XtResourceList *resources,
                        Cardinal *num_resources) ;

extern Cardinal _XmSecondaryResourceData( 
                        XmBaseClassExt bcePtr,
                        XmSecondaryResourceData **secResDataRtn,
                        XtPointer client_data,
                        String name,
                        String class_name,
                        XmResourceBaseProc basefunctionpointer) ;

/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmBaseClassI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
