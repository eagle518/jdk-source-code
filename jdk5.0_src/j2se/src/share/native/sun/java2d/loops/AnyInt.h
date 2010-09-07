/*
 * @(#)AnyInt.h	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "GraphicsPrimitiveMgr.h"
#include "LoopMacros.h"

/*
 * This file contains macro and type definitions used by the macros in
 * LoopMacros.h to manipulate a surface of type "AnyInt".
 */

typedef jint	AnyIntDataType;

#define AnyIntPixelStride	4

#define DeclareAnyIntLoadVars(PREFIX)
#define DeclareAnyIntStoreVars(PREFIX)
#define InitAnyIntLoadVars(PREFIX, pRasInfo)
#define InitAnyIntStoreVarsY(PREFIX, pRasInfo)
#define InitAnyIntStoreVarsX(PREFIX, pRasInfo)
#define NextAnyIntStoreVarsX(PREFIX)
#define NextAnyIntStoreVarsY(PREFIX)

#define DeclareAnyIntPixelData(PREFIX)

#define ExtractAnyIntPixelData(PIXEL, PREFIX)

#define StoreAnyIntPixelData(pPix, x, pixel, PREFIX) \
    (pPix)[x] = (pixel)

#define CopyAnyIntPixelData(pSrc, sx, pDst, dx) \
    (pDst)[dx] = (pSrc)[sx]

#define XorCopyAnyIntPixelData(pSrc, pDst, x, xorpixel, XORPREFIX) \
    (pDst)[x] ^= (pSrc)[x] ^ (xorpixel)

#define XorAnyIntPixelData(srcpixel, SRCPREFIX, pDst, x, \
                           xorpixel, XORPREFIX, mask, MASKPREFIX) \
    (pDst)[x] ^= (((srcpixel) ^ (xorpixel)) & ~(mask))

DECLARE_ISOCOPY_BLIT(AnyInt);
DECLARE_ISOSCALE_BLIT(AnyInt);
DECLARE_ISOXOR_BLIT(AnyInt);
DECLARE_CONVERT_BLIT(ByteIndexed, IntArgb);
DECLARE_SCALE_BLIT(ByteIndexed, IntArgb);
DECLARE_XPAR_CONVERT_BLIT(ByteIndexedBm, IntArgb);
DECLARE_XPAR_SCALE_BLIT(ByteIndexedBm, IntArgb);
DECLARE_XPAR_BLITBG(ByteIndexedBm, IntArgb);

#define REGISTER_ANYINT_ISOCOPY_BLIT(INTTYPE) \
    REGISTER_ISOCOPY_BLIT(INTTYPE, AnyInt)

#define REGISTER_ANYINT_ISOSCALE_BLIT(INTTYPE) \
    REGISTER_ISOSCALE_BLIT(INTTYPE, AnyInt)

#define REGISTER_ANYINT_ISOXOR_BLIT(INTTYPE) \
    REGISTER_ISOXOR_BLIT(INTTYPE, AnyInt)
