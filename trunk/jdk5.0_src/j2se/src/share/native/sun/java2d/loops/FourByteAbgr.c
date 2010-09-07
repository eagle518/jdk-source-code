/*
 * @(#)FourByteAbgr.c	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "Any4Byte.h"
#include "FourByteAbgr.h"
#include "AlphaMacros.h"

#include "IntArgb.h"
#include "IntArgbBm.h"
#include "IntRgb.h"
#include "ThreeByteBgr.h"
#include "ByteGray.h"
#include "ByteIndexed.h"

/*
 * This file declares, registers, and defines the various graphics
 * primitive loops to manipulate surfaces of type "FourByteAbgr".
 *
 * See also LoopMacros.h
 */

RegisterFunc RegisterFourByteAbgr;

DECLARE_CONVERT_BLIT(FourByteAbgr, IntArgb);
DECLARE_CONVERT_BLIT(IntArgb, FourByteAbgr);
DECLARE_CONVERT_BLIT(IntRgb, FourByteAbgr);
DECLARE_CONVERT_BLIT(ThreeByteBgr, FourByteAbgr);
DECLARE_CONVERT_BLIT(ByteGray, FourByteAbgr);
DECLARE_CONVERT_BLIT(ByteIndexed, FourByteAbgr);
DECLARE_SCALE_BLIT(FourByteAbgr, IntArgb);
DECLARE_SCALE_BLIT(IntArgb, FourByteAbgr);
DECLARE_SCALE_BLIT(IntRgb, FourByteAbgr);
DECLARE_SCALE_BLIT(ThreeByteBgr, FourByteAbgr);
DECLARE_SCALE_BLIT(ByteGray, FourByteAbgr);
DECLARE_SCALE_BLIT(ByteIndexed, FourByteAbgr);
DECLARE_XPAR_CONVERT_BLIT(ByteIndexedBm, FourByteAbgr);
DECLARE_XPAR_SCALE_BLIT(ByteIndexedBm, FourByteAbgr);
DECLARE_XPAR_SCALE_BLIT(IntArgbBm, FourByteAbgr);
DECLARE_XPAR_BLITBG(ByteIndexedBm, FourByteAbgr);

DECLARE_XOR_BLIT(IntArgb, FourByteAbgr);
DECLARE_SRC_MASKFILL(FourByteAbgr);
DECLARE_SRCOVER_MASKFILL(FourByteAbgr);
DECLARE_ALPHA_MASKFILL(FourByteAbgr);
DECLARE_SRCOVER_MASKBLIT(IntArgb, FourByteAbgr);
DECLARE_ALPHA_MASKBLIT(IntArgb, FourByteAbgr);
DECLARE_ALPHA_MASKBLIT(IntRgb, FourByteAbgr);
DECLARE_SOLID_DRAWGLYPHLISTAA(FourByteAbgr);

NativePrimitive FourByteAbgrPrimitives[] = {
    REGISTER_ANY4BYTE_ISOCOPY_BLIT(FourByteAbgr),
    REGISTER_ANY4BYTE_ISOSCALE_BLIT(FourByteAbgr),
    REGISTER_CONVERT_BLIT(FourByteAbgr, IntArgb),
    REGISTER_CONVERT_BLIT(IntArgb, FourByteAbgr),
    REGISTER_CONVERT_BLIT(IntRgb, FourByteAbgr),
    REGISTER_CONVERT_BLIT(ThreeByteBgr, FourByteAbgr),
    REGISTER_CONVERT_BLIT(ByteGray, FourByteAbgr),
    REGISTER_CONVERT_BLIT(ByteIndexed, FourByteAbgr),
    REGISTER_SCALE_BLIT(FourByteAbgr, IntArgb),
    REGISTER_SCALE_BLIT(IntArgb, FourByteAbgr),
    REGISTER_SCALE_BLIT(IntRgb, FourByteAbgr),
    REGISTER_SCALE_BLIT(ThreeByteBgr, FourByteAbgr),
    REGISTER_SCALE_BLIT(ByteGray, FourByteAbgr),
    REGISTER_SCALE_BLIT(ByteIndexed, FourByteAbgr),
    REGISTER_XPAR_CONVERT_BLIT(ByteIndexedBm, FourByteAbgr),
    REGISTER_XPAR_SCALE_BLIT(ByteIndexedBm, FourByteAbgr),
    REGISTER_XPAR_SCALE_BLIT(IntArgbBm, FourByteAbgr),
    REGISTER_XPAR_BLITBG(ByteIndexedBm, FourByteAbgr),

    REGISTER_XOR_BLIT(IntArgb, FourByteAbgr),
    REGISTER_SRC_MASKFILL(FourByteAbgr),
    REGISTER_SRCOVER_MASKFILL(FourByteAbgr),
    REGISTER_ALPHA_MASKFILL(FourByteAbgr),
    REGISTER_SRCOVER_MASKBLIT(IntArgb, FourByteAbgr),
    REGISTER_ALPHA_MASKBLIT(IntArgb, FourByteAbgr),
    REGISTER_ALPHA_MASKBLIT(IntRgb, FourByteAbgr),
    REGISTER_SOLID_DRAWGLYPHLISTAA(FourByteAbgr),
};

