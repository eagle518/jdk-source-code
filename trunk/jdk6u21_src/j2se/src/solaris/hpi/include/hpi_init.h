/*
 * @(#)hpi_init.h	1.10 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef _JAVASOFT_SOLARIS_HPI_INIT_H_
#define _JAVASOFT_SOLARIS_HPI_INIT_H_

#ifndef NATIVE
extern void InitializeSbrk(void);
extern void InitializeAsyncIO(void);
extern void InitializeHelperThreads(void);
#endif /* NATIVE */

extern void InitializeMem(void);

#endif /* _JAVASOFT_SOLARIS_HPI_INIT_H_ */
