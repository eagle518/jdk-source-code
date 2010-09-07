/*
 * @(#)ThreeByteBgr.c	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "Any3Byte.h"
#include "ThreeByteBgr.h"
#include "AlphaMacros.h"

#include "IntArgb.h"
#include "IntArgbBm.h"
#include "IntRgb.h"
#include "ByteGray.h"
#include "ByteIndexed.h"

/*
 * This file declares, registers, and defines the various graphics
 * primitive loops to manipulate surfaces of type "ThreeByteBgr".
 *
 * See also LoopMacros.h
 */

RegisterFunc RegisterThreeByteBgr;

DECLARE_CONVERT_BLIT(ThreeByteBgr, IntArgb);
DECLARE_CONVERT_BLIT(IntArgb, ThreeByteBgr);
DECLARE_CONVERT_BLIT(ByteGray, ThreeByteBgr);
DECLARE_CONVERT_BLIT(ByteIndexed, ThreeByteBgr);
DECLARE_SCALE_BLIT(ThreeByteBgr, IntArgb);
DECLARE_SCALE_BLIT(IntArgb, ThreeByteBgr);
DECLARE_SCALE_BLIT(ByteGray, ThreeByteBgr);
DECLARE_SCALE_BLIT(ByteIndexed, ThreeByteBgr);
DECLARE_XPAR_CONVERT_BLIT(ByteIndexedBm, ThreeByteBgr);
DECLARE_XPAR_SCALE_BLIT(ByteIndexedBm, ThreeByteBgr);
DECLARE_XPAR_SCALE_BLIT(IntArgbBm, ThreeByteBgr);
DECLARE_XPAR_BLITBG(ByteIndexedBm, ThreeByteBgr);
DECLARE_XPAR_CONVERT_BLIT(IntArgbBm, ThreeByteBgr);
DECLARE_XPAR_BLITBG(IntArgbBm, ThreeByteBgr);

DECLARE_XOR_BLIT(IntArgb, ThreeByteBgr);
DECLARE_SRC_MASKFILL(ThreeByteBgr);
DECLARE_SRCOVER_MASKFILL(ThreeByteBgr);
DECLARE_ALPHA_MASKFILL(ThreeByteBgr);
DECLARE_SRCOVER_MASKBLIT(IntArgb, ThreeByteBgr);
DECLARE_ALPHA_MASKBLIT(IntArgb, ThreeByteBgr);
DECLARE_ALPHA_MASKBLIT(IntRgb, ThreeByteBgr);
DECLARE_SOLID_DRAWGLYPHLISTAA(ThreeByteBgr);

NativePrimitive ThreeByteBgrPrimitives[] = {
    REGISTER_ANY3BYTE_ISOCOPY_BLIT(ThreeByteBgr),
    REGISTER_ANY3BYTE_ISOSCALE_BLIT(ThreeByteBgr),
    REGISTER_ANY3BYTE_ISOXOR_BLIT(ThreeByteBgr),
    REGISTER_CONVERT_BLIT(ThreeByteBgr, IntArgb),
    REGISTER_CONVERT_BLIT(IntArgb, ThreeByteBgr),
    REGISTER_CONVERT_BLIT_EQUIV(IntRgb, ThreeByteBgr,
				NAME_CONVERT_BLIT(IntArgb, ThreeByteBgr)),
    REGISTER_CONVERT_BLIT_EQUIV(IntArgbBm, ThreeByteBgr,
				NAME_CONVERT_BLIT(IntArgb, ThreeByteBgr)),
    REGISTER_CONVERT_BLIT(ByteGray, ThreeByteBgr),
    REGISTER_CONVERT_BLIT(ByteIndexed, ThreeByteBgr),
    REGISTER_SCALE_BLIT(ThreeByteBgr, IntArgb),
    REGISTER_SCALE_BLIT(IntArgb, ThreeByteBgr),
    REGISTER_SCALE_BLIT_EQUIV(IntRgb, ThreeByteBgr,
			      NAME_SCALE_BLIT(IntArgb, ThreeByteBgr)),
    REGISTER_SCALE_BLIT_EQUIV(IntArgbBm, ThreeByteBgr,
			      NAME_SCALE_BLIT(IntArgb, ThreeByteBgr)),
    REGISTER_SCALE_BLIT(ByteGray, ThreeByteBgr),
    REGISTER_SCALE_BLIT(ByteIndexed, ThreeByteBgr),
    REGISTER_XPAR_CONVERT_BLIT(ByteIndexedBm, ThreeByteBgr),
    REGISTER_XPAR_SCALE_BLIT(ByteIndexedBm, ThreeByteBgr),
    REGISTER_XPAR_SCALE_BLIT(IntArgbBm, ThreeByteBgr),
    REGISTER_XPAR_BLITBG(ByteIndexedBm, ThreeByteBgr),
    REGISTER_XPAR_CONVERT_BLIT(IntArgbBm, ThreeByteBgr),
    REGISTER_XPAR_BLITBG(IntArgbBm, ThreeByteBgr),
    
    REGISTER_XOR_BLIT(IntArgb, ThreeByteBgr),
    REGISTER_SRC_MASKFILL(ThreeByteBgr),
    REGISTER_SRCOVER_MASKFILL(ThreeByteBgr),
    REGISTER_ALPHA_MASKFILL(ThreeByteBgr),
    REGISTER_SRCOVER_MASKBLIT(IntArgb, ThreeByteBgr),
    REGISTER_ALPHA_MASKBLIT(IntArgb, ThreeByteBgr),
    REGISTER_ALPHA_MASKBLIT(IntRgb, ThreeByteBgr),
    REGISTER_SOLID_DRAWGLYPHLISTAA(ThreeByteBgr),
};

