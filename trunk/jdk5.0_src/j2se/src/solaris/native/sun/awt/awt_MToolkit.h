/*
 * @(#)awt_MToolkit.h	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef _MTOOLKIT_H_
#define _MTOOLKIT_H_
#ifndef HEADLESS
extern void *findPeer(Widget * pwidget);
extern Widget findWindowsProxy(jobject window, JNIEnv *env);
extern struct WidgetInfo *findWidgetInfo(Widget widget);
extern Boolean isAncestor(Window ancestor, Window child);
extern void clearFocusPath(Widget shell);
extern void globalClearFocusPath(Widget focusOwnerShell);
extern Boolean isFrameOrDialog(jobject target, JNIEnv * env);

#define SPECIAL_KEY_EVENT 2

#endif /* !HEADLESS */
#endif           /* _MTOOLKIT_H_ */
