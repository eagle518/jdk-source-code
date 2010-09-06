/*
 * @(#)buffered_pipe_interface.c	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#ifdef __linux__
#include <stdarg.h>
#else
#include <sys/varargs.h>
#endif

#include <fcntl.h>

#include "pipe_interface.h"

#define MAX_PIPES 500
#define INIT_DATA_ALLOC 200

/* Implementation of the pipe_interface.h.
   This implementation buffers the message read and then returns
   it during subsequent reads. */
typedef struct pipe_storage_struct {
  char* data;
  int index;
  int avail;
  int allocated;
} pipe_storage;


pipe_storage* pipe_data[MAX_PIPES];

static void 
err(char *format,...) {
    va_list(args);
    va_start(args, format);
    fprintf(stderr, "\n**************** SERVER PIPE ERROR **************\n");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n**************** ************ **************\n");
    va_end(args);
}

void
init_pipe_interface() {
  int i;


  for(i = 0; i < MAX_PIPES; i++) {
    pipe_storage* p = (pipe_storage *) malloc(sizeof(pipe_storage));
    p->index = 0;
    p->avail = 0;
    p->data = (char*) malloc(INIT_DATA_ALLOC);
    p->allocated = INIT_DATA_ALLOC;
    pipe_data[i] = p;
  }
}

  
static void 
buffered_pipe_read(int pipe, char *into, int len) {
  int rc;
  int rem_len = len;
  int i, k, l;

  /* Put this in a loop in case the fd is non-blocking */
  for(i = 0;;i++) {
    rc = read(pipe, into, rem_len);

    if (rc != -1) {
      rem_len -= rc;
      into += rc;
      if (rem_len == 0) {
	return;
      }
    } 
  }
}

/* Read the next message on the pipe. The message
   will start with a size, and then the rest 
   of the message. */
void
read_message(int pipe) {
  int n;
  int readres;
  pipe_storage* ps = pipe_data[pipe];

  buffered_pipe_read(pipe, (char *) &n, 4);
  
  if (ps->allocated < n) {
    fprintf(stdout, "Allocating more space for read msg %d\n", n);
    if (ps->data != NULL) 
      free(ps->data);
    ps->data = (char *) malloc(n);
    ps->allocated = n;
  }
  /*
  if (ps->index != ps->avail) 
    err("server get_message: Old data! %d %d\n",
	    ps->index, ps->avail);
  */
  ps->index = 0;
  buffered_pipe_read(pipe, (char *) ps->data, n);

  ps->avail = n;
}

void
get_bytes(int pipe, void* into, int nbytes) {
  pipe_storage* ps = pipe_data[pipe];
  if ((ps->avail - ps->index) < nbytes) 
    err("get_bytes: Not enough data "
	" avail=%d ind=%d nbytes=%d\n",
	ps->avail, ps->index, nbytes);
  memcpy(into, &(ps->data[ps->index]), nbytes);
  ps->index += nbytes;
}

int
get_bits32(int pipe) {
  int res;
  get_bytes(pipe, &res, 4);
  return res;
}

char* 
get_string(int pipe) {
    short len;
    int readres;
    char *str;
    
    /* Read the string length */

    get_bytes(pipe, &len, 2);

    /* Malloc the string with null termination */
    str = (char*) malloc(len+1);

    get_bytes(pipe, str, len);

    str[len] = '\0';

    return str;
}


