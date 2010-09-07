/*
 * @(#)gdefs_md.h	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Win32 dependent type definitions
 */

#include <stddef.h>
#ifndef _WIN64
typedef int intptr_t;
typedef unsigned int uintptr_t;
typedef unsigned long DWORD_PTR, *PDWORD_PTR;
#endif
