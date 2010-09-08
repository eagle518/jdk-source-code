/*
 * @(#)splashscreen_config.h	1.4 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* platform-dependent definitions */

#ifndef SPLASHSCREEN_CONFIG_H
#define SPLASHSCREEN_CONFIG_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/shape.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <pthread.h>
#include <signal.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef uint32_t rgbquad_t;
typedef uint16_t word_t;
typedef uint8_t byte_t;
typedef XRectangle RECT_T;

#define RECT_EQ_X(r1,r2)        ((r1).x==(r2).x && (r1).width==(r2).width)
#define RECT_SET(r,xx,yy,ww,hh) (r).x=(xx), (r).y=(yy); (r).width=(ww); \
                                      (r).height=(hh);
#define RECT_INC_HEIGHT(r)      (r).height++;

#define SPLASHCTL_QUIT          'Q'
#define SPLASHCTL_UPDATE        'U'
#define SPLASHCTL_RECONFIGURE   'R'

#define INLINE static

#define SPLASHEXPORT

#endif
