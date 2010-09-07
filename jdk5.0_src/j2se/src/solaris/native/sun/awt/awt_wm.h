/*
 * @(#)awt_wm.h	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef _AWT_WM_H_
#define _AWT_WM_H_

#ifndef HEADLESS

#include "awt_p.h"

/* 
 * Window Managers we care to distinguish.
 * See awt_wm_getRunningWM()
 */
enum wmgr_t {
    UNDETERMINED_WM,
    NO_WM,
    OTHER_WM,
    OPENLOOK_WM,
    MOTIF_WM,
    CDE_WM,
    ENLIGHTEN_WM,
    KDE2_WM,
    SAWFISH_WM,
    ICE_WM,
    METACITY_WM
};

extern void awt_wm_init(void);

extern enum wmgr_t awt_wm_getRunningWM(void);
extern Boolean awt_wm_configureGravityBuggy(void);
extern Boolean awt_wm_supportsExtendedState(jint state);

/* XWMHints.flags is declared long, so 'mask' argument is declared long too */
extern void awt_wm_removeSizeHints(Widget shell, long mask);

extern void awt_wm_setShellDecor(struct FrameData *wdata, Boolean resizable);
extern void awt_wm_setShellResizable(struct FrameData *wdata);
extern void awt_wm_setShellNotResizable(struct FrameData *wdata,
					int32_t width, int32_t height, 
					Boolean justChangeSize);

extern Boolean awt_wm_getInsetsFromProp(Window w,
	         int32_t *top, int32_t *left, int32_t *bottom, int32_t *right);

/*
 * WM_STATE: WithdrawnState, NormalState, IconicState.
 * Absence of WM_STATE is treated as WithdrawnState.
 */
extern int awt_wm_getWMState(Window w);

extern void awt_wm_setExtendedState(struct FrameData *wdata, jint state);
extern Boolean awt_wm_isStateChange(struct FrameData *wdata, XPropertyEvent *e,
				    jint *pstate);

extern void awt_wm_unshadeKludge(struct FrameData *wdata);
extern void awt_wm_updateAlwaysOnTop(struct FrameData *wdata, jboolean bLayerState);

#endif /* !HEADLESS */
#endif /* _AWT_WM_H_ */
