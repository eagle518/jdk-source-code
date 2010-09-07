/*
 * @(#)io_util_md.h	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jni_util.h"

/*
 * Macros to use the right data type for file descriptors
 */
#define FD jint

/*
 * Macros to set/get fd from the java.io.FileDescriptor.  These
 * macros rely on having an appropriately defined 'this' object
 * within the scope in which they're used.
 */
#define SET_FD(this, fd, fid) (*env)->SetIntField(env, \
		    (*env)->GetObjectField(env, (this), (fid)),IO_fd_fdID, (fd))
#define GET_FD(this, fid) (*env)->GetIntField(env, \
		    (*env)->GetObjectField(env, (this), (fid)), IO_fd_fdID)
/*
 * Macros to set/get fd when inside java.io.FileDescriptor
 */
#define THIS_FD(obj) (*env)->GetIntField(env, obj, IO_fd_fdID)

/*
 * Route the routines through HPI
 */
#define IO_Write JVM_Write
#define IO_Sync JVM_Sync
#define IO_Read JVM_Read
#define IO_Lseek JVM_Lseek
#define IO_Available JVM_Available
#define IO_SetLength JVM_SetLength

/*
 * On Solaris, the handle field is unused
 */
#define SET_HANDLE(fd) return (jlong)-1

/*
 * IO helper function(s)
 */
void fileClose(JNIEnv *env, jobject this, jfieldID fid);

