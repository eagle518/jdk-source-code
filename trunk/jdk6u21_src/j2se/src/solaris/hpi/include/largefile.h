/*
 * @(#)largefile.h	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef _JAVASOFT_LARGEFILE_SUPPORT_H_
#define _JAVASOFT_LARGEFILE_SUPPORT_H_

#ifdef __solaris__
#include "largefile_solaris.h"
#endif

#ifdef __linux__
#include "largefile_linux.h"
#endif

/*
 * Prototypes for wrappers that we define.  These wrapper functions
 * are low-level I/O routines that will use 64 bit versions if
 * available, else revert to the 32 bit ones.
 */
extern off64_t lseek64_w(int fd, off64_t offset, int whence);
extern int fstat64_w(int fd, struct stat *buf);
extern int ftruncate64_w(int fd, off64_t length);
extern int open64_w(const char *path, int oflag, int mode);

/* This is defined in system_md.c */
extern int sysFfileMode(int fd, int* mode);

#endif /* _JAVASOFT_LARGEFILE_SUPPORT_H_ */