jboolean RegisterFourByteAbgr(JNIEnv *env)
{
    return RegisterPrimitives(env, FourByteAbgrPrimitives,
			      ArraySize(FourByteAbgrPrimitives));
}

jint PixelForFourByteAbgr(SurfaceDataRasInfo *pRasInfo, jint rgb)
{
    return ((rgb << 8) | (((juint) rgb) >> 24));
}

DEFINE_CONVERT_BLIT(FourByteAbgr, IntArgb, 1IntArgb)

DEFINE_CONVERT_BLIT(IntArgb, FourByteAbgr, 4ByteArgb)

DEFINE_CONVERT_BLIT(IntRgb, FourByteAbgr, 3ByteRgb)

DEFINE_CONVERT_BLIT(ThreeByteBgr, FourByteAbgr, 3ByteRgb)

DEFINE_CONVERT_BLIT(ByteGray, FourByteAbgr, 3ByteRgb)

DEFINE_CONVERT_BLIT_LUT8(ByteIndexed, FourByteAbgr, ConvertOnTheFly)

DEFINE_SCALE_BLIT(FourByteAbgr, IntArgb, 1IntArgb)

DEFINE_SCALE_BLIT(IntArgb, FourByteAbgr, 4ByteArgb)

DEFINE_SCALE_BLIT(IntRgb, FourByteAbgr, 3ByteRgb)

DEFINE_SCALE_BLIT(ThreeByteBgr, FourByteAbgr, 3ByteRgb)

DEFINE_SCALE_BLIT(ByteGray, FourByteAbgr, 3ByteRgb)

DEFINE_SCALE_BLIT_LUT8(ByteIndexed, FourByteAbgr, ConvertOnTheFly)

DEFINE_XPAR_CONVERT_BLIT_LUT8(ByteIndexedBm, FourByteAbgr, ConvertOnTheFly)

DEFINE_XPAR_SCALE_BLIT_LUT8(ByteIndexedBm, FourByteAbgr, ConvertOnTheFly)

DEFINE_XPAR_SCALE_BLIT(IntArgbBm, FourByteAbgr, 1IntRgb)

DEFINE_XPAR_BLITBG_LUT8(ByteIndexedBm, FourByteAbgr, ConvertOnTheFly)

DEFINE_XOR_BLIT(IntArgb, FourByteAbgr, Any4Byte)

DEFINE_SRC_MASKFILL(FourByteAbgr, 4ByteArgb)

DEFINE_SRCOVER_MASKFILL(FourByteAbgr, 4ByteArgb)

DEFINE_ALPHA_MASKFILL(FourByteAbgr, 4ByteArgb)

DEFINE_SRCOVER_MASKBLIT(IntArgb, FourByteAbgr, 4ByteArgb)

DEFINE_ALPHA_MASKBLIT(IntArgb, FourByteAbgr, 4ByteArgb)

DEFINE_ALPHA_MASKBLIT(IntRgb, FourByteAbgr, 4ByteArgb)

DEFINE_SOLID_DRAWGLYPHLISTAA(FourByteAbgr, 4ByteArgb)
