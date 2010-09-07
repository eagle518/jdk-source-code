/*
 * @(#)Any4Byte.c	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <string.h>

#include "Any4Byte.h"

#include "AlphaMath.h"
#include "IntDcm.h"

/*
 * This file declares, registers, and defines the various graphics
 * primitive loops to manipulate surfaces of type "Any4Byte".
 *
 * See also LoopMacros.h
 */

RegisterFunc RegisterAny4Byte;

DECLARE_SOLID_FILLRECT(Any4Byte);
DECLARE_SOLID_FILLSPANS(Any4Byte);
DECLARE_SOLID_DRAWLINE(Any4Byte);
DECLARE_XOR_FILLRECT(Any4Byte);
DECLARE_XOR_FILLSPANS(Any4Byte);
DECLARE_XOR_DRAWLINE(Any4Byte);
DECLARE_SOLID_DRAWGLYPHLIST(Any4Byte);
DECLARE_XOR_DRAWGLYPHLIST(Any4Byte);

NativePrimitive Any4BytePrimitives[] = {
    REGISTER_SOLID_FILLRECT(Any4Byte),
    REGISTER_SOLID_FILLSPANS(Any4Byte),
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

DEFINE_SOLID_DRAWLINE(Any4Byte)

DEFINE_XOR_FILLRECT(Any4Byte)

DEFINE_XOR_FILLSPANS(Any4Byte)

DEFINE_XOR_DRAWLINE(Any4Byte)

DEFINE_SOLID_DRAWGLYPHLIST(Any4Byte)

DEFINE_XOR_DRAWGLYPHLIST(Any4Byte)
