/*
 * @(#)io_util_md.h	1.4 04/01/08
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jni_util.h"

/*
 * Prototypes for functions in io_util_md.c called from io_util, 
 * FileDescriptor.c, FileInputStream.c, FileOutputStream.c
 */
WCHAR* pathToNTPath(JNIEnv *env, jstring path, jboolean throwFNFE);
WCHAR* fileToNTPath(JNIEnv *env, jobject file, jfieldID id);
void fileOpen(JNIEnv *env, jobject this, jstring path, jfieldID fid, int flags);
int handleAvailable(jlong fd, jlong *pbytes);
JNIEXPORT int handleSync(jlong fd);
int handleSetLength(jlong fd, jlong length);
int handleFileSizeFD(jlong fd, jlong *size);
JNIEXPORT size_t handleRead(jlong fd, void *buf, jint len);
JNIEXPORT size_t handleWrite(jlong fd, const void *buf, jint len);
jint handleClose(JNIEnv *env, jobject this, jfieldID fid);
jlong handleLseek(jlong fd, jlong offset, jint whence);

/*
 * Macros to use the right data type for file descriptors
 */
#define FD jlong

/*
 * Macros to set/get fd from the java.io.FileDescriptor.
 */
#define SET_FD(this, fd, fid) (*env)->SetLongField(env, \
	      (*env)->GetObjectField(env, (this), (fid)),IO_handle_fdID, (fd))
#define GET_FD(this, fid) (*env)->GetLongField(env, \
	      (*env)->GetObjectField(env, (this), (fid)), IO_handle_fdID)
/*
 * Macros to set/get fd when inside java.io.FileDescriptor
 */
#define THIS_FD(obj) (*env)->GetLongField(env, obj, IO_handle_fdID)

/*
 * Route the routines away from HPI layer
 */
#define IO_Write handleWrite
#define IO_Sync handleSync
#define IO_Read handleRead
#define IO_Lseek handleLseek
#define IO_Available handleAvailable
#define IO_SetLength handleSetLength

/*
 * Setting the handle field in Java_java_io_FileDescriptor_set for 
 * standard handles stdIn, stdOut, stdErr
 */
#define SET_HANDLE(fd) \
if (fd == 0) { \
    return (jlong)GetStdHandle(STD_INPUT_HANDLE); \
} else if (fd == 1) { \
    return (jlong)GetStdHandle(STD_OUTPUT_HANDLE); \
} else if (fd == 2) { \
    return (jlong)GetStdHandle(STD_ERROR_HANDLE); \
} else { \
    return (jlong)-1; \
} \

