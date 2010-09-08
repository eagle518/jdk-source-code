/*
 * @(#)awt_mgrsel.h	1.5 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef _AWT_MGRSEL_H_
#define _AWT_MGRSEL_H_

#ifndef HEADLESS

#include "awt_p.h"

extern void awt_mgrsel_init(void);
extern int awt_mgrsel_processEvent(XEvent *);

extern const Window * awt_mgrsel_select(const char *, long,
					void *,
					void (*)(int, XEvent *, void *),
					void (*)(int, Window, long *, void *));

#endif /* !HEADLESS */
#endif /* _AWT_MGRSEL_H_ */

