/*
 * @(#)native.c	1.49 03/10/21
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


/*
 * Native Methods to support Java Plug-in on Un*x systems.
 *
 * This gets compiled with -DJDK11 for 1.1 and -DJDK12 for 1.2
 *
 *						    KGH Dec 97
 */

#include "sun_plugin_navig_motif_Plugin.h"
#include "sun_plugin_navig_motif_Worker.h"
#include "sun_plugin_viewer_MNetscapePluginContext.h"
#include "sun_plugin_JavaRunTime.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#if 0
#include <sys/socket.h>
#else
#include <stropts.h>
#endif
#include <X11/Xatom.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Vendor.h>
#include <X11/Shell.h>
#include <X11/IntrinsicP.h>
#include "plugin_defs.h"
#include <dlfcn.h>
#include "unistd.h"

/* This is the one and only AWT header file allowed. */
/*#include "awt_Plugin.h"*/

/* AWT interface functions. */
static void (* LockIt)(JNIEnv *) = NULL;
static void (* UnLockIt)(JNIEnv *) = NULL;
static void (* NoFlushUnlockIt)(JNIEnv *) = NULL;
static void *res = NULL;
static int initialized_lock = 0;
static Display *display;


static int trace_native = 1;
/*
 * Create a Java FileDescriptor object for a given Unix descriptor number
 *      fd is the file descriptor number
 */

#ifdef DO_TRACE
#define TRACE(i) fprintf(stderr, "%s\n", i);
#else
#define TRACE(i) 
#endif

/*
 * Returns a new Java string object for the specified platform string.
 */
static jstring
NewPlatformString(JNIEnv *env, char *s) {
    
    int len = (int)strlen(s);
    jclass cls;
    jmethodID mid;
    jbyteArray ary;

    if ((cls = (*env)->FindClass(env, "java/lang/String")) == NULL)
      return 0;

    if ((mid = (*env)->GetMethodID(env, cls, "<init>", "([B)V")) == NULL)
      return 0;

    ary = (*env)->NewByteArray(env, len);

    if (ary != 0) {
	jstring str = 0;
	(*env)->SetByteArrayRegion(env, ary, 0, len, (jbyte *)s);
	if (!(*env)->ExceptionOccurred(env)) {
	    str = (*env)->NewObject(env, cls, mid, ary);
	}
	(*env)->DeleteLocalRef(env, ary);
	return str;
    }
    return 0;
}
  
static void *awtHandle = NULL;

static void initAwtHandle() {
    char awtPath[MAXPATHLEN];
    const char *libname;

    libname = "libawt.so";

    sprintf(awtPath, "%s/lib/" LIBARCH "/%s",
	    getenv("JAVA_HOME"), libname);

    awtHandle = dlopen(awtPath, RTLD_LAZY);
    if (awtHandle == NULL) {
	fprintf(stderr,"reflect - bad awtHandle.\n");
	exit(123);
    }
    return;
}


