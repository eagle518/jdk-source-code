/* $XConsortium: LayoutT.h /main/5 1995/07/15 20:52:38 drk $ */
/*
 * @OSF_COPYRIGHT@
 *  COPYRIGHT NOTICE
 *  Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 *  ALL RIGHTS RESERVED (MOTIF). See the file named COPYRIGHT.MOTIF for
 *  the full copyright text.
 */
/*
 * HISTORY
 */
#ifndef _XmLayoutT_H
#define _XmLayoutT_H

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref XrmQuark XmQTspecifyLayoutDirection;

/* Trait structures and typedefs, place typedefs first */

typedef XmDirection (*XmGetLayoutDirectionProc)(Widget);

/* Version 0: initial release. */

typedef struct {
  int			   version;		/* 0 */
  XmGetLayoutDirectionProc get_direction;
} XmSpecifyLayoutDirectionTraitRec, *XmSpecifyLayoutDirectionTrait;

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmLayoutT_H */
