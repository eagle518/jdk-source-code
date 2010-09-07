/*
 * @(#)UnixFileSystem_md.c	1.28 04/01/12
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>

#include "jni.h"
#include "jni_util.h"
#include "jlong.h"
#include "jvm.h"
#include "io_util.h"
#include "java_io_FileSystem.h"
#include "java_io_UnixFileSystem.h"


/* -- Field IDs -- */

static struct {
    jfieldID path;
} ids;


JNIEXPORT void JNICALL
Java_java_io_UnixFileSystem_initIDs(JNIEnv *env, jclass cls)
{
    jclass fileClass = (*env)->FindClass(env, "java/io/File");
    if (!fileClass) return;
    ids.path = (*env)->GetFieldID(env, fileClass,
				  "path", "Ljava/lang/String;");
}


/* -- Large-file support -- */

/* LINUX_FIXME: ifdef __solaris__ here is wrong.  We need to move the
 * definition of stat64 into a solaris_largefile.h and create a
 * linux_largefile.h with a good stat64 structure to compile on
 * glibc2.0 based systems.
 */
#if defined(__solaris__) && !defined(_LFS_LARGEFILE) || !_LFS_LARGEFILE

/* The stat64 structure must be provided for systems without large-file support
   (e.g., Solaris 2.5.1).  These definitions are copied from the Solaris 2.6
   <sys/stat.h> and <sys/types.h> files.
 */

typedef longlong_t	off64_t;	/* offsets within files */
typedef u_longlong_t	ino64_t;	/* expanded inode type	*/
typedef longlong_t	blkcnt64_t;	/* count of file blocks */

struct	stat64 {
	dev_t	st_dev;
	long	st_pad1[3];
	ino64_t	st_ino;
	mode_t	st_mode;
	nlink_t st_nlink;
	uid_t 	st_uid;
	gid_t 	st_gid;
	dev_t	st_rdev;
	long	st_pad2[2];
	off64_t	st_size;
	timestruc_t st_atim;
	timestruc_t st_mtim;
	timestruc_t st_ctim;
	long	st_blksize;
	blkcnt64_t st_blocks;
	char	st_fstype[_ST_FSTYPSZ];
	long	st_pad4[8];
};

#endif  /* !_LFS_LARGEFILE */

typedef int (*STAT64)(const char *, struct stat64 *);

#if defined(__linux__) && defined(_LARGEFILE64_SOURCE)
static STAT64 stat64_ptr = &stat64;
#else
static STAT64 stat64_ptr = NULL;
#endif

#ifndef __linux__
#ifdef __GNUC__
static void init64IO(void) __attribute__((constructor));
#else
#pragma init(init64IO)
#endif
#endif

static void init64IO(void) {
    void *handle = dlopen(0, RTLD_LAZY);
    stat64_ptr = (STAT64) dlsym(handle, "_stat64");
    dlclose(handle);
}


/* -- Path operations -- */

extern int canonicalize(char *path, const char *out, int len);

JNIEXPORT jstring JNICALL
Java_java_io_UnixFileSystem_canonicalize0(JNIEnv *env, jobject this,
                                          jstring pathname)
{
    jstring rv = NULL;

    WITH_PLATFORM_STRING(env, pathname, path) {
	char canonicalPath[JVM_MAXPATHLEN];
	if (canonicalize(JVM_NativePath((char *)path),
			 canonicalPath, JVM_MAXPATHLEN) < 0) {
	    JNU_ThrowIOExceptionWithLastError(env, "Bad pathname");
	} else {
	    rv = JNU_NewStringPlatform(env, canonicalPath);
	}
    } END_PLATFORM_STRING(env, path);
    return rv;
}


/* -- Attribute accessors -- */


static jboolean
statMode(const char *path, int *mode)
{
    if (stat64_ptr) {
	struct stat64 sb;
	if (((*stat64_ptr)(path, &sb)) == 0) {
	    *mode = sb.st_mode;
	    return JNI_TRUE;
	}
    } else {
	struct stat sb;
	if (stat(path, &sb) == 0) {
	    *mode = sb.st_mode;
	    return JNI_TRUE;
	}
    }
    return JNI_FALSE;
}


JNIEXPORT jint JNICALL
Java_java_io_UnixFileSystem_getBooleanAttributes0(JNIEnv *env, jobject this,
						  jobject file)
{
    jint rv = 0;

    WITH_FIELD_PLATFORM_STRING(env, file, ids.path, path) {
	int mode;
	if (statMode(path, &mode)) {
	    int fmt = mode & S_IFMT;
	    rv = (jint) (java_io_FileSystem_BA_EXISTS
                  | ((fmt == S_IFREG) ? java_io_FileSystem_BA_REGULAR : 0)
                  | ((fmt == S_IFDIR) ? java_io_FileSystem_BA_DIRECTORY : 0));
	}
    } END_PLATFORM_STRING(env, path);
    return rv;
}


