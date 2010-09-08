/*
 * @(#)OGLBufImgOps.h	1.2 10/03/23
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef OGLBufImgOps_h_Included
#define OGLBufImgOps_h_Included

#include "OGLContext.h"

void OGLBufImgOps_EnableConvolveOp(OGLContext *oglc, jlong pSrcOps,
                                   jboolean edgeZeroFill,
                                   jint kernelWidth, jint KernelHeight,
                                   unsigned char *kernelVals);
void OGLBufImgOps_DisableConvolveOp(OGLContext *oglc);
void OGLBufImgOps_EnableRescaleOp(OGLContext *oglc, jlong pSrcOps,
                                  jboolean nonPremult,
                                  unsigned char *scaleFactors,
                                  unsigned char *offsets);
void OGLBufImgOps_DisableRescaleOp(OGLContext *oglc);
void OGLBufImgOps_EnableLookupOp(OGLContext *oglc, jlong pSrcOps,
                                 jboolean nonPremult, jboolean shortData,
                                 jint numBands, jint bandLength, jint offset,
                                 void *tableValues);
void OGLBufImgOps_DisableLookupOp(OGLContext *oglc);

#endif /* OGLBufImgOps_h_Included */
