/*
 * @(#)XToolkit.c	1.41 04/07/26
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>


#include <jvm.h>
#include <jni.h>
#include <jlong.h>
#include <jni_util.h>

#ifndef HEADLESS
#include "GLXSurfaceData.h"
#endif

#include "awt_p.h"
#include "awt_Component.h"
#include "awt_MenuComponent.h"
#include "awt_KeyboardFocusManager.h"
#include "awt_Font.h"

#include "sun_awt_X11_XToolkit.h"
#include "java_awt_SystemColor.h"

uint32_t awt_NumLockMask = 0;
#define SPECIAL_KEY_EVENT 2

extern JavaVM *jvm;


#ifdef DEBUG_AWT_LOCK

int32_t awt_locked = 0;
char *lastF = "";
int32_t lastL = -1;

#endif

struct ComponentIDs componentIDs;

struct MenuComponentIDs menuComponentIDs;

struct KeyboardFocusManagerIDs keyboardFocusManagerIDs;

static jfieldID pData=NULL;
jobject awt_lock;



#ifndef HEADLESS

extern Display* awt_init_Display(JNIEnv *env, jobject this);

extern struct MFontPeerIDs mFontPeerIDs;

JNIEXPORT void JNICALL
Java_sun_awt_X11_XFontPeer_initIDs
  (JNIEnv *env, jclass cls)
{
    mFontPeerIDs.xfsname =
      (*env)->GetFieldID(env, cls, "xfsname", "Ljava/lang/String;");
}
#endif /* !HEADLESS */

/* This function gets called from the static initializer for FileDialog.java
   to initialize the fieldIDs for fields that may be accessed from C */

JNIEXPORT void JNICALL
Java_java_awt_FileDialog_initIDs
  (JNIEnv *env, jclass cls)
{

}

JNIEXPORT void JNICALL
Java_sun_awt_X11_XToolkit_initIDs
  (JNIEnv *env, jclass clazz)
{
    /* printf("Java_sun_awt_X11_XToolkit_initIDs\n");     */
    jfieldID fid = (*env)->GetStaticFieldID(env, clazz, "numLockMask", "I"); 
    awt_NumLockMask = (*env)->GetStaticIntField(env, clazz, fid); 
    DTRACE_PRINTLN1("awt_NumLockMask = %u", awt_NumLockMask); 
}

/*
 * Class:     sun_awt_X11_XToolkit
 * Method:    getDefaultXColormap
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_sun_awt_X11_XToolkit_getDefaultXColormap
  (JNIEnv *env, jclass clazz) {
    AwtGraphicsConfigDataPtr defaultConfig =
        getDefaultConfig(DefaultScreen(awt_display));
    
    return (jlong) defaultConfig->awt_cmap;
  }

JNIEXPORT jlong JNICALL Java_sun_awt_X11_XToolkit_xGetDisplay (JNIEnv *env, jclass class) {
    
    if (awt_display) {
	return (jlong) awt_display;
    }
    else {
        awt_init_Display(env, class);
	return (jlong) awt_display;
    }
}


JNIEXPORT jobject JNICALL Java_sun_awt_X11_XToolkit_makeColorModel
  (JNIEnv *env, jclass clasz) 
{
    AwtGraphicsConfigDataPtr defaultConfig =
        getDefaultConfig(DefaultScreen(awt_display));
    
    return awtJNI_GetColorModel(env, defaultConfig);
}

JNIEXPORT jlong JNICALL Java_sun_awt_X11_XToolkit_getDefaultScreenData
  (JNIEnv *env, jclass clazz)
{
    return (jlong) getDefaultConfig(DefaultScreen(awt_display));
}


JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved)
{
    jvm = vm;
 return JNI_VERSION_1_2;

}

/*
 * Class:     sun_awt_X11_XToolkit
 * Method:    nativeLoadSystemColors
 * Signature: ([I)V
 */
JNIEXPORT void JNICALL Java_sun_awt_X11_XToolkit_nativeLoadSystemColors
  (JNIEnv *env, jobject this, jintArray systemColors) 
{
    AwtGraphicsConfigDataPtr defaultConfig =
        getDefaultConfig(DefaultScreen(awt_display));
    awtJNI_CreateColorData(env, defaultConfig, 1);
}

