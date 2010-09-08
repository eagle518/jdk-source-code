/*
 * @(#)j2d_md.h	1.10 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef j2d_md_h_Included
#define j2d_md_h_Included
#include <sys/types.h>

/*
 * Linux version of <sys/types.h> does not define intptr_t
 */
#ifdef __linux__
#include <stdint.h>
#endif /* __linux__ */

typedef unsigned char	jubyte;
typedef unsigned short	jushort;
typedef unsigned int	juint;

#endif /* j2d_md_h_Included */


