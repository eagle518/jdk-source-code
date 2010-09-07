/*
 * @(#)timeval_md.h	1.19 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef _JAVASOFT_SOLARIS_TIMEVAL_H_
#define _JAVASOFT_SOLARIS_TIMEVAL_H_

typedef struct {
	long tv_sec;		/* seconds */
	long tv_usec;		/* microseconds (NOT milliseconds) */
} timeval_t;

#endif /* !_JAVASOFT_SOLARIS_TIMEVAL_H_ */
