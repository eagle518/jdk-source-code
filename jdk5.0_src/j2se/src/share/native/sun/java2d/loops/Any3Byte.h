/*
 * @(#)Any3Byte.h	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "GraphicsPrimitiveMgr.h"
#include "LoopMacros.h"

/*
 * This file contains macro and type definitions used by the macros in
 * LoopMacros.h to manipulate a surface of type "Any3Byte".
 */

typedef jubyte	Any3ByteDataType;

#define Any3BytePixelStride	3

#define DeclareAny3ByteLoadVars(PREFIX)
#define DeclareAny3ByteStoreVars(PREFIX)
#define InitAny3ByteLoadVars(PREFIX, pRasInfo)
#define InitAny3ByteStoreVarsY(PREFIX, pRasInfo)
#define InitAny3ByteStoreVarsX(PREFIX, pRasInfo)
#define NextAny3ByteStoreVarsX(PREFIX)
#define NextAny3ByteStoreVarsY(PREFIX)

#define DeclareAny3BytePixelData(PREFIX) \
    jubyte PREFIX ## 0, PREFIX ## 1, PREFIX ## 2;

#define ExtractAny3BytePixelData(PIXEL, PREFIX) \
    do { \
	PREFIX ## 0 = (jubyte) (PIXEL); \
	PREFIX ## 1 = (jubyte) (PIXEL >> 8); \
	PREFIX ## 2 = (jubyte) (PIXEL >> 16); \
    } while (0)

#define StoreAny3BytePixelData(pPix, x, pixel, PREFIX) \
    do { \
	(pPix)[3*x+0] = PREFIX ## 0; \
	(pPix)[3*x+1] = PREFIX ## 1; \
	(pPix)[3*x+2] = PREFIX ## 2; \
    } while (0)

#define CopyAny3BytePixelData(pSrc, sx, pDst, dx) \
    do { \
	(pDst)[3*dx+0] = (pSrc)[3*sx+0]; \
	(pDst)[3*dx+1] = (pSrc)[3*sx+1]; \
	(pDst)[3*dx+2] = (pSrc)[3*sx+2]; \
    } while (0)

#define XorCopyAny3BytePixelData(pSrc, pDst, x, xorpixel, XORPREFIX) \
    do { \
	(pDst)[3*x+0] ^= (pSrc)[3*x+0] ^ XORPREFIX ## 0; \
	(pDst)[3*x+1] ^= (pSrc)[3*x+1] ^ XORPREFIX ## 1; \
	(pDst)[3*x+2] ^= (pSrc)[3*x+2] ^ XORPREFIX ## 2; \
    } while (0)

#define XorAny3BytePixelData(srcpixel, SRCPREFIX, pDst, x, \
                             xorpixel, XORPREFIX, mask, MASKPREFIX) \
    do { \
	(pDst)[3*x+0] ^= ((SRCPREFIX ## 0 ^ XORPREFIX ## 0) & \
                          ~MASKPREFIX ## 0); \
	(pDst)[3*x+1] ^= ((SRCPREFIX ## 1 ^ XORPREFIX ## 1) & \
                          ~MASKPREFIX ## 1); \
	(pDst)[3*x+2] ^= ((SRCPREFIX ## 2 ^ XORPREFIX ## 2) & \
                          ~MASKPREFIX ## 2); \
    } while (0)

DECLARE_ISOCOPY_BLIT(Any3Byte);
DECLARE_ISOSCALE_BLIT(Any3Byte);
DECLARE_ISOXOR_BLIT(Any3Byte);

#define REGISTER_ANY3BYTE_ISOCOPY_BLIT(THREEBYTETYPE) \
    REGISTER_ISOCOPY_BLIT(THREEBYTETYPE, Any3Byte)

#define REGISTER_ANY3BYTE_ISOSCALE_BLIT(THREEBYTETYPE) \
    REGISTER_ISOSCALE_BLIT(THREEBYTETYPE, Any3Byte)

#define REGISTER_ANY3BYTE_ISOXOR_BLIT(THREEBYTETYPE) \
    REGISTER_ISOXOR_BLIT(THREEBYTETYPE, Any3Byte)
