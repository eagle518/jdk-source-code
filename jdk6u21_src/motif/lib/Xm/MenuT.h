/* $XConsortium: MenuT.h /main/5 1995/07/15 20:53:03 drk $ */
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

#ifndef _XmMenuT_H
#define _XmMenuT_H

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Menu System Traits */
externalref XrmQuark XmQTmenuSystem;
externalref XrmQuark XmQTmenuSavvy;

/* Trait structures and typedefs, place typedefs first */

/* Used by the disable callback method of the menu savvy trait */
typedef enum {
    XmDISABLE_ACTIVATE,   /* defer to the entryCallback */
    XmENABLE_ACTIVATE     /* invoke own activate callback */
} XmActivateState;

/* Menu trait typedefs */

typedef int (*XmMenuSystemWidgetProc)(Widget);
typedef Boolean (*XmMenuSystemVerifyProc)(Widget, XEvent*);
typedef void (*XmMenuSystemControlTraversalProc)(Widget, Boolean);
typedef void (*XmMenuSystemCascadeProc)(Widget, Widget, XEvent*);
typedef void (*XmMenuSystemPositionProc)(Widget, XEvent*);
typedef Boolean (*XmMenuSystemPopdownProc)(Widget, XEvent*);
typedef void (*XmMenuSystemEntryCallbackProc)(Widget, Widget, XtPointer);
typedef Boolean (*XmMenuSystemUpdateHistoryProc)(Widget, Widget, Boolean);
typedef void (*XmMenuSystemUpdateBindingsProc)(Widget, int);
typedef void (*XmMenuSystemRecordPostFromWidgetProc)(Widget, Widget, Boolean);
typedef void (*XmMenuSystemDisarmProc)(Widget);
typedef Widget (*XmMenuSystemPopupPostedProc)(Widget);
typedef void (*XmMenuSavvyDisableProc)(Widget, XmActivateState);
typedef char* (*XmMenuSavvyGetAcceleratorProc)(Widget);
typedef KeySym (*XmMenuSavvyGetMnemonicProc)(Widget);
typedef char* (*XmMenuSavvyGetActivateCBNameProc)();
#define XmMenuSystemTypeProc		XmMenuSystemWidgetProc
#define XmMenuSystemStatusProc		XmMenuSystemWidgetProc
#define XmMenuSystemGetPostedFromWidgetProc	XmMenuSystemDisarmProc
#define XmMenuSystemArmProc		XmMenuSystemDisarmProc
#define XmMenuSystemMenuBarCleanupProc	XmMenuSystemDisarmProc
#define XmMenuSystemTearOffArmProc	XmMenuSystemDisarmProc
#define XmMenuSystemReparentProc	XmMenuSystemPositionProc
#define XmMenuSystemPopdownAllProc	XmMenuSystemPositionProc
#define XmMenuSystemChildFocusProc	XmMenuSystemDisarmProc

/* XmTmenuProcTrait */

/* Version 0: initial release. */

typedef struct _XmMenuSystemTraitRec
{
  int					version;		/* 0 */
  XmMenuSystemTypeProc			type;
  XmMenuSystemStatusProc		status;
  XmMenuSystemCascadeProc		cascade;
  XmMenuSystemVerifyProc		verifyButton;
  XmMenuSystemControlTraversalProc	controlTraversal;
  XmMenuSystemMenuBarCleanupProc	menuBarCleanup;
  XmMenuSystemPopdownProc		popdown;
  XmMenuSystemPopdownProc		buttonPopdown;
  XmMenuSystemReparentProc		reparentToTearOffShell;
  XmMenuSystemReparentProc		reparentToMenuShell;
  XmMenuSystemArmProc			arm;
  XmMenuSystemDisarmProc		disarm;
  XmMenuSystemTearOffArmProc		tearOffArm;
  XmMenuSystemEntryCallbackProc		entryCallback;
  XmMenuSystemUpdateHistoryProc		updateHistory;
  XmMenuSystemGetPostedFromWidgetProc	getLastSelectToplevel;
  XmMenuSystemPositionProc		position;
  XmMenuSystemUpdateBindingsProc	updateBindings;
  XmMenuSystemRecordPostFromWidgetProc	recordPostFromWidget;
  XmMenuSystemPopdownAllProc		popdownEveryone;
  XmMenuSystemChildFocusProc		childFocus;
  XmMenuSystemPopupPostedProc		getPopupPosted;
} XmMenuSystemTraitRec, *XmMenuSystemTrait;

/* XmTmenuSavvyTrait */

/* Version 0: initial release. */

typedef struct _XmMenuSavvyTraitRec
{
  int					version;		/* 0 */
  XmMenuSavvyDisableProc		disableCallback;
  XmMenuSavvyGetAcceleratorProc 	getAccelerator;
  XmMenuSavvyGetMnemonicProc		getMnemonic;
  XmMenuSavvyGetActivateCBNameProc	getActivateCBName;
} XmMenuSavvyTraitRec, *XmMenuSavvyTrait;

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmMenuT_H */
