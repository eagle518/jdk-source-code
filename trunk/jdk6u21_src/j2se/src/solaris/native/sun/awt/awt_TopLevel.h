/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)awt_TopLevel.h	1.10 10/03/23
 */
#ifndef _TOPLEVEL_H_
#define _TOPLEVEL_H_
#ifndef HEADLESS

extern Widget findFocusProxy(Widget widget);
extern Widget findTopLevelByShell(Widget widget);
extern jobject findTopLevel(jobject peer, JNIEnv *env);
extern void shellEH(Widget w, XtPointer data, XEvent *event, Boolean *continueToDispatch);
extern Boolean isFocusableWindowByShell(JNIEnv * env, Widget shell);
extern Boolean isFocusableWindowByPeer(JNIEnv * env, jobject peer);
extern Widget getShellWidget(Widget child);
extern Boolean isFocusableComponentTopLevelByWidget(JNIEnv * env, Widget child);
#endif /* !HEADLESS */
#endif           /* _TOPLEVEL_H_ */