JNIEXPORT void JNICALL
Java_java_awt_Component_initIDs
  (JNIEnv *env, jclass cls)
{
    jclass keyclass = NULL;


    componentIDs.x = (*env)->GetFieldID(env, cls, "x", "I");
    componentIDs.y = (*env)->GetFieldID(env, cls, "y", "I");
    componentIDs.width = (*env)->GetFieldID(env, cls, "width", "I");
    componentIDs.height = (*env)->GetFieldID(env, cls, "height", "I");
    componentIDs.isPacked = (*env)->GetFieldID(env, cls, "isPacked", "Z");
    componentIDs.peer =
      (*env)->GetFieldID(env, cls, "peer", "Ljava/awt/peer/ComponentPeer;");
    componentIDs.background =
      (*env)->GetFieldID(env, cls, "background", "Ljava/awt/Color;");
    componentIDs.foreground =
      (*env)->GetFieldID(env, cls, "foreground", "Ljava/awt/Color;");
    componentIDs.graphicsConfig =
        (*env)->GetFieldID(env, cls, "graphicsConfig", 
                           "Ljava/awt/GraphicsConfiguration;");
    componentIDs.privateKey =
        (*env)->GetFieldID(env, cls, "privateKey", 
                           "Ljava/lang/Object;");
    componentIDs.name =
      (*env)->GetFieldID(env, cls, "name", "Ljava/lang/String;");

    /* Use _NoClientCode() methods for trusted methods, so that we
     *  know that we are not invoking client code on trusted threads
     */
    componentIDs.getParent = 
      (*env)->GetMethodID(env, cls, "getParent_NoClientCode",
                         "()Ljava/awt/Container;");

    componentIDs.getLocationOnScreen = 
      (*env)->GetMethodID(env, cls, "getLocationOnScreen_NoTreeLock",
                         "()Ljava/awt/Point;");

    componentIDs.resetGCMID = 
      (*env)->GetMethodID(env, cls, "resetGC", "()V");

    keyclass = (*env)->FindClass(env, "java/awt/event/KeyEvent");
    DASSERT (keyclass != NULL);

    componentIDs.isProxyActive = 
        (*env)->GetFieldID(env, keyclass, "isProxyActive", 
                           "Z");

    componentIDs.appContext =
        (*env)->GetFieldID(env, cls, "appContext", 
                           "Lsun/awt/AppContext;");

    (*env)->DeleteLocalRef(env, keyclass);

    DASSERT(componentIDs.resetGCMID);

}


JNIEXPORT void JNICALL
Java_java_awt_Container_initIDs
  (JNIEnv *env, jclass cls)
{

}


JNIEXPORT void JNICALL
Java_java_awt_Button_initIDs
  (JNIEnv *env, jclass cls)
{

}

JNIEXPORT void JNICALL
Java_java_awt_Scrollbar_initIDs
  (JNIEnv *env, jclass cls)
{

}


JNIEXPORT void JNICALL
Java_java_awt_Window_initIDs
  (JNIEnv *env, jclass cls)
{

}

JNIEXPORT void JNICALL
Java_java_awt_Frame_initIDs
  (JNIEnv *env, jclass cls)
{

}


JNIEXPORT void JNICALL
Java_java_awt_MenuComponent_initIDs(JNIEnv *env, jclass cls)
{
    menuComponentIDs.appContext =
      (*env)->GetFieldID(env, cls, "appContext", "Lsun/awt/AppContext;");
}

JNIEXPORT void JNICALL
Java_java_awt_Cursor_initIDs(JNIEnv *env, jclass cls)
{
}


JNIEXPORT void JNICALL Java_java_awt_MenuItem_initIDs
  (JNIEnv *env, jclass cls)
{
}


JNIEXPORT void JNICALL Java_java_awt_Menu_initIDs
  (JNIEnv *env, jclass cls)
{
}

JNIEXPORT void JNICALL
Java_java_awt_TextArea_initIDs
  (JNIEnv *env, jclass cls)
{
}


JNIEXPORT void JNICALL
Java_java_awt_Checkbox_initIDs
  (JNIEnv *env, jclass cls)
{
}


JNIEXPORT void JNICALL Java_java_awt_ScrollPane_initIDs
  (JNIEnv *env, jclass cls)
{
}

JNIEXPORT void JNICALL
Java_java_awt_TextField_initIDs
  (JNIEnv *env, jclass cls)
{
}

/*
 * Class:     sun_awt_SunToolkit
 * Method:    getPrivateKey
 * Signature: (Ljava/lang/Object;)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_sun_awt_SunToolkit_getPrivateKey(JNIEnv *env, jclass cls, jobject obj) {
    jobject key = obj;

    static jclass componentCls = NULL;
    static jclass menuComponentCls = NULL;

    // get global reference of java/awt/Component class (run only once)
    if (componentCls == NULL) {
        jclass componentClsLocal = (*env)->FindClass(env, "java/awt/Component");
        DASSERT(componentClsLocal != NULL);
        if (componentClsLocal == NULL) {
            /* exception already thrown */
            return key;
        }
    componentCls = (jclass)(*env)->NewGlobalRef(env, componentClsLocal);
        (*env)->DeleteLocalRef(env, componentClsLocal);
    }

    // get global reference of java/awt/MenuComponent class (run only once)
    if (menuComponentCls == NULL) {
        jclass menuComponentClsLocal = (*env)->FindClass(env, "java/awt/MenuComponent");
        DASSERT(menuComponentClsLocal != NULL);
        if (menuComponentClsLocal == NULL) {
            /* exception already thrown */
            return key;
        }
    menuComponentCls = (jclass)(*env)->NewGlobalRef(env, menuComponentClsLocal);
        (*env)->DeleteLocalRef(env, menuComponentClsLocal);
    }

    /*
     * Fix for BugTraq ID 4254701.
     * Don't use Components and MenuComponents as keys in hash maps.
     * We use private keys instead.
     */
    if ((*env)->IsInstanceOf(env, obj, componentCls)) {
    key = (*env)->GetObjectField(env, obj, componentIDs.privateKey);
    } 
    return key;
}


