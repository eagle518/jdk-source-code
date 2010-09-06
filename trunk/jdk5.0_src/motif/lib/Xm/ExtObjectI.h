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
/* $XConsortium: ExtObjectI.h /main/6 1995/07/14 10:32:34 drk $ */
#ifndef _XmExtObjectI_h
#define _XmExtObjectI_h

#include <Xm/ExtObjectP.h>

#ifdef __cplusplus
extern "C" {
#endif


/********    Private Function Declarations    ********/

extern char * _XmExtObjAlloc(size_t size); /* Wyoming 64-bit fix */ 
extern void _XmExtObjFree(XtPointer element);
extern void _XmBuildExtResources(WidgetClass c);

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmExtObjectI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
