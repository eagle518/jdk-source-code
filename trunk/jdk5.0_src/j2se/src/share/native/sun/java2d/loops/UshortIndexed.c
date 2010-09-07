/*
 * @(#)UshortIndexed.c	1.2 04/07/27
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <string.h>

#include "AnyByte.h"
#include "UshortIndexed.h"
#include "AlphaMacros.h"

#include "IntArgb.h"
#include "IntArgbBm.h"
#include "IntRgb.h"
#include "ThreeByteBgr.h"
#include "ByteGray.h"
#include "Index12Gray.h"

/*
 * This file declares, registers, and defines the various graphics
 * primitive loops to manipulate surfaces of type "UshortIndexed".
 *
 * See also LoopMacros.h
 */

RegisterFunc RegisterUshortIndexed;

DECLARE_CONVERT_BLIT(IntArgb, UshortIndexed);
DECLARE_CONVERT_BLIT(ThreeByteBgr, UshortIndexed);
DECLARE_CONVERT_BLIT(ByteGray, UshortIndexed);
DECLARE_CONVERT_BLIT(UshortIndexed, UshortIndexed);
DECLARE_CONVERT_BLIT(Index12Gray, UshortIndexed);
DECLARE_CONVERT_BLIT(UshortIndexed, IntArgb);
DECLARE_SCALE_BLIT(IntArgb, UshortIndexed);
DECLARE_SCALE_BLIT(ThreeByteBgr, UshortIndexed);
DECLARE_SCALE_BLIT(ByteGray, UshortIndexed);
DECLARE_SCALE_BLIT(Index12Gray, UshortIndexed);
DECLARE_SCALE_BLIT(UshortIndexed, UshortIndexed);
DECLARE_SCALE_BLIT(UshortIndexed, IntArgb);
DECLARE_XPAR_CONVERT_BLIT(ByteIndexedBm, UshortIndexed);
DECLARE_XPAR_SCALE_BLIT(ByteIndexedBm, UshortIndexed);
DECLARE_XPAR_SCALE_BLIT(IntArgbBm, UshortIndexed);
DECLARE_XPAR_BLITBG(ByteIndexedBm, UshortIndexed);
DECLARE_XPAR_CONVERT_BLIT(IntArgbBm, UshortIndexed);
DECLARE_XPAR_BLITBG(IntArgbBm, UshortIndexed);

DECLARE_XOR_BLIT(IntArgb, UshortIndexed);
DECLARE_ALPHA_MASKFILL(UshortIndexed);
DECLARE_ALPHA_MASKBLIT(IntArgb, UshortIndexed);
DECLARE_ALPHA_MASKBLIT(IntRgb, UshortIndexed);
DECLARE_SOLID_DRAWGLYPHLISTAA(UshortIndexed); 

