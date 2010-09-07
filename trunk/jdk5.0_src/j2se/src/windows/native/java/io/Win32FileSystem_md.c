/*
 * @(#)Win32FileSystem_md.c	1.19 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <direct.h>
#include <windows.h>
#include <io.h>

#include "jvm.h"
#include "jni.h"
#include "jni_util.h"
#include "io_util.h"
#include "dirent_md.h"
#include "java_io_FileSystem.h"


/* This macro relies upon the fact that JNU_GetStringPlatformChars always makes
   a copy of the string */

#define WITH_NATIVE_PATH(env, object, id, var)				      \
    WITH_FIELD_PLATFORM_STRING(env, object, id, var)			      \
        JVM_NativePath((char *)var);

#define END_NATIVE_PATH(env, var)    END_PLATFORM_STRING(env, var)


static struct {
    jfieldID path;
} ids;

JNIEXPORT void JNICALL
Java_java_io_Win32FileSystem_initIDs(JNIEnv *env, jclass cls)
{
    jclass fileClass = (*env)->FindClass(env, "java/io/File");
    if (!fileClass) return;
    ids.path = (*env)->GetFieldID(env, fileClass,
				  "path", "Ljava/lang/String;");
}


/* -- Path operations -- */


extern int canonicalize(char *path, const char *out, int len);
extern int canonicalizeWithPrefix(const char* canonicalPrefix, const char *pathWithCanonicalPrefix, char *out, int len);

