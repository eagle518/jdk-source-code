/*
 * @(#)FileSystemImpl.c	1.1 04/03/08
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <sys/types.h>
#include <sys/stat.h>

#include "jni.h"
#include "jni_util.h"
#include "sun_management_FileSystemImpl.h"

/*
 * Class:     sun_management_FileSystemImpl
 * Method:    isAccessUserOnly0
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_sun_management_FileSystemImpl_isAccessUserOnly0
  (JNIEnv *env, jclass ignored, jstring str)
{
    jboolean res = JNI_FALSE;
    jboolean isCopy;
    const char *path = JNU_GetStringPlatformChars(env, str, &isCopy);
    if (path != NULL) {
 	struct stat64 sb;
        if (stat64(path, &sb) == 0) {
	    res = ((sb.st_mode & (S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) == 0) ? JNI_TRUE : JNI_FALSE;
	} else {
	    JNU_ThrowIOExceptionWithLastError(env, "stat64 failed");
	}
	if (isCopy) {
	    JNU_ReleaseStringPlatformChars(env, str, path);
  	}
    }
    return res;
}

