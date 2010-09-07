/* $XConsortium: ColorObjI.h /main/6 1995/07/15 20:49:00 drk $ */
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
#ifndef _XmColorObjI_h
#define _XmColorObjI_h

#include <Xm/ColorObjP.h>

#ifdef __cplusplus
extern "C" {
#endif

/* defines for palette.c */
#define VALUE_THRESHOLD 225

externalref XmColorObj _XmDefaultColorObj;
externalref XContext _XmColorObjCache;
externalref Display  *_XmColorObjCacheDisplay;

/********    Private Function Declarations    ********/

extern void _XmColorObjCreate( 
                        Widget w,
                        ArgList al,
                        Cardinal *acPtr) ;

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmColorObjI_h */

