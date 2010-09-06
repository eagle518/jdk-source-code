/* $XConsortium: ActivatableT.h /main/5 1995/07/15 20:48:08 drk $ */
/*
 *  @OSF_COPYRIGHT@
 *  COPYRIGHT NOTICE
 *  Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 *  ALL RIGHTS RESERVED (MOTIF). See the file named COPYRIGHT.MOTIF for
 *  the full copyright text.
 */
/*
 * HISTORY
 */
#ifndef _XmActivatableT_H
#define _XmActivatableT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Xm/Xm.h>

externalref XrmQuark XmQTactivatable;

/* Trait structures and typedefs, place typedefs first */

typedef void (*XmActivatableCallBackProc)(Widget w, 
					  XtCallbackProc activCB,
					  XtPointer closure,
					  Boolean setunset);

/* Version 0: initial release. */

typedef struct _XmActivatableTraitRec {
  int 			    version;	/* 0 */
  XmActivatableCallBackProc changeCB;
} XmActivatableTraitRec, *XmActivatableTrait;


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmActivatableT_H */
