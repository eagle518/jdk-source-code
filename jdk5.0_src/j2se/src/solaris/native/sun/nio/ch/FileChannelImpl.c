/*
 * @(#)FileChannelImpl.c	1.38 04/06/07
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jni_util.h"
#include "jvm.h"
#include "jlong.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include "sun_nio_ch_FileChannelImpl.h"
#include "java_lang_Integer.h"
#include "nio.h"
#include "nio_util.h"
#include <dlfcn.h>

static jfieldID chan_fd;	/* jobject 'fd' in sun.io.FileChannelImpl */

#ifdef __solaris__
typedef struct sendfilevec64 {
    int     sfv_fd;         /* input fd */
    uint_t  sfv_flag;       /* Flags. see below */
    off64_t sfv_off;        /* offset to start reading from */
    size_t  sfv_len;        /* amount of data */
} sendfilevec_t;

/* Function pointer for sendfilev on Solaris 8+ */
typedef ssize_t sendfile_func(int fildes, const struct sendfilevec64 *vec,
                              int sfvcnt, size_t *xferred);

sendfile_func* my_sendfile_func = NULL;
#endif

JNIEXPORT jlong JNICALL 
Java_sun_nio_ch_FileChannelImpl_initIDs(JNIEnv *env, jclass clazz)
{
    jlong pageSize = sysconf(_SC_PAGESIZE);
    chan_fd = (*env)->GetFieldID(env, clazz, "fd", "Ljava/io/FileDescriptor;");

#ifdef __solaris__
    if (dlopen("/usr/lib/libsendfile.so.1", RTLD_GLOBAL | RTLD_LAZY) != NULL) {
        my_sendfile_func = (sendfile_func*) dlsym(RTLD_DEFAULT, "sendfilev64");
    }
#endif

    return pageSize;
}

static jlong
handle(JNIEnv *env, jlong rv, char *msg)
{
    if (rv >= 0)
	return rv;
    if (errno == EINTR)
	return IOS_INTERRUPTED;
    JNU_ThrowIOExceptionWithLastError(env, msg);
    return IOS_THROWN;
}


JNIEXPORT jlong JNICALL
Java_sun_nio_ch_FileChannelImpl_map0(JNIEnv *env, jobject this,
				     jint prot, jlong off, jlong len)
{
    void *mapAddress = 0;
    jobject fdo = (*env)->GetObjectField(env, this, chan_fd);
    jint fd = fdval(env, fdo);
    int protections = 0;
    int flags = 0;

    if (prot == sun_nio_ch_FileChannelImpl_MAP_RO) {
        protections = PROT_READ;
        flags = MAP_SHARED;
    } else if (prot == sun_nio_ch_FileChannelImpl_MAP_RW) {
        protections = PROT_WRITE | PROT_READ;
        flags = MAP_SHARED;
    } else if (prot == sun_nio_ch_FileChannelImpl_MAP_PV) {
        protections =  PROT_WRITE | PROT_READ;
        flags = MAP_PRIVATE;
    }

    mapAddress = mmap64(
        0,                    /* Let OS decide location */
        len,                  /* Number of bytes to map */
        protections,          /* File permissions */
        flags,                /* Changes are shared */
        fd,                   /* File descriptor of mapped file */
        off);                 /* Offset into file */

    if (mapAddress == MAP_FAILED) {
	return handle(env, -1, "Map failed");
    }

    return ptr_to_jlong(mapAddress);
}


JNIEXPORT jint JNICALL
Java_sun_nio_ch_FileChannelImpl_unmap0(JNIEnv *env, jobject this,
				       jlong address, jlong len)
{
    void *a = (void *)jlong_to_ptr(address);
    return handle(env,
		  munmap(a, (size_t)len),
		  "Unmap failed");
}


JNIEXPORT jint JNICALL
Java_sun_nio_ch_FileChannelImpl_truncate0(JNIEnv *env, jobject this,
					  jobject fdo, jlong size)
{
    return handle(env,
		  ftruncate64(fdval(env, fdo), size),
		  "Truncation failed");
}


JNIEXPORT jint JNICALL
Java_sun_nio_ch_FileChannelImpl_force0(JNIEnv *env, jobject this,
				       jobject fdo, jboolean md)
{
    jint fd = fdval(env, fdo);
    int result = 0;

    if (md == JNI_FALSE) {
        result = fdatasync(fd);
    } else {
        result = fsync(fd);
    }
    return handle(env, result, "Force failed");
}


JNIEXPORT jlong JNICALL
Java_sun_nio_ch_FileChannelImpl_position0(JNIEnv *env, jobject this,
					  jobject fdo, jlong offset)
{
    jint fd = fdval(env, fdo);
    jlong result = 0;

    if (offset < 0) {
        result = lseek64(fd, 0, SEEK_CUR);
    } else {
        result = lseek64(fd, offset, SEEK_SET);
    }
    return handle(env, result, "Position failed");
}


