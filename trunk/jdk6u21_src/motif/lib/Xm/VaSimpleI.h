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
/* $XConsortium: VaSimpleI.h /main/5 1995/07/13 18:18:01 drk $ */
#ifndef _XmVaSimpleI_h
#define _XmVaSimpleI_h

#include <Xm/VaSimpleP.h>

#ifdef __cplusplus
extern "C" {
#endif


/********  Private Function Declarations  ********/

extern void _XmCountVaList( 
                        va_list var,
                        int *button_count,
                        int *args_count,
                        int *typed_count,
                        int *total_count) ;
extern void _XmVaToTypedArgList( 
                        va_list var,
                        int max_count,
                        XtTypedArgList *args_return,
                        Cardinal *num_args_return) ;

/********  End Private Function Declarations  ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmVaSimpleI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