#define REFLECT_VOID_FUNCTION(name, arglist, paramlist)			\
typedef name##_type arglist;						\
static void name arglist						\
{									\
    static name##_type *name##_ptr = NULL;				\
    if (name##_ptr == NULL) {						\
        if (awtHandle == NULL) {					\
	    initAwtHandle();						\
	}								\
	name##_ptr = (name##_type *) dlsym(awtHandle, #name);		\
	if (name##_ptr == NULL) {					\
	    fprintf(stderr,"reflect failed to find " #name ".\n");	\
	    exit(123);							\
	    return;							\
	}								\
    }									\
    (*name##_ptr)paramlist;						\
}

#define REFLECT_FUNCTION(return_type, name, arglist, paramlist)		\
typedef return_type name##_type arglist;				\
static return_type name arglist						\
{									\
    static name##_type *name##_ptr = NULL;				\
    if (name##_ptr == NULL) {						\
        if (awtHandle == NULL) {					\
	    initAwtHandle();						\
	}								\
	name##_ptr = (name##_type *) dlsym(awtHandle, #name);		\
	if (name##_ptr == NULL) {					\
	    fprintf(stderr,"reflect failed to find " #name ".\n");	\
	    exit(123);							\
	    return NULL;							\
	}								\
    }									\
    return (*name##_ptr)paramlist;					\
}


/*
 * These entry points must remain in libawt.so ***for Java Plugin ONLY***
 * Reflect this call over to the correct libmawt.so.
 */

REFLECT_VOID_FUNCTION(getAwtLockFunctions,
		      (void (**AwtLock)(JNIEnv *), void (**AwtUnlock)(JNIEnv *),
		       void (**AwtNoFlushUnlock)(JNIEnv *), void *reserved), 
		      (AwtLock, AwtUnlock, AwtNoFlushUnlock, reserved))

REFLECT_VOID_FUNCTION(getAwtData,
		      (int *awt_depth, Colormap *awt_cmap, Visual **awt_visual,
		       int *awt_num_colors, void *pReserved),
		      (awt_depth, awt_cmap, awt_visual,
		       awt_num_colors, pReserved))

REFLECT_FUNCTION(Display *, getAwtDisplay, (void), ())


/* The following portion of codes contains Xt dependencies and is used
 * only for NS4.x purpose.  Hence it should not
 * be compiled for Linux platform to avoid Xt/Xm link error at runtime.
 */
#if !defined(LINUX)
/* Event Handler to correct for Shell position */
static void
checkPos(Widget w, XtPointer data, XEvent *event)
{
        /* this is heinous, but necessary as we need to update
        ** the X,Y position of the shell if netscape has moved.
        ** we have to do this so that XtTranslateCoords used by
        ** popups and the like get the proper screen positions
        ** Additionally we can use XtSet/ XtMove/ConfigureWidget
        ** As the widget code will think the the shell has moved
        ** and generate a XConfigure which WILL move the window
        ** We are only trying to correct for the reparent hack.
        ** sigh.
        */

        w->core.x = event->xcrossing.x_root - event->xcrossing.x;
        w->core.y = event->xcrossing.y_root - event->xcrossing.y;
}
/* Event Handler to correct for Shell position */
static void
propertyHandler(Widget w, XtPointer data, XEvent *event)
{
        /* this is heinous, but necessary as we need to update
        ** the X,Y position of the shell is changed to wrong value.
        ** we have to do this so that XtTranslateCoords used by
        ** popups and the like get the proper screen positions
        ** Additionally we can use XtSet/ XtMove/ConfigureWidget
        ** 
        */
  int px, py;
  Window dummy;
  
  XTranslateCoordinates(display, XtWindow(w), DefaultRootWindow(display), 0,0, &px, &py, &dummy);
  
  w->core.x=px;
  w->core.y=py;
     
}
#endif	/* !defined(LINUX) */


/**
 * Return true if our parent process is still alive.
 */
JNIEXPORT jboolean JNICALL
Java_sun_plugin_navig_motif_Plugin_parentAlive(JNIEnv *env, jclass clz)
{
    pid_t ppid = getppid();

    if (ppid >= 0 && ppid < 4) {
	return JNI_FALSE;
    } else {
        return JNI_TRUE;
    }
}

/*
 * Retrieve an Environment variable.
 */
JNIEXPORT jstring JNICALL
Java_sun_plugin_navig_motif_Plugin_getenv(
		JNIEnv *env, jclass clz, jstring nameString)
{
    const char *name;  
    const char *res;
    jstring resString = 0;

    if (nameString == 0) {
	return NULL;
    }
    name = (*env)->GetStringUTFChars(env, nameString, 0);

    res = getenv(name);
    
    if (res != 0) {
      resString = NewPlatformString(env, res);
    }
   
    (*env)->ReleaseStringUTFChars(env, nameString, name);

    return resString;
}

JNIEXPORT jstring JNICALL
Java_sun_plugin_javascript_navig_JSObject_evalScript(JNIEnv *env, jobject obj,
						   jint instance, jstring s) {
    jclass clz = (*env)->FindClass(env, "sun/plugin/navig/motif/Plugin");
    jmethodID meth;
    meth = (*env)->GetStaticMethodID(env, clz, "evalString",
				     "(ILjava/lang/String;)Ljava/lang/String;");
    return (*env)->CallStaticObjectMethod(env, clz, meth, instance, s);
}

JNIEXPORT jobject JNICALL
Java_sun_plugin_navig_motif_Plugin_getPipe(JNIEnv *env, jclass clz, jint fd)
{
    jobject result;
    jclass fd_clz = (*env)->FindClass(env, "java/io/FileDescriptor");
    jfieldID field = (*env)->GetFieldID(env, fd_clz, "fd", "I");

    result = (*env)->AllocObject(env, fd_clz);

    /* On JDK1.2 the fd field gets set to fd */
    (*env)->SetIntField(env, result, field, fd);

    return result;
}

