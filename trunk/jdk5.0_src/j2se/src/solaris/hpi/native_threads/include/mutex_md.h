/*
 * @(#)mutex_md.h	1.17 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Interface to mutex HPI implementation for Solaris
 */

#ifndef _JAVASOFT_MUTEX_MD_H_
#define _JAVASOFT_MUTEX_MD_H_

#include "porting.h"

/*
 * Generally, we would typedef mutex_t to be whatever the system
 * supplies.  But Solaris gives us mutex_t directly.
 */

#ifdef USE_PTHREADS
#define mutexInit(m) pthread_mutex_init(m, 0)
#else
#define mutexInit(m) mutex_init(m, USYNC_THREAD, 0)
#endif
#define mutexDestroy(m) mutex_destroy(m)
#define mutexLock(m) mutex_lock(m)
#define mutexUnlock(m) mutex_unlock(m)
bool_t mutexLocked(mutex_t *);

#endif /* !_JAVASOFT_MUTEX_MD_H_ */
