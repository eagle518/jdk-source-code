/*
 * @(#)mutex_md.c	1.25 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
