/*
 * @(#)awt.h	1.47 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Common AWT definitions
 */

#ifndef _AWT_
#define _AWT_

#include "jvm.h"
#include "jni_util.h"
#include "debug_util.h"

#ifndef HEADLESS
#include <X11/Intrinsic.h>
#endif /* !HEADLESS */


/* The JVM instance: defined in awt_MToolkit.c */
extern JavaVM *jvm;


/* The lock object: defined in awt_MToolkit.c */
extern jobject awt_lock;

/* Use of JNI vs old locking primitives */
#define JNI_AWT_LOCK

/* Perform sanity and consistency checks on AWT locking */
#ifdef DEBUG
# define DEBUG_AWT_LOCK
#endif

/* 
 * The following locking primitives should be defined
 *
#define AWT_LOCK()
#define AWT_NOFLUSH_UNLOCK()
#define AWT_WAIT(tm)
#define AWT_NOTIFY()
#define AWT_NOTIFY_ALL()
 */

/*
 * Convenience macros based on AWT_NOFLUSH_UNLOCK
 */
extern void awt_output_flush();
#define AWT_UNLOCK() AWT_FLUSH_UNLOCK()
#define AWT_FLUSH_UNLOCK() do {			\
    awt_output_flush();				\
    AWT_NOFLUSH_UNLOCK();			\
} while (0)


#ifdef NETSCAPE
/* Netscape uses its own locking */

#include "prmon.h"

extern PRMonitor *_pr_rusty_lock;
extern void PR_XWait(long ms);
extern void PR_XNotify(void);
extern void PR_XNotifyAll(void);

#define AWT_LOCK()		PR_XLock()
#define AWT_NOFLUSH_UNLOCK()	PR_XUnlock()
#define AWT_WAIT(tm)		PR_XWait(tm)
#define AWT_NOTIFY()		PR_XNotify()
#define AWT_NOTIFY_ALL()	PR_XNotifyAll()

#else  /* ! NETSCAPE */
#ifdef DEBUG_AWT_LOCK
#ifndef XAWT  /* Unfortunately AWT_LOCK debugging does not work with XAWT due to mixed Java/C use of AWT lock. */
extern int awt_locked;
extern char *lastF;
extern int lastL;

#ifdef JNI_AWT_LOCK

#define AWT_LOCK() do {							\
    if (awt_lock == 0) {						\
	jio_fprintf(stderr, "AWT lock error, awt_lock is null\n");	\
    }									\
    if (awt_locked < 0) {						\
	jio_fprintf(stderr,						\
		    "AWT lock error (%s,%d) (last held by %s,%d) %d\n",	\
		    __FILE__, __LINE__, lastF, lastL, awt_locked);	\
    }									\
    lastF = __FILE__;							\
    lastL = __LINE__;							\
    (*env)->MonitorEnter(env, awt_lock);				\
    ++awt_locked;							\
} while (0)

#define AWT_NOFLUSH_UNLOCK() do {				\
    lastF = "";							\
    lastL = -1;							\
    if (awt_locked < 1) {					\
	jio_fprintf(stderr, "AWT unlock error (%s,%d,%d)\n",	\
		    __FILE__, __LINE__, awt_locked);		\
    }								\
    --awt_locked;						\
    (*env)->MonitorExit(env, awt_lock);				\
} while (0)

#define AWT_WAIT(tm) do {					\
    int old_lockcount = awt_locked;				\
    if (awt_locked < 1) {					\
	jio_fprintf(stderr, "AWT wait error (%s,%d,%d)\n",	\
		    __FILE__, __LINE__, awt_locked);		\
    }								\
    awt_locked = 0;						\
    JNU_MonitorWait(env, awt_lock, (tm));			\
    awt_locked = old_lockcount;					\
} while (0)

#define AWT_NOTIFY() do {					\
    if (awt_locked < 1) {					\
	jio_fprintf(stderr, "AWT notify error (%s,%d,%d)\n",	\
		    __FILE__, __LINE__, awt_locked);		\
    }								\
    JNU_Notify(env, awt_lock);					\
} while(0)

#define AWT_NOTIFY_ALL() do {						\
    if (awt_locked < 1) {						\
	jio_fprintf(stderr, "AWT notify all error (%s,%d,%d)\n",	\
		    __FILE__, __LINE__, awt_locked);			\
    }									\
    JNU_NotifyAll(env, awt_lock);					\
} while (0)

#else /* ! JNI_AWT_LOCK */

