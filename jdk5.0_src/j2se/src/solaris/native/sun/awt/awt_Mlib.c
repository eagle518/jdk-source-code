/*
 * @(#)awt_Mlib.c	1.19 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <errno.h>
#include <dlfcn.h>
#include "jni.h"
#include <jni_util.h>
#include "awt_ImagingLib.h"
#include "awt_Mlib.h"
#include "java_awt_image_BufferedImage.h"

static void start_timer(int numsec);
static void stop_timer(int numsec, int ntimes);

/*
 * This is called by awt_ImagingLib.initLib() to figure out if we
 * can use the VIS version of medialib
 */
void awt_getImagingLib(JNIEnv *env, mlibFnS_t *sMlibFns,
                      mlibSysFnS_t *sMlibSysFns) {

#ifdef __linux__
#   define ULTRA_CHIP   "sparc64"
#else
#   define ULTRA_CHIP   "sun4u"
#endif
    
    struct utsname name;
    int status;
    jstring jstr = NULL;
    mlibFnS_t *mptr;
    void *(*vPtr)();
    int (*intPtr)();
    mlib_status (*fPtr)();
    int i;
    void *handle;
    mlibSysFnS_t tempSysFns;
    static int s_timeIt = 0;
    static int s_verbose = 1;

    /*
     * Find out the machine name. If it is an SUN ultra, we
     * can use the vis library
     */
    if ((uname(&name) < 0) || (getenv("NO_VIS") != NULL) ||
        (strcmp(name.machine, ULTRA_CHIP) != 0)) {
        return;
    }
#ifdef DEBUG    
    if ((handle = dlopen("libmlib_image_v_g.so", RTLD_LAZY)) == NULL) {
#else        
    if ((handle = dlopen("libmlib_image_v.so", RTLD_LAZY)) == NULL) {
#endif        
        if (s_timeIt || s_verbose) {
            printf ("error in dlopen: %s", dlerror());
        }
        return;
    }
    
    if ((tempSysFns.createFP = (MlibCreateFP_t)dlsym(handle,
                                       "j2d_mlib_ImageCreate")) == NULL) {
        if (s_timeIt) {
            printf ("error in dlsym: %s", dlerror());
        }
        return;
    }

    if ((tempSysFns.createStructFP = (MlibCreateStructFP_t)dlsym(handle,
                                   "j2d_mlib_ImageCreateStruct")) == NULL) {
        if (s_timeIt) {
            printf ("error in dlsym: %s", dlerror());
        }
        return;
    }


    /* Set the system functions */
    *sMlibSysFns = tempSysFns;
    
    /* Loop through all of the fns and load them from the next library */
    mptr = sMlibFns;
    i = 0;
    while (mptr[i].fptr != NULL) {
        fPtr = (mlib_status (*)())dlsym(handle, mptr[i].fname);
        if (fPtr != NULL) {
            mptr[i].fptr = fPtr;
        }
	i++;
    }
}

mlib_start_timer awt_setMlibStartTimer() {
    return start_timer;
}

mlib_stop_timer awt_setMlibStopTimer() {
    return stop_timer;
}

void awt_getBIColorOrder(int type, int *colorOrder) {
    switch(type) {
    case java_awt_image_BufferedImage_TYPE_INT_ARGB:
    case java_awt_image_BufferedImage_TYPE_INT_ARGB_PRE:
        colorOrder[0] = 1;
        colorOrder[1] = 2;
        colorOrder[2] = 3;
        colorOrder[3] = 0;
        break;
    case java_awt_image_BufferedImage_TYPE_INT_BGR:
        colorOrder[0] = 2;
        colorOrder[1] = 1;
        colorOrder[2] = 0;
        break;
    case java_awt_image_BufferedImage_TYPE_4BYTE_ABGR:
    case java_awt_image_BufferedImage_TYPE_4BYTE_ABGR_PRE:
        colorOrder[0] = 3;
        colorOrder[1] = 2;
        colorOrder[2] = 1;
        colorOrder[3] = 0;
        break;
    case java_awt_image_BufferedImage_TYPE_3BYTE_BGR:
        colorOrder[0] = 2;
        colorOrder[1] = 1;
        colorOrder[2] = 0;
        break;
    case java_awt_image_BufferedImage_TYPE_INT_RGB:
        colorOrder[0] = 1;
        colorOrder[1] = 2;
        colorOrder[2] = 3;
        break;
    case java_awt_image_BufferedImage_TYPE_USHORT_565_RGB:
    case java_awt_image_BufferedImage_TYPE_USHORT_555_RGB:
        colorOrder[0] = 0;
        colorOrder[1] = 1;
        colorOrder[2] = 2;
        break;
    case java_awt_image_BufferedImage_TYPE_BYTE_GRAY:
    case java_awt_image_BufferedImage_TYPE_USHORT_GRAY:
    case java_awt_image_BufferedImage_TYPE_BYTE_BINARY:
    case java_awt_image_BufferedImage_TYPE_BYTE_INDEXED:
        colorOrder[0] = 0;
        break;
    }
}
        
/***************************************************************************
 *                          Static Functions                               *
 ***************************************************************************/

static void start_timer(int numsec)
{
    struct itimerval interval;

    interval.it_interval.tv_sec = numsec;
    interval.it_interval.tv_usec = 0;
    interval.it_value.tv_sec = numsec;
    interval.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &interval, 0);
}


static void stop_timer(int numsec, int ntimes)
{
    struct itimerval interval;
    double sec;

    getitimer(ITIMER_REAL, &interval);
    sec = (((double) (numsec - 1)) - (double) interval.it_value.tv_sec) +
            (1000000.0 - interval.it_value.tv_usec)/1000000.0;
    sec = sec/((double) ntimes);
    printf("%f msec per update\n", sec * 1000.0);
    interval.it_interval.tv_sec = 0;
    interval.it_interval.tv_usec = 0;
    interval.it_value.tv_sec = 0;
    interval.it_value.tv_usec = 0;
    setitimer(ITIMER_PROF, &interval, 0);
}



