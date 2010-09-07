/*
 * @(#)BufImgSurfaceData.h	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "SurfaceData.h"
#include "colordata.h"


typedef struct _BufImgSDOps {
    SurfaceDataOps	sdOps;
    jobject		array;
    jint		offset;
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
