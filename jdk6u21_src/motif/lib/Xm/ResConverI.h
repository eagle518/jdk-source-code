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
/* $XConsortium: ResConverI.h /main/5 1995/07/13 17:48:35 drk $ */
#ifndef _XmResConvertI_h
#define _XmResConvertI_h

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations for ResConvert.c    ********/

extern void _XmRegisterConverters( void ) ;
extern char * _XmConvertCSToString( 
                        XmString cs) ;
extern Boolean _XmCvtXmStringToCT( 
                        XrmValue *from,
                        XrmValue *to) ;

/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmResConvertI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
