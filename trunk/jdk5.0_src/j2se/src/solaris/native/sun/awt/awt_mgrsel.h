/*
 * @(#)awt_mgrsel.h	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

