/*
 * @(#)AnyByte.c	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <string.h>

#include "AnyByte.h"

/*
 * This file declares, registers, and defines the various graphics
 * primitive loops to manipulate surfaces of type "AnyByte".
 *
 * See also LoopMacros.h
 */

RegisterFunc RegisterAnyByte;

DECLARE_SOLID_FILLRECT(AnyByte);
DECLARE_SOLID_FILLSPANS(AnyByte);
DECLARE_SOLID_DRAWLINE(AnyByte);
DECLARE_XOR_FILLRECT(AnyByte);
DECLARE_XOR_FILLSPANS(AnyByte);
DECLARE_XOR_DRAWLINE(AnyByte);
DECLARE_SOLID_DRAWGLYPHLIST(AnyByte);
DECLARE_XOR_DRAWGLYPHLIST(AnyByte);

NativePrimitive AnyBytePrimitives[] = {
    REGISTER_SOLID_FILLRECT(AnyByte),
    REGISTER_SOLID_FILLSPANS(AnyByte),
    REGISTER_SOLID_LINE_PRIMITIVES(AnyByte),
    REGISTER_XOR_FILLRECT(AnyByte),
    REGISTER_XOR_FILLSPANS(AnyByte),
    REGISTER_XOR_LINE_PRIMITIVES(AnyByte),
    REGISTER_SOLID_DRAWGLYPHLIST(AnyByte),
    REGISTER_XOR_DRAWGLYPHLIST(AnyByte),
};

jboolean RegisterAnyByte(JNIEnv *env)
{
    return RegisterPrimitives(env, AnyBytePrimitives,
			      ArraySize(AnyBytePrimitives));
}

DEFINE_ISOCOPY_BLIT(AnyByte)

DEFINE_ISOSCALE_BLIT(AnyByte)

DEFINE_ISOXOR_BLIT(AnyByte)

DEFINE_SOLID_FILLRECT(AnyByte)

DEFINE_SOLID_FILLSPANS(AnyByte)

DEFINE_SOLID_DRAWLINE(AnyByte)

DEFINE_XOR_FILLRECT(AnyByte)

DEFINE_XOR_FILLSPANS(AnyByte)

DEFINE_XOR_DRAWLINE(AnyByte)

DEFINE_SOLID_DRAWGLYPHLIST(AnyByte)

DEFINE_XOR_DRAWGLYPHLIST(AnyByte)