JNIEXPORT jboolean JNICALL
Java_java_io_UnixFileSystem_checkAccess(JNIEnv *env, jobject this,
					jobject file, jboolean write)
{
    jboolean rv = JNI_FALSE;

    WITH_FIELD_PLATFORM_STRING(env, file, ids.path, path) {
	if (access(path, (write ? W_OK : R_OK)) == 0) {
	    rv = JNI_TRUE;
	}
    } END_PLATFORM_STRING(env, path);
    return rv;
}


JNIEXPORT jlong JNICALL
Java_java_io_UnixFileSystem_getLastModifiedTime(JNIEnv *env, jobject this,
						jobject file)
{
    jlong rv = 0;

    WITH_FIELD_PLATFORM_STRING(env, file, ids.path, path) {
        if (stat64_ptr) {
            struct stat64 sb;
            if (((*stat64_ptr)(path, &sb)) == 0) {
                rv = 1000 * (jlong)sb.st_mtime;
            }
        } else {
            struct stat sb;
            if (stat(path, &sb) == 0) {
                rv = 1000 * (jlong)sb.st_mtime;
            }
        }
    } END_PLATFORM_STRING(env, path);
    return rv;
}


JNIEXPORT jlong JNICALL
Java_java_io_UnixFileSystem_getLength(JNIEnv *env, jobject this,
				      jobject file)
{
    jlong rv = 0;

    WITH_FIELD_PLATFORM_STRING(env, file, ids.path, path) {
	if (stat64_ptr) {
	    struct stat64 sb;
	    if (((*stat64_ptr)(path, &sb)) == 0) {
		rv = sb.st_size;
	    }
	} else {
	    struct stat sb;
	    if (stat(path, &sb) == 0) {
		rv = sb.st_size;
	    }
	}
    } END_PLATFORM_STRING(env, path);
    return rv;
}


/* -- File operations -- */


JNIEXPORT jboolean JNICALL
Java_java_io_UnixFileSystem_createFileExclusively(JNIEnv *env, jclass cls,
						  jstring pathname)
{
    jboolean rv = JNI_FALSE;

    WITH_PLATFORM_STRING(env, pathname, path) {
	int fd;
	if (!strcmp (path, "/")) {
	    fd = JVM_EEXIST;	/* The root directory always exists */
	} else {
	    fd = JVM_Open(path, JVM_O_RDWR | JVM_O_CREAT | JVM_O_EXCL, 0666);
	}
	if (fd < 0) {
	    if (fd != JVM_EEXIST) {
		JNU_ThrowIOExceptionWithLastError(env, path);
	    }
	} else {
	    JVM_Close(fd);
	    rv = JNI_TRUE;
	}
    } END_PLATFORM_STRING(env, path);
    return rv;
}


JNIEXPORT jboolean JNICALL
Java_java_io_UnixFileSystem_delete0(JNIEnv *env, jobject this,
                                    jobject file)
{
    jboolean rv = JNI_FALSE;

    WITH_FIELD_PLATFORM_STRING(env, file, ids.path, path) {
	if (remove(path) == 0) {
	    rv = JNI_TRUE;
	}
    } END_PLATFORM_STRING(env, path);
    return rv;
}


JNIEXPORT jboolean JNICALL
Java_java_io_UnixFileSystem_deleteOnExit(JNIEnv *env, jobject this,
					 jobject file)
{
    WITH_FIELD_PLATFORM_STRING(env, file, ids.path, path) {
	deleteOnExit(env, path, remove);
    } END_PLATFORM_STRING(env, path);
    return JNI_TRUE;
}


