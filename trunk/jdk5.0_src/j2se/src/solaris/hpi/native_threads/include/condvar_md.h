/*
 * @(#)condvar_md.h	1.20 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Interface to condition variable HPI implementation for Solaris
 */

#ifndef _JAVASOFT_CONDVAR_MD_H_
#define _JAVASOFT_CONDVAR_MD_H_

#include "threads_md.h"

typedef	struct condvar {
    cond_t  cond;	    /* Manual-reset event for notifications */
    unsigned int counter;   /* Current number of notifications */
} condvar_t;

int condvarInit(condvar_t *);
int condvarDestroy(condvar_t *);
int condvarWait(condvar_t *, mutex_t *, thread_state_t wtype);
int condvarTimedWait(condvar_t *, mutex_t *, jlong millis, thread_state_t wtype);
int condvarSignal(condvar_t *);
int condvarBroadcast(condvar_t *);

#endif /* !_JAVASOFT_CONDVAR_MD_H_ */
