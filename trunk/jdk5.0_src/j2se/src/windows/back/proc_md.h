/*
 * @(#)proc_md.h	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* Windows process id's and threads */

#include <process.h>
#include <time.h>

#define MUTEX_T 	int
#define MUTEX_INIT 	0
#define MUTEX_LOCK(x) 		/* FIXUP? */
#define MUTEX_UNLOCK(x) 	/* FIXUP? */
#define GET_THREAD_ID()	GetCurrentThreadId()
#define THREAD_T 	unsigned long
#define PID_T 		int
#define GETPID() 	getpid()
#define GETMILLSECS(millisecs) (millisecs=0)

#define popen	_popen
#define pclose	_pclose
#define sleep	_sleep