JNIEXPORT jboolean JNICALL AWTIsHeadless() {
#ifdef HEADLESS
    return JNI_TRUE;
#else
    return JNI_FALSE;
#endif
}



/*
 * Class:     sun_awt_SunToolkit
 * Method:    wakeupEventQueue
 * Signature: (Ljava/awt/EventQueue;Z)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_SunToolkit_wakeupEventQueue(JNIEnv *env, jclass cls, jobject eq, jboolean b)
{

    // get global reference of java/awt/EventQueue class and its wakeup method
    // (run only once)
    static jclass eventQueueCls = NULL;
    static jmethodID wakeupMethodID = NULL;
    if (eventQueueCls == NULL) {
        jclass eventQueueClsLocal = (*env)->FindClass(env, "java/awt/EventQueue");
        DASSERT(eventQueueClsLocal != NULL);
        if (eventQueueClsLocal == NULL) {
            /* exception already thrown */
            return;
        }
        eventQueueCls = (jclass)(*env)->NewGlobalRef(env, eventQueueClsLocal);
        (*env)->DeleteLocalRef(env, eventQueueClsLocal);

        wakeupMethodID = (*env)->GetMethodID(env, eventQueueCls,
                         "wakeup", "(Z)V");
        DASSERT(wakeupMethodID != NULL);
        if (wakeupMethodID == NULL) {
            /* exception already thrown */
            return;
        }
    }

    DASSERT((*env)->IsInstanceOf(env, eq, eventQueueCls));
    (*env)->CallVoidMethod(env, eq, wakeupMethodID, b);
}



JNIEXPORT void JNICALL Java_java_awt_Dialog_initIDs (JNIEnv *env, jclass cls)
{
}


/* Begin poll section. */


/* #include "awt_p.h" */

#include <sys/time.h>
#include <limits.h>
#include <locale.h>

static void awt_pipe_init(void);
static void processOneEvent(XtInputMask iMask);
extern void waitForEvents(JNIEnv *env, int32_t fdXPipe, int32_t fdAWTPipe);
#ifdef USE_SELECT
static void performSelect(JNIEnv *env, int32_t fdXPipe, int32_t fdAWTPipe);
#else
static void performPoll(JNIEnv *env,int32_t fdXPipe, int32_t fdAWTPipe);
#endif

#include <dlfcn.h>
#include <fcntl.h>

#ifdef USE_SELECT
#if defined(AIX)
#include <sys/select.h>
#endif
#else
#include <poll.h>
#ifndef POLLRDNORM
#define POLLRDNORM POLLIN
#endif
#endif

#ifndef bzero
#define bzero(a,b) memset(a, 0, b)
#endif

/* implement a "putback queue" -- see comments on awt_put_back_event() */
#define PUTBACK_QUEUE_MIN_INCREMENT 5   /* min size increase */
static XEvent *putbackQueue = NULL; /* the queue -- next event is 0 */
static int32_t putbackQueueCount = 0;   /* # of events available on queue */
static int32_t putbackQueueCapacity = 0;    /* total capacity of queue */
static XtInputMask awt_events_pending(XtAppContext appContext);
static int32_t awt_get_next_put_back_event(XEvent *xev_out);
 
extern jboolean needGLFlush;

#define AWT_FLUSH_TIMEOUT    ((uint32_t)100) /* milliseconds */
#define AWT_MIN_POLL_TIMEOUT ((uint32_t)0) /* milliseconds */
#define AWT_MAX_POLL_TIMEOUT ((uint32_t)250) /* milliseconds */

#define AWT_POLL_BUFSIZE        100
#define AWT_READPIPE            (awt_pipe_fds[0])
#define AWT_WRITEPIPE           (awt_pipe_fds[1])
#define AWT_FLUSHOUTPUT_NOW()                    \
{                                                \
    if (needGLFlush) {                           \
        OGLContext_InvokeGLFlush(env);           \
        needGLFlush = JNI_FALSE;                 \
    }                                            \
    XFlush(awt_display);                         \
    awt_next_flush_time = 0LL;                   \
}

