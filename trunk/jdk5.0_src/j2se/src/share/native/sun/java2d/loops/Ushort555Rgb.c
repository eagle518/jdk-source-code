/*
 * @(#)Ushort555Rgb.c	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "AnyShort.h"
#include "Ushort555Rgb.h"
#include "AlphaMacros.h"

#include "IntArgb.h"
#include "IntArgbBm.h"
#include "IntRgb.h"
#include "ThreeByteBgr.h"
#include "ByteGray.h"
#include "ByteIndexed.h"

/*
 * This file declares, registers, and defines the various graphics
 * primitive loops to manipulate surfaces of type "Ushort555Rgb".
 *
 * See also LoopMacros.h
 */

RegisterFunc RegisterUshort555Rgb;

DECLARE_CONVERT_BLIT(Ushort555Rgb, IntArgb);
DECLARE_CONVERT_BLIT(IntArgb, Ushort555Rgb);
DECLARE_CONVERT_BLIT(ThreeByteBgr, Ushort555Rgb);
DECLARE_CONVERT_BLIT(ByteGray, Ushort555Rgb);
DECLARE_CONVERT_BLIT(ByteIndexed, Ushort555Rgb);
DECLARE_SCALE_BLIT(Ushort555Rgb, IntArgb);
DECLARE_SCALE_BLIT(IntArgb, Ushort555Rgb);
DECLARE_SCALE_BLIT(ThreeByteBgr, Ushort555Rgb);
DECLARE_SCALE_BLIT(ByteGray, Ushort555Rgb);
DECLARE_SCALE_BLIT(ByteIndexed, Ushort555Rgb);
DECLARE_XPAR_CONVERT_BLIT(ByteIndexedBm, Ushort555Rgb);
DECLARE_XPAR_SCALE_BLIT(ByteIndexedBm, Ushort555Rgb);
DECLARE_XPAR_SCALE_BLIT(IntArgbBm, Ushort555Rgb);
DECLARE_XPAR_BLITBG(ByteIndexedBm, Ushort555Rgb);
DECLARE_XPAR_CONVERT_BLIT(IntArgbBm, Ushort555Rgb);
DECLARE_XPAR_BLITBG(IntArgbBm, Ushort555Rgb);

DECLARE_XOR_BLIT(IntArgb, Ushort555Rgb);
DECLARE_SRC_MASKFILL(Ushort555Rgb);
DECLARE_SRCOVER_MASKFILL(Ushort555Rgb);
DECLARE_ALPHA_MASKFILL(Ushort555Rgb);
DECLARE_SRCOVER_MASKBLIT(IntArgb, Ushort555Rgb);
DECLARE_ALPHA_MASKBLIT(IntArgb, Ushort555Rgb);
DECLARE_ALPHA_MASKBLIT(IntRgb, Ushort555Rgb);
DECLARE_SOLID_DRAWGLYPHLISTAA(Ushort555Rgb); 

NativePrimitive Ushort555RgbPrimitives[] = {
    REGISTER_ANYSHORT_ISOCOPY_BLIT(Ushort555Rgb),
    REGISTER_ANYSHORT_ISOSCALE_BLIT(Ushort555Rgb),
    REGISTER_ANYSHORT_ISOXOR_BLIT(Ushort555Rgb),
    REGISTER_CONVERT_BLIT(Ushort555Rgb, IntArgb),
    REGISTER_CONVERT_BLIT(IntArgb, Ushort555Rgb),
    REGISTER_CONVERT_BLIT_EQUIV(IntRgb, Ushort555Rgb,
				NAME_CONVERT_BLIT(IntArgb, Ushort555Rgb)),
    REGISTER_CONVERT_BLIT_EQUIV(IntArgbBm, Ushort555Rgb,
				NAME_CONVERT_BLIT(IntArgb, Ushort555Rgb)),
    REGISTER_CONVERT_BLIT(ThreeByteBgr, Ushort555Rgb),
    REGISTER_CONVERT_BLIT(ByteGray, Ushort555Rgb),
    REGISTER_CONVERT_BLIT(ByteIndexed, Ushort555Rgb),
    REGISTER_SCALE_BLIT(Ushort555Rgb, IntArgb),
    REGISTER_SCALE_BLIT(IntArgb, Ushort555Rgb),
    REGISTER_SCALE_BLIT_EQUIV(IntRgb, Ushort555Rgb,
			      NAME_SCALE_BLIT(IntArgb, Ushort555Rgb)),
    REGISTER_SCALE_BLIT_EQUIV(IntArgbBm, Ushort555Rgb,
			      NAME_SCALE_BLIT(IntArgb, Ushort555Rgb)),
    REGISTER_SCALE_BLIT(ThreeByteBgr, Ushort555Rgb),
    REGISTER_SCALE_BLIT(ByteGray, Ushort555Rgb),
    REGISTER_SCALE_BLIT(ByteIndexed, Ushort555Rgb),
    REGISTER_XPAR_CONVERT_BLIT(ByteIndexedBm, Ushort555Rgb),
    REGISTER_XPAR_SCALE_BLIT(ByteIndexedBm, Ushort555Rgb),
    REGISTER_XPAR_SCALE_BLIT(IntArgbBm, Ushort555Rgb),
    REGISTER_XPAR_BLITBG(ByteIndexedBm, Ushort555Rgb),
    REGISTER_XPAR_CONVERT_BLIT(IntArgbBm, Ushort555Rgb),
    REGISTER_XPAR_BLITBG(IntArgbBm, Ushort555Rgb),

    REGISTER_XOR_BLIT(IntArgb, Ushort555Rgb),
    REGISTER_SRC_MASKFILL(Ushort555Rgb),
    REGISTER_SRCOVER_MASKFILL(Ushort555Rgb),
    REGISTER_ALPHA_MASKFILL(Ushort555Rgb),
    REGISTER_ALPHA_MASKBLIT(IntArgb, Ushort555Rgb),
    REGISTER_ALPHA_MASKBLIT(IntRgb, Ushort555Rgb),
    REGISTER_SOLID_DRAWGLYPHLISTAA(Ushort555Rgb), 
};

