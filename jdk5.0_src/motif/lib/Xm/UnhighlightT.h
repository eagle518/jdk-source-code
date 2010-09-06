/* $XConsortium: UnhighlightT.h /main/5 1995/07/15 20:56:56 drk $ */
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

#ifndef _XmSpecifyUnhighlightT_H
#define _XmSpecifyUnhighlightT_H

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref XrmQuark XmQTspecifyUnhighlight;

/* Trait structures and typedefs, place typedefs first */

typedef GC (*XmSpecifyUnhighlightProc)(Widget wid, Widget child);

/* Version 0: initial release. */

typedef struct _XmSpecifyUnhighlightTraitRec {
  int			   version;		/* 0 */
  XmSpecifyUnhighlightProc getUnhighlightGC;
} XmSpecifyUnhighlightTraitRec, *XmSpecifyUnhighlightTrait;

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmSpecifyUnhighlightT_H */
