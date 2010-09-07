/*
 * @(#)io_util_md.c	1.19 04/01/27
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jni_util.h"
#include "jvm.h"
#include "io_util.h"
#include "io_util_md.h"
#include <stdio.h>

#include "java_io_RandomAccessFile.h"

#include <io.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <wincon.h>

extern jboolean onNT = JNI_FALSE;

static int MAX_INPUT_EVENTS = 2000;

void
initializeWindowsVersion() {
    OSVERSIONINFO ver;
    ver.dwOSVersionInfoSize = sizeof(ver);
    GetVersionEx(&ver);
    if (ver.dwPlatformId == VER_PLATFORM_WIN32_NT) {
        onNT = JNI_TRUE;
    } else {
        onNT = JNI_FALSE;
    }
}

/* If this returns NULL then an exception is pending */
WCHAR* fileToNTPath(JNIEnv *env, jobject file, jfieldID id)
{
    jstring path = NULL;
    if (file != NULL) {
        path = (*(env))->GetObjectField(env, file, id);
    }
    return pathToNTPath(env, path, JNI_FALSE);
}

/* If this returns NULL then an exception is pending */
WCHAR* pathToNTPath(JNIEnv *env, jstring path, jboolean throwFNFE)
{
    int pathlen = 0;
    WCHAR *pathbuf;

    WITH_UNICODE_STRING(env, path, ps) {
        pathlen = wcslen(ps);
        if (pathlen != 0) {
            if (pathlen > MAX_PATH - 1) {
                /* copy \\?\ to the front of path */
                pathbuf = (WCHAR*)malloc((pathlen + 10) * 2);
                if (pathbuf != 0) {                    
                    pathbuf[0] = L'\0';
                    wcscpy(pathbuf, L"\\\\?\\\0");
                    wcscat(pathbuf, ps);
                }
            } else {
                pathbuf = (WCHAR*)malloc((pathlen + 6) * 2);
                if (pathbuf != 0) {
                    pathbuf[0] = L'\0';
                    wcscpy(pathbuf, ps);
                }
            }
        }
    } END_UNICODE_STRING(env, ps);

    if (pathlen == 0) {
        if (throwFNFE == JNI_TRUE) {
            throwFileNotFoundException(env, path);
            return NULL;
        } else {
            pathbuf = (WCHAR*)malloc(2);
            pathbuf[0] = L'\0';
        }
    }
    if (pathbuf == 0) {
        JNU_ThrowOutOfMemoryError(env, 0);
        return NULL;
    }
    return pathbuf;
}

void
fileOpen(JNIEnv *env, jobject this, jstring path, jfieldID fid, int flags)
{
    DWORD access = 0;
    DWORD sharing = FILE_SHARE_READ | FILE_SHARE_WRITE;
    DWORD disposition = OPEN_EXISTING;
    DWORD flagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
    HANDLE h = NULL;
    int pathlen = 0;

    /* Note: O_TRUNC overrides O_CREAT */
    if (flags & O_TRUNC) {
	disposition = CREATE_ALWAYS;
    } else if (flags & O_CREAT) {
	disposition = OPEN_ALWAYS;
    }
    if (flags & O_SYNC) {
	flagsAndAttributes = FILE_FLAG_WRITE_THROUGH;
    }
    if (flags & O_DSYNC) {
	flagsAndAttributes = FILE_FLAG_WRITE_THROUGH;
    }
    if (flags & O_RDONLY) {
	access = GENERIC_READ;
    }
    if (flags & O_WRONLY) {
	access = GENERIC_WRITE;
    }
    if (flags & O_RDWR) {
	access = GENERIC_READ | GENERIC_WRITE;
    }
    if (flags == 0) {
	access = GENERIC_READ;
    }

    if (onNT) {
	WCHAR *pathbuf = pathToNTPath(env, path, JNI_TRUE);
	if (pathbuf == NULL) {
	    /* Exception already pending */
	    return;
	}
	
        h = CreateFileW(
            pathbuf,   /* Wide char path name */
            access,    /* Combine read and/or write permission */
            sharing,   /* File sharing flags */
            NULL,      /* Security attributes */
            disposition,         /* creation disposition */
            flagsAndAttributes,  /* flags and attributes */
            NULL);

        free(pathbuf);
    } else {
	WITH_PLATFORM_STRING(env, path, _ps) {
	    h = CreateFile(_ps, access, sharing, NULL, disposition,
			   flagsAndAttributes, NULL);
	} END_PLATFORM_STRING(env, _ps);
    }

    if (h == INVALID_HANDLE_VALUE) {
	int error = GetLastError();
	if (error == ERROR_TOO_MANY_OPEN_FILES) {
	    JNU_ThrowByName(env, JNU_JAVAIOPKG "IOException",
                            "Too many open files");
	    return;
	}
	throwFileNotFoundException(env, path);
	return;
    }
    SET_FD(this, (jlong)h, fid);
}

