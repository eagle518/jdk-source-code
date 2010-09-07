/*
 * @(#)Index12Gray.c	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <string.h>

#include "AnyShort.h"
#include "Index12Gray.h"
#include "AlphaMacros.h"

#include "IntArgb.h"
#include "IntRgb.h"
#include "ThreeByteBgr.h"
#include "ByteGray.h"
#include "ByteIndexed.h"
#include "Index8Gray.h"

/*
 * This file declares, registers, and defines the various graphics
 * primitive loops to manipulate surfaces of type "Index12Gray".
 *
 * See also LoopMacros.h
 */

RegisterFunc RegisterIndex12Gray;

DECLARE_CONVERT_BLIT(Index12Gray, IntArgb);
DECLARE_CONVERT_BLIT(IntArgb, Index12Gray);
DECLARE_CONVERT_BLIT(ThreeByteBgr, Index12Gray);
DECLARE_CONVERT_BLIT(ByteGray, Index12Gray);
DECLARE_CONVERT_BLIT(Index8Gray, Index12Gray);
DECLARE_CONVERT_BLIT(ByteIndexed, Index12Gray);
DECLARE_CONVERT_BLIT(Index12Gray, Index12Gray);

DECLARE_SCALE_BLIT(Index12Gray, Index12Gray);
DECLARE_SCALE_BLIT(Index12Gray, IntArgb);
DECLARE_SCALE_BLIT(IntArgb, Index12Gray);
DECLARE_SCALE_BLIT(ThreeByteBgr, Index12Gray);
DECLARE_SCALE_BLIT(UshortGray, Index12Gray);
DECLARE_SCALE_BLIT(ByteIndexed, Index12Gray);
DECLARE_SCALE_BLIT(ByteGray, Index12Gray);
DECLARE_SCALE_BLIT(Index8Gray, Index12Gray);

DECLARE_XPAR_CONVERT_BLIT(ByteIndexedBm, Index12Gray);
DECLARE_XPAR_BLITBG(ByteIndexedBm, Index12Gray);

DECLARE_XOR_BLIT(IntArgb, Index12Gray);
DECLARE_ALPHA_MASKFILL(Index12Gray);
DECLARE_ALPHA_MASKBLIT(IntArgb, Index12Gray);
DECLARE_ALPHA_MASKBLIT(IntRgb, Index12Gray);
DECLARE_SRCOVER_MASKFILL(Index12Gray);
DECLARE_SRCOVER_MASKBLIT(IntArgb, Index12Gray);
DECLARE_SOLID_DRAWGLYPHLISTAA(Index12Gray); 

NativePrimitive Index12GrayPrimitives[] = {
    REGISTER_CONVERT_BLIT(IntArgb, Index12Gray),
    REGISTER_CONVERT_BLIT_EQUIV(IntRgb, Index12Gray,
				NAME_CONVERT_BLIT(IntArgb, Index12Gray)),
    REGISTER_CONVERT_BLIT(ThreeByteBgr, Index12Gray),
    REGISTER_CONVERT_BLIT(ByteGray, Index12Gray),
    REGISTER_CONVERT_BLIT(Index8Gray, Index12Gray),
    REGISTER_CONVERT_BLIT_FLAGS(Index12Gray, Index12Gray, 
				SD_LOCK_LUT,
				SD_LOCK_LUT | SD_LOCK_INVGRAY),
    REGISTER_CONVERT_BLIT(ByteIndexed, Index12Gray),

    REGISTER_SCALE_BLIT(Index12Gray, IntArgb),
    REGISTER_SCALE_BLIT(IntArgb, Index12Gray),
    REGISTER_SCALE_BLIT_EQUIV(IntRgb, Index12Gray,
			      NAME_SCALE_BLIT(IntArgb, Index12Gray)),
    REGISTER_SCALE_BLIT(ThreeByteBgr, Index12Gray),
    REGISTER_SCALE_BLIT(UshortGray, Index12Gray),
    REGISTER_SCALE_BLIT(ByteIndexed, Index12Gray),
    REGISTER_SCALE_BLIT(ByteGray, Index12Gray),
    REGISTER_SCALE_BLIT(Index8Gray, Index12Gray),
    REGISTER_SCALE_BLIT_FLAGS(Index12Gray, Index12Gray, 0, 
			      SD_LOCK_LUT | SD_LOCK_INVGRAY),

    REGISTER_XPAR_CONVERT_BLIT(ByteIndexedBm, Index12Gray),
    REGISTER_XPAR_BLITBG(ByteIndexedBm, Index12Gray),

    REGISTER_XOR_BLIT(IntArgb, Index12Gray),
    REGISTER_ALPHA_MASKFILL(Index12Gray),
    REGISTER_ALPHA_MASKBLIT(IntArgb, Index12Gray),
    REGISTER_ALPHA_MASKBLIT(IntRgb, Index12Gray),
    REGISTER_SRCOVER_MASKFILL(Index12Gray),
    REGISTER_SRCOVER_MASKBLIT(IntArgb, Index12Gray),
    REGISTER_SOLID_DRAWGLYPHLISTAA(Index12Gray),
};

extern jboolean checkSameLut(jint *SrcReadLut, jint *DstReadLut,
			     SurfaceDataRasInfo *pSrcInfo,
			     SurfaceDataRasInfo *pDstInfo);

jboolean RegisterIndex12Gray(JNIEnv *env)
{
    return RegisterPrimitives(env, Index12GrayPrimitives,
			      ArraySize(Index12GrayPrimitives));
}

jint PixelForIndex12Gray(SurfaceDataRasInfo *pRasInfo, jint rgb)
{
    jint r, g, b, gray;
    ExtractIntDcmComponentsX123(rgb, r, g, b);
    gray = ComposeByteGrayFrom3ByteRgb(r, g, b);
    return pRasInfo->invGrayTable[gray];
}

