/*
 * @(#)io_util_md.c	1.9 04/01/08
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jni_util.h"
#include "jvm.h"
#include "io_util.h"
#include "io_util_md.h"
#include <string.h>

void
fileOpen(JNIEnv *env, jobject this, jstring path, jfieldID fid, int flags)
{
    WITH_PLATFORM_STRING(env, path, ps) {
        FD fd;

#ifdef __linux__
        /* Remove trailing slashes, since the kernel won't */
        char *p = (char *)ps + strlen(ps) - 1;
        while ((p > ps) && (*p == '/'))
            *p-- = '\0';
#endif
        fd = JVM_Open(ps, flags, 0666);
        if (fd >= 0) {
            SET_FD(this, fd, fid);
        } else {
            throwFileNotFoundException(env, path);
        }
    } END_PLATFORM_STRING(env, ps);
}


void
fileClose(JNIEnv *env, jobject this, jfieldID fid) 
{
    FD fd = GET_FD(this, fid);

    /*
     * Don't close file descriptors 0, 1, or 2. If we close these stream
     * then a subsequent file open or socket will use them. Instead we
     * just redirect these file descriptors to /dev/null.
     */
    if (fd >= STDIN_FILENO && fd <= STDERR_FILENO) {
	int devnull = open("/dev/null", O_WRONLY);
	if (devnull < 0) {
            JNU_ThrowIOExceptionWithLastError(env, "open /dev/null failed");
	} else {
	    SET_FD(this, -1, fid);
	    dup2(devnull, fd);
            close(devnull);
	}
    } else if (fd != -1) {
        if (JVM_Close(fd) == -1) {
            JNU_ThrowIOExceptionWithLastError(env, "close failed");
	} else {
            SET_FD(this, -1, fid);
        }
    }
}

