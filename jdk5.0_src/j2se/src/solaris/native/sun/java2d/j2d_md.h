/*
 * @(#)j2d_md.h	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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