jboolean RegisterUshort555Rgb(JNIEnv *env)
{
    return RegisterPrimitives(env, Ushort555RgbPrimitives,
			      ArraySize(Ushort555RgbPrimitives));
}

jint PixelForUshort555Rgb(SurfaceDataRasInfo *pRasInfo, jint rgb)
{
    return IntArgbToUshort555Rgb(rgb);
}

DEFINE_CONVERT_BLIT(Ushort555Rgb, IntArgb, 3ByteRgb)

DEFINE_CONVERT_BLIT(IntArgb, Ushort555Rgb, 1IntRgb)

DEFINE_CONVERT_BLIT(ThreeByteBgr, Ushort555Rgb, 3ByteRgb)

DEFINE_CONVERT_BLIT(ByteGray, Ushort555Rgb, 3ByteRgb)

DEFINE_CONVERT_BLIT_LUT8(ByteIndexed, Ushort555Rgb, PreProcessLut)

DEFINE_SCALE_BLIT(Ushort555Rgb, IntArgb, 3ByteRgb)

DEFINE_SCALE_BLIT(IntArgb, Ushort555Rgb, 1IntRgb)

DEFINE_SCALE_BLIT(ThreeByteBgr, Ushort555Rgb, 3ByteRgb)

DEFINE_SCALE_BLIT(ByteGray, Ushort555Rgb, 3ByteRgb)

DEFINE_SCALE_BLIT_LUT8(ByteIndexed, Ushort555Rgb, PreProcessLut)

DEFINE_XPAR_CONVERT_BLIT_LUT8(ByteIndexedBm, Ushort555Rgb, PreProcessLut)

DEFINE_XPAR_SCALE_BLIT_LUT8(ByteIndexedBm, Ushort555Rgb, PreProcessLut)

DEFINE_XPAR_SCALE_BLIT(IntArgbBm, Ushort555Rgb, 1IntRgb)

DEFINE_XPAR_BLITBG_LUT8(ByteIndexedBm, Ushort555Rgb, PreProcessLut)

DEFINE_XPAR_CONVERT_BLIT(IntArgbBm, Ushort555Rgb, 1IntRgb)

DEFINE_XPAR_BLITBG(IntArgbBm, Ushort555Rgb, 1IntRgb)

DEFINE_XOR_BLIT(IntArgb, Ushort555Rgb, AnyShort)

DEFINE_SRC_MASKFILL(Ushort555Rgb, 4ByteArgb)

DEFINE_SRCOVER_MASKFILL(Ushort555Rgb, 4ByteArgb)

DEFINE_ALPHA_MASKFILL(Ushort555Rgb, 4ByteArgb)

DEFINE_SRCOVER_MASKBLIT(IntArgb, Ushort555Rgb, 4ByteArgb)

DEFINE_ALPHA_MASKBLIT(IntArgb, Ushort555Rgb, 4ByteArgb)

DEFINE_ALPHA_MASKBLIT(IntRgb, Ushort555Rgb, 4ByteArgb)

DEFINE_SOLID_DRAWGLYPHLISTAA(Ushort555Rgb, 3ByteRgb) 
