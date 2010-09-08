/*
 * @(#)BufImgSurfaceData.h	1.8 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "SurfaceData.h"
#include "colordata.h"


typedef struct _BufImgSDOps {
    SurfaceDataOps	sdOps;
    jobject		array;
    jint		offset;
    jint                bitoffset;
    jint		pixStr;
    jint		scanStr;
    jobject		icm;
    jobject		lutarray;
    jint		lutsize;
    SurfaceDataBounds	rasbounds;
} BufImgSDOps;

typedef struct _BufImgRIPrivate {
    jint		lockFlags;
    void		*base;
    void		*lutbase;
    ColorData		*cData;
} BufImgRIPrivate;
