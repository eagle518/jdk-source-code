/*
 * @(#)ByteIndexed.c	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <string.h>

#include "AnyByte.h"
#include "ByteIndexed.h"
#include "AlphaMacros.h"

#include "IntArgb.h"
#include "IntArgbBm.h"
#include "IntRgb.h"
#include "ThreeByteBgr.h"
#include "ByteGray.h"
#include "Index12Gray.h"

/*
 * This file declares, registers, and defines the various graphics
 * primitive loops to manipulate surfaces of type "ByteIndexed".
 *
 * See also LoopMacros.h
 */

RegisterFunc RegisterByteIndexed;

DECLARE_CONVERT_BLIT(IntArgb, ByteIndexed);
DECLARE_CONVERT_BLIT(ThreeByteBgr, ByteIndexed);
DECLARE_CONVERT_BLIT(ByteGray, ByteIndexed);
DECLARE_CONVERT_BLIT(ByteIndexed, ByteIndexed);
DECLARE_CONVERT_BLIT(Index12Gray, ByteIndexed);
DECLARE_SCALE_BLIT(IntArgb, ByteIndexed);
DECLARE_SCALE_BLIT(ThreeByteBgr, ByteIndexed);
DECLARE_SCALE_BLIT(ByteGray, ByteIndexed);
DECLARE_SCALE_BLIT(Index12Gray, ByteIndexed);
DECLARE_SCALE_BLIT(ByteIndexed, ByteIndexed);
DECLARE_XPAR_CONVERT_BLIT(ByteIndexedBm, ByteIndexed);
DECLARE_XPAR_SCALE_BLIT(ByteIndexedBm, ByteIndexed);
DECLARE_XPAR_SCALE_BLIT(IntArgbBm, ByteIndexed);
DECLARE_XPAR_BLITBG(ByteIndexedBm, ByteIndexed);
DECLARE_XPAR_CONVERT_BLIT(IntArgbBm, ByteIndexed);
DECLARE_XPAR_BLITBG(IntArgbBm, ByteIndexed);

DECLARE_XOR_BLIT(IntArgb, ByteIndexed);
DECLARE_ALPHA_MASKFILL(ByteIndexed);
DECLARE_ALPHA_MASKBLIT(IntArgb, ByteIndexed);
DECLARE_ALPHA_MASKBLIT(IntRgb, ByteIndexed);
DECLARE_SOLID_DRAWGLYPHLISTAA(ByteIndexed); 

NativePrimitive ByteIndexedPrimitives[] = {
    REGISTER_CONVERT_BLIT(IntArgb, ByteIndexed),
    REGISTER_CONVERT_BLIT_EQUIV(IntRgb, ByteIndexed,
				NAME_CONVERT_BLIT(IntArgb, ByteIndexed)),
    REGISTER_CONVERT_BLIT_EQUIV(IntArgbBm, ByteIndexed,
				NAME_CONVERT_BLIT(IntArgb, ByteIndexed)),
    REGISTER_CONVERT_BLIT(ThreeByteBgr, ByteIndexed),
    REGISTER_CONVERT_BLIT(ByteGray, ByteIndexed),
    REGISTER_CONVERT_BLIT(Index12Gray, ByteIndexed),
    REGISTER_CONVERT_BLIT_FLAGS(ByteIndexed, ByteIndexed, 0, SD_LOCK_LUT),
    REGISTER_SCALE_BLIT(IntArgb, ByteIndexed),
    REGISTER_SCALE_BLIT_EQUIV(IntRgb, ByteIndexed,
			      NAME_SCALE_BLIT(IntArgb, ByteIndexed)),
    REGISTER_SCALE_BLIT_EQUIV(IntArgbBm, ByteIndexed,
			      NAME_SCALE_BLIT(IntArgb, ByteIndexed)),
    REGISTER_SCALE_BLIT(ThreeByteBgr, ByteIndexed),
    REGISTER_SCALE_BLIT(ByteGray, ByteIndexed),
    REGISTER_SCALE_BLIT(Index12Gray, ByteIndexed),
    REGISTER_SCALE_BLIT_FLAGS(ByteIndexed, ByteIndexed, 0, SD_LOCK_LUT),
    REGISTER_XPAR_CONVERT_BLIT(ByteIndexedBm, ByteIndexed),
    REGISTER_XPAR_SCALE_BLIT(ByteIndexedBm, ByteIndexed),
    REGISTER_XPAR_SCALE_BLIT(IntArgbBm, ByteIndexed),
    REGISTER_XPAR_BLITBG(ByteIndexedBm, ByteIndexed),
    REGISTER_XPAR_CONVERT_BLIT(IntArgbBm, ByteIndexed),
    REGISTER_XPAR_BLITBG(IntArgbBm, ByteIndexed),

    REGISTER_XOR_BLIT(IntArgb, ByteIndexed),
    REGISTER_ALPHA_MASKFILL(ByteIndexed),
    REGISTER_ALPHA_MASKBLIT(IntArgb, ByteIndexed),
    REGISTER_ALPHA_MASKBLIT(IntRgb, ByteIndexed),
    REGISTER_SOLID_DRAWGLYPHLISTAA(ByteIndexed),
};

