/*
 * @(#)vm_calls.h	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef _JAVASOFT_VM_CALLS_H_
#define _JAVASOFT_VM_CALLS_H_

/* This file defines the function table and macros exported from the VM
 * for the implementation of HPI.
 */

extern vm_calls_t *vm_calls;

#define VM_CALLS_READY() vm_calls
#define VM_CALL(f) (vm_calls->f)

#undef sysAssert

#ifdef DEBUG
#define sysAssert(expression) {		\
    if (!(expression)) {		\
	vm_calls->panic \
            ("\"%s\", line %d: assertion failure\n", __FILE__, __LINE__); \
    }					\
}
#else
#define sysAssert(expression) ((void) 0)
#endif

#ifdef LOGGING

#define Log(level, message) {						\
    if (vm_calls && level <= logging_level)			\
	vm_calls->jio_fprintf(stderr, message);				\
}

#define Log1(level, message, x1) {					\
    if (vm_calls && level <= logging_level)			\
	vm_calls->jio_fprintf(stderr, message, (x1));			\
}

#define Log2(level, message, x1, x2) {					\
    if (vm_calls && level <= logging_level)			\
	vm_calls->jio_fprintf(stderr, message, (x1), (x2));		\
}

#define Log3(level, message, x1, x2, x3) {				\
    if (vm_calls && level <= logging_level)			\
	vm_calls->jio_fprintf(stderr, message, (x1), (x2), (x3));	\
}

#define Log4(level, message, x1, x2, x3, x4) {				\
    if (vm_calls && level <= logging_level)			\
	vm_calls->jio_fprintf(stderr, message, (x1), (x2), (x3), (x4)); \
}

#else

#define Log(level, message)			((void) 0)
#define Log1(level, message, x1)		((void) 0)
#define Log2(level, message, x1, x2)		((void) 0)
#define Log3(level, message, x1, x2, x3)	((void) 0)
#define Log4(level, message, x1, x2, x3, x4)	((void) 0)

#endif /* LOGGING */

#endif /* !_JAVASOFT_VM_CALLS_H_ */
