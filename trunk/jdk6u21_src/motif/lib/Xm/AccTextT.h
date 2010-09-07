/* $XConsortium: AccTextT.h /main/5 1995/07/15 20:48:04 drk $ */
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
#ifndef _XmAccessTextualT_H
#define _XmAccessTextualT_H

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref XrmQuark XmQTaccessTextual;

typedef XtPointer (*XmAccessTextualGetValuesProc)(Widget, int);
typedef int       (*XmAccessTextualPreferredProc)(Widget);
typedef void      (*XmAccessTextualSetValuesProc)(Widget, XtPointer, int);

enum { XmFORMAT_XmSTRING, XmFORMAT_MBYTE, XmFORMAT_WCS };

/* Trait structures and typedefs, place typedefs first */

/* Version 0: initial release. */

typedef struct _XmAccessTextualTraitRec {
  int				version;	 /* 0 */
  XmAccessTextualGetValuesProc	getValue;
  XmAccessTextualSetValuesProc	setValue;
  XmAccessTextualPreferredProc	preferredFormat;
} XmAccessTextualTraitRec, *XmAccessTextualTrait;

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmAccessTextualT_H */
