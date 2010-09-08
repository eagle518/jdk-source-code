/*
 * @(#)FileSystemImpl.c	1.3 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

