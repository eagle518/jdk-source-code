/*
 * @(#)FourByteAbgrPre.c	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "Any4Byte.h"
#include "FourByteAbgrPre.h"
#include "AlphaMacros.h"

#include "IntArgb.h"
#include "IntArgbBm.h"
#include "IntRgb.h"
#include "ThreeByteBgr.h"
#include "ByteGray.h"
#include "ByteIndexed.h"

/*
 * This file declares, registers, and defines the various graphics
 * primitive loops to manipulate surfaces of type "FourByteAbgrPre".
 *
 * See also LoopMacros.h
 */

RegisterFunc RegisterFourByteAbgrPre;

DECLARE_CONVERT_BLIT(FourByteAbgrPre, IntArgb);
DECLARE_CONVERT_BLIT(IntArgb, FourByteAbgrPre);
DECLARE_CONVERT_BLIT(IntRgb, FourByteAbgrPre);
DECLARE_CONVERT_BLIT(ThreeByteBgr, FourByteAbgrPre);
DECLARE_CONVERT_BLIT(ByteGray, FourByteAbgrPre);
DECLARE_CONVERT_BLIT(ByteIndexed, FourByteAbgrPre);
DECLARE_SCALE_BLIT(FourByteAbgrPre, IntArgb);
DECLARE_SCALE_BLIT(IntArgb, FourByteAbgrPre);
DECLARE_SCALE_BLIT(IntRgb, FourByteAbgrPre);
DECLARE_SCALE_BLIT(ThreeByteBgr, FourByteAbgrPre);
DECLARE_SCALE_BLIT(ByteGray, FourByteAbgrPre);
DECLARE_SCALE_BLIT(ByteIndexed, FourByteAbgrPre);
DECLARE_XPAR_CONVERT_BLIT(ByteIndexedBm, FourByteAbgrPre);
DECLARE_XPAR_SCALE_BLIT(ByteIndexedBm, FourByteAbgrPre);
DECLARE_XPAR_SCALE_BLIT(IntArgbBm, FourByteAbgrPre);
DECLARE_XPAR_BLITBG(ByteIndexedBm, FourByteAbgrPre);

DECLARE_XOR_BLIT(IntArgb, FourByteAbgrPre);
DECLARE_SRC_MASKFILL(FourByteAbgrPre);
DECLARE_SRCOVER_MASKFILL(FourByteAbgrPre);
DECLARE_ALPHA_MASKFILL(FourByteAbgrPre);
DECLARE_SRCOVER_MASKBLIT(IntArgb, FourByteAbgrPre);
DECLARE_ALPHA_MASKBLIT(IntArgb, FourByteAbgrPre);
DECLARE_ALPHA_MASKBLIT(IntRgb, FourByteAbgrPre);
DECLARE_SOLID_DRAWGLYPHLISTAA(FourByteAbgrPre);

NativePrimitive FourByteAbgrPrePrimitives[] = {
    REGISTER_ANY4BYTE_ISOCOPY_BLIT(FourByteAbgrPre),
    REGISTER_ANY4BYTE_ISOSCALE_BLIT(FourByteAbgrPre),
    REGISTER_CONVERT_BLIT(FourByteAbgrPre, IntArgb),
    REGISTER_CONVERT_BLIT(IntArgb, FourByteAbgrPre),
    REGISTER_CONVERT_BLIT(IntRgb, FourByteAbgrPre),
    REGISTER_CONVERT_BLIT(ThreeByteBgr, FourByteAbgrPre),
    REGISTER_CONVERT_BLIT(ByteGray, FourByteAbgrPre),
    REGISTER_CONVERT_BLIT(ByteIndexed, FourByteAbgrPre),
    REGISTER_SCALE_BLIT(FourByteAbgrPre, IntArgb),
    REGISTER_SCALE_BLIT(IntArgb, FourByteAbgrPre),
    REGISTER_SCALE_BLIT(IntRgb, FourByteAbgrPre),
    REGISTER_SCALE_BLIT(ThreeByteBgr, FourByteAbgrPre),
    REGISTER_SCALE_BLIT(ByteGray, FourByteAbgrPre),
    REGISTER_SCALE_BLIT(ByteIndexed, FourByteAbgrPre),
    REGISTER_XPAR_CONVERT_BLIT(ByteIndexedBm, FourByteAbgrPre),
    REGISTER_XPAR_SCALE_BLIT(ByteIndexedBm, FourByteAbgrPre),
    REGISTER_XPAR_SCALE_BLIT(IntArgbBm, FourByteAbgrPre),
    REGISTER_XPAR_BLITBG(ByteIndexedBm, FourByteAbgrPre),

    REGISTER_XOR_BLIT(IntArgb, FourByteAbgrPre),      
    REGISTER_SRC_MASKFILL(FourByteAbgrPre),
    REGISTER_SRCOVER_MASKFILL(FourByteAbgrPre),
    REGISTER_ALPHA_MASKFILL(FourByteAbgrPre),
    REGISTER_SRCOVER_MASKBLIT(IntArgb, FourByteAbgrPre),
    REGISTER_ALPHA_MASKBLIT(IntArgb, FourByteAbgrPre),
    REGISTER_ALPHA_MASKBLIT(IntRgb, FourByteAbgrPre),
    REGISTER_SOLID_DRAWGLYPHLISTAA(FourByteAbgrPre),
};

