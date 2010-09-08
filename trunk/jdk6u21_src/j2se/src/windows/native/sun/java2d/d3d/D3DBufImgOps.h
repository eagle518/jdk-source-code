/*
 * @(#)D3DBufImgOps.h	1.2 10/03/23
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef D3DBufImgOps_h_Included
#define D3DBufImgOps_h_Included

#include "D3DContext.h"

/**************************** ConvolveOp support ****************************/

/**
 * Flags that can be bitwise-or'ed together to control how the shader
 * source code is generated.
 */
#define CONVOLVE_EDGE_ZERO_FILL (1 << 0)
#define CONVOLVE_5X5            (1 << 1)
#define MAX_CONVOLVE            (1 << 2)

HRESULT D3DBufImgOps_EnableConvolveOp(D3DContext *oglc, jlong pSrcOps,
                                      jboolean edgeZeroFill,
                                      jint kernelWidth, jint KernelHeight,
                                      unsigned char *kernelVals);
HRESULT D3DBufImgOps_DisableConvolveOp(D3DContext *oglc);

/**************************** RescaleOp support *****************************/

/**
 * Flags that can be bitwise-or'ed together to control how the shader
 * source code is generated.
 */
#define RESCALE_NON_PREMULT (1 << 0)
#define MAX_RESCALE         (1 << 1)

HRESULT D3DBufImgOps_EnableRescaleOp(D3DContext *oglc,
                                     jboolean nonPremult,
                                     unsigned char *scaleFactors,
                                     unsigned char *offsets);
HRESULT D3DBufImgOps_DisableRescaleOp(D3DContext *oglc);

/**************************** LookupOp support ******************************/

/**
 * Flags that can be bitwise-or'ed together to control how the shader
 * source code is generated.
 */
#define LOOKUP_USE_SRC_ALPHA (1 << 0)
#define LOOKUP_NON_PREMULT   (1 << 1)
#define MAX_LOOKUP           (1 << 2)

HRESULT D3DBufImgOps_EnableLookupOp(D3DContext *oglc,
                                    jboolean nonPremult, jboolean shortData,
                                    jint numBands, jint bandLength, jint offset,
                                    void *tableValues);
HRESULT D3DBufImgOps_DisableLookupOp(D3DContext *oglc);

#endif /* D3DBufImgOps_h_Included */
