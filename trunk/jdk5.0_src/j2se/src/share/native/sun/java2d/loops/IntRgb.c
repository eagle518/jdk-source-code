/*
 * @(#)IntRgb.c	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "AnyInt.h"
#include "IntRgb.h"
#include "AlphaMacros.h"

#include "IntArgb.h"
#include "IntArgbBm.h"
#include "ThreeByteBgr.h"
#include "ByteGray.h"
#include "Index12Gray.h"

/*
 * This file declares, registers, and defines the various graphics
 * primitive loops to manipulate surfaces of type "IntRgb".
 *
 * See also LoopMacros.h
 */

RegisterFunc RegisterIntRgb;

DECLARE_CONVERT_BLIT(IntRgb, IntArgb);
DECLARE_CONVERT_BLIT(ThreeByteBgr, IntRgb);
DECLARE_CONVERT_BLIT(ByteGray, IntRgb);
DECLARE_CONVERT_BLIT(Index12Gray, IntArgb);

DECLARE_XPAR_CONVERT_BLIT(IntArgbBm, IntRgb);
DECLARE_XPAR_BLITBG(IntArgbBm, IntRgb);

DECLARE_SCALE_BLIT(IntRgb, IntArgb);
DECLARE_SCALE_BLIT(ThreeByteBgr, IntRgb);
DECLARE_SCALE_BLIT(ByteGray, IntRgb);
DECLARE_SCALE_BLIT(Index12Gray, IntArgb);

DECLARE_XOR_BLIT(IntArgb, IntRgb);
DECLARE_SRC_MASKFILL(IntRgb);
DECLARE_SRCOVER_MASKFILL(IntRgb);
DECLARE_ALPHA_MASKFILL(IntRgb);
DECLARE_SRCOVER_MASKBLIT(IntArgb, IntRgb);
DECLARE_ALPHA_MASKBLIT(IntArgb, IntRgb);
DECLARE_ALPHA_MASKBLIT(IntRgb, IntRgb);
DECLARE_SOLID_DRAWGLYPHLISTAA(IntRgb);
DECLARE_XPAR_SCALE_BLIT(IntArgbBm, IntArgb);

NativePrimitive IntRgbPrimitives[] = {
    REGISTER_ANYINT_ISOCOPY_BLIT(IntRgb),
    REGISTER_ANYINT_ISOSCALE_BLIT(IntRgb),
    REGISTER_ANYINT_ISOXOR_BLIT(IntRgb),
    REGISTER_CONVERT_BLIT(IntRgb, IntArgb),
    REGISTER_CONVERT_BLIT_EQUIV(IntArgb, IntRgb,
				NAME_ISOCOPY_BLIT(AnyInt)),
    REGISTER_CONVERT_BLIT_EQUIV(IntArgbBm, IntRgb,
				NAME_ISOCOPY_BLIT(AnyInt)),
    REGISTER_CONVERT_BLIT(ThreeByteBgr, IntRgb),
    REGISTER_CONVERT_BLIT(ByteGray, IntRgb),
    REGISTER_CONVERT_BLIT_EQUIV(ByteIndexed, IntRgb,
				NAME_CONVERT_BLIT(ByteIndexed, IntArgb)),
    REGISTER_CONVERT_BLIT_EQUIV(Index12Gray, IntRgb,
				NAME_CONVERT_BLIT(Index12Gray, IntArgb)),
    REGISTER_SCALE_BLIT(IntRgb, IntArgb),
    REGISTER_SCALE_BLIT_EQUIV(IntArgb, IntRgb,
			      NAME_ISOSCALE_BLIT(AnyInt)),
    REGISTER_SCALE_BLIT_EQUIV(IntArgbBm, IntRgb,
			      NAME_ISOSCALE_BLIT(AnyInt)),
    REGISTER_SCALE_BLIT(ThreeByteBgr, IntRgb),
    REGISTER_SCALE_BLIT(ByteGray, IntRgb),
    REGISTER_SCALE_BLIT_EQUIV(ByteIndexed, IntRgb,
			      NAME_SCALE_BLIT(ByteIndexed, IntArgb)),
    REGISTER_SCALE_BLIT_EQUIV(Index12Gray, IntRgb,
			      NAME_SCALE_BLIT(Index12Gray, IntArgb)),
    REGISTER_XPAR_CONVERT_BLIT(IntArgbBm, IntRgb),
    REGISTER_XPAR_CONVERT_BLIT_EQUIV(ByteIndexedBm, IntRgb,
				     NAME_XPAR_CONVERT_BLIT(ByteIndexedBm,
							    IntArgb)),
    REGISTER_XPAR_SCALE_BLIT_EQUIV(ByteIndexedBm, IntRgb,
				   NAME_XPAR_SCALE_BLIT(ByteIndexedBm,
							IntArgb)),
    REGISTER_XPAR_SCALE_BLIT_EQUIV(IntArgbBm, IntRgb,
				   NAME_XPAR_SCALE_BLIT(IntArgbBm,
							IntArgb)),
    REGISTER_XPAR_BLITBG(IntArgbBm, IntRgb),
    REGISTER_XPAR_BLITBG_EQUIV(ByteIndexedBm, IntRgb,
			       NAME_XPAR_BLITBG(ByteIndexedBm, IntArgb)),

    REGISTER_XOR_BLIT(IntArgb, IntRgb),
    REGISTER_SRC_MASKFILL(IntRgb),
    REGISTER_SRCOVER_MASKFILL(IntRgb),
    REGISTER_ALPHA_MASKFILL(IntRgb),
    REGISTER_SRCOVER_MASKBLIT(IntArgb, IntRgb),
    REGISTER_ALPHA_MASKBLIT(IntArgb, IntRgb),
    REGISTER_ALPHA_MASKBLIT(IntRgb, IntRgb),
    REGISTER_SOLID_DRAWGLYPHLISTAA(IntRgb),
};

jboolean RegisterIntRgb(JNIEnv *env)
{
    return RegisterPrimitives(env, IntRgbPrimitives,
			      ArraySize(IntRgbPrimitives));
}

DEFINE_CONVERT_BLIT(IntRgb, IntArgb, 1IntRgb)

DEFINE_CONVERT_BLIT(ThreeByteBgr, IntRgb, 1IntRgb)

DEFINE_CONVERT_BLIT(ByteGray, IntRgb, 1IntRgb)

DEFINE_XPAR_CONVERT_BLIT(IntArgbBm, IntRgb, 1IntRgb)

DEFINE_XPAR_BLITBG(IntArgbBm, IntRgb, 1IntRgb)

DEFINE_SCALE_BLIT(IntRgb, IntArgb, 1IntRgb)

DEFINE_SCALE_BLIT(ThreeByteBgr, IntRgb, 1IntRgb)

DEFINE_SCALE_BLIT(ByteGray, IntRgb, 1IntRgb)

DEFINE_XOR_BLIT(IntArgb, IntRgb, AnyInt)

DEFINE_SRC_MASKFILL(IntRgb, 4ByteArgb)

DEFINE_SRCOVER_MASKFILL(IntRgb, 4ByteArgb)

DEFINE_ALPHA_MASKFILL(IntRgb, 4ByteArgb)

DEFINE_SRCOVER_MASKBLIT(IntArgb, IntRgb, 4ByteArgb)

DEFINE_ALPHA_MASKBLIT(IntArgb, IntRgb, 4ByteArgb)

DEFINE_ALPHA_MASKBLIT(IntRgb, IntRgb, 4ByteArgb)

DEFINE_SOLID_DRAWGLYPHLISTAA(IntRgb, 3ByteRgb)
