/*
 * @(#)AnyInt.c	1.10 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <string.h>

#include "AnyInt.h"

/*
 * This file declares, registers, and defines the various graphics
 * primitive loops to manipulate surfaces of type "AnyInt".
 *
 * See also LoopMacros.h
 */

RegisterFunc RegisterAnyInt;

DECLARE_SOLID_FILLRECT(AnyInt);
DECLARE_SOLID_FILLSPANS(AnyInt);
DECLARE_SOLID_PARALLELOGRAM(AnyInt);
DECLARE_SOLID_DRAWLINE(AnyInt);
DECLARE_XOR_FILLRECT(AnyInt);
DECLARE_XOR_FILLSPANS(AnyInt);
DECLARE_XOR_DRAWLINE(AnyInt);
DECLARE_SOLID_DRAWGLYPHLIST(AnyInt);
DECLARE_XOR_DRAWGLYPHLIST(AnyInt);

NativePrimitive AnyIntPrimitives[] = {
    REGISTER_SOLID_FILLRECT(AnyInt),
    REGISTER_SOLID_FILLSPANS(AnyInt),
    REGISTER_SOLID_PARALLELOGRAM(AnyInt),
    REGISTER_SOLID_LINE_PRIMITIVES(AnyInt),
    REGISTER_XOR_FILLRECT(AnyInt),
    REGISTER_XOR_FILLSPANS(AnyInt),
    REGISTER_XOR_LINE_PRIMITIVES(AnyInt),
    REGISTER_SOLID_DRAWGLYPHLIST(AnyInt),
    REGISTER_XOR_DRAWGLYPHLIST(AnyInt),
};

jboolean RegisterAnyInt(JNIEnv *env)
{
    return RegisterPrimitives(env, AnyIntPrimitives,
			      ArraySize(AnyIntPrimitives));
}

DEFINE_ISOCOPY_BLIT(AnyInt)

DEFINE_ISOXOR_BLIT(AnyInt)

DEFINE_ISOSCALE_BLIT(AnyInt)

DEFINE_SOLID_FILLRECT(AnyInt)

DEFINE_SOLID_FILLSPANS(AnyInt)

DEFINE_SOLID_PARALLELOGRAM(AnyInt)

DEFINE_SOLID_DRAWLINE(AnyInt)

DEFINE_XOR_FILLRECT(AnyInt)

DEFINE_XOR_FILLSPANS(AnyInt)

DEFINE_XOR_DRAWLINE(AnyInt)

DEFINE_SOLID_DRAWGLYPHLIST(AnyInt)

DEFINE_XOR_DRAWGLYPHLIST(AnyInt)
