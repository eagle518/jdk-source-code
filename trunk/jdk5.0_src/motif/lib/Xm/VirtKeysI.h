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
/* $XConsortium: VirtKeysI.h /main/5 1995/07/13 18:20:49 drk $ */
#ifndef _XmVirtKeyI_h
#define _XmVirtKeyI_h

#include <Xm/VirtKeysP.h>

#ifdef __cplusplus
extern "C" {
#endif


/********    Private Function Declarations    ********/

extern void _XmVirtKeysInitialize( 
                        Widget widget) ;
extern void _XmVirtKeysDestroy( 
                        Widget widget) ;
extern void _XmVirtKeysStoreBindings( 
                        Widget shell,
                        String binding) ;
extern Boolean _XmVirtKeysLoadFileBindings( 
                        char *fileName,
                        String *binding) ;
extern int _XmVirtKeysLoadFallbackBindings(
			Display *display,
			String *binding) ;
/* bug Id : 4106529 */
extern void _XmVirtKeysHandler(
                        Widget widget,
                        XtPointer client_data,
                        XEvent *event,
                        Boolean *dontSwallow) ;

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmVirtKeyI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
