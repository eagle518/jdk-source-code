/* $XConsortium: PointInT.h /main/5 1995/07/15 20:54:18 drk $ */
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

#ifndef _XmPointInT_H
#define _XmPointInT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Xm/Xm.h>

externalref XrmQuark XmQTpointIn;

/* Trait structures and typedefs, place typedefs first */

typedef Boolean (*XmPointInProc)(Widget w, 
				 Position x,
				 Position y);

typedef struct _XmPointInTraitRec {
    int version ;
    XmPointInProc	pointIn;
} XmPointInTraitRec, *XmPointInTrait;


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmPointInT_H */
