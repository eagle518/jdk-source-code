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
/* $XConsortium: MenuProcI.h /main/5 1995/07/13 17:35:47 drk $ */
#ifndef _XmMenuProcI_h
#define _XmMenuProcI_h

#include <Xm/MenuProcP.h>

#ifdef __cplusplus
extern "C" {
#endif


/********    Private Function Declarations    ********/

extern void _XmSaveMenuProcContext(XtPointer address) ;
extern XtPointer _XmGetMenuProcContext( void ) ;
extern void _XmSaveCoreClassTranslations(Widget widget) ;
extern void _XmRestoreCoreClassTranslations(Widget widget) ;
extern void _XmDeleteCoreClassTranslations(Widget widget);

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmMenuProcI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
