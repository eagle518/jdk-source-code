/*
 * @(#)proc_md.h	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* Posix threads (Solaris and Linux) */

#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#define MUTEX_T pthread_mutex_t
#define MUTEX_INIT PTHREAD_MUTEX_INITIALIZER
#define MUTEX_LOCK(x)   (void)pthread_mutex_lock(&x)
#define MUTEX_UNLOCK(x) (void)pthread_mutex_unlock(&x)
#define GET_THREAD_ID()	pthread_self()
#define THREAD_T pthread_t
#define PID_T pid_t
#define GETPID() getpid()
#define GETMILLSECS(millisecs) 					\
	{ 							\
		struct timeval tval;				\
		(void)gettimeofday(&tval,NULL);			\
		millisecs = ((int)(tval.tv_usec/1000));  	\
	}
