/*
 * @(#)IntArgb.c	1.18 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "AnyInt.h"
#include "IntArgb.h"
#include "IntArgbBm.h"
#include "AlphaMacros.h"

#include "IntArgbPre.h"
#include "IntRgb.h"
#include "ByteIndexed.h"
#include "Index12Gray.h"

/*
 * This file declares, registers, and defines the various graphics
 * primitive loops to manipulate surfaces of type "IntArgb".
 *
 * See also LoopMacros.h
 */

RegisterFunc RegisterIntArgb;

DECLARE_CONVERT_BLIT(Index12Gray, IntArgb);
DECLARE_XOR_BLIT(IntArgb, IntArgb);
DECLARE_SRC_MASKFILL(IntArgb);
DECLARE_SRCOVER_MASKFILL(IntArgb);
DECLARE_ALPHA_MASKFILL(IntArgb);
DECLARE_SRCOVER_MASKBLIT(IntArgb, IntArgb);
DECLARE_ALPHA_MASKBLIT(IntArgb, IntArgb);
DECLARE_SRCOVER_MASKBLIT(IntArgbPre, IntArgb);
DECLARE_ALPHA_MASKBLIT(IntArgbPre, IntArgb);
DECLARE_ALPHA_MASKBLIT(IntRgb, IntArgb);
DECLARE_SOLID_DRAWGLYPHLISTAA(IntArgb);
DECLARE_SOLID_DRAWGLYPHLISTLCD(IntArgb);
DECLARE_XPAR_SCALE_BLIT(IntArgbBm, IntArgb);

DECLARE_TRANSFORMHELPER_FUNCS(IntArgb);

NativePrimitive IntArgbPrimitives[] = {
    REGISTER_ANYINT_ISOCOPY_BLIT(IntArgb),
    REGISTER_ANYINT_ISOSCALE_BLIT(IntArgb),
    REGISTER_CONVERT_BLIT(ByteIndexed, IntArgb),
    REGISTER_CONVERT_BLIT(Index12Gray, IntArgb),
    REGISTER_SCALE_BLIT(ByteIndexed, IntArgb),
    REGISTER_XPAR_CONVERT_BLIT(ByteIndexedBm, IntArgb),
    REGISTER_XPAR_SCALE_BLIT(ByteIndexedBm, IntArgb),
    REGISTER_XPAR_SCALE_BLIT(IntArgbBm, IntArgb),
    REGISTER_XPAR_BLITBG(ByteIndexedBm, IntArgb),

    REGISTER_XOR_BLIT(IntArgb, IntArgb),
    REGISTER_SRC_MASKFILL(IntArgb),
    REGISTER_SRCOVER_MASKFILL(IntArgb),
    REGISTER_ALPHA_MASKFILL(IntArgb),
    REGISTER_SRCOVER_MASKBLIT(IntArgb, IntArgb),
    REGISTER_ALPHA_MASKBLIT(IntArgb, IntArgb),
    REGISTER_SRCOVER_MASKBLIT(IntArgbPre, IntArgb),
    REGISTER_ALPHA_MASKBLIT(IntArgbPre, IntArgb),
    REGISTER_ALPHA_MASKBLIT(IntRgb, IntArgb),
    REGISTER_SOLID_DRAWGLYPHLISTAA(IntArgb),
    REGISTER_SOLID_DRAWGLYPHLISTLCD(IntArgb),

    REGISTER_TRANSFORMHELPER_FUNCS(IntArgb),
};

jboolean RegisterIntArgb(JNIEnv *env)
{
    return RegisterPrimitives(env, IntArgbPrimitives,
			      ArraySize(IntArgbPrimitives));
}

DEFINE_CONVERT_BLIT_LUT8(ByteIndexed, IntArgb, ConvertOnTheFly)

DEFINE_CONVERT_BLIT_LUT8(Index12Gray, IntArgb, ConvertOnTheFly)

DEFINE_SCALE_BLIT_LUT8(ByteIndexed, IntArgb, ConvertOnTheFly)

DEFINE_XPAR_CONVERT_BLIT_LUT8(ByteIndexedBm, IntArgb, ConvertOnTheFly)

DEFINE_XPAR_SCALE_BLIT_LUT8(ByteIndexedBm, IntArgb, ConvertOnTheFly)

DEFINE_XPAR_SCALE_BLIT(IntArgbBm, IntArgb, 1IntRgb)

DEFINE_XPAR_BLITBG_LUT8(ByteIndexedBm, IntArgb, ConvertOnTheFly)

DEFINE_XOR_BLIT(IntArgb, IntArgb, AnyInt)

DEFINE_SRC_MASKFILL(IntArgb, 4ByteArgb)

DEFINE_SRCOVER_MASKFILL(IntArgb, 4ByteArgb)

DEFINE_ALPHA_MASKFILL(IntArgb, 4ByteArgb)

DEFINE_SRCOVER_MASKBLIT(IntArgb, IntArgb, 4ByteArgb)

DEFINE_ALPHA_MASKBLIT(IntArgb, IntArgb, 4ByteArgb)

DEFINE_SRCOVER_MASKBLIT(IntArgbPre, IntArgb, 4ByteArgb)

DEFINE_ALPHA_MASKBLIT(IntArgbPre, IntArgb, 4ByteArgb)

DEFINE_ALPHA_MASKBLIT(IntRgb, IntArgb, 4ByteArgb)

DEFINE_SOLID_DRAWGLYPHLISTAA(IntArgb, 4ByteArgb)

DEFINE_SOLID_DRAWGLYPHLISTLCD(IntArgb, 4ByteArgb)

DEFINE_TRANSFORMHELPERS(IntArgb)
