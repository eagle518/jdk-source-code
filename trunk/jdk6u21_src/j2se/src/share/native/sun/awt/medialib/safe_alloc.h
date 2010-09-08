/*
 * @(#)safe_alloc.h	1.3 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef __SAFE_ALLOC_H__
#define __SAFE_ALLOC_H__

#include "j2d_md.h"

/*
 * Macroses defined below are wrappers for alloc functions
 * that perform buffer size calculation with integer overflow
 * check.
 */
#define SAFE_TO_ALLOC_2(c, sz)                                             \
    (((c) > 0) && ((sz) > 0) &&                                            \
     ((0xffffffffu / ((juint)(c))) > ((juint)(sz))))

#define SAFE_TO_ALLOC_3(w, h, sz)                                          \
    (((w) > 0) && ((h) > 0) && ((sz) > 0) &&                               \
     (((0xffffffffu / ((juint)(w))) / ((juint)(h))) > ((juint)(sz))))


#endif // __SAFE_ALLOC_H__