jboolean RegisterByteIndexed(JNIEnv *env)
{
    return RegisterPrimitives(env, ByteIndexedPrimitives,
			      ArraySize(ByteIndexedPrimitives));
}

jint PixelForByteIndexed(SurfaceDataRasInfo *pRasInfo, jint rgb)
{
    jint r, g, b;
    ExtractIntDcmComponentsX123(rgb, r, g, b);
    return SurfaceData_InvColorMap(pRasInfo->invColorTable, r, g, b);
}

jboolean checkSameLut(jint *SrcReadLut, jint *DstReadLut,
		      SurfaceDataRasInfo *pSrcInfo,
		      SurfaceDataRasInfo *pDstInfo)
{
    if (SrcReadLut != DstReadLut) {
	juint lutSize = pSrcInfo->lutSize;
	if (lutSize > pDstInfo->lutSize) {
	    return JNI_FALSE;
	} else {
	    juint i;
	    for (i = 0; i < lutSize; i++) {
		if (SrcReadLut[i] != DstReadLut[i]) {
		    return JNI_FALSE;
		}
	    }
	}
    }
    return JNI_TRUE;
}

DEFINE_CONVERT_BLIT(IntArgb, ByteIndexed, 3ByteRgb)

DEFINE_CONVERT_BLIT(ThreeByteBgr, ByteIndexed, 3ByteRgb)

DEFINE_CONVERT_BLIT(ByteGray, ByteIndexed, 3ByteRgb)

DEFINE_CONVERT_BLIT(Index12Gray, ByteIndexed, 3ByteRgb)

void NAME_CONVERT_BLIT(ByteIndexed, ByteIndexed)
    (void *srcBase, void *dstBase,
     juint width, juint height,
     SurfaceDataRasInfo *pSrcInfo,
     SurfaceDataRasInfo *pDstInfo,
     NativePrimitive *pPrim,
     CompositeInfo *pCompInfo)
{
    DeclareByteIndexedLoadVars(SrcRead)
    DeclareByteIndexedLoadVars(DstRead)
    jint srcScan = pSrcInfo->scanStride;
    jint dstScan = pDstInfo->scanStride;

    InitByteIndexedLoadVars(SrcRead, pSrcInfo);
    InitByteIndexedLoadVars(DstRead, pDstInfo);

    if (checkSameLut(SrcReadLut, DstReadLut, pSrcInfo, pDstInfo)) {
	do {
	    memcpy(dstBase, srcBase, width);
	    srcBase = PtrAddBytes(srcBase, srcScan);
	    dstBase = PtrAddBytes(dstBase, dstScan);
	} while (--height > 0);
    } else {
	DeclareByteIndexedStoreVars(DstWrite);

	BlitLoopWidthHeight(ByteIndexed, pSrc, srcBase, pSrcInfo,
			    ByteIndexed, pDst, dstBase, pDstInfo, DstWrite,
			    width, height,
			    ConvertVia3ByteRgb
                                (pSrc, ByteIndexed, SrcRead,
				 pDst, ByteIndexed, DstWrite, 0, 0));
    }
}

