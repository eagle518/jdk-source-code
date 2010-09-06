/* $XConsortium: JoinSideT.h /main/5 1995/07/15 20:52:34 drk $ */
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
#ifndef _XmJoinSideT_H
#define _XmJoinSideT_H

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref XrmQuark XmQTjoinSide;

/* Trait structures and typedefs, place typedefs first */

typedef void (*XmJoinSideSetValueProc)(Widget tab,
				       unsigned char join_side, 
				       Dimension join_thickness) ;
typedef unsigned char (*XmJoinSideGetValueProc)(Widget tab,
						Dimension * join_thickness);


/* Version 0: initial release. */

typedef struct _XmJoinSideTraitRec {
  int			 version;	/* 0 */
  XmJoinSideSetValueProc setValue;
  XmJoinSideGetValueProc getValue;
} XmJoinSideTraitRec, *XmJoinSideTrait;

enum {/* XmNONE already defined in Xm.h */ 
	 XmLEFT = 1, XmRIGHT, XmTOP, XmBOTTOM} ;

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmJoinSideT_H */