/* These are functions that use a handle fd instead of the
   old C style int fd as is used in HPI layer */

static int
handleNonSeekAvailable(jlong, long *);
static int
handleStdinAvailable(jlong, long *);

int
handleAvailable(jlong fd, jlong *pbytes) {
    jlong current, end;
    HANDLE h = (HANDLE)fd;
    DWORD type = 0;

    type = GetFileType(h);
    /* Handle is for keyboard or pipe */
    if (type == FILE_TYPE_CHAR || type == FILE_TYPE_PIPE) {
        int ret;
        long lpbytes;
        HANDLE stdInHandle = GetStdHandle(STD_INPUT_HANDLE);
        if (stdInHandle == h) {
            ret = handleStdinAvailable(fd, &lpbytes); /* keyboard */
        } else {
            ret = handleNonSeekAvailable(fd, &lpbytes); /* pipe */
        }
        (*pbytes) = (jlong)(lpbytes);
        return ret;
    }
    /* Handle is for regular file */
    if (type == FILE_TYPE_DISK) {
        long highPos = 0;
        DWORD sizeLow = 0;
        DWORD sizeHigh = 0;
        DWORD lowPos = SetFilePointer(h, 0, &highPos, FILE_CURRENT);
        if (lowPos == ((DWORD)-1)) {
            return FALSE;
        }
        current = (((jlong)highPos) << 32) | lowPos;
        end = GetFileSize(h, &sizeHigh);
        if (sizeLow == ((DWORD)-1)) {
            return FALSE;
        }
        *pbytes = end - current;
        return TRUE;
    }
    return FALSE;
}

static int
handleNonSeekAvailable(jlong fd, long *pbytes) {
    /* This is used for available on non-seekable devices
     * (like both named and anonymous pipes, such as pipes
     *  connected to an exec'd process).
     * Standard Input is a special case.
     *
     */
    HANDLE han;

    if ((han = (HANDLE) fd) == INVALID_HANDLE_VALUE) {
	return FALSE;
    }

    if (! PeekNamedPipe(han, NULL, 0, NULL, pbytes, NULL)) {
	/* PeekNamedPipe fails when at EOF.  In that case we
	 * simply make *pbytes = 0 which is consistent with the
	 * behavior we get on Solaris when an fd is at EOF.
	 * The only alternative is to raise and Exception,
	 * which isn't really warranted.
	 */
	if (GetLastError() != ERROR_BROKEN_PIPE) {
	    return FALSE;
	}
	*pbytes = 0;
    }
    return TRUE;
}