DEFINE_SCALE_BLIT(IntArgb, ByteIndexed, 3ByteRgb)

DEFINE_SCALE_BLIT(ThreeByteBgr, ByteIndexed, 3ByteRgb)

DEFINE_SCALE_BLIT(ByteGray, ByteIndexed, 3ByteRgb)

DEFINE_SCALE_BLIT(Index12Gray, ByteIndexed, 3ByteRgb)

void NAME_SCALE_BLIT(ByteIndexed, ByteIndexed)
    (void *srcBase, void *dstBase,
     juint width, juint height,
     jint sxloc, jint syloc,
     jint sxinc, jint syinc, jint shift,
     SurfaceDataRasInfo *pSrcInfo,
     SurfaceDataRasInfo *pDstInfo,
     NativePrimitive *pPrim,
     CompositeInfo *pCompInfo)
{
    DeclareByteIndexedLoadVars(SrcRead)
    DeclareByteIndexedLoadVars(DstRead)
    jint srcScan = pSrcInfo->scanStride;
    jint dstScan = pDstInfo->scanStride;
    DeclareByteIndexedStoreVars(DstWrite)

    InitByteIndexedLoadVars(SrcRead, pSrcInfo);
    InitByteIndexedLoadVars(DstRead, pDstInfo);

    if (checkSameLut(SrcReadLut, DstReadLut, pSrcInfo, pDstInfo)) {
	BlitLoopScaleWidthHeight(ByteIndexed, pSrc, srcBase, pSrcInfo,
				 ByteIndexed, pDst, dstBase, pDstInfo, DstWrite,
				 x, width, height,
				 sxloc, syloc, sxinc, syinc, shift,
				 pDst[0] = pSrc[x]);
    } else {
	BlitLoopScaleWidthHeight(ByteIndexed, pSrc, srcBase, pSrcInfo,
				 ByteIndexed, pDst, dstBase, pDstInfo, DstWrite,
				 x, width, height,
				 sxloc, syloc, sxinc, syinc, shift,
				 ConvertVia3ByteRgb(pSrc, ByteIndexed, SrcRead,
						    pDst, ByteIndexed, DstWrite,
						    x, 0));
    }
}

DEFINE_XPAR_CONVERT_BLIT_LUT8(ByteIndexedBm, ByteIndexed, ConvertOnTheFly)

DEFINE_XPAR_SCALE_BLIT_LUT8(ByteIndexedBm, ByteIndexed, ConvertOnTheFly)

DEFINE_XPAR_SCALE_BLIT(IntArgbBm, ByteIndexed, 1IntRgb)

DEFINE_XPAR_BLITBG_LUT8(ByteIndexedBm, ByteIndexed, ConvertOnTheFly)

DEFINE_XPAR_CONVERT_BLIT(IntArgbBm, ByteIndexed, 1IntRgb)

DEFINE_XPAR_BLITBG(IntArgbBm, ByteIndexed, 1IntRgb)

DEFINE_XOR_BLIT(IntArgb, ByteIndexed, AnyByte)

DEFINE_ALPHA_MASKFILL(ByteIndexed, 4ByteArgb)

DEFINE_ALPHA_MASKBLIT(IntArgb, ByteIndexed, 4ByteArgb)

DEFINE_ALPHA_MASKBLIT(IntRgb, ByteIndexed, 4ByteArgb)

DEFINE_SOLID_DRAWGLYPHLISTAA(ByteIndexed, 3ByteRgb)
