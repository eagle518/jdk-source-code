/* $XConsortium: DialogSavvyT.h /main/5 1995/07/15 20:50:29 drk $ */
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
#ifndef _XmDialogSavvyT_H
#define _XmDialogSavvyT_H

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref XrmQuark XmQTdialogShellSavvy;
/* This trait also requires a resource named "defaultPosition".
   If the child has the trait, the resource will be get and set by 
   the DialogShell ChangeManaged */
   
/* Trait structures and typedefs, place typedefs first */

typedef void (*XmDialogSavvyMapUnmapProc)(Widget wid, 
					  Boolean map_unmap);


/* Version 0: initial release. */

typedef struct _XmDialogSavvyTraitRec	 {
  int			    version;		/* 0 */
  XmDialogSavvyMapUnmapProc callMapUnmapCB;
} XmDialogSavvyTraitRec,*XmDialogSavvyTrait;


/* This macro is part of the trait and is used for the following situation
   DialogShells always mimic the child position on themselves.
   If the SetValues on a bb child position was 0,
   which is always the _current_ position of the bb in a DialogShell,
   Xt does not see a change and therefore not trigerred a geometry request.
   So BB (or any dialogShellSavvy child) has to catch this case
   and change the position request to use a special value in its
   SetValues method, XmDIALOG_SAVVY_FORCE_ORIGIN, to notify the Dialog that 
   it really wants to move in 0 */

#define XmDIALOG_SAVVY_FORCE_ORIGIN ((Position)~0L)

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDialogSavvyT_H */
