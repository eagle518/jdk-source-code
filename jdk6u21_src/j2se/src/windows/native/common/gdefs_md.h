/*
 * @(#)gdefs_md.h	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
