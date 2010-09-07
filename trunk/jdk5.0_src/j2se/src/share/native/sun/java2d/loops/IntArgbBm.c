/*
 * @(#)IntArgbBm.c	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "AnyInt.h"
#include "IntArgbBm.h"
#include "AlphaMacros.h"

#include "IntArgb.h"
#include "ByteIndexed.h"

/*
 * This file declares, registers, and defines the various graphics
 * primitive loops to manipulate surfaces of type "IntArgbBm".
 *
 * See also LoopMacros.h
 */

RegisterFunc RegisterIntArgbBm;

DECLARE_CONVERT_BLIT(IntArgbBm, IntArgb);
DECLARE_CONVERT_BLIT(IntArgb, IntArgbBm);
DECLARE_CONVERT_BLIT(ByteIndexed, IntArgbBm);
DECLARE_XPAR_CONVERT_BLIT(ByteIndexedBm, IntArgbBm);

DECLARE_SCALE_BLIT(IntArgb, IntArgbBm);
DECLARE_SCALE_BLIT(ByteIndexed, IntArgbBm);
DECLARE_XPAR_SCALE_BLIT(ByteIndexedBm, IntArgbBm);

DECLARE_XPAR_BLITBG(ByteIndexedBm, IntArgbBm);

DECLARE_XOR_BLIT(IntArgb, IntArgbBm);
DECLARE_ALPHA_MASKFILL(IntArgbBm);
DECLARE_ALPHA_MASKBLIT(IntArgb, IntArgbBm);
DECLARE_SOLID_DRAWGLYPHLISTAA(IntArgbBm);

NativePrimitive IntArgbBmPrimitives[] = {
    REGISTER_ANYINT_ISOCOPY_BLIT(IntArgbBm),
    REGISTER_ANYINT_ISOSCALE_BLIT(IntArgbBm),
    REGISTER_ANYINT_ISOXOR_BLIT(IntArgbBm),
    REGISTER_CONVERT_BLIT(IntArgbBm, IntArgb),
    REGISTER_CONVERT_BLIT(IntArgb, IntArgbBm),
    REGISTER_CONVERT_BLIT(ByteIndexed, IntArgbBm),
    REGISTER_SCALE_BLIT(IntArgb, IntArgbBm),
    REGISTER_SCALE_BLIT(ByteIndexed, IntArgbBm),
    REGISTER_XPAR_CONVERT_BLIT(ByteIndexedBm, IntArgbBm),
    REGISTER_XPAR_SCALE_BLIT(ByteIndexedBm, IntArgbBm),
    REGISTER_XPAR_BLITBG(ByteIndexedBm, IntArgbBm),

    REGISTER_XOR_BLIT(IntArgb, IntArgbBm),
    REGISTER_ALPHA_MASKFILL(IntArgbBm),
    REGISTER_ALPHA_MASKBLIT(IntArgb, IntArgbBm),
    REGISTER_SOLID_DRAWGLYPHLISTAA(IntArgbBm),
};

jboolean RegisterIntArgbBm(JNIEnv *env)
{
    return RegisterPrimitives(env, IntArgbBmPrimitives,
			      ArraySize(IntArgbBmPrimitives));
}

jint PixelForIntArgbBm(SurfaceDataRasInfo *pRasInfo, jint rgb)
{
    return (rgb | ((rgb >> 31) << 24));
}

DEFINE_CONVERT_BLIT(IntArgbBm, IntArgb, 1IntArgb)

DEFINE_CONVERT_BLIT(IntArgb, IntArgbBm, 1IntArgb)

DEFINE_CONVERT_BLIT(ByteIndexed, IntArgbBm, 1IntArgb)

DEFINE_SCALE_BLIT(IntArgb, IntArgbBm, 1IntArgb)

DEFINE_SCALE_BLIT(ByteIndexed, IntArgbBm, 1IntArgb)

DEFINE_XPAR_CONVERT_BLIT_LUT8(ByteIndexedBm, IntArgbBm, PreProcessLut)

DEFINE_XPAR_SCALE_BLIT_LUT8(ByteIndexedBm, IntArgbBm, PreProcessLut)

DEFINE_XPAR_BLITBG_LUT8(ByteIndexedBm, IntArgbBm, PreProcessLut)

DEFINE_XOR_BLIT(IntArgb, IntArgbBm, AnyInt)

DEFINE_ALPHA_MASKFILL(IntArgbBm, 4ByteArgb)

DEFINE_ALPHA_MASKBLIT(IntArgb, IntArgbBm, 4ByteArgb)

DEFINE_SOLID_DRAWGLYPHLISTAA(IntArgbBm, 4ByteArgb)
