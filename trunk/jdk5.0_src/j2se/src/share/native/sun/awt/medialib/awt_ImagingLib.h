/*
 * @(#)awt_ImagingLib.h	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef _AWT_IMAGINGLIB_H_
#define _AWT_IMAGINGLIB_H_

#include "mlib_image_proto.h"
#include "mlib_image_get.h"

/* Struct that holds the mlib function ptrs and names */
typedef struct {
    mlib_status (*fptr)();
    char *fname;
} mlibFnS_t;

typedef mlib_image *(*MlibCreateFP_t)(mlib_type, mlib_s32, mlib_s32,
                                       mlib_s32);
typedef mlib_image *(*MlibCreateStructFP_t)(mlib_type, mlib_s32, mlib_s32,
                                             mlib_s32, mlib_s32, void *);

typedef struct {
    MlibCreateFP_t createFP;
    MlibCreateStructFP_t createStructFP;
} mlibSysFnS_t;
    
#endif /* _AWT_IMAGINGLIB_H */
