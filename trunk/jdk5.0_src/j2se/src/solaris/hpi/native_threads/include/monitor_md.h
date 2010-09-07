/*
 * @(#)monitor_md.h	1.34 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Monitor interface	10/25/94
 *
 * Private data structures and interfaces used in the monitor code.
 * This file is used to share declarations and such between the different
 * files implementing monitors.  It does not contain exported API.
 */

#ifndef _JAVASOFT_SOLARIS_MONITOR_MD_H_
#define _JAVASOFT_SOLARIS_MONITOR_MD_H_

#include <mutex_md.h>
#include <condvar_md.h>
#include <threads_md.h>

/*
 * Type definitions.
 */

typedef struct monitor_waiter monitor_waiter_t;
typedef struct monitor_wait_queue monitor_wait_queue_t;

/* Element of the MonitorWaitQ - representing thread doing a monitor wait */
/*
 * The only reason we do the queueing is for sysMonitorDumpInfo.
 * The counting, though, is used to avoid extraneous calls to
 * condvarBroadcast and condvarSignal, for instance.
 */
struct monitor_waiter {
    monitor_waiter_t   *next;
    monitor_waiter_t  **prev;
    sys_thread_t       *waiting_thread;
};

struct monitor_wait_queue {
    monitor_waiter_t   *head;		/* linked list of waiting threads */
    short		count;		/* number of waiters on the list */
};

#define ANY_WAITING(mwq) ((mwq).count > 0)
#define INIT_MONITOR_WAIT_QUEUE(mwq) { (mwq).head = NULL; (mwq).count = 0; }

/* The system-level monitor data structure */
struct sys_mon {
    mutex_t     	mutex;          /* The monitor's mutex */
    condvar_t		cv_monitor;	/* Notify those doing monitorWait on
					   the monitor */
    /*
     * Threads waiting on either of the above condvars put themselves
     * on one of these lists.
     */
    monitor_wait_queue_t mwait_queue;	/* Head of MonitorWaitQ */

    /* Thread currently executing in this monitor */
    sys_thread_t 	*monitor_owner;
    long        	entry_count;    /* Recursion depth */
    int                 contention_count;
};

void initializeContentionCountMutex();

typedef enum {
	ASYNC_REGISTER,
	ASYNC_UNREGISTER
} async_action_t;

/*
 * Macros
 */

#define SYS_MID_NULL ((sys_mon_t *) 0)

typedef enum {
	SYS_ASYNC_MON_ALARM = 1,
 	SYS_ASYNC_MON_IO,
 	SYS_ASYNC_MON_EVENT,
	SYS_ASYNC_MON_CHILD,
 	SYS_ASYNC_MON_MAX
} async_mon_key_t;

#define SYS_ASYNC_MON_INPUT SYS_ASYNC_MON_IO
#define SYS_ASYNC_MON_OUTPUT SYS_ASYNC_MON_IO

sys_mon_t *asyncMon(async_mon_key_t);

#endif /* !_JAVASOFT_SOLARIS_MONITOR_MD_H_ */