static jobject  awt_MainThread = NULL;
static char     read_buf[AWT_POLL_BUFSIZE + 1];    /* dummy buf to empty pipe */
static int32_t      awt_pipe_fds[2];                   /* fds for wkaeup pipe */
static Boolean  awt_pipe_inited = False;           /* make sure pipe is initialized before write */
static int32_t      def_poll_timeout = AWT_MAX_POLL_TIMEOUT;   /* default value for timeout */
static jlong awt_next_flush_time = 0LL; /* 0 == no scheduled flush */
static uint32_t curPollTimeout = AWT_MAX_POLL_TIMEOUT;
#ifdef DEBUG
static int32_t debugPrintLineCount = 0;   /* limit debug output per line */
#endif


static void
awt_set_poll_timeout (uint32_t newTimeout)
{
    DTRACE_PRINTLN1("awt_set_poll_timeout(%lu)", newTimeout);

    newTimeout = max(AWT_MIN_POLL_TIMEOUT, newTimeout);
    newTimeout = min(AWT_MAX_POLL_TIMEOUT, newTimeout);
    newTimeout = min(newTimeout, curPollTimeout);
    curPollTimeout = newTimeout;

} /* awt_set_poll_timeout */

static jlong 
awtJNI_TimeMillis(void)
{
    struct timeval t;

    gettimeofday(&t, 0);

    return jlong_add(jlong_mul(jint_to_jlong(t.tv_sec), jint_to_jlong(1000)),
             jint_to_jlong(t.tv_usec / 1000));
}

/*
 * Gets the best timeout for the next call to poll() or select().
 * If timedOut is True, we assume that our previous timeout elapsed
 * with no events/timers arriving. Therefore, we can increase the
 * next timeout slightly.
 */
static uint32_t
awt_get_poll_timeout( Boolean timedOut )
{
    uint32_t timeout = AWT_MAX_POLL_TIMEOUT;

    DTRACE_PRINTLN2("awt_get_poll_timeout(%s), awt_next_flush_time:%ld",
    (remove?"true":"false"), 
    awt_next_flush_time);

    if (timedOut) {
    /* add 1/16 (plus 1, in case the division truncates to 0) */
    curPollTimeout += ((curPollTimeout>>4) + 1);
        curPollTimeout = min(AWT_MAX_POLL_TIMEOUT, curPollTimeout);
    }
    if (awt_next_flush_time > 0) {
    int32_t flushDiff = (int32_t)(awt_next_flush_time - awtJNI_TimeMillis());
    timeout = min(curPollTimeout, flushDiff);
    } else {
    timeout = curPollTimeout;
    }

    return timeout;
} /* awt_get_poll_timeout() */

/*
 * Waits for X/Xt events to appear on the pipe. Returns only when
 * it is likely (but not definite) that there are events waiting to
 * be processed.
 *
 * This routine also flushes the outgoing X queue, when the 
 * awt_next_flush_time has been reached.
 *
 * If fdAWTPipe is greater or equal than zero the routine also 
 * checks if there are events pending on the putback queue.
 */
static int32_t  fdXPipe = -1;
void
waitForEvents(JNIEnv *env, int32_t fdXPipe, int32_t fdAWTPipe) {

#if 0
        if (!awt_pipe_inited) {
            awt_pipe_init();
            /* The pipe where X events arrive */
            fdXPipe = ConnectionNumber(awt_display) ;
        }
#endif

        if (fdXPipe == -1) {
            /* The pipe where X events arrive */
            fdXPipe = ConnectionNumber(awt_display) ;
        }

/*      while ((fdAWTPipe >= 0 && awt_events_pending(awt_appContext) == 0) ||
               (fdAWTPipe <  0 && XtAppPending(awt_appContext) == 0)) {
*/
//        while ((XEventsQueued(awt_display, QueuedAfterReading) == 0) &&
//               (XEventsQueued(awt_display, QueuedAfterFlush) == 0)) {
#ifdef USE_SELECT
        performSelect(env,fdXPipe,fdAWTPipe);
#else
        performPoll(env,fdXPipe,fdAWTPipe);
#endif
        if ((awt_next_flush_time > 0) && (awtJNI_TimeMillis() > awt_next_flush_time)) {
            AWT_FLUSHOUTPUT_NOW();
        }
//        }  /* end while awt_events_pending() == 0 */ 
} /* waitForEvents() */

JNIEXPORT void JNICALL Java_sun_awt_X11_XToolkit_waitForEvents (JNIEnv *env, jclass class) {
    waitForEvents(env, fdXPipe, AWT_READPIPE);
}

/*************************************************************************
 **                                 **
 ** WE USE EITHER select() OR poll(), DEPENDING ON THE USE_SELECT       **
 ** COMPILE-TIME CONSTANT.                      **
 **                                 **
 *************************************************************************/

#ifdef USE_SELECT

static struct fd_set rdset;
struct timeval sel_time;

/*
 * Performs select() on both the X pipe and our AWT utility pipe. 
 * Returns when data arrives or the operation times out.
 *
 * Not all Xt events come across the X pipe (e.g., timers
 * and alternate inputs), so we must time out every now and 
 * then to check the Xt event queue.
 *
 * The fdAWTPipe will be empty when this returns.
 */