NativePrimitive UshortIndexedPrimitives[] = {
    REGISTER_CONVERT_BLIT(IntArgb, UshortIndexed),
    REGISTER_CONVERT_BLIT_EQUIV(IntRgb, UshortIndexed,
				NAME_CONVERT_BLIT(IntArgb, UshortIndexed)),
    REGISTER_CONVERT_BLIT_EQUIV(IntArgbBm, UshortIndexed,
				NAME_CONVERT_BLIT(IntArgb, UshortIndexed)),
    REGISTER_CONVERT_BLIT(ThreeByteBgr, UshortIndexed),
    REGISTER_CONVERT_BLIT(ByteGray, UshortIndexed),
    REGISTER_CONVERT_BLIT(Index12Gray, UshortIndexed),
    REGISTER_CONVERT_BLIT_FLAGS(UshortIndexed, UshortIndexed, 0, SD_LOCK_LUT),
    REGISTER_CONVERT_BLIT(UshortIndexed, IntArgb),
    REGISTER_CONVERT_BLIT_EQUIV(UshortIndexed, IntRgb,
				NAME_CONVERT_BLIT(UshortIndexed, IntArgb)),
    REGISTER_SCALE_BLIT(IntArgb, UshortIndexed),
    REGISTER_SCALE_BLIT_EQUIV(IntRgb, UshortIndexed,
			      NAME_SCALE_BLIT(IntArgb, UshortIndexed)),
    REGISTER_SCALE_BLIT_EQUIV(IntArgbBm, UshortIndexed,
			      NAME_SCALE_BLIT(IntArgb, UshortIndexed)),
    REGISTER_SCALE_BLIT(ThreeByteBgr, UshortIndexed),
    REGISTER_SCALE_BLIT(ByteGray, UshortIndexed),
    REGISTER_SCALE_BLIT(Index12Gray, UshortIndexed),
    REGISTER_SCALE_BLIT_FLAGS(UshortIndexed, UshortIndexed, 0, SD_LOCK_LUT),
    REGISTER_SCALE_BLIT(UshortIndexed, IntArgb),
    REGISTER_SCALE_BLIT_EQUIV(UshortIndexed, IntRgb,
			      NAME_SCALE_BLIT(UshortIndexed, IntArgb)),
    REGISTER_XPAR_CONVERT_BLIT(ByteIndexedBm, UshortIndexed),
    REGISTER_XPAR_SCALE_BLIT(ByteIndexedBm, UshortIndexed),
    REGISTER_XPAR_SCALE_BLIT(IntArgbBm, UshortIndexed),
    REGISTER_XPAR_BLITBG(ByteIndexedBm, UshortIndexed),
    REGISTER_XPAR_CONVERT_BLIT(IntArgbBm, UshortIndexed),
    REGISTER_XPAR_BLITBG(IntArgbBm, UshortIndexed),

    REGISTER_XOR_BLIT(IntArgb, UshortIndexed),
    REGISTER_ALPHA_MASKFILL(UshortIndexed),
    REGISTER_ALPHA_MASKBLIT(IntArgb, UshortIndexed),
    REGISTER_ALPHA_MASKBLIT(IntRgb, UshortIndexed),
    REGISTER_SOLID_DRAWGLYPHLISTAA(UshortIndexed),
};

extern jint PixelForByteIndexed(SurfaceDataRasInfo *pRasInfo, jint rgb);
extern jboolean checkSameLut(jint *SrcReadLut, jint *DstReadLut,
			     SurfaceDataRasInfo *pSrcInfo,
			     SurfaceDataRasInfo *pDstInfo);

jboolean RegisterUshortIndexed(JNIEnv *env)
{
    return RegisterPrimitives(env, UshortIndexedPrimitives,
			      ArraySize(UshortIndexedPrimitives));
}

jint PixelForUshortIndexed(SurfaceDataRasInfo *pRasInfo, jint rgb)
{
    return PixelForByteIndexed(pRasInfo, rgb);
}


DEFINE_CONVERT_BLIT(IntArgb, UshortIndexed, 3ByteRgb)

DEFINE_CONVERT_BLIT(ThreeByteBgr, UshortIndexed, 3ByteRgb)

DEFINE_CONVERT_BLIT(ByteGray, UshortIndexed, 3ByteRgb)

DEFINE_CONVERT_BLIT(Index12Gray, UshortIndexed, 3ByteRgb)

DEFINE_CONVERT_BLIT_LUT(UshortIndexed, IntArgb, ConvertOnTheFly)

DEFINE_SCALE_BLIT_LUT(UshortIndexed, IntArgb, ConvertOnTheFly)

void NAME_CONVERT_BLIT(UshortIndexed, UshortIndexed)
    (void *srcBase, void *dstBase,
     juint width, juint height,
     SurfaceDataRasInfo *pSrcInfo,
     SurfaceDataRasInfo *pDstInfo,
     NativePrimitive *pPrim,
     CompositeInfo *pCompInfo)
{
    DeclareUshortIndexedLoadVars(SrcRead)
    DeclareUshortIndexedLoadVars(DstRead)
    jint srcScan = pSrcInfo->scanStride;
    jint dstScan = pDstInfo->scanStride;
    jint bytesToCopy = width * pDstInfo->pixelStride;

    InitUshortIndexedLoadVars(SrcRead, pSrcInfo);
    InitUshortIndexedLoadVars(DstRead, pDstInfo);

    if (checkSameLut(SrcReadLut, DstReadLut, pSrcInfo, pDstInfo)) {
	do {
	    memcpy(dstBase, srcBase, bytesToCopy);
	    srcBase = PtrAddBytes(srcBase, srcScan);
	    dstBase = PtrAddBytes(dstBase, dstScan);
	} while (--height > 0);
    } else {
	DeclareUshortIndexedStoreVars(DstWrite);

	BlitLoopWidthHeight(UshortIndexed, pSrc, srcBase, pSrcInfo,
			    UshortIndexed, pDst, dstBase, pDstInfo, DstWrite,
			    width, height,
			    ConvertVia3ByteRgb
                                (pSrc, UshortIndexed, SrcRead,
				 pDst, UshortIndexed, DstWrite, 0, 0));
    }
}

