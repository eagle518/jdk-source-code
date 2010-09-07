/*
 * @(#)porting.h	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef _JAVASOFT_PORTING_H_
#define _JAVASOFT_PORTING_H_

#ifndef USE_PTHREADS

#include <thread.h>
#include <sys/lwp.h>
#include <synch.h>

#else  /* USE_PTHREADS */

#include <pthread.h>

/* There is a handshake between a newly created thread and its creator
 * at thread startup because the creator thread needs to suspend the
 * new thread.  Currently there are two ways to do this -- with
 * semaphores and with mutexes.  The semaphore based implementation is
 * cleaner and hence is the default.  We wish the mutex based one will
 * go away, but turns out the implementation of semaphores on
 * Linux/ppc etc is flaky, so the mutex based solution lives for now.
 */
#ifndef USE_MUTEX_HANDSHAKE
#include <semaphore.h>
#endif

#undef BOUND_THREADS

#define thread_t		pthread_t

#define mutex_t			pthread_mutex_t
#define mutex_lock		pthread_mutex_lock
#define mutex_trylock		pthread_mutex_trylock
#define mutex_unlock		pthread_mutex_unlock
#define mutex_destroy		pthread_mutex_destroy

#define cond_t			pthread_cond_t
#define cond_destroy		pthread_cond_destroy
#define cond_wait		pthread_cond_wait
#define cond_timedwait		pthread_cond_timedwait
#define cond_signal		pthread_cond_signal
#define cond_broadcast		pthread_cond_broadcast

#define thread_key_t		pthread_key_t
#define thr_setspecific		pthread_setspecific
#define thr_keycreate		pthread_key_create

#define thr_sigsetmask		pthread_sigmask
#define thr_self		pthread_self
#define thr_yield		sched_yield
#define thr_kill		pthread_kill
#define thr_exit		pthread_exit
#ifdef __linux__
void intrHandler(void*);
#endif
#endif /* USE_PTHREADS  */

#endif /* !_JAVASOFT_PORTING_H_ */
