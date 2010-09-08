/*
 * @(#)mutex_md.c	1.27 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Mutex HPI implementation for Solaris
 *
 * Mutexes are used both by the system-independent monitor implementation and
 * to implement critical regions elsewhere within the runtime.
 */

#include <errno.h>

#include "hpi_impl.h"

#include "mutex_md.h"
#include "threads_md.h"

/*
 * Return true of the mutex in question is already locked.  note:
 * this does not tell if the mutex is already locked by *this*
 * thread, only that is is locked by *some* thread.
 */
bool_t
mutexLocked(mutex_t *mutex)
{
    if (mutex_trylock(mutex) == 0) {
	mutex_unlock(mutex);
	return FALSE;
    }
    return TRUE;
}
