/*
 * @(#)io_util.c	1.91 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdlib.h>
#include <string.h>

#include "jni.h"
#include "jni_util.h"
#include "jvm.h"
#include "io_util.h"
#include "io_util_md.h"

/* IO helper functions */

int
readSingle(JNIEnv *env, jobject this, jfieldID fid) {
    int nread;
    char ret;
    FD fd = GET_FD(this, fid);

    nread = IO_Read(fd, &ret, 1);
    if (nread == 0) { /* EOF */
	return -1;
    } else if (nread == JVM_IO_ERR) { /* error */
	JNU_ThrowIOExceptionWithLastError(env, "Read error");
    } else if (nread == JVM_IO_INTR) {
        JNU_ThrowByName(env, "java/io/InterruptedIOException", 0);
    }
    return ret & 0xFF;
}

/* The maximum size of a stack-allocated buffer.
 */
#define BUF_SIZE 8192


int
readBytes(JNIEnv *env, jobject this, jbyteArray bytes,
	  jint off, jint len, jfieldID fid)
{
    int nread, datalen;
    char stackBuf[BUF_SIZE];
    char *buf = 0;
    FD fd;

    if (IS_NULL(bytes)) {
	JNU_ThrowNullPointerException(env, 0);
	return -1;
    }
    datalen = (*env)->GetArrayLength(env, bytes);

    if ((off < 0) || (off > datalen) ||
        (len < 0) || ((off + len) > datalen) || ((off + len) < 0)) {
        JNU_ThrowByName(env, "java/lang/IndexOutOfBoundsException", 0);
	return -1;
    }

    if (len == 0) {
	return 0;
    } else if (len > BUF_SIZE) {
        buf = malloc(len);
	if (buf == 0) {
	    JNU_ThrowOutOfMemoryError(env, 0);
	    return 0;
        }
    } else {
        buf = stackBuf;
    }

    fd = GET_FD(this, fid);
    nread = IO_Read(fd, buf, len);
    if (nread > 0) {
        (*env)->SetByteArrayRegion(env, bytes, off, nread, (jbyte *)buf);
    } else if (nread == JVM_IO_ERR) {
	JNU_ThrowIOExceptionWithLastError(env, "Read error");
    } else if (nread == JVM_IO_INTR) { /* EOF */
        JNU_ThrowByName(env, "java/io/InterruptedIOException", 0);
    } else { /* EOF */
	nread = -1;
    }

    if (buf != stackBuf) {
        free(buf);
    }
    return nread;
}

void
writeSingle(JNIEnv *env, jobject this, jint byte, jfieldID fid) {
    char c = byte;
    FD fd = GET_FD(this, fid);
    int n = IO_Write(fd, &c, 1);
    if (n == JVM_IO_ERR) {
	JNU_ThrowIOExceptionWithLastError(env, "Write error");
    } else if (n == JVM_IO_INTR) {
        JNU_ThrowByName(env, "java/io/InterruptedIOException", 0);
    }
}

void
writeBytes(JNIEnv *env, jobject this, jbyteArray bytes,
	  jint off, jint len, jfieldID fid)
{
    int n, datalen;
    char stackBuf[BUF_SIZE];
    char *buf = 0;
    FD fd;

    if (IS_NULL(bytes)) {
	JNU_ThrowNullPointerException(env, 0);
	return;
    }
    datalen = (*env)->GetArrayLength(env, bytes);

    if ((off < 0) || (off > datalen) ||
        (len < 0) || ((off + len) > datalen) || ((off + len) < 0)) {
        JNU_ThrowByName(env, "java/lang/IndexOutOfBoundsException", 0);
	return;
    }

    if (len == 0) {
        return;
    } else if (len > BUF_SIZE) {
        buf = malloc(len);
	if (buf == 0) {
	    JNU_ThrowOutOfMemoryError(env, 0);
	    return;
	}
    } else {
        buf = stackBuf;
    }

    fd = GET_FD(this, fid);
    (*env)->GetByteArrayRegion(env, bytes, off, len, (jbyte *)buf);

    if (!(*env)->ExceptionOccurred(env)) {
        off = 0;
	while (len > 0) {
	    n = IO_Write(fd, buf+off, len);
	    if (n == JVM_IO_ERR) {
	        JNU_ThrowIOExceptionWithLastError(env, "Write error");
		break;
	    } else if (n == JVM_IO_INTR) {
	        JNU_ThrowByName(env, "java/io/InterruptedIOException", 0);
		break;
	    }
	    off += n;
	    len -= n;
	}
    }
    if (buf != stackBuf) {
        free(buf);
    }
}


/* File.deleteOnExit support
   No synchronization is done here; that is left to the Java level.
 */

static struct dlEntry {
    struct dlEntry *next;
    DELETEPROC deleteProc;
    char name[JVM_MAXPATHLEN + 1];
} *deletionList = NULL;

static void
deleteOnExitHook(void)		/* Called by the VM on exit */
{
    struct dlEntry *e, *next;
    for (e = deletionList; e; e = next) {
	next = e->next;
	e->deleteProc(e->name);
	free(e);
    }
}

void
deleteOnExit(JNIEnv *env, const char *path, DELETEPROC dp)
{
    struct dlEntry *dl = deletionList;
    struct dlEntry *e = (struct dlEntry *)malloc(sizeof(struct dlEntry));
    if (e == NULL) {
	JNU_ThrowOutOfMemoryError(env, 0);
    }
    strcpy(e->name, path);
    e->deleteProc = dp;

    if (dl == NULL) {
	JVM_OnExit(deleteOnExitHook);
    }	
    e->next = deletionList;
    deletionList = e;
}

void
throwFileNotFoundException(JNIEnv *env, jstring path)
{
    char buf[256];
    int n;
    jobject x;
    jstring why = NULL;

    n = JVM_GetLastErrorString(buf, sizeof(buf));
    if (n > 0) {
    why = JNU_NewStringPlatform(env, buf);
    }
    x = JNU_NewObjectByName(env,
                "java/io/FileNotFoundException",
                "(Ljava/lang/String;Ljava/lang/String;)V",
                path, why);
    if (x != NULL) {
    (*env)->Throw(env, x);
    }
}


