/*
 * @(#)IntRgbx.c	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "AnyInt.h"
#include "IntRgbx.h"

#include "IntArgb.h"
#include "IntArgbBm.h"
#include "ThreeByteBgr.h"
#include "ByteGray.h"
#include "ByteIndexed.h"

/*
 * This file declares, registers, and defines the various graphics
 * primitive loops to manipulate surfaces of type "IntRgbx".
 *
 * See also LoopMacros.h
 */

RegisterFunc RegisterIntRgbx;

DECLARE_CONVERT_BLIT(IntRgbx, IntArgb);
DECLARE_CONVERT_BLIT(IntArgb, IntRgbx);
DECLARE_CONVERT_BLIT(ThreeByteBgr, IntRgbx);
DECLARE_CONVERT_BLIT(ByteGray, IntRgbx);
DECLARE_CONVERT_BLIT(ByteIndexed, IntRgbx);
DECLARE_SCALE_BLIT(IntRgbx, IntArgb);
DECLARE_SCALE_BLIT(IntArgb, IntRgbx);
DECLARE_SCALE_BLIT(ThreeByteBgr, IntRgbx);
DECLARE_SCALE_BLIT(ByteGray, IntRgbx);
DECLARE_SCALE_BLIT(ByteIndexed, IntRgbx);
DECLARE_XPAR_CONVERT_BLIT(ByteIndexedBm, IntRgbx);
DECLARE_XPAR_SCALE_BLIT(ByteIndexedBm, IntRgbx);
DECLARE_XPAR_SCALE_BLIT(IntArgbBm, IntRgbx);
DECLARE_XPAR_BLITBG(ByteIndexedBm, IntRgbx);
DECLARE_XOR_BLIT(IntArgb, IntRgbx);
DECLARE_SOLID_DRAWGLYPHLISTAA(IntRgbx);

NativePrimitive IntRgbxPrimitives[] = {
    REGISTER_ANYINT_ISOCOPY_BLIT(IntRgbx),
    REGISTER_ANYINT_ISOSCALE_BLIT(IntRgbx),
    REGISTER_ANYINT_ISOXOR_BLIT(IntRgbx),
    REGISTER_CONVERT_BLIT(IntRgbx, IntArgb),
    REGISTER_CONVERT_BLIT(IntArgb, IntRgbx),
    REGISTER_CONVERT_BLIT(ThreeByteBgr, IntRgbx),
    REGISTER_CONVERT_BLIT(ByteGray, IntRgbx),
    REGISTER_CONVERT_BLIT_EQUIV(IntRgb, IntRgbx,
				NAME_CONVERT_BLIT(IntArgb, IntRgbx)),
    REGISTER_CONVERT_BLIT(ByteIndexed, IntRgbx),
    REGISTER_SCALE_BLIT(IntRgbx, IntArgb),
    REGISTER_SCALE_BLIT(IntArgb, IntRgbx),
    REGISTER_SCALE_BLIT(ThreeByteBgr, IntRgbx),
    REGISTER_SCALE_BLIT(ByteGray, IntRgbx),
    REGISTER_SCALE_BLIT_EQUIV(IntRgb, IntRgbx,
			      NAME_SCALE_BLIT(IntArgb, IntRgbx)),
    REGISTER_SCALE_BLIT(ByteIndexed, IntRgbx),
    REGISTER_XPAR_CONVERT_BLIT(ByteIndexedBm, IntRgbx),
    REGISTER_XPAR_SCALE_BLIT(ByteIndexedBm, IntRgbx),
    REGISTER_XPAR_SCALE_BLIT(IntArgbBm, IntRgbx),
    REGISTER_XPAR_BLITBG(ByteIndexedBm, IntRgbx),
    REGISTER_XOR_BLIT(IntArgb, IntRgbx),
    REGISTER_SOLID_DRAWGLYPHLISTAA(IntRgbx)
};

jboolean RegisterIntRgbx(JNIEnv *env)
{
    return RegisterPrimitives(env, IntRgbxPrimitives,
			      ArraySize(IntRgbxPrimitives));
}

jint PixelForIntRgbx(SurfaceDataRasInfo *pRasInfo, jint rgb)
{
    return (rgb << 8);
}

DEFINE_CONVERT_BLIT(IntRgbx, IntArgb, 1IntRgb)

DEFINE_CONVERT_BLIT(IntArgb, IntRgbx, 1IntRgb)

DEFINE_CONVERT_BLIT(ThreeByteBgr, IntRgbx, 1IntRgb)

DEFINE_CONVERT_BLIT(ByteGray, IntRgbx, 1IntRgb)

DEFINE_CONVERT_BLIT_LUT8(ByteIndexed, IntRgbx, ConvertOnTheFly)

DEFINE_SCALE_BLIT(IntRgbx, IntArgb, 1IntRgb)

DEFINE_SCALE_BLIT(IntArgb, IntRgbx, 1IntRgb)

DEFINE_SCALE_BLIT(ThreeByteBgr, IntRgbx, 1IntRgb)

DEFINE_SCALE_BLIT(ByteGray, IntRgbx, 1IntRgb)

DEFINE_SCALE_BLIT_LUT8(ByteIndexed, IntRgbx, ConvertOnTheFly)

DEFINE_XPAR_CONVERT_BLIT_LUT8(ByteIndexedBm, IntRgbx, ConvertOnTheFly)

DEFINE_XPAR_SCALE_BLIT_LUT8(ByteIndexedBm, IntRgbx, ConvertOnTheFly)

DEFINE_XPAR_SCALE_BLIT(IntArgbBm, IntRgbx, 1IntRgb)

DEFINE_XPAR_BLITBG_LUT8(ByteIndexedBm, IntRgbx, ConvertOnTheFly)

DEFINE_XOR_BLIT(IntArgb, IntRgbx, AnyInt)

DEFINE_SOLID_DRAWGLYPHLISTAA(IntRgbx, 3ByteRgb)
