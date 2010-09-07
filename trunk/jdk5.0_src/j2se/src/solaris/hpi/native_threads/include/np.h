/*
 * @(#)np.h	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *  Non-posix parts of the threads HPI.
 */

#ifndef _JAVASOFT_NP_H_
#define _JAVASOFT_NP_H_

extern int	np_suspend(sys_thread_t *tid);
extern int	np_continue(sys_thread_t *tid);

extern int	np_single(void);
extern void	np_multi(void);
extern int	np_stackinfo(void **addr, long *size);
extern int	np_initialize(void);
extern void	np_initialize_thread(sys_thread_t *tid);
#ifdef __linux__
extern int      np_initial_suspend(sys_thread_t *tid);
extern void   np_free_thread(sys_thread_t *tid);
#endif

extern void	np_profiler_init(sys_thread_t *tid);
extern int	np_profiler_suspend(sys_thread_t *tid);
extern int	np_profiler_continue(sys_thread_t *tid);
extern bool_t	np_profiler_thread_is_running(sys_thread_t *tid);

#endif /* !_JAVASOFT_NP_H_ */
