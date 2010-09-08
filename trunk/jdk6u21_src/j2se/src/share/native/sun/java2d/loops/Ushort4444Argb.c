/*
 * @(#)Ushort4444Argb.c	1.5 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "AnyShort.h"
#include "Ushort4444Argb.h"
#include "AlphaMacros.h"

#include "IntArgb.h"
#include "IntArgbBm.h"
#include "IntRgb.h"
#include "ThreeByteBgr.h"
#include "ByteGray.h"
#include "ByteIndexed.h"

/*
 * This file declares, registers, and defines the various graphics
 * primitive loops to manipulate surfaces of type "Ushort4444Argb".
 *
 * See also LoopMacros.h
 */

RegisterFunc RegisterUshort4444Argb;

DECLARE_SRCOVER_MASKBLIT(IntArgb, Ushort4444Argb);

NativePrimitive Ushort4444ArgbPrimitives[] = {
    REGISTER_ANYSHORT_ISOCOPY_BLIT(Ushort4444Argb),
    REGISTER_SRCOVER_MASKBLIT(IntArgb, Ushort4444Argb),
};

jboolean RegisterUshort4444Argb(JNIEnv *env)
{
    return RegisterPrimitives(env, Ushort4444ArgbPrimitives,
			      ArraySize(Ushort4444ArgbPrimitives));
}

jint PixelForUshort4444Argb(SurfaceDataRasInfo *pRasInfo, jint rgb)
{
    return IntArgbToUshort4444Argb(rgb);
}

DEFINE_SRCOVER_MASKBLIT(IntArgb, Ushort4444Argb, 4ByteArgb)
