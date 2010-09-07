/*
 * @(#)AnyByte.h	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "GraphicsPrimitiveMgr.h"
#include "LoopMacros.h"

/*
 * This file contains macro and type definitions used by the macros in
 * LoopMacros.h to manipulate a surface of type "AnyByte".
 */

typedef jbyte	AnyByteDataType;

#define AnyBytePixelStride	1

#define DeclareAnyByteLoadVars(PREFIX)
#define DeclareAnyByteStoreVars(PREFIX)
#define InitAnyByteLoadVars(PREFIX, pRasInfo)
#define InitAnyByteStoreVarsY(PREFIX, pRasInfo)
#define InitAnyByteStoreVarsX(PREFIX, pRasInfo)
#define NextAnyByteStoreVarsX(PREFIX)
#define NextAnyByteStoreVarsY(PREFIX)

#define DeclareAnyBytePixelData(PREFIX)

#define ExtractAnyBytePixelData(PIXEL, PREFIX)

#define StoreAnyBytePixelData(pPix, x, pixel, PREFIX) \
    (pPix)[x] = (jbyte) (pixel)

#define CopyAnyBytePixelData(pSrc, sx, pDst, dx) \
    (pDst)[dx] = (pSrc)[sx]

#define XorCopyAnyBytePixelData(pSrc, pDst, x, xorpixel, XORPREFIX) \
    (pDst)[x] ^= (pSrc)[x] ^ (xorpixel)

#define XorAnyBytePixelData(srcpixel, SRCPREFIX, pDst, x, \
                            xorpixel, XORPREFIX, mask, MASKPREFIX) \
    (pDst)[x] ^= (((srcpixel) ^ (xorpixel)) & ~(mask))

DECLARE_ISOCOPY_BLIT(AnyByte);
DECLARE_ISOSCALE_BLIT(AnyByte);
DECLARE_ISOXOR_BLIT(AnyByte);

#define REGISTER_ANYBYTE_ISOCOPY_BLIT(BYTETYPE) \
    REGISTER_ISOCOPY_BLIT(BYTETYPE, AnyByte)

#define REGISTER_ANYBYTE_ISOSCALE_BLIT(BYTETYPE) \
    REGISTER_ISOSCALE_BLIT(BYTETYPE, AnyByte)

#define REGISTER_ANYBYTE_ISOXOR_BLIT(BYTETYPE) \
    REGISTER_ISOXOR_BLIT(BYTETYPE, AnyByte)