jboolean RegisterThreeByteBgr(JNIEnv *env)
{
    return RegisterPrimitives(env, ThreeByteBgrPrimitives,
			      ArraySize(ThreeByteBgrPrimitives));
}

DEFINE_CONVERT_BLIT(ThreeByteBgr, IntArgb, 1IntArgb)

DEFINE_CONVERT_BLIT(IntArgb, ThreeByteBgr, 1IntRgb)

DEFINE_CONVERT_BLIT(ByteGray, ThreeByteBgr, 3ByteRgb)

DEFINE_CONVERT_BLIT_LUT8(ByteIndexed, ThreeByteBgr, ConvertOnTheFly)

DEFINE_SCALE_BLIT(ThreeByteBgr, IntArgb, 1IntArgb)

DEFINE_SCALE_BLIT(IntArgb, ThreeByteBgr, 1IntRgb)

DEFINE_SCALE_BLIT(ByteGray, ThreeByteBgr, 3ByteRgb)

DEFINE_SCALE_BLIT_LUT8(ByteIndexed, ThreeByteBgr, ConvertOnTheFly)

DEFINE_XPAR_CONVERT_BLIT_LUT8(ByteIndexedBm, ThreeByteBgr, ConvertOnTheFly)

DEFINE_XPAR_SCALE_BLIT_LUT8(ByteIndexedBm, ThreeByteBgr, ConvertOnTheFly)

DEFINE_XPAR_SCALE_BLIT(IntArgbBm, ThreeByteBgr, 1IntRgb)

DEFINE_XPAR_BLITBG_LUT8(ByteIndexedBm, ThreeByteBgr, ConvertOnTheFly)

DEFINE_XPAR_CONVERT_BLIT(IntArgbBm, ThreeByteBgr, 1IntRgb)

DEFINE_XPAR_BLITBG(IntArgbBm, ThreeByteBgr, 1IntRgb)

DEFINE_XOR_BLIT(IntArgb, ThreeByteBgr, Any3Byte)

DEFINE_SRC_MASKFILL(ThreeByteBgr, 4ByteArgb)

DEFINE_SRCOVER_MASKFILL(ThreeByteBgr, 4ByteArgb)

DEFINE_ALPHA_MASKFILL(ThreeByteBgr, 4ByteArgb)

DEFINE_SRCOVER_MASKBLIT(IntArgb, ThreeByteBgr, 4ByteArgb)

DEFINE_ALPHA_MASKBLIT(IntArgb, ThreeByteBgr, 4ByteArgb)

DEFINE_ALPHA_MASKBLIT(IntRgb, ThreeByteBgr, 4ByteArgb)

DEFINE_SOLID_DRAWGLYPHLISTAA(ThreeByteBgr, 3ByteRgb)