static void
performSelect(JNIEnv *env, int32_t fdXPipe, int32_t fdAWTPipe) {

        int32_t result;
        int32_t count;
        int32_t nfds = 1;
        uint32_t timeout = awt_get_poll_timeout(False);

            /* Fixed 4250354 7/28/99 ssi@sparc.spb.su
         * Cleaning up Global Refs in case of No Events
         */
/*      awtJNI_CleanupGlobalRefs();
*/
        
            FD_ZERO( &rdset );
            FD_SET(fdXPipe, &rdset);
        if (fdAWTPipe >= 0) {
        nfds++;
                FD_SET(fdAWTPipe, &rdset);
        }
        if (timeout == 0) {
            // be sure other threads get a chance
            awtJNI_ThreadYield(env);
        }
        // set the appropriate time values. The DASSERT() in
        // MToolkit_run() makes sure that this will not overflow
        sel_time.tv_sec = (timeout * 1000) / (1000 * 1000);
        sel_time.tv_usec = (timeout * 1000) % (1000 * 1000);
        AWT_NOFLUSH_UNLOCK();
            result = select(nfds, &rdset, 0, 0, &sel_time);
        AWT_LOCK();

        /* reset tick if this was not a time out */
        if (result == 0) {
        /* select() timed out -- update timeout value */
        awt_get_poll_timeout(True);
        }
        if (fdAWTPipe >= 0 && FD_ISSET ( fdAWTPipe, &rdset ) )
        {
        /* There is data on the AWT pipe - empty it */
            do {
                count = read(fdAWTPipe, read_buf, AWT_POLL_BUFSIZE );
            } while (count == AWT_POLL_BUFSIZE ); 
        }
} /* performSelect() */

#else /* !USE_SELECT */

/*
 * Polls both the X pipe and our AWT utility pipe. Returns
 * when there is data on one of the pipes, or the operation times 
 * out.
 *
 * Not all Xt events come across the X pipe (e.g., timers
 * and alternate inputs), so we must time out every now and 
 * then to check the Xt event queue.
 *
 * The fdAWTPipe will be empty when this returns.
 */
static void
performPoll(JNIEnv *env, int32_t fdXPipe, int32_t fdAWTPipe) {

            static struct pollfd pollFds[2];
        uint32_t timeout = 1;
        int32_t result;
        int32_t count;

            /* Fixed 4250354 7/28/99 ssi@sparc.spb.su
         * Cleaning up Global Refs in case of No Events
         */
//      awtJNI_CleanupGlobalRefs();

            pollFds[0].fd = fdXPipe;
            pollFds[0].events = POLLRDNORM;
        pollFds[0].revents = 0;

/*
        pollFds[1].fd = fdAWTPipe;
            pollFds[1].events = POLLRDNORM;
        pollFds[1].revents = 0;
*/

        AWT_NOFLUSH_UNLOCK();

        /* print the poll timeout time in brackets */
        DTRACE_PRINT1("[%dms]",(int32_t)timeout);
#ifdef DEBUG        
        if (++debugPrintLineCount > 8) {
        DTRACE_PRINTLN("");
        debugPrintLineCount = 0;
        }
#endif
        /* ACTUALLY DO THE POLL() */
        if (timeout == 0) {
            // be sure other threads get a chance
            awtJNI_ThreadYield(env);
        }
        result = poll( pollFds, 1, (int32_t) timeout );
//      result = poll( pollFds, 2, (int32_t) timeout );

#ifdef DEBUG        
        DTRACE_PRINT1("[poll()->%d]", result);
        if (++debugPrintLineCount > 8) {
        DTRACE_PRINTLN("");
        debugPrintLineCount = 0;
        }
#endif
        AWT_LOCK();
        if (result == 0) {
            /* poll() timed out -- update timeout value */
        awt_get_poll_timeout(True);
        }
        if ( pollFds[1].revents )
        {
        /* There is data on the AWT pipe - empty it */
            do {
                count = read(AWT_READPIPE, read_buf, AWT_POLL_BUFSIZE );
            } while (count == AWT_POLL_BUFSIZE ); 
        DTRACE_PRINTLN1("wokeup on AWTPIPE, timeout:%d", timeout);
        }
        return;

} /* performPoll() */

#endif /* !USE_SELECT */


void awt_output_flush() {
    char c = 'p';
    if (awt_next_flush_time == 0) {
	Boolean needsWakeup = False;
        JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
	if (awt_pipe_inited && (awt_get_poll_timeout(False) > (2*AWT_FLUSH_TIMEOUT))) {
	    needsWakeup = True;
	}
	/* awt_next_flush_time affects awt_get_poll_timeout(), so set
	 * the variable *after* calling the function.
	 */
    	awt_next_flush_time = awtJNI_TimeMillis() + AWT_FLUSH_TIMEOUT;
	if (needsWakeup) {
	    /* write to the utility pipe to wake up the event
	     * loop, if it's sleeping
	     */
	    write(AWT_WRITEPIPE, &c, 1);
	}
    }
}


