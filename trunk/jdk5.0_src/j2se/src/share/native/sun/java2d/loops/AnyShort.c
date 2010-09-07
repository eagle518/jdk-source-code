/*
 * @(#)AnyShort.c	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <string.h>

#include "AnyShort.h"

/*
 * This file declares, registers, and defines the various graphics
 * primitive loops to manipulate surfaces of type "AnyShort".
 *
 * See also LoopMacros.h
 */

RegisterFunc RegisterAnyShort;

DECLARE_SOLID_FILLRECT(AnyShort);
DECLARE_SOLID_FILLSPANS(AnyShort);
DECLARE_SOLID_DRAWLINE(AnyShort);
DECLARE_XOR_FILLRECT(AnyShort);
DECLARE_XOR_FILLSPANS(AnyShort);
DECLARE_XOR_DRAWLINE(AnyShort);
DECLARE_SOLID_DRAWGLYPHLIST(AnyShort);
DECLARE_XOR_DRAWGLYPHLIST(AnyShort);

NativePrimitive AnyShortPrimitives[] = {
    REGISTER_SOLID_FILLRECT(AnyShort),
    REGISTER_SOLID_FILLSPANS(AnyShort),
    REGISTER_SOLID_LINE_PRIMITIVES(AnyShort),
    REGISTER_XOR_FILLRECT(AnyShort),
    REGISTER_XOR_FILLSPANS(AnyShort),
    REGISTER_XOR_LINE_PRIMITIVES(AnyShort),
    REGISTER_SOLID_DRAWGLYPHLIST(AnyShort),
    REGISTER_XOR_DRAWGLYPHLIST(AnyShort),
};

jboolean RegisterAnyShort(JNIEnv *env)
{
    return RegisterPrimitives(env, AnyShortPrimitives,
			      ArraySize(AnyShortPrimitives));
}

DEFINE_ISOCOPY_BLIT(AnyShort)

DEFINE_ISOSCALE_BLIT(AnyShort)

DEFINE_ISOXOR_BLIT(AnyShort)

DEFINE_SOLID_FILLRECT(AnyShort)

DEFINE_SOLID_FILLSPANS(AnyShort)

DEFINE_SOLID_DRAWLINE(AnyShort)

DEFINE_XOR_FILLRECT(AnyShort)

DEFINE_XOR_FILLSPANS(AnyShort)

DEFINE_XOR_DRAWLINE(AnyShort)

DEFINE_SOLID_DRAWGLYPHLIST(AnyShort)

DEFINE_XOR_DRAWGLYPHLIST(AnyShort)