JNIEXPORT jobjectArray JNICALL
Java_java_io_UnixFileSystem_list(JNIEnv *env, jobject this,
				 jobject file)
{
    DIR *dir = NULL;
    struct dirent *ptr;
    int len, maxlen;
    jobjectArray rv, old;

    WITH_FIELD_PLATFORM_STRING(env, file, ids.path, path) {
	dir = opendir(path);
    } END_PLATFORM_STRING(env, path);
    if (dir == NULL) return NULL;

    /* Allocate an initial String array */
    len = 0;
    maxlen = 16;
    rv = (*env)->NewObjectArray(env, maxlen, JNU_ClassString(env), NULL);
    if (rv == NULL) goto error;

    /* Scan the directory */
    while ((ptr = readdir(dir)) != NULL) {
	jstring name;
  	if (!strcmp(ptr->d_name, ".") || !strcmp(ptr->d_name, ".."))
	    continue;
	if (len == maxlen) {
	    old = rv;
	    rv = (*env)->NewObjectArray(env, maxlen <<= 1,
					JNU_ClassString(env), NULL);
	    if (rv == NULL) goto error;
	    if (JNU_CopyObjectArray(env, rv, old, len) < 0) goto error;
	    (*env)->DeleteLocalRef(env, old);
	}
	name = JNU_NewStringPlatform(env, ptr->d_name);
	if (name == NULL) goto error;
	(*env)->SetObjectArrayElement(env, rv, len++, name);
	(*env)->DeleteLocalRef(env, name);
    }
    closedir(dir);

    /* Copy the final results into an appropriately-sized array */
    old = rv;
    rv = (*env)->NewObjectArray(env, len, JNU_ClassString(env), NULL);
    if (rv == NULL) {
        return NULL;
    }
    if (JNU_CopyObjectArray(env, rv, old, len) < 0) {
        return NULL;
    }
    return rv;

 error:
    closedir(dir);
    return NULL;
}


JNIEXPORT jboolean JNICALL
Java_java_io_UnixFileSystem_createDirectory(JNIEnv *env, jobject this,
					    jobject file)
{
    jboolean rv = JNI_FALSE;

    WITH_FIELD_PLATFORM_STRING(env, file, ids.path, path) {
	if (mkdir(path, 0777) == 0) {
	    rv = JNI_TRUE;
	}
    } END_PLATFORM_STRING(env, path);
    return rv;
}


JNIEXPORT jboolean JNICALL
Java_java_io_UnixFileSystem_rename0(JNIEnv *env, jobject this,
                                    jobject from, jobject to)
{
    jboolean rv = JNI_FALSE;

    WITH_FIELD_PLATFORM_STRING(env, from, ids.path, fromPath) {
	WITH_FIELD_PLATFORM_STRING(env, to, ids.path, toPath) {
	    if (rename(fromPath, toPath) == 0) {
		rv = JNI_TRUE;
	    }
	} END_PLATFORM_STRING(env, toPath);
    } END_PLATFORM_STRING(env, fromPath);
    return rv;
}


/* Bug in solaris /usr/include/sys/time.h? */
#ifdef __solaris__
extern int utimes(const char *, const struct timeval *);
#elif defined(__linux___)
extern int utimes(const char *, struct timeval *);
#endif


JNIEXPORT jboolean JNICALL  
Java_java_io_UnixFileSystem_setLastModifiedTime(JNIEnv *env, jobject this,
						jobject file, jlong time)
{
    jboolean rv = JNI_FALSE;

    WITH_FIELD_PLATFORM_STRING(env, file, ids.path, path) {
	struct timeval tv[2];
#ifdef __solaris__
	timestruc_t ts;

        if (stat64_ptr) {
            struct stat64 sb;
            if (((*stat64_ptr)(path, &sb)) == 0)
		ts = sb.st_atim;
	    else
		goto error;
	} else {
            struct stat sb;
            if (stat(path, &sb) == 0)
		ts = sb.st_atim;
	    else
		goto error;
	}
#endif

	/* Preserve access time */
#ifdef __linux__
        struct stat sb;

        if (stat(path, &sb) == 0) {

	tv[0].tv_sec = sb.st_atime;
	tv[0].tv_usec = 0;
        }
#else
	tv[0].tv_sec = ts.tv_sec;
	tv[0].tv_usec = ts.tv_nsec / 1000;
#endif

	/* Change last-modified time */
	tv[1].tv_sec = time / 1000;
	tv[1].tv_usec = (time % 1000) * 1000;

        if (utimes(path, tv) >= 0)
            rv = JNI_TRUE;

    error: ;
    } END_PLATFORM_STRING(env, path);

    return rv;
}


JNIEXPORT jboolean JNICALL  
Java_java_io_UnixFileSystem_setReadOnly(JNIEnv *env, jobject this,
					jobject file)
{
    jboolean rv = JNI_FALSE;

    WITH_FIELD_PLATFORM_STRING(env, file, ids.path, path) {
	int mode;
	if (statMode(path, &mode)) {
	    if (chmod(path, mode & ~(S_IWUSR | S_IWGRP | S_IWOTH)) >= 0) {
                rv = JNI_TRUE;
            }
	}
    } END_PLATFORM_STRING(env, path);
    return rv;
}
