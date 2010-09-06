/*
 * @(#)util5.h	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

extern "C" {
char* checked_malloc(int nbytes);

int wrap_dup2(int from, int to);

int read_JD_fully(const char* msg, void* pr, char* buffer, int length);

int write_JD_fully(const char* msg, void* pr, char* buff, int len);

void wrap_JD_CreatePipe(const char* msg, void **readPipe,  void **writePipe);

int wrap_JD_CreateSocketPair(const char* msg, void* fds[]);
}

