/*
 * @(#)awt_MToolkit.h	1.11 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
extern jobject getOwningFrameOrDialog(jobject target, JNIEnv *env);

#define SPECIAL_KEY_EVENT 2

#endif /* !HEADLESS */
#endif           /* _MTOOLKIT_H_ */