/*
 * Pushes an X event back on the queue to be handled
 * later.
 *
 * Ignores the request if event is NULL
 */
void
awt_put_back_event(JNIEnv *env, XEvent *event) {

    Boolean addIt = True;
    if (putbackQueueCount >= putbackQueueCapacity) {
    /* not enough room - alloc 50% more space */
    int32_t newCapacity;
    XEvent *newQueue;
    newCapacity = putbackQueueCapacity * 3 / 2;
    if ((newCapacity - putbackQueueCapacity) 
                    < PUTBACK_QUEUE_MIN_INCREMENT) {
        /* always increase by at least min increment */
        newCapacity = putbackQueueCapacity + PUTBACK_QUEUE_MIN_INCREMENT;
    }
    newQueue = (XEvent*)realloc(
            putbackQueue, newCapacity*(sizeof(XEvent)));
    if (newQueue == NULL) {
        JNU_ThrowOutOfMemoryError(env, "OutOfMemoryError");
        addIt = False;
    } else {
        putbackQueue = newQueue;
        putbackQueueCapacity = newCapacity;
    }
    }
    if (addIt) {
    char oneChar = 'p';
    memcpy(&(putbackQueue[putbackQueueCount]), event, sizeof(XEvent));
    putbackQueueCount++;

    // wake up the event loop, if it's sleeping
        write (AWT_WRITEPIPE, &oneChar, 1);
    }

    return;
} /* awt_put_back_event() */

/*
 * Gets the next event that has been pushed back onto the queue. 
 * Returns 0 and fills in xev_out if successful
 */
static int32_t
awt_get_next_put_back_event(XEvent *xev_out) {

    Boolean err = False;
    if (putbackQueueCount < 1) {
    err = True;
    } else {
    memcpy(xev_out, &(putbackQueue[0]), sizeof(XEvent));
    }
    if (!err) {
    /* remove it from the queue */
    if (putbackQueueCount == 1) {

        // queue is now empty
        if (putbackQueueCapacity > PUTBACK_QUEUE_MIN_INCREMENT) {

        /* Too much space -- delete it and rebuild later */
        free(putbackQueue);
        putbackQueue = NULL;
        putbackQueueCapacity = 0;
        }
    } else {
        /* more than 1 event in queue - shift all events to the left */
        /* We don't free the allocated memory until the queue
           becomes empty, just 'cause it's easier that way. */
        /* NOTE: use memmove(), because the memory blocks overlap */
        memmove(&(putbackQueue[0]), &(putbackQueue[1]), 
        (putbackQueueCount-1)*sizeof(XEvent));
    }
    --putbackQueueCount;
    }
    DASSERT(putbackQueueCount >= 0);

    return (err? -1:0);
    
} /* awt_get_next_put_back_event() */

/**
 * Determines whether or not there are X or Xt events pending.
 * Looks at the putbackQueue.
 */
#if 0
static XtInputMask 
awt_events_pending(XtAppContext appContext) {
    XtInputMask imask = 0L;
    imask = XtAppPending(appContext);
    if (putbackQueueCount > 0) {
    imask |= XtIMXEvent;
    }
    return imask;
}
#endif

/*
 * Creates the AWT utility pipe. This pipe exists solely so that
 * we can cause the main event thread to wake up from a poll() or
 * select() by writing to this pipe.
 */
static void
awt_pipe_init(void) {

    if (awt_pipe_inited) {
    return;
    }

    if ( pipe ( awt_pipe_fds ) == 0 )
    {
    /* 
    ** the write wakes us up from the infinite sleep, which 
    ** then we cause a delay of AWT_FLUSHTIME and then we 
    ** flush.
    */
    int32_t flags = 0;
    awt_set_poll_timeout (def_poll_timeout);
    /* set the pipe to be non-blocking */
    flags = fcntl ( AWT_READPIPE, F_GETFL, 0 );
    fcntl( AWT_READPIPE, F_SETFL, flags | O_NDELAY | O_NONBLOCK ); 
    flags = fcntl ( AWT_WRITEPIPE, F_GETFL, 0 );
    fcntl( AWT_WRITEPIPE, F_SETFL, flags | O_NDELAY | O_NONBLOCK );
        awt_pipe_inited = True;
    }
    else
    {
    AWT_READPIPE = -1;
    AWT_WRITEPIPE = -1;
        awt_pipe_inited = False;
    }
} /* awt_pipe_init() */


/* End poll section. */

/*
 * Class:     java_awt_KeyboardFocusManager
 * Method:    initIDs
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_java_awt_KeyboardFocusManager_initIDs
    (JNIEnv *env, jclass cls)
{
}


/*
 * Returns a reference to the class java.awt.Component.
 */
