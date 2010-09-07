/*
 * @(#)sys.h	1.13 04/01/12
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JDWP_SYS_H
#define JDWP_SYS_H

#define SYS_OK	       0
#define SYS_ERR	       -1
#define SYS_INTRPT     -2
#define SYS_TIMEOUT    -3
#define SYS_NOMEM      -5
#define SYS_NORESOURCE -6
#define SYS_INUSE      -7
#define SYS_DIED       -8

/* Implemented in linker_md.c */

void	dbgsysBuildLibName(char *, int, char *, char *);
int     dbgsysBuildFunName(char *, int, int, int);
void *  dbgsysLoadLibrary(const char *, char *err_buf, int err_buflen);
void    dbgsysUnloadLibrary(void *);
void *  dbgsysFindLibraryEntry(void *, const char *);

/* Implemented in exec_md.c */
int     dbgsysExec(char *cmdLine);

#endif
