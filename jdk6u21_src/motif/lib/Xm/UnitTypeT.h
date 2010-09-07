/* $XConsortium: UnitTypeT.h /main/5 1995/07/15 20:57:00 drk $ */
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
#ifndef _XmUnitTypeT_H
#define _XmUnitTypeT_H

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref XrmQuark XmQTspecifyUnitType;

/* Trait structures and typedefs, place typedefs first */

typedef unsigned char (*XmSpecUnitTypeGetProc)(Widget wid);

/* Version 0: initial release. */

typedef struct _XmSpecUnitTypeTraitRec {
  int			version;	/* 0 */
  XmSpecUnitTypeGetProc getUnitType; 
} XmSpecUnitTypeTraitRec, *XmSpecUnitTypeTrait;


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmUnitTypeT_H */