jclass
getComponentClass(JNIEnv *env)
{
    static jclass componentCls = NULL;

    // get global reference of java/awt/Component class (run only once)
    if (componentCls == NULL) {
        jclass componentClsLocal = (*env)->FindClass(env, "java/awt/Component");
        DASSERT(componentClsLocal != NULL);
        if (componentClsLocal == NULL) {
            /* exception already thrown */
            return NULL;
        }
	componentCls = (jclass)(*env)->NewGlobalRef(env, componentClsLocal);
        (*env)->DeleteLocalRef(env, componentClsLocal);
    }
    return componentCls;
}


/*
 * Returns a reference to the class java.awt.MenuComponent.
 */
jclass
getMenuComponentClass(JNIEnv *env)
{
    static jclass menuComponentCls = NULL;

    // get global reference of java/awt/MenuComponent class (run only once)
    if (menuComponentCls == NULL) {
        jclass menuComponentClsLocal = (*env)->FindClass(env, "java/awt/MenuComponent");
        DASSERT(menuComponentClsLocal != NULL);
        if (menuComponentClsLocal == NULL) {
            /* exception already thrown */
            return NULL;
        }
	menuComponentCls = (jclass)(*env)->NewGlobalRef(env, menuComponentClsLocal);
        (*env)->DeleteLocalRef(env, menuComponentClsLocal);
    }
    return menuComponentCls;
}

/*
 * Class:     sun_awt_SunToolkit
 * Method:    getAppContext
 * Signature: (Ljava/awt/Object;)Lsun/awt/AppContext;
 */
JNIEXPORT jobject JNICALL
Java_sun_awt_SunToolkit_getAppContext(JNIEnv *env, jclass cls, jobject obj)
{
    jobject appContext = NULL;

    if ((*env)->IsInstanceOf(env, obj, getComponentClass(env))) {
	appContext = (*env)->GetObjectField(env, obj, componentIDs.appContext);
    } else if ((*env)->IsInstanceOf(env, obj, getMenuComponentClass(env))) {
	appContext = (*env)->GetObjectField(env, obj,
					    menuComponentIDs.appContext);
    }
    return appContext;
}

/*
 * Class:     sun_awt_SunToolkit
 * Method:    setAppContext
 * Signature: (Ljava/lang/Object;Lsun/awt/AppContext;)Z
 */
JNIEXPORT jboolean JNICALL
Java_sun_awt_SunToolkit_setAppContext(JNIEnv *env, jclass cls, jobject comp,
				      jobject appContext)
{
    jboolean isComponent;
    if ((*env)->IsInstanceOf(env, comp, getComponentClass(env))) {
	(*env)->SetObjectField(env, comp, componentIDs.appContext, appContext);
	isComponent = JNI_TRUE;
    } else if ((*env)->IsInstanceOf(env, comp, getMenuComponentClass(env))) {
	(*env)->SetObjectField(env, comp, menuComponentIDs.appContext,
			       appContext);
	isComponent = JNI_TRUE;
    } else {
        isComponent = JNI_FALSE;
    }
    return isComponent;
}

/*
 * Class:     sun_awt_X11_XToolkit
 * Method:    getEnv
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_sun_awt_X11_XToolkit_getEnv
(JNIEnv *env , jclass clazz, jstring key) {
    char *ptr = NULL;
    const char *keystr = NULL;
    jstring ret = NULL;
    
    keystr = JNU_GetStringPlatformChars(env, key, NULL);
    if (key) {
        ptr = getenv(keystr);
        if (ptr) {
            ret = JNU_NewStringPlatform(env, (const char *) ptr);
        }
        JNU_ReleaseStringPlatformChars(env, key, (const char*)keystr);
    }
    return ret;
}

Window get_xawt_root_shell(JNIEnv *env) {
  static jclass classXRootWindow = NULL;
  static jmethodID methodGetXRootWindow = NULL;
  static Window xawt_root_shell = None;

  if (xawt_root_shell == None){
      if (classXRootWindow == NULL){
          jclass cls_tmp = (*env)->FindClass(env, "sun/awt/X11/XRootWindow");
          classXRootWindow = (jclass)(*env)->NewGlobalRef(env, cls_tmp);
          (*env)->DeleteLocalRef(env, cls_tmp);
      }
      if( classXRootWindow != NULL) {
          methodGetXRootWindow = (*env)->GetStaticMethodID(env, classXRootWindow, "getXRootWindow", "()J");
      }
      if( classXRootWindow != NULL && methodGetXRootWindow !=NULL ) {
          xawt_root_shell = (Window) (*env)->CallStaticLongMethod(env, classXRootWindow, methodGetXRootWindow);
      }
      if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
      }
  }
  return xawt_root_shell;
}

/*
 * Old, compatibility, backdoor for DT.  This is a different
 * implementation.  It keeps the signature, but acts on
 * awt_root_shell, not the frame passed as an argument.  Note, that
 * the code that uses the old backdoor doesn't work correctly with
 * gnome session proxy that checks for WM_COMMAND when the window is
 * firts mapped, because DT code calls this old backdoor *after* the
 * frame is shown or it would get NPE with old AWT (previous
 * implementation of this backdoor) otherwise.  Old style session
 * managers (e.g. CDE) that check WM_COMMAND only during session
 * checkpoint should work fine, though.
 *
 * NB: The function name looks deceptively like a JNI native method
 * name.  It's not!  It's just a plain function.
 */

