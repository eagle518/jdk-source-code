/*
 * @(#)OpenTypeTables.h	1.8 03/08/01
 *
 * (C) Copyright IBM Corp. 1998-2003 - All Rights Reserved
 *
 */

#ifndef __OPENTYPETABLES_H
#define __OPENTYPETABLES_H

#include "LETypes.h"

#define ANY_NUMBER 1

typedef le_uint16 Offset;
typedef le_uint8  ATag[4];
typedef le_uint32 fixed32;

#define SWAPT(atag) ((LETag) ((atag[0] << 24) + (atag[1] << 16) + (atag[2] << 8) + atag[3]))

struct TagAndOffsetRecord
{
    ATag    tag;
    Offset  offset;
};

struct GlyphRangeRecord
{
    TTGlyphID firstGlyph;
    TTGlyphID lastGlyph;
    le_int16  rangeValue;
};

#endif
