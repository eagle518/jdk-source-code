/*
 * @(#)system_md.c	1.82 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "hpi_impl.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h> /* timeval */
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <netdb.h>
#include <limits.h>
#include <errno.h>

#include <dlfcn.h>

#include "jni_md.h"
#include "mutex_md.h"

#include "hpi_init.h"

#include "interrupt.h"
#include "threads_md.h"
#include "monitor_md.h"
#include "largefile.h"


#define O_DELETE 0x10000

int sysThreadBootstrap(sys_thread_t **tidP, sys_mon_t **lockP, int nb)
{
    threadBootstrapMD(tidP, lockP, nb);

    intrInit();

#ifndef NATIVE
    /* Initialize the special case for sbrk on Solaris (see synch.c) */
    InitializeSbrk();
    /* Initialize the async io */
    InitializeAsyncIO();
    InitializeMem();
    /* Initialize Clock and Idle threads */
    InitializeHelperThreads();
#else /* if NATIVE */
    initializeContentionCountMutex();
    InitializeMem();
#endif /* NATIVE */

    return SYS_OK;
}

int sysShutdown()
{
    return SYS_OK;
}

long
sysGetMilliTicks()
{
    struct timeval tv;

    (void) gettimeofday(&tv, (void *) 0);
    return((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

jlong
sysTimeMillis()
{
    struct timeval t;
    gettimeofday(&t, 0);
    return ((jlong)t.tv_sec) * 1000 + (jlong)(t.tv_usec/1000);
}

int
sysGetLastErrorString(char *buf, int len)
{
    if (errno == 0) {
	return 0;
    } else {
	const char *s = strerror(errno);
	int n = strlen(s);
	if (n >= len) n = len - 1;
	strncpy(buf, s, n);
	buf[n] = '\0';
	return n;
    }
}

/*
 * File system
 *
 * These are all the sys API which implement the straight POSIX
 * API. Those that do not are defined by thread-specific files
 * (i.e. io_md.c)
 */

/*
 * Open a file. Unlink the file immediately after open returns 
 * if the specified oflag has the O_DELETE flag set.
 */
int sysOpen(const char *path, int oflag, int mode)
{
    int fd;
    int delete = (oflag & O_DELETE);
    oflag = oflag & ~O_DELETE;
    fd = open64_w(path, oflag, mode);
    if (delete != 0) {
        unlink(path);
    }
    return fd;
}

char *sysNativePath(char *path)
{
    return path;
}

int
sysFileSizeFD(int fd, jlong *size)
{
    struct stat64 buf64;
    int ret = fstat64(fd, &buf64);
    *size = buf64.st_size;
    return ret;
}

int
sysFfileMode(int fd, int *mode)
{
    struct stat64 buf64;
    int ret = fstat64(fd, &buf64);
    (*mode) = buf64.st_mode;
    return ret;
}

int
sysFileType(const char *path)
{
    int ret;
    struct stat buf;

    if ((ret = stat(path, &buf)) == 0) {
      mode_t mode = buf.st_mode & S_IFMT;
      if (mode == S_IFREG) return SYS_FILETYPE_REGULAR;
      if (mode == S_IFDIR) return SYS_FILETYPE_DIRECTORY;
      return SYS_FILETYPE_OTHER;
    }
    return ret;
}

/* 
 * Wrapper functions for low-level I/O routines - use the 64 bit
 * version if available, else revert to the 32 bit versions.
 */

off64_t 
lseek64_w(int fd, off64_t offset, int whence)
{
    return lseek64(fd, offset, whence);
}

int 
ftruncate64_w(int fd, off64_t length) 
{
    return ftruncate64(fd, length);
}

int 
open64_w(const char *path, int oflag, int mode)
{
    int result = open64(path, oflag, mode);
    if (result != -1) {
	/* If the open succeeded, the file might still be a directory */
	int st_mode;
	if (sysFfileMode(result, &st_mode) != -1) {
	    if ((st_mode & S_IFMT) == S_IFDIR) {
		errno = EISDIR;
		close(result);
		return -1;
	    }
	} else {
	    close(result);
	    return -1;
	}
    } 
    return result;
}  

void setFPMode(void)
{
#ifdef i386
    asm("	pushl $575");
    asm("	fldcw (%esp)");
    asm("	popl %eax");
#endif
#if defined(__linux__) && defined(__mc68000__)
    asm("     fmovel #0x80,%fpcr");
#endif
}
