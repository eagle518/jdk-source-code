/*
 * @(#)util5.cpp	1.13 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* Some helper routines for nspr methods */

#include "commonhdr.h"
#include "utils.h"
#include "util5.h"
#include "IUnixService.h"

extern IUnixService* g_unixService;

#define UNUSED(x) x=x

char*
checked_malloc(int nbytes) {
    /* Use the nspr malloc. For now use plain malloc */
    return (char*) malloc(nbytes);
}
	
/* Read length bytes fully from 'pr' into buff, reporting an error
   if reading fails for some reason. 0 on failure */
int
read_JD_fully(const char* msg, void* pr, char* buffer, int length) {
    int cur, res = 0;
    int rem_length = length;
    if (length == 0) return 1;
    while (rem_length > 0) {
	cur = g_unixService->JD_Read(pr, buffer, rem_length);
	if (cur == rem_length) return 1;
	if (cur <= 0) {
	    plugin_error("Pipe closed during read? State may be corrupt");
	    return 0;
	}
	buffer = ((char*) buffer + cur);
	res += cur;
	rem_length -= cur;
    }
    UNUSED(msg);
    return 1;
}


/* Counterpart to read fully. 0 on failure */
int
write_JD_fully(const char* msg, void* pr, char* buff, int len) {
    int offset = 0;
    int rc;
    while (offset < len) {
        rc = g_unixService->JD_Write(pr, buff + offset, len - offset);
        if (rc <= 0) {
	    plugin_error("Write failed. %s \n", msg);
	    return 0;
	}
	offset += rc;
    }
    return 1;
}
		
/* Create a half duplex nspr pipe */
void
wrap_JD_CreatePipe(const char* msg, void **readPipe, void **writePipe) {
    /* We could use the TCPSocketPair to get a full duplex channel but
       there was a comment in prio.h (the nspr header) about whether
       TCPSocketPair is implemented on all platforms
       */
    g_unixService->JD_CreatePipe(readPipe, writePipe);
    if (tracing) {
	int readfd = g_unixService->JDFileDesc_To_FD(*readPipe);
	int writefd = g_unixService->JDFileDesc_To_FD(*readPipe);
	int readflags = fcntl(readfd, F_GETFL);
	int writeflags = fcntl(writefd, F_GETFL);
	trace("Created pipe %s read=%d write=%d\n", msg, readfd, writefd);
	trace(" read flags=%X write flags=%X\n", readflags, writeflags);
    }
}




/* Dup2 and check */
int
wrap_dup2(int from, int to) {
    int res = -1;
    if ((res = dup2(from, to)) < 0) {
	/* Don't use plugin error - may be called from new process which
	   may not have access to files for plugin error */
	fprintf(stderr, "Internal error : Could not dup %d into %d\n", 
		from, to);
    } 
    return res;
}
		
int
wrap_JD_CreateSocketPair(const char* msg, void* fds[]){
    int rc = (int)(g_unixService->JD_NewTCPSocketPair(fds));
    if (tracing) {
	int readfd = g_unixService->JDFileDesc_To_FD(fds[0]);
	int writefd = g_unixService->JDFileDesc_To_FD(fds[1]);
	int readflags = fcntl(readfd, F_GETFL);
	int writeflags = fcntl(writefd, F_GETFL);
	trace("Created pipe %s read=%d write=%d\n", msg, readfd, writefd);
	trace(" read flags=%X write flags=%X\n", readflags, writeflags);
    }
    return rc;
}




