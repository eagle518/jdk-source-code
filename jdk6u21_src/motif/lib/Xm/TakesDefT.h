/* $XConsortium: TakesDefT.h /main/5 1995/07/15 20:55:59 drk $ */
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
#ifndef _XmTakesDefaultT_H
#define _XmTakesDefaultT_H

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref XrmQuark XmQTtakesDefault;

/* Trait structures and typedefs, place typedefs first */

typedef void (*XmTakesDefaultNotifyProc)(Widget w, 
					 XtEnum state);

/* Version 0: initial release. */

typedef struct _XmTakesDefaultTraitRec {
  int			   version;		/* 0 */
  XmTakesDefaultNotifyProc showAsDefault;
} XmTakesDefaultTraitRec, *XmTakesDefaultTrait;

enum {XmDEFAULT_READY, XmDEFAULT_ON, XmDEFAULT_OFF, XmDEFAULT_FORGET} ;


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmTakesDefaultT_H */
