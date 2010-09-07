/*
 * @(#)AnyShort.h	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "GraphicsPrimitiveMgr.h"
#include "LoopMacros.h"

/*
 * This file contains macro and type definitions used by the macros in
 * LoopMacros.h to manipulate a surface of type "AnyShort".
 */

typedef jshort	AnyShortDataType;

#define AnyShortPixelStride	2

#define DeclareAnyShortLoadVars(PREFIX)
#define DeclareAnyShortStoreVars(PREFIX)
#define InitAnyShortLoadVars(PREFIX, pRasInfo)
#define InitAnyShortStoreVarsY(PREFIX, pRasInfo)
#define InitAnyShortStoreVarsX(PREFIX, pRasInfo)
#define NextAnyShortStoreVarsX(PREFIX)
#define NextAnyShortStoreVarsY(PREFIX)

#define DeclareAnyShortPixelData(PREFIX)

#define ExtractAnyShortPixelData(PIXEL, PREFIX)

#define StoreAnyShortPixelData(pPix, x, pixel, PREFIX) \
    (pPix)[x] = (jshort) (pixel)

#define CopyAnyShortPixelData(pSrc, sx, pDst, dx) \
    (pDst)[dx] = (pSrc)[sx]

#define XorCopyAnyShortPixelData(pSrc, pDst, x, xorpixel, XORPREFIX) \
    (pDst)[x] ^= (pSrc)[x] ^ (xorpixel)

#define XorAnyShortPixelData(srcpixel, SRCPREFIX, pDst, x, \
                             xorpixel, XORPREFIX, mask, MASKPREFIX) \
    (pDst)[x] ^= (((srcpixel) ^ (xorpixel)) & ~(mask))

DECLARE_ISOCOPY_BLIT(AnyShort);
DECLARE_ISOSCALE_BLIT(AnyShort);
DECLARE_ISOXOR_BLIT(AnyShort);

#define REGISTER_ANYSHORT_ISOCOPY_BLIT(SHORTTYPE) \
    REGISTER_ISOCOPY_BLIT(SHORTTYPE, AnyShort)

#define REGISTER_ANYSHORT_ISOSCALE_BLIT(SHORTTYPE) \
    REGISTER_ISOSCALE_BLIT(SHORTTYPE, AnyShort)

#define REGISTER_ANYSHORT_ISOXOR_BLIT(SHORTTYPE) \
    REGISTER_ISOXOR_BLIT(SHORTTYPE, AnyShort)
