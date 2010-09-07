/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "commonhdr.h"
#include "CReadBuffer.h"
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

CReadBuffer::CReadBuffer(int fd) {
    m_fd = fd;
}

int CReadBuffer::getInt(int * x) {

    char buff[4];
    int rv = getIt(buff, 4);
    int result = (buff[0] << 24) |
		 ((buff[1] << 16) & 0xFF0000) |
		 ((buff[2] << 8) & 0xFF00) |
		 (buff[3] & 0xFF);
    *x = result;
    return rv;
}

int CReadBuffer::getShort(short * x) {

    char buff[2];
    int rv = getIt(buff, 2);
    short result = (buff[0] << 8) |
		   (buff[1] & 0xFF);
    *x = result;
    return rv;
}

/* getString tries to read string from the pipe
   If succeed, return 0, -1 on error
*/
int CReadBuffer::getString(char ** x) {

    if (x == NULL)
      return -1;

    *x = NULL;
    
    int rc = 0;
    int length;

    rc = getInt(&length);
    if (rc <= 0) {
        *x = NULL;
        return -1;
    }

    char* buff;
    buff = (char *) malloc((length + 1) * sizeof(char));
    
    if (buff == NULL) {
      *x = NULL;
      return -1;
    }
    
    rc = getIt(buff,length); 

    if (rc == length) {
      buff[length] = 0;
      *x = buff;
    }
    else
      ::free(buff);

    return rc == length ? 0:-1;
}

int CReadBuffer::getByte(char * x) {
    return getIt((char *) x, 1);
}


void CReadBuffer::free(char * buf) {
  if (buf)
    ::free(buf);
}

/* getIt is a helper function used by all other funcs
   it returns the number of characters reading from
   the pipe
*/
int CReadBuffer::getIt(char * buff,int length) {

    int rc;
    int offset = 0;

    errno = 0;
    
    while(offset != length) {
	rc = read(m_fd, buff+offset, length-offset);
	/* The read system call may return EAGAIN due to 
	   resource limitation, in that case, we should
	   really try again and again unless the errno is 
	   other than EAGAIN */
	if (rc <= 0 && errno != EAGAIN)
	  return offset;

	if (rc > 0)
	  offset += rc;
    }
    return offset;
}
