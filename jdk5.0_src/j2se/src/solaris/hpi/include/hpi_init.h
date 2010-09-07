/*
 * @(#)hpi_init.h	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
