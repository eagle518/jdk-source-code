/* $XConsortium: TravConT.h /main/5 1995/07/15 20:56:44 drk $ */
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
#ifndef _XmTravConT_h
#define _XmTravConT_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref XrmQuark XmQTtraversalControl;


typedef Widget (*XmTraversalRedirectionProc)(Widget               old_focus,
					     Widget               new_focus,
					     unsigned int         focus_policy,
					     XmTraversalDirection direction,
					     unsigned int         pass);


/* Version 0: initial release. */

typedef struct _XmTraversalControlTraitRec {
  int				version;	/* 0 */
  XmTraversalRedirectionProc	redirect;
} XmTraversalControlTraitRec, *XmTraversalControlTrait;


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* XmTravConT_h */