DEFINE_CONVERT_BLIT(IntArgb, Index12Gray, 3ByteRgb)

DEFINE_CONVERT_BLIT(ThreeByteBgr, Index12Gray, 3ByteRgb)

DEFINE_CONVERT_BLIT(ByteGray, Index12Gray, 1ByteGray)

DEFINE_CONVERT_BLIT(Index8Gray, Index12Gray, 1ByteGray)

DEFINE_CONVERT_BLIT(ByteIndexed, Index12Gray, 3ByteRgb)

void NAME_CONVERT_BLIT(Index12Gray, Index12Gray)
    (void *srcBase, void *dstBase,
     juint width, juint height,
     SurfaceDataRasInfo *pSrcInfo,
     SurfaceDataRasInfo *pDstInfo,
     NativePrimitive *pPrim,
     CompositeInfo *pCompInfo)
{
    DeclareIndex12GrayLoadVars(SrcRead)
    DeclareIndex12GrayLoadVars(DstRead)
    jint srcScan = pSrcInfo->scanStride;
    jint dstScan = pDstInfo->scanStride;

    InitIndex12GrayLoadVars(SrcRead, pSrcInfo);
    InitIndex12GrayLoadVars(DstRead, pDstInfo);

    if (checkSameLut(SrcReadLut, DstReadLut, pSrcInfo, pDstInfo)) {
	do {
	    memcpy(dstBase, srcBase, width);
	    srcBase = PtrAddBytes(srcBase, srcScan);
	    dstBase = PtrAddBytes(dstBase, dstScan);
	} while (--height > 0);
    } else {
	DeclareIndex12GrayStoreVars(DstWrite);
	InitIndex12GrayStoreVarsY(DstWrite, pDstInfo);

	BlitLoopWidthHeight(Index12Gray, pSrc, srcBase, pSrcInfo,
			    Index12Gray, pDst, dstBase, pDstInfo, DstWrite,
			    width, height,
			    ConvertVia1ByteGray
                                (pSrc, Index12Gray, SrcRead,
				 pDst, Index12Gray, DstWrite, 0, 0));
    }
}

void NAME_SCALE_BLIT(Index12Gray, Index12Gray)
    (void *srcBase, void *dstBase,
     juint width, juint height,
     jint sxloc, jint syloc,
     jint sxinc, jint syinc, jint shift,
     SurfaceDataRasInfo *pSrcInfo,
     SurfaceDataRasInfo *pDstInfo,
     NativePrimitive *pPrim,
     CompositeInfo *pCompInfo)
{
    DeclareIndex8GrayLoadVars(SrcRead)
    DeclareIndex8GrayLoadVars(DstRead)
    jint srcScan = pSrcInfo->scanStride;
    jint dstScan = pDstInfo->scanStride;
    DeclareIndex8GrayStoreVars(DstWrite)

    InitIndex8GrayLoadVars(SrcRead, pSrcInfo);
    InitIndex8GrayLoadVars(DstRead, pDstInfo);

    if (checkSameLut(SrcReadLut, DstReadLut, pSrcInfo, pDstInfo)) {
	BlitLoopScaleWidthHeight(Index8Gray, pSrc, srcBase, pSrcInfo,
				 Index8Gray, pDst, dstBase, pDstInfo, DstWrite,
				 x, width, height,
				 sxloc, syloc, sxinc, syinc, shift,
				 pDst[0] = pSrc[x]);
    } else {
	DeclareIndex8GrayStoreVars(DstWrite);
	InitIndex8GrayStoreVarsY(DstWrite, pDstInfo);
	BlitLoopScaleWidthHeight(Index8Gray, pSrc, srcBase, pSrcInfo,
				 Index8Gray, pDst, dstBase, pDstInfo, DstWrite,
				 x, width, height,
				 sxloc, syloc, sxinc, syinc, shift,
				 ConvertVia1ByteGray(pSrc, Index8Gray, SrcRead,
						     pDst, Index8Gray, DstWrite,
						     x, 0));
    }
}

DEFINE_SCALE_BLIT(Index12Gray, IntArgb, 1IntArgb)

DEFINE_SCALE_BLIT(IntArgb, Index12Gray, 3ByteRgb)

DEFINE_SCALE_BLIT(ThreeByteBgr, Index12Gray, 3ByteRgb)

DEFINE_SCALE_BLIT(UshortGray, Index12Gray, 1ByteGray)

DEFINE_SCALE_BLIT_LUT8(ByteIndexed, Index12Gray, PreProcessLut)

DEFINE_SCALE_BLIT(ByteGray, Index12Gray, 1ByteGray)

DEFINE_SCALE_BLIT_LUT8(Index8Gray, Index12Gray, PreProcessLut)

DEFINE_XPAR_CONVERT_BLIT_LUT8(ByteIndexedBm, Index12Gray, PreProcessLut)

DEFINE_XPAR_BLITBG_LUT8(ByteIndexedBm, Index12Gray, PreProcessLut)

DEFINE_XOR_BLIT(IntArgb, Index12Gray, AnyShort)

DEFINE_ALPHA_MASKFILL(Index12Gray, 1ByteGray)

DEFINE_ALPHA_MASKBLIT(IntArgb, Index12Gray, 1ByteGray)

DEFINE_ALPHA_MASKBLIT(IntRgb, Index12Gray, 1ByteGray)

DEFINE_SRCOVER_MASKFILL(Index12Gray, 1ByteGray)

DEFINE_SRCOVER_MASKBLIT(IntArgb, Index12Gray, 1ByteGray)

DEFINE_SOLID_DRAWGLYPHLISTAA(Index12Gray, 1ByteGray)
