/*
 * @(#)splashscreen_config.h	1.3 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* platform-dependent definitions */

#ifndef SPLASHSCREEN_CONFIG_H
#define SPLASHSCREEN_CONFIG_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <memory.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>

typedef DWORD rgbquad_t;
typedef WORD word_t;
typedef BYTE byte_t;
typedef RECT RECT_T;

#define RECT_EQ_X(r1, r2)   ((r1).left==(r2).left && (r1).right==(r2).right)
#define RECT_SET(r,xx,yy,ww,hh) (r).left=(xx);(r).top=(yy); \
                                    (r).right=(xx)+(ww); (r).bottom=(yy)+(hh);
#define RECT_INC_HEIGHT(r) (r).bottom++;

#define INLINE __inline

#define SPLASHEXPORT __declspec(dllexport)


#endif
