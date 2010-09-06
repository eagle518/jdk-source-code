/* 
 * @OSF_COPYRIGHT@
 * (c) Copyright 1990, 1991, 1992, 1993, 1994 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *  
*/ 
/*
 * HISTORY
 * Motif Release 1.2.5
*/
#ifdef  SUN_MOTIF
#ifndef  __VirtKeysI_h
#define  __VirtKeysI_h
/*
 *      $XConsortium: _VirtKeysI.h /main/cde1_maint/2 1995/08/18 19:40:44 drk $
 *      @(#)_VirtKeysI.h	1.3 05 May 1994
 *	Added for backward compatibility
 */
/*************************************************************************
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 2002 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 *************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { Alt=0, Meta=1, Hyper=2, Super=3, NumLock=4, ModeSwitch=5 
	} ModifierType;

#ifdef _NO_PROTO
extern void _XmGetKPKeysymToKeycodeList();
extern Boolean _XmIsKPKey ();
extern Modifiers _XmGetModifierBinding();
extern void _XmGetModifierMapping();
extern int _XmTranslateKPKeySym ();
#else
extern void _XmGetKPKeysymToKeycodeList(
			Widget w);
extern Boolean _XmIsKPKey(
			Display *dpy, 
			KeyCode keycode, 
			KeySym *keysym_return);
extern Modifiers _XmGetModifierBinding(
			Display *dpy, 
			ModifierType modifier_type);
extern void _XmGetModifierMapping(
			Widget w);
extern int _XmTranslateKPKeySym (
			KeySym keysym, 
			char *buffer, 
			int nbytes);
#endif /* _NO_PROTO */
#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif
#endif /* _VirtKeysI_h */
#endif /* SUN_MOTIF */
