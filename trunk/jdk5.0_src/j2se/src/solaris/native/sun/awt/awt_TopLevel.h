/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)awt_TopLevel.h	1.7 03/12/19
 */
#ifndef _TOPLEVEL_H_
#define _TOPLEVEL_H_
#ifndef HEADLESS

extern Widget findFocusProxy(Widget widget);
extern Widget findTopLevelByShell(Widget widget);
extern jobject findTopLevel(jobject peer, JNIEnv *env);
extern void shellEH(Widget w, XtPointer data, XEvent *event, Boolean *continueToDispatch);
extern Boolean isFocusableWindowByShell(JNIEnv * env, Widget shell);
extern Boolean isFocusableWindowByObject(JNIEnv * env, jobject winObject);
extern Widget getShellWidget(Widget child);
extern Boolean isFocusableComponentTopLevelByWidget(JNIEnv * env, Widget child);
#endif /* !HEADLESS */
#endif           /* _TOPLEVEL_H_ */