JNIEXPORT jstring JNICALL
Java_java_io_Win32FileSystem_canonicalize0(JNIEnv *env, jobject this,
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


JNIEXPORT jstring JNICALL
Java_java_io_Win32FileSystem_canonicalizeWithPrefix0(JNIEnv *env, jobject this,
                                                     jstring canonicalPrefixString,
                                                     jstring pathWithCanonicalPrefixString)
{
    jstring rv = NULL;
    char canonicalPath[JVM_MAXPATHLEN];

    WITH_PLATFORM_STRING(env, canonicalPrefixString, canonicalPrefix) {
        WITH_PLATFORM_STRING(env, pathWithCanonicalPrefixString, pathWithCanonicalPrefix) {
            if (canonicalizeWithPrefix(canonicalPrefix,
                                       pathWithCanonicalPrefix,
                                       canonicalPath, JVM_MAXPATHLEN) < 0) {
                JNU_ThrowIOExceptionWithLastError(env, "Bad pathname");
            } else {
                rv = JNU_NewStringPlatform(env, canonicalPath);
            }
        } END_PLATFORM_STRING(env, pathWithCanonicalPrefix);
    } END_PLATFORM_STRING(env, canonicalPrefix);
    return rv;
}



/* -- Attribute accessors -- */


JNIEXPORT jint JNICALL
Java_java_io_Win32FileSystem_getBooleanAttributes(JNIEnv *env, jobject this,
						  jobject file)
{
    jint rv = 0;

    WITH_NATIVE_PATH(env, file, ids.path, path) {
	DWORD a = GetFileAttributes(path);
	if (a != ((DWORD)-1)) {
	    rv = (java_io_FileSystem_BA_EXISTS
		  | ((a & FILE_ATTRIBUTE_DIRECTORY)
		     ? java_io_FileSystem_BA_DIRECTORY
		     : java_io_FileSystem_BA_REGULAR)
		  | ((a & FILE_ATTRIBUTE_HIDDEN)
		     ? java_io_FileSystem_BA_HIDDEN : 0));
	}
    } END_NATIVE_PATH(env, path);
    return rv;
}


JNIEXPORT jboolean JNICALL
Java_java_io_Win32FileSystem_checkAccess(JNIEnv *env, jobject this,
					 jobject file, jboolean write)
{
    jboolean rv = JNI_FALSE;

    WITH_NATIVE_PATH(env, file, ids.path, path) {
	if (access(path, (write ? 2 : 4)) == 0) {
	    rv = JNI_TRUE;
	}
    } END_NATIVE_PATH(env, path);
    return rv;
}


JNIEXPORT jlong JNICALL
Java_java_io_Win32FileSystem_getLastModifiedTime(JNIEnv *env, jobject this,
						 jobject file)
{
    jlong rv = 0;
    WITH_NATIVE_PATH(env, file, ids.path, path) {
    /* Win95, Win98, WinME */
    WIN32_FIND_DATA fd;
    jlong temp = 0;
    LARGE_INTEGER modTime;
    HANDLE h = FindFirstFile(path, &fd);
    if (h != INVALID_HANDLE_VALUE) {
        FindClose(h);
        modTime.LowPart = (DWORD) fd.ftLastWriteTime.dwLowDateTime;
        modTime.HighPart = (LONG) fd.ftLastWriteTime.dwHighDateTime;
        rv = modTime.QuadPart / 10000;
        rv -= 11644473600000;
    }
    } END_NATIVE_PATH(env, path);
    return rv;
}

JNIEXPORT jlong JNICALL
Java_java_io_Win32FileSystem_getLength(JNIEnv *env, jobject this,
				       jobject file)
{
    jlong rv = 0;

    WITH_NATIVE_PATH(env, file, ids.path, path) {
	struct _stati64 sb;
	if (_stati64(path, &sb) == 0) {
	    rv = sb.st_size;
	}
    } END_NATIVE_PATH(env, path);
    return rv;
}


/* -- File operations -- */


JNIEXPORT jboolean JNICALL
Java_java_io_Win32FileSystem_createFileExclusively(JNIEnv *env, jclass cls,
						   jstring pathname)
{
    jboolean rv = JNI_FALSE;

    WITH_PLATFORM_STRING(env, pathname, path) {
	int orv;
	JVM_NativePath((char *)path);
	orv = JVM_Open(path, JVM_O_RDWR | JVM_O_CREAT | JVM_O_EXCL, 0666);
	if (orv < 0) {
	    if (orv != JVM_EEXIST) {
		JNU_ThrowIOExceptionWithLastError(env, path);
	    }
	} else {
	    JVM_Close(orv);
	    rv = JNI_TRUE;
	}
    } END_PLATFORM_STRING(env, path);
    return rv;
}


static int
removeFileOrDirectory(const char *path) /* Returns 0 on success */
{
    DWORD a;

    SetFileAttributes(path, 0);
    a = GetFileAttributes(path);
    if (a == ((DWORD)-1)) {
	return 1;
    } else if (a & FILE_ATTRIBUTE_DIRECTORY) {
	return !RemoveDirectory(path);
    } else {
	return !DeleteFile(path);
    }
}


JNIEXPORT jboolean JNICALL
Java_java_io_Win32FileSystem_delete0(JNIEnv *env, jobject this,
                                     jobject file)
{
    jboolean rv = JNI_FALSE;

    WITH_NATIVE_PATH(env, file, ids.path, path) {
	if (removeFileOrDirectory(path) == 0) {
	    rv = JNI_TRUE;
	}
    } END_NATIVE_PATH(env, path);
    return rv;
}


JNIEXPORT jboolean JNICALL
Java_java_io_Win32FileSystem_deleteOnExit(JNIEnv *env, jobject this,
					  jobject file)
{
    WITH_NATIVE_PATH(env, file, ids.path, path) {
	deleteOnExit(env, path, removeFileOrDirectory);
    } END_NATIVE_PATH(env, path);
    return JNI_TRUE;
}


/* ## Clean this up to use direct Win32 calls */

JNIEXPORT jobjectArray JNICALL
Java_java_io_Win32FileSystem_list(JNIEnv *env, jobject this,
				  jobject file)
{
    DIR *dir;
    struct dirent *ptr;
    int len, maxlen;
    jobjectArray rv, old;

    WITH_NATIVE_PATH(env, file, ids.path, path) {
	dir = opendir(path);
    } END_NATIVE_PATH(env, path);
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
    if (rv == NULL) goto error;
    if (JNU_CopyObjectArray(env, rv, old, len) < 0) goto error;
    return rv;

 error:
    closedir(dir);
    return NULL;
}


JNIEXPORT jboolean JNICALL
Java_java_io_Win32FileSystem_createDirectory(JNIEnv *env, jobject this,
					     jobject file)
{
    jboolean rv = JNI_FALSE;

    WITH_NATIVE_PATH(env, file, ids.path, path) {
	if (mkdir(path) == 0) {
	    rv = JNI_TRUE;
	}
    } END_NATIVE_PATH(env, path);
    return rv;
}


JNIEXPORT jboolean JNICALL
Java_java_io_Win32FileSystem_rename0(JNIEnv *env, jobject this,
                                     jobject from, jobject to)
{
    jboolean rv = JNI_FALSE;

    WITH_NATIVE_PATH(env, from, ids.path, fromPath) {
	WITH_NATIVE_PATH(env, to, ids.path, toPath) {
	    if (rename(fromPath, toPath) == 0) {
		rv = JNI_TRUE;
	    }
	} END_NATIVE_PATH(env, toPath);
    } END_NATIVE_PATH(env, fromPath);
    return rv;
}


JNIEXPORT jboolean JNICALL  
Java_java_io_Win32FileSystem_setLastModifiedTime(JNIEnv *env, jobject this,
						 jobject file, jlong time)
{
    jboolean rv = JNI_FALSE;

    WITH_NATIVE_PATH(env, file, ids.path, path) {
	HANDLE h;
	h = CreateFile(path, GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
		       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, 0);
	if (h != INVALID_HANDLE_VALUE) {
	    LARGE_INTEGER modTime;
	    FILETIME t;
	    modTime.QuadPart = (time + 11644473600000L) * 10000L;
	    t.dwLowDateTime = (DWORD)modTime.LowPart;
	    t.dwHighDateTime = (DWORD)modTime.HighPart;
	    if (SetFileTime(h, NULL, NULL, &t)) {
		rv = JNI_TRUE;
	    }
	    CloseHandle(h);
	}
    } END_NATIVE_PATH(env, path);

    return rv;
}


JNIEXPORT jboolean JNICALL  
Java_java_io_Win32FileSystem_setReadOnly(JNIEnv *env, jobject this,
					 jobject file)
{
    jboolean rv = JNI_FALSE;

    WITH_NATIVE_PATH(env, file, ids.path, path) {
	DWORD a;
	a = GetFileAttributes(path);
	if (a != ((DWORD)-1)) {
	    if (SetFileAttributes(path, a | FILE_ATTRIBUTE_READONLY))
		rv = JNI_TRUE;
	}
    } END_NATIVE_PATH(env, path);
    return rv;
}


/* -- Filesystem interface -- */


JNIEXPORT jobject JNICALL
Java_java_io_Win32FileSystem_getDriveDirectory(JNIEnv *env, jclass ignored,
					       jint drive)
{
    char buf[_MAX_PATH];
    char *p = _getdcwd(drive, buf, sizeof(buf));
    if (p == NULL) return NULL;
    if (isalpha(*p) && (p[1] == ':')) p += 2;
    return JNU_NewStringPlatform(env, p);
}


JNIEXPORT jint JNICALL
Java_java_io_Win32FileSystem_listRoots0(JNIEnv *env, jclass ignored)
{
    return GetLogicalDrives();
}