static int
handleStdinAvailable(jlong fd, long *pbytes) {
    HANDLE han;
    DWORD numEventsRead = 0;	/* Number of events read from buffer */
    DWORD numEvents = 0;	/* Number of events in buffer */
    DWORD i = 0;		/* Loop index */
    DWORD curLength = 0;	/* Position marker */
    DWORD actualLength = 0;	/* Number of bytes readable */
    BOOL error = FALSE;         /* Error holder */
    INPUT_RECORD *lpBuffer;     /* Pointer to records of input events */
    DWORD bufferSize = 0;

    if ((han = GetStdHandle(STD_INPUT_HANDLE)) == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    /* Construct an array of input records in the console buffer */
    error = GetNumberOfConsoleInputEvents(han, &numEvents);
    if (error == 0) {
        return handleNonSeekAvailable(fd, pbytes);
    }

    /* lpBuffer must fit into 64K or else PeekConsoleInput fails */
    if (numEvents > MAX_INPUT_EVENTS) {
        numEvents = MAX_INPUT_EVENTS;
    }

    bufferSize = numEvents * sizeof(INPUT_RECORD);
    if (bufferSize == 0)
        bufferSize = 1;
    lpBuffer = malloc(bufferSize);
    if (lpBuffer == NULL) {
     	return FALSE;
    }

    error = PeekConsoleInput(han, lpBuffer, numEvents, &numEventsRead);
    if (error == 0) {
	free(lpBuffer);
	return FALSE;
    }

    /* Examine input records for the number of bytes available */
    for(i=0; i<numEvents; i++) {
	if (lpBuffer[i].EventType == KEY_EVENT) {
            KEY_EVENT_RECORD *keyRecord = (KEY_EVENT_RECORD *)
                                          &(lpBuffer[i].Event);
	    if (keyRecord->bKeyDown == TRUE) {
                CHAR *keyPressed = (CHAR *) &(keyRecord->uChar);
	       	curLength++;
	       	if (*keyPressed == '\r')
                    actualLength = curLength;
            }
        }
    }
    if(lpBuffer != NULL)
       	free(lpBuffer);
    *pbytes = (long) actualLength;
    return TRUE;
}

/*
 * This is documented to succeed on read-only files, but Win32's
 * FlushFileBuffers functions fails with "access denied" in such a
 * case.  So we only signal an error if the error is *not* "access
 * denied".
 */

JNIEXPORT int
handleSync(jlong fd) {
    /*
     * From the documentation:
     *
     *	   On Windows NT, the function FlushFileBuffers fails if hFile
     *	   is a handle to console output. That is because console
     *	   output is not buffered. The function returns FALSE, and
     *	   GetLastError returns ERROR_INVALID_HANDLE.
     *
     * On the other hand, on Win95, it returns without error.  I cannot
     * assume that 0, 1, and 2 are console, because if someone closes
     * System.out and then opens a file, they might get file descriptor
     * 1.  An error on *that* version of 1 should be reported, whereas
     * an error on System.out (which was the original 1) should be
     * ignored.  So I use isatty() to ensure that such an error was due
     * to this bogosity, and if it was, I ignore the error.
     */

    HANDLE handle = (HANDLE)fd;

    if (!FlushFileBuffers(handle)) {
	if (GetLastError() != ERROR_ACCESS_DENIED) {	/* from winerror.h */
	    return -1;
	}
    }
    return 0;
}


int
handleSetLength(jlong fd, jlong length) {
    HANDLE h = (HANDLE)fd;
    long high = (long)(length >> 32);
    DWORD ret;

    if (h == (HANDLE)(-1)) return -1;
    ret = SetFilePointer(h, (long)(length), &high, FILE_BEGIN);
    if (ret == 0xFFFFFFFF && GetLastError() != NO_ERROR) {
        return -1;
    }
    if (SetEndOfFile(h) == FALSE) return -1;
    return 0;
}

int
handleFileSizeFD(jlong fd, jlong *size)
{
    DWORD sizeLow = 0;
    DWORD sizeHigh = 0;
    HANDLE h = (HANDLE)fd;
    if (h == INVALID_HANDLE_VALUE) {
        return -1;
    }
    sizeLow = GetFileSize(h, &sizeHigh);
    if (sizeLow == ((DWORD)-1)) {
        if (GetLastError() != ERROR_SUCCESS) {
            return -1;
        }
    }
    return (((jlong)sizeHigh) << 32) | sizeLow;
}

JNIEXPORT 
size_t 
handleRead(jlong fd, void *buf, jint len)
{
    DWORD read = 0;
    BOOL result = 0;
    HANDLE h = (HANDLE)fd;
    if (h == INVALID_HANDLE_VALUE) {
        return -1;
    }
    result = ReadFile(h,          /* File handle to read */
                      buf,        /* address to put data */
                      len,        /* number of bytes to read */
                      &read,      /* number of bytes read */
                      NULL);      /* no overlapped struct */
    if (result == 0) {
        int error = GetLastError();
        if (error == ERROR_BROKEN_PIPE) {
            return 0; /* EOF */
        }
        return -1;
    }
    return read;
}

JNIEXPORT 
size_t 
handleWrite(jlong fd, const void *buf, jint len)
{
    BOOL result = 0;
    DWORD written = 0;
    HANDLE h = (HANDLE)fd;
    if (h != INVALID_HANDLE_VALUE) {
        result = WriteFile(h,           /* File handle to write */
                      buf,              /* pointers to the buffers */
                      len,              /* number of bytes to write */
                      &written,         /* receives number of bytes written */
                      NULL);            /* no overlapped struct */
    }
    if ((h == INVALID_HANDLE_VALUE) || (result == 0)) {
        return -1;
    }
    return written;
}

jint
handleClose(JNIEnv *env, jobject this, jfieldID fid)
{
    FD fd = GET_FD(this, fid);
    HANDLE h = (HANDLE)fd;
    if (fd != -1) {
        jint result = CloseHandle(h);
        if (result == 0) { /* Returns zero on failure */
            JNU_ThrowIOExceptionWithLastError(env, "close failed");
        } else {
            SET_FD(this, -1, fid);
        }
    }
    return 0;
}

jlong 
handleLseek(jlong fd, jlong offset, jint whence)
{
    DWORD lowPos = 0;
    long highPos = 0;
    DWORD op = FILE_CURRENT;
    HANDLE h = (HANDLE)fd;

    if (whence == SEEK_END) {
        op = FILE_END;
    }
    if (whence == SEEK_CUR) {
        op = FILE_CURRENT;
    }
    if (whence == SEEK_SET) {
        op = FILE_BEGIN;
    }

    lowPos = (DWORD)offset;
    highPos = (long)(offset >> 32);
    lowPos = SetFilePointer(h, lowPos, &highPos, op);
    if (lowPos == ((DWORD)-1)) {
        if (GetLastError() != ERROR_SUCCESS) {
            return -1;
        }
    }
    return (((jlong)highPos) << 32) | lowPos;
}
