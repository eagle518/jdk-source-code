/*
 * @(#)Any4Byte.c	1.13 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <string.h>

#include "Any4Byte.h"

/*
 * This file declares, registers, and defines the various graphics
 * primitive loops to manipulate surfaces of type "Any4Byte".
 *
 * See also LoopMacros.h
 */

RegisterFunc RegisterAny4Byte;

DECLARE_SOLID_FILLRECT(Any4Byte);
DECLARE_SOLID_FILLSPANS(Any4Byte);
DECLARE_SOLID_PARALLELOGRAM(Any4Byte);
DECLARE_SOLID_DRAWLINE(Any4Byte);
DECLARE_XOR_FILLRECT(Any4Byte);
DECLARE_XOR_FILLSPANS(Any4Byte);
DECLARE_XOR_DRAWLINE(Any4Byte);
DECLARE_SOLID_DRAWGLYPHLIST(Any4Byte);
DECLARE_XOR_DRAWGLYPHLIST(Any4Byte);

NativePrimitive Any4BytePrimitives[] = {
    REGISTER_SOLID_FILLRECT(Any4Byte),
    REGISTER_SOLID_FILLSPANS(Any4Byte),
    REGISTER_SOLID_PARALLELOGRAM(Any4Byte),
    REGISTER_SOLID_LINE_PRIMITIVES(Any4Byte),
    REGISTER_XOR_FILLRECT(Any4Byte),
    REGISTER_XOR_FILLSPANS(Any4Byte),
    REGISTER_XOR_LINE_PRIMITIVES(Any4Byte),
    REGISTER_SOLID_DRAWGLYPHLIST(Any4Byte),
    REGISTER_XOR_DRAWGLYPHLIST(Any4Byte),
};

jboolean RegisterAny4Byte(JNIEnv *env)
{
    return RegisterPrimitives(env, Any4BytePrimitives,
			      ArraySize(Any4BytePrimitives));
}

DEFINE_ISOCOPY_BLIT(Any4Byte)

DEFINE_ISOSCALE_BLIT(Any4Byte)

DEFINE_ISOXOR_BLIT(Any4Byte)

DEFINE_SOLID_FILLRECT(Any4Byte)

DEFINE_SOLID_FILLSPANS(Any4Byte)

DEFINE_SOLID_PARALLELOGRAM(Any4Byte)

DEFINE_SOLID_DRAWLINE(Any4Byte)

DEFINE_XOR_FILLRECT(Any4Byte)

DEFINE_XOR_FILLSPANS(Any4Byte)

DEFINE_XOR_DRAWLINE(Any4Byte)

DEFINE_SOLID_DRAWGLYPHLIST(Any4Byte)

DEFINE_XOR_DRAWGLYPHLIST(Any4Byte)