JNIEXPORT jlong JNICALL
Java_sun_nio_ch_FileChannelImpl_size0(JNIEnv *env, jobject this, jobject fdo)
{
    struct stat64 fbuf;

    if (fstat64(fdval(env, fdo), &fbuf) < 0)
	return handle(env, -1, "Size failed");
    return fbuf.st_size;
}


JNIEXPORT void JNICALL
Java_sun_nio_ch_FileChannelImpl_close0(JNIEnv *env, jobject this, jobject fdo)
{
    jint fd = fdval(env, fdo);
    if (fd != -1) {
        jlong result = close(fd);
        if (result < 0) {
            JNU_ThrowIOExceptionWithLastError(env, "Close failed");
        }
    }
}

#ifdef __linux__
#include <sys/sendfile.h>
#endif

JNIEXPORT jlong JNICALL
Java_sun_nio_ch_FileChannelImpl_transferTo0(JNIEnv *env, jobject this,
					    jint srcFD,
					    jlong position, jlong count,
					    jint dstFD)
{
#ifdef __linux__
    off_t offset = (off_t)position;
    jlong n = sendfile(dstFD, srcFD, &offset, (size_t)count);
    if (n < 0) {
        jlong max = (jlong)java_lang_Integer_MAX_VALUE;
        if (count > max) {
            n = sendfile(dstFD, srcFD, &offset, (size_t)max);
            if (n >= 0) {
                return n;
            }
        }
        if ((errno == EINVAL) && ((ssize_t)count >= 0))
            return IOS_UNSUPPORTED_CASE;
	JNU_ThrowIOExceptionWithLastError(env, "Transfer failed");
    }
    return n;
#endif

#ifdef __solaris__
    if (my_sendfile_func == NULL) {
        return IOS_UNSUPPORTED;
    } else {
        sendfilevec_t sfv;
        size_t numBytes = 0;
        jlong result;

        sfv.sfv_fd = srcFD;
        sfv.sfv_flag = 0;
        sfv.sfv_off = (off64_t)position;
        sfv.sfv_len = count;

        result = (*my_sendfile_func)(dstFD, &sfv, 1, &numBytes);

	/* Solaris sendfilev() will return -1 even if some bytes have been
	 * transferred, so we check numBytes first.
	 */
	if (numBytes > 0)
	    return numBytes;
        if (result < 0) {
	    if (errno == EAGAIN)
		return IOS_UNAVAILABLE;
            if ((errno == EINVAL) && ((ssize_t)count >= 0))
                return IOS_UNSUPPORTED_CASE;
	    if (errno == EINTR)
		return IOS_INTERRUPTED;
            JNU_ThrowIOExceptionWithLastError(env, "Transfer failed");
	    return IOS_THROWN;
        }
        return result;
    }
#endif
}

JNIEXPORT jint JNICALL
Java_sun_nio_ch_FileChannelImpl_lock0(JNIEnv *env, jobject this, jobject fdo,
                                      jboolean block, jlong pos, jlong size,
                                      jboolean shared)
{
    jint fd = fdval(env, fdo);
    jint lockResult = 0;
    int cmd = 0;
    struct flock64 fl;

    fl.l_whence = SEEK_SET;
    fl.l_len = (off64_t)size;
    fl.l_start = (off64_t)pos;
    if (shared == JNI_TRUE) {
        fl.l_type = F_RDLCK;
    } else {
        fl.l_type = F_WRLCK;
    }
    if (block == JNI_TRUE) {
        cmd = F_SETLKW64;
    } else {
        cmd = F_SETLK64;
    }
    lockResult = fcntl(fd, cmd, &fl);
    if (lockResult < 0) {
        if ((cmd == F_SETLK64) && (errno == EAGAIN))
            return sun_nio_ch_FileChannelImpl_NO_LOCK;
	if (errno == EINTR)
            return sun_nio_ch_FileChannelImpl_INTERRUPTED;
        JNU_ThrowIOExceptionWithLastError(env, "Lock failed");
    }
    return 0;
}


JNIEXPORT void JNICALL
Java_sun_nio_ch_FileChannelImpl_release0(JNIEnv *env, jobject this,
                                         jobject fdo, jlong pos, jlong size)
{
    jint fd = fdval(env, fdo);
    jint lockResult = 0;
    struct flock64 fl;
    int cmd = F_SETLK64;

    fl.l_whence = SEEK_SET;
    fl.l_len = (off64_t)size;
    fl.l_start = (off64_t)pos;
    fl.l_type = F_UNLCK;
    lockResult = fcntl(fd, cmd, &fl);
    if (lockResult < 0) {
        JNU_ThrowIOExceptionWithLastError(env, "Release failed");
    }
}
