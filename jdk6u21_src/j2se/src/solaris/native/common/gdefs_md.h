/*
 * @(#)gdefs_md.h	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Solaris dependent type definitions  includes intptr_t, etc 
 */


#include <sys/types.h>
/*
 * Linux version of <sys/types.h> does not define intptr_t
 */
#ifdef __linux__
#include <stdint.h>
#include <malloc.h>
#endif /* __linux__ */

