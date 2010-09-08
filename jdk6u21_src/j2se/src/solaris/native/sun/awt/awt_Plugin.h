/*
 * @(#)awt_Plugin.h	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Fix 4221246: Export functions for Netscape to use to get AWT info
 */

#ifndef _AWT_PLUGIN_H_
#define _AWT_PLUGIN_H_

#include <jni.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

void getAwtLockFunctions(void (**AwtLock)(JNIEnv *),
			 void (**AwtUnlock)(JNIEnv *),
			 void (**AwtNoFlushUnlock)(JNIEnv *),
			 void *);

void getExtAwtData(Display *,
		   int32_t,
		   int32_t *,      /* awt_depth */
		   Colormap *,     /* awt_cmap  */
		   Visual **,      /* awt_visInfo.visual */
		   int32_t *,      /* awt_num_colors */
		   void *);

void getAwtData(int32_t *, Colormap *, Visual **, int32_t *, void *);

Display *getAwtDisplay(void);

#endif /* _AWT_PLUGIN_H_ */
