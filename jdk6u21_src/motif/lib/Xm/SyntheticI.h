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
/* $XConsortium: SyntheticI.h /main/6 1996/04/18 12:01:21 daniel $ */
#ifndef _XmSyntheticI_h
#define _XmSyntheticI_h

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations for Synthetic.c    ********/

extern void _XmBuildResources( 
                        XmSyntheticResource **wc_resources_ptr,
                        int *wc_num_resources_ptr,
                        XmSyntheticResource *sc_resources,
                        int sc_num_resources) ;
extern void _XmInitializeSyntheticResources( 
                        XmSyntheticResource *resources,
                        int num_resources) ;
extern void _XmPrimitiveGetValuesHook( 
                        Widget w,
                        ArgList args,
                        Cardinal *num_args) ;
extern void _XmGadgetGetValuesHook( 
                        Widget w,
                        ArgList args,
                        Cardinal *num_args) ;
extern void _XmManagerGetValuesHook( 
                        Widget w,
                        ArgList args,
                        Cardinal *num_args) ;
/* Solaris 2.7 bugfix # 4072236 - 6 lines */
/*
 *extern void _XmPrintShellGetValuesHook( 
 *                    Widget w,
 *                    ArgList args,
 *                    Cardinal *num_args) ;
 */
extern void _XmExtGetValuesHook( 
                        Widget w,
                        ArgList args,
                        Cardinal *num_args) ;
extern void _XmExtImportArgs( 
                        Widget w,
                        ArgList args,
                        Cardinal *num_args) ;
extern void _XmPrimitiveImportArgs( 
                        Widget w,
                        ArgList args,
                        Cardinal *num_args) ;
extern void _XmGadgetImportArgs( 
                        Widget w,
                        ArgList args,
                        Cardinal *num_args) ;
extern void _XmGadgetImportSecondaryArgs( 
                        Widget w,
                        ArgList args,
                        Cardinal *num_args) ;
extern void _XmManagerImportArgs( 
                        Widget w,
                        ArgList args,
                        Cardinal *num_args) ;

/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmSyntheticI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