DEFINE_SCALE_BLIT(IntArgb, UshortIndexed, 3ByteRgb)

DEFINE_SCALE_BLIT(ThreeByteBgr, UshortIndexed, 3ByteRgb)

DEFINE_SCALE_BLIT(ByteGray, UshortIndexed, 3ByteRgb)

DEFINE_SCALE_BLIT(Index12Gray, UshortIndexed, 3ByteRgb)

void NAME_SCALE_BLIT(UshortIndexed, UshortIndexed)
    (void *srcBase, void *dstBase,
     juint width, juint height,
     jint sxloc, jint syloc,
     jint sxinc, jint syinc, jint shift,
     SurfaceDataRasInfo *pSrcInfo,
     SurfaceDataRasInfo *pDstInfo,
     NativePrimitive *pPrim,
     CompositeInfo *pCompInfo)
{
    DeclareUshortIndexedLoadVars(SrcRead)
    DeclareUshortIndexedLoadVars(DstRead)
    jint srcScan = pSrcInfo->scanStride;
    jint dstScan = pDstInfo->scanStride;
    DeclareUshortIndexedStoreVars(DstWrite)

    InitUshortIndexedLoadVars(SrcRead, pSrcInfo);
    InitUshortIndexedLoadVars(DstRead, pDstInfo);

    if (checkSameLut(SrcReadLut, DstReadLut, pSrcInfo, pDstInfo)) {
	BlitLoopScaleWidthHeight(UshortIndexed, pSrc, srcBase, pSrcInfo,
				 UshortIndexed, pDst, dstBase, pDstInfo, DstWrite,
				 x, width, height,
				 sxloc, syloc, sxinc, syinc, shift,
				 pDst[0] = pSrc[x]);
    } else {
	BlitLoopScaleWidthHeight(UshortIndexed, pSrc, srcBase, pSrcInfo,
				 UshortIndexed, pDst, dstBase, pDstInfo, DstWrite,
				 x, width, height,
				 sxloc, syloc, sxinc, syinc, shift,
				 ConvertVia3ByteRgb(pSrc, UshortIndexed, SrcRead,
						    pDst, UshortIndexed, DstWrite,
						    x, 0));
    }
}

DEFINE_XPAR_CONVERT_BLIT_LUT(ByteIndexedBm, UshortIndexed, ConvertOnTheFly)

DEFINE_XPAR_SCALE_BLIT_LUT(ByteIndexedBm, UshortIndexed, ConvertOnTheFly)

DEFINE_XPAR_SCALE_BLIT(IntArgbBm, UshortIndexed, 1IntRgb)

DEFINE_XPAR_BLITBG_LUT(ByteIndexedBm, UshortIndexed, ConvertOnTheFly)

DEFINE_XPAR_CONVERT_BLIT(IntArgbBm, UshortIndexed, 1IntRgb)

DEFINE_XPAR_BLITBG(IntArgbBm, UshortIndexed, 1IntRgb)

DEFINE_XOR_BLIT(IntArgb, UshortIndexed, AnyByte)

DEFINE_ALPHA_MASKFILL(UshortIndexed, 4ByteArgb)

DEFINE_ALPHA_MASKBLIT(IntArgb, UshortIndexed, 4ByteArgb)

DEFINE_ALPHA_MASKBLIT(IntRgb, UshortIndexed, 4ByteArgb)

DEFINE_SOLID_DRAWGLYPHLISTAA(UshortIndexed, 3ByteRgb)
