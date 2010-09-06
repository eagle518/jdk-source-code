/* $XConsortium: SpecRenderT.h /main/5 1995/07/15 20:55:28 drk $ */
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
#ifndef _XmSpecRenderT_H
#define _XmSpecRenderT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Xm/Xm.h>

externalref XrmQuark XmQTspecifyRenderTable;

/* Trait structures and typedefs, place typedefs first */

typedef XmFontList (*XmSpecRenderGetTableProc)(Widget wid,
					       XtEnum type);

/* Version 0: initial release. */

typedef struct _XmSpecRenderTraitRec {
  int			   version;		/* 0 */
  XmSpecRenderGetTableProc getRenderTable;
} XmSpecRenderTraitRec, *XmSpecRenderTrait;


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmSpecRenderT_H */

