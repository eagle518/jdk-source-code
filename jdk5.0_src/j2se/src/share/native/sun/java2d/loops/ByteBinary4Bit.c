/*
 * @(#)ByteBinary4Bit.c	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "ByteBinary4Bit.h"

#include "IntArgb.h"

/*
 * This file declares, registers, and defines the various graphics
 * primitive loops to manipulate surfaces of type "ByteBinary4Bit".
 *
 * See also LoopMacros.h
 */

RegisterFunc RegisterByteBinary4Bit;

DECLARE_SOLID_FILLRECT(ByteBinary4Bit);
DECLARE_SOLID_FILLSPANS(ByteBinary4Bit);
DECLARE_SOLID_DRAWLINE(ByteBinary4Bit);
DECLARE_XOR_FILLRECT(ByteBinary4Bit);
DECLARE_XOR_FILLSPANS(ByteBinary4Bit);
DECLARE_XOR_DRAWLINE(ByteBinary4Bit);
DECLARE_SOLID_DRAWGLYPHLIST(ByteBinary4Bit);
DECLARE_SOLID_DRAWGLYPHLISTAA(ByteBinary4Bit);
DECLARE_XOR_DRAWGLYPHLIST(ByteBinary4Bit);

DECLARE_CONVERT_BLIT(ByteBinary4Bit, ByteBinary4Bit);
DECLARE_CONVERT_BLIT(ByteBinary4Bit, IntArgb);
DECLARE_CONVERT_BLIT(IntArgb, ByteBinary4Bit);
DECLARE_XOR_BLIT(IntArgb, ByteBinary4Bit);

DECLARE_ALPHA_MASKBLIT(ByteBinary4Bit, IntArgb);
DECLARE_ALPHA_MASKBLIT(IntArgb, ByteBinary4Bit);
DECLARE_ALPHA_MASKFILL(ByteBinary4Bit);

NativePrimitive ByteBinary4BitPrimitives[] = {
    REGISTER_SOLID_FILLRECT(ByteBinary4Bit),
    REGISTER_SOLID_FILLSPANS(ByteBinary4Bit),
    REGISTER_SOLID_LINE_PRIMITIVES(ByteBinary4Bit),
    REGISTER_XOR_FILLRECT(ByteBinary4Bit),
    REGISTER_XOR_FILLSPANS(ByteBinary4Bit),
    REGISTER_XOR_LINE_PRIMITIVES(ByteBinary4Bit),
    REGISTER_SOLID_DRAWGLYPHLIST(ByteBinary4Bit),
    REGISTER_SOLID_DRAWGLYPHLISTAA(ByteBinary4Bit),
    REGISTER_XOR_DRAWGLYPHLIST(ByteBinary4Bit),

    REGISTER_CONVERT_BLIT(ByteBinary4Bit, ByteBinary4Bit),
    REGISTER_CONVERT_BLIT(ByteBinary4Bit, IntArgb),
    REGISTER_CONVERT_BLIT(IntArgb, ByteBinary4Bit),
    REGISTER_XOR_BLIT(IntArgb, ByteBinary4Bit),

    REGISTER_ALPHA_MASKBLIT(ByteBinary4Bit, IntArgb),
    REGISTER_ALPHA_MASKBLIT(IntArgb, ByteBinary4Bit),
    REGISTER_ALPHA_MASKFILL(ByteBinary4Bit),
};

jboolean RegisterByteBinary4Bit(JNIEnv *env)
{
    return RegisterPrimitives(env, ByteBinary4BitPrimitives,
			      ArraySize(ByteBinary4BitPrimitives));
}

DEFINE_BYTE_BINARY_SOLID_FILLRECT(ByteBinary4Bit)

DEFINE_BYTE_BINARY_SOLID_FILLSPANS(ByteBinary4Bit)

DEFINE_BYTE_BINARY_SOLID_DRAWLINE(ByteBinary4Bit)

DEFINE_BYTE_BINARY_XOR_FILLRECT(ByteBinary4Bit)

DEFINE_BYTE_BINARY_XOR_FILLSPANS(ByteBinary4Bit)

DEFINE_BYTE_BINARY_XOR_DRAWLINE(ByteBinary4Bit)

DEFINE_BYTE_BINARY_SOLID_DRAWGLYPHLIST(ByteBinary4Bit)

DEFINE_BYTE_BINARY_SOLID_DRAWGLYPHLISTAA(ByteBinary4Bit, 3ByteRgb)

DEFINE_BYTE_BINARY_XOR_DRAWGLYPHLIST(ByteBinary4Bit)

DEFINE_BYTE_BINARY_CONVERT_BLIT(ByteBinary4Bit, ByteBinary4Bit, 1IntRgb)

DEFINE_BYTE_BINARY_CONVERT_BLIT(ByteBinary4Bit, IntArgb, 1IntArgb)

DEFINE_BYTE_BINARY_CONVERT_BLIT(IntArgb, ByteBinary4Bit, 1IntRgb)

DEFINE_BYTE_BINARY_XOR_BLIT(IntArgb, ByteBinary4Bit)

DEFINE_BYTE_BINARY_ALPHA_MASKBLIT(ByteBinary4Bit, IntArgb, 4ByteArgb)

DEFINE_BYTE_BINARY_ALPHA_MASKBLIT(IntArgb, ByteBinary4Bit, 4ByteArgb)

DEFINE_BYTE_BINARY_ALPHA_MASKFILL(ByteBinary4Bit, 4ByteArgb)
