/*
 * @(#)exec_md.c	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "sys.h"
#include "util.h"

#ifdef LINUX
extern int fork1(void);
#endif

static char *skipWhitespace(char *p) {
    while ((*p != '\0') && isspace(*p)) {
        p++;
    }
    return p;
}

static char *skipNonWhitespace(char *p) {
    while ((*p != '\0') && !isspace(*p)) {
        p++;
    }
    return p;
}

int 
dbgsysExec(char *cmdLine)
{
    int i;
    int argc;
    pid_t pid_err = (pid_t)(-1); /* this is the error return value */
    pid_t pid;
    char **argv = NULL;
    char *p;
    char *args;

    /* Skip leading whitespace */
    cmdLine = skipWhitespace(cmdLine);

    /*LINTED*/
    args = jvmtiAllocate((jint)strlen(cmdLine)+1);
    if (args == NULL) {
        return SYS_NOMEM;
    }
    (void)strcpy(args, cmdLine);

    p = args;

    argc = 0;
    while (*p != '\0') {
        p = skipNonWhitespace(p);
        argc++;
        if (*p == '\0') {
            break;
        }
        p = skipWhitespace(p);
    }

    /*LINTED*/
    argv = jvmtiAllocate((argc + 1) * (jint)sizeof(char *));
    if (argv == 0) {
        jvmtiDeallocate(args);
        return SYS_NOMEM;
    }

    for (i = 0, p = args; i < argc; i++) {
        argv[i] = p;
        p = skipNonWhitespace(p);
        *p++ = '\0';
        p = skipWhitespace(p);
    }
    argv[i] = NULL;  /* NULL terminate */

    if ((pid = fork1()) == 0) {
        /* Child process */
        int i;
	long max_fd;

        /* close everything */
        max_fd = sysconf(_SC_OPEN_MAX);
	/*LINTED*/
        for (i = 3; i < (int)max_fd; i++) {
            (void)close(i);
        }

        (void)execvp(argv[0], argv);

        exit(-1);
    }
    jvmtiDeallocate(args);
    jvmtiDeallocate(argv);
    if (pid == pid_err) {
        return SYS_ERR;
    } else {
        return SYS_OK;
    }
}