JNIEXPORT void JNICALL
Java_sun_awt_motif_XsessionWMcommand(JNIEnv *env, jobject this,
    jobject frame, jstring jcommand)
{
    const char *command;
    XTextProperty text_prop;
    char *c[1];
    int32_t status;
    Window xawt_root_window;
    
    AWT_LOCK();
    xawt_root_window = get_xawt_root_shell(env);

    if ( xawt_root_window == None ) {
	JNU_ThrowNullPointerException(env, "AWT root shell is unrealized");
	AWT_UNLOCK();
	return;
    }

    command = (char *) JNU_GetStringPlatformChars(env, jcommand, NULL);
    c[0] = (char *)command;
    status = XmbTextListToTextProperty(awt_display, c, 1,
				       XStdICCTextStyle, &text_prop);

    if (status == Success || status > 0) {
	XSetTextProperty(awt_display, xawt_root_window,
			 &text_prop, XA_WM_COMMAND);
	if (text_prop.value != NULL)
            XFree(text_prop.value);
    }
    JNU_ReleaseStringPlatformChars(env, jcommand, command);
    AWT_UNLOCK();
}


/*
 * New DT backdoor to set WM_COMMAND.  New code should use this
 * backdoor and call it *before* the first frame is shown so that
 * gnome session proxy can correctly handle it.
 *
 * NB: The function name looks deceptively like a JNI native method
 * name.  It's not!  It's just a plain function.
 */
JNIEXPORT void JNICALL
Java_sun_awt_motif_XsessionWMcommand_New(JNIEnv *env, jobjectArray jargv)
{
    static const char empty[] = "";

    int argc;
    const char **cargv;
    XTextProperty text_prop;
    int status;
    int i;
    Window xawt_root_window;

    AWT_LOCK();
    xawt_root_window = get_xawt_root_shell(env);

    if (xawt_root_window == None) {
      JNU_ThrowNullPointerException(env, "AWT root shell is unrealized");
      AWT_UNLOCK();
      return;
    }
    
    argc = (int)(*env)->GetArrayLength(env, jargv);
    if (argc == 0) {
	AWT_UNLOCK();
	return;
    }

    /* array of C strings */
    cargv = (const char **)calloc(argc, sizeof(char *));
    if (cargv == NULL) {
	JNU_ThrowOutOfMemoryError(env, "Unable to allocate cargv");
	AWT_UNLOCK();
	return;
    }

    /* fill C array with platform chars of java strings */
      for (i = 0; i < argc; ++i) {
	jstring js;
	const char *cs;

	cs = NULL;
	js = (*env)->GetObjectArrayElement(env, jargv, i);
	if (js != NULL) {
	    cs = JNU_GetStringPlatformChars(env, js, NULL);
	}
	if (cs == NULL) {
	    cs = empty;
	}
	cargv[i] = cs;
        (*env)->DeleteLocalRef(env, js);
    }

    /* grr, X prototype doesn't declare cargv as const, thought it really is */
    status = XmbTextListToTextProperty(awt_display, (char **)cargv, argc,
				       XStdICCTextStyle, &text_prop);
    if (status < 0) {
	switch (status) {
	case XNoMemory:
	    JNU_ThrowOutOfMemoryError(env,
	        "XmbTextListToTextProperty: XNoMemory");
	    break;
	case XLocaleNotSupported:
	    JNU_ThrowInternalError(env,
	        "XmbTextListToTextProperty: XLocaleNotSupported");
	    break;
	case XConverterNotFound:
	    JNU_ThrowNullPointerException(env,
	        "XmbTextListToTextProperty: XConverterNotFound");
	    break;
	default:
	    JNU_ThrowInternalError(env,
	        "XmbTextListToTextProperty: unknown error");
	}
    } else {

    XSetTextProperty(awt_display, xawt_root_window,
			 &text_prop, XA_WM_COMMAND);
    }

    for (i = 0; i < argc; ++i) {
	jstring js;

	if (cargv[i] == empty)
	    continue;

	js = (*env)->GetObjectArrayElement(env, jargv, i);
	JNU_ReleaseStringPlatformChars(env, js, cargv[i]);
        (*env)->DeleteLocalRef(env, js);
    }
    if (text_prop.value != NULL)
        XFree(text_prop.value);
    AWT_UNLOCK();
}