#define AWT_LOCK() do {							\
    if (awt_lock == 0) {						\
	jio_fprintf(stderr, "AWT lock error, awt_lock is null\n");	\
    }									\
    if (awt_locked < 0) {						\
	jio_fprintf(stderr,						\
		    "AWT lock error (%s,%d) (last held by %s,%d) %d\n",	\
		    __FILE__, __LINE__, lastF, lastL, awt_locked);	\
    }									\
    lastF = __FILE__;							\
    lastL = __LINE__;							\
    monitorEnter(obj_monitor(awt_lock));				\
    ++awt_locked;							\
} while (0)

#define AWT_NOFLUSH_UNLOCK() do {				\
    lastF = "";							\
    lastL = -1;							\
    if (awt_locked < 1) {					\
	jio_fprintf(stderr, "AWT unlock error (%s,%d,%d)\n",	\
		    __FILE__, __LINE__, awt_locked);		\
    }								\
    --awt_locked;						\
    monitorExit(obj_monitor(awt_lock));				\
} while (0)

#define AWT_WAIT(tm) do {					\
    int old_lockcount = awt_locked;				\
    if (awt_locked < 1) {					\
	jio_fprintf(stderr, "AWT wait error (%s,%d,%d)\n",	\
		    __FILE__, __LINE__, awt_locked);		\
    }								\
    awt_locked = 0;						\
    monitorWait(obj_monitor(awt_lock), (tm));			\
    awt_locked = old_lockcount;					\
} while (0)

#define AWT_NOTIFY() do {					\
    if (awt_locked < 1) {					\
	jio_fprintf(stderr, "AWT notify error (%s,%d,%d)\n",	\
		    __FILE__, __LINE__, awt_locked);		\
    }								\
    monitorNotify(obj_monitor(awt_lock));			\
} while (0)

#define AWT_NOTIFY_ALL() do {						\
    if (awt_locked < 1) {						\
	jio_fprintf(stderr, "AWT notify all error (%s,%d,%d)\n",	\
		    __FILE__, __LINE__, awt_locked);			\
    }									\
    monitorNotifyAll(obj_monitor(awt_lock));				\
} while (0)

#endif /* ! JNI_AWT_LOCK */
#else /* XAWT */
#define AWT_LOCK()		(*env)->MonitorEnter(env, awt_lock)
#define AWT_NOFLUSH_UNLOCK()	(*env)->MonitorExit(env, awt_lock)
#define AWT_WAIT(tm)		JNU_MonitorWait(env, awt_lock, (tm))
#define AWT_NOTIFY()		JNU_Notify(env, awt_lock)
#define AWT_NOTIFY_ALL()	JNU_NotifyAll(env, awt_lock)
#endif
#else /* ! DEBUG_AWT_LOCK */
#ifdef JNI_AWT_LOCK

#define AWT_LOCK()		(*env)->MonitorEnter(env, awt_lock)
#define AWT_NOFLUSH_UNLOCK()	(*env)->MonitorExit(env, awt_lock)
#define AWT_WAIT(tm)		JNU_MonitorWait(env, awt_lock, (tm))
#define AWT_NOTIFY()		JNU_Notify(env, awt_lock)
#define AWT_NOTIFY_ALL()	JNU_NotifyAll(env, awt_lock)

#else /* ! JNI_AWT_LOCK */

#define AWT_LOCK()		monitorEnter(obj_monitor(awt_lock))
#define AWT_NOFLUSH_UNLOCK()	monitorExit(obj_monitor(awt_lock))
#define AWT_WAIT(tm)		monitorWait(obj_monitor(awt_lock), (tm))
#define AWT_NOTIFY()		monitorNotify(obj_monitor(awt_lock))
#define AWT_NOTIFY_ALL()	monitorNotifyAll(obj_monitor(awt_lock))

#endif /* ! JNI_AWT_LOCK */
#endif /* ! DEBUG_AWT_LOCK */
#endif /* ! NETSCAPE */

#ifndef HEADLESS
extern Display	       *awt_display;		/* awt_GraphicsEnv.c */
extern XtAppContext	awt_appContext;		/* awt_MToolkit.c */
extern Widget           awt_root_shell;
extern Pixel		awt_defaultBg;
extern Pixel		awt_defaultFg;
extern int		awt_multiclick_time;	/* awt_MToolkit.c */
extern int              awt_multiclick_smudge;	/* canvas.c */
extern unsigned int	awt_MetaMask;		/* awt_MToolkit.c */
extern unsigned int	awt_AltMask;
extern unsigned int     awt_NumLockMask;
extern unsigned int     awt_ModeSwitchMask;
extern Cursor           awt_scrollCursor;	/* awt_MToolkit.c */

#endif /* !HEADLESS */

#endif /* ! _AWT_ */
