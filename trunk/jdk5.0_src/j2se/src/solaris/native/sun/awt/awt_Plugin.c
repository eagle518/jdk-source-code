/*
 * @(#)awt_Plugin.c	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifdef HEADLESS
    #error This file should not be included in headless library
#endif

#include <jni.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <awt.h>
#include <awt_p.h>

/*
 * Fix 4221246: Provide utility function for Netscape to use to
 * get AWT display, depth, colormap, and number of colors.
 *
 */

Display *getAwtDisplay(void)
{
  return awt_display;
}

void getExtAwtData(Display      *display,
		   int32_t      screen,
		   int32_t      *awt_depth,
		   Colormap     *awt_cmap,
		   Visual       **awt_visual,
		   int32_t      *awt_num_colors,
		   void         *pReserved)
{
  AwtGraphicsConfigDataPtr defaultConfig = NULL;

#ifdef DEBUG
  if (pReserved != NULL) {
    jio_fprintf(stderr,
                "getExtAwtData: warning: reserved pointer is not null\n");
  }
#endif

  if (screen >= 0) {
    defaultConfig = getDefaultConfig(screen);
  }

  if (defaultConfig) {
    if (awt_depth != NULL) {
      *awt_depth = defaultConfig->awt_depth;
    }
    
    if (awt_cmap != NULL) {
      *awt_cmap = defaultConfig->awt_cmap;
    }
    
    if (awt_visual != NULL) {
      *awt_visual = defaultConfig->awt_visInfo.visual;
    }
    
    if (awt_num_colors != NULL) {
      *awt_num_colors = defaultConfig->awt_num_colors;
    }
  }
}

/*
 * getAwtData provided for compatibility with Solaris 1.2 Java Plug-in
 *
 */
void getAwtData(int32_t          *awt_depth,
		Colormap     *awt_cmap,
		Visual       **awt_visual,
		int32_t          *awt_num_colors,
		void         *pReserved)
{
  Display *display = getAwtDisplay();

  getExtAwtData(display,
		DefaultScreen(display),
		awt_depth,
		awt_cmap,
		awt_visual,
		awt_num_colors,
		pReserved);
}

/*
 * Fix 4221246: Provide utility funtion for Netscape to get
 * function pointers to AWT lock functions.
 *
 */

static void awt_lock_wrapper(JNIEnv *env) {
  AWT_LOCK();
}

static void awt_unlock_wrapper(JNIEnv *env) {
  AWT_UNLOCK();
}

static void awt_noflush_unlock_wrapper(JNIEnv *env) {
  AWT_NOFLUSH_UNLOCK();
}

void getAwtLockFunctions(void (**AwtLock)(JNIEnv *),
			 void (**AwtUnlock)(JNIEnv *),
			 void (**AwtNoFlushUnlock)(JNIEnv *),
			 void *pReserved)
{
#ifdef DEBUG
  if (pReserved != NULL) {
    jio_fprintf(stderr,
		"getAwtLockFunctions: warning: reserved pointer is not null\n");
  }
#endif

  if (AwtLock != NULL) {
    *AwtLock = awt_lock_wrapper;
  }

  if (AwtUnlock != NULL) {
    *AwtUnlock = awt_unlock_wrapper;
  }

  if (AwtNoFlushUnlock != NULL) {
    *AwtNoFlushUnlock = awt_noflush_unlock_wrapper;
  }
}