jboolean RegisterFourByteAbgrPre(JNIEnv *env)
{
    return RegisterPrimitives(env, FourByteAbgrPrePrimitives,
			      ArraySize(FourByteAbgrPrePrimitives));
}

jint PixelForFourByteAbgrPre(SurfaceDataRasInfo *pRasInfo, jint rgb)
{
    jint a, r, g, b;
    if ((rgb >> 24) == -1) {
        return ((rgb << 8) | (((juint) rgb) >> 24));
    }
    ExtractIntDcmComponents1234(rgb, a, r, g, b);
    r = MUL8(a, r);
    g = MUL8(a, g);
    b = MUL8(a, b);
    return ComposeIntDcmComponents1234(r, g, b, a);
}

DEFINE_CONVERT_BLIT(FourByteAbgrPre, IntArgb, 1IntArgb)

DEFINE_CONVERT_BLIT(IntArgb, FourByteAbgrPre, 4ByteArgb)

DEFINE_CONVERT_BLIT(IntRgb, FourByteAbgrPre, 3ByteRgb)

DEFINE_CONVERT_BLIT(ThreeByteBgr, FourByteAbgrPre, 3ByteRgb)

DEFINE_CONVERT_BLIT(ByteGray, FourByteAbgrPre, 3ByteRgb)

DEFINE_CONVERT_BLIT_LUT8(ByteIndexed, FourByteAbgrPre, ConvertOnTheFly)

DEFINE_SCALE_BLIT(FourByteAbgrPre, IntArgb, 1IntArgb)

DEFINE_SCALE_BLIT(IntArgb, FourByteAbgrPre, 4ByteArgb)

DEFINE_SCALE_BLIT(IntRgb, FourByteAbgrPre, 3ByteRgb)

DEFINE_SCALE_BLIT(ThreeByteBgr, FourByteAbgrPre, 3ByteRgb)

DEFINE_SCALE_BLIT(ByteGray, FourByteAbgrPre, 3ByteRgb)

DEFINE_SCALE_BLIT_LUT8(ByteIndexed, FourByteAbgrPre, ConvertOnTheFly)

DEFINE_XPAR_CONVERT_BLIT_LUT8(ByteIndexedBm, FourByteAbgrPre,ConvertOnTheFly)

DEFINE_XPAR_SCALE_BLIT_LUT8(ByteIndexedBm, FourByteAbgrPre,ConvertOnTheFly)

DEFINE_XPAR_SCALE_BLIT(IntArgbBm, FourByteAbgrPre, 1IntRgb)

DEFINE_XPAR_BLITBG_LUT8(ByteIndexedBm, FourByteAbgrPre, ConvertOnTheFly)

DEFINE_XOR_BLIT(IntArgb, FourByteAbgrPre, Any4Byte)

DEFINE_SRC_MASKFILL(FourByteAbgrPre, 4ByteArgb)

DEFINE_SRCOVER_MASKFILL(FourByteAbgrPre, 4ByteArgb)

DEFINE_ALPHA_MASKFILL(FourByteAbgrPre, 4ByteArgb)

DEFINE_SRCOVER_MASKBLIT(IntArgb, FourByteAbgrPre, 4ByteArgb)

DEFINE_ALPHA_MASKBLIT(IntArgb, FourByteAbgrPre, 4ByteArgb)

DEFINE_ALPHA_MASKBLIT(IntRgb, FourByteAbgrPre, 4ByteArgb)

DEFINE_SOLID_DRAWGLYPHLISTAA(FourByteAbgrPre, 4ByteArgb)
