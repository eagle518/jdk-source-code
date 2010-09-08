/*
 * @(#)MarkArrays.h	1.8 05/05/11
 *
 * (C) Copyright IBM Corp. 1998-2004 - All Rights Reserved
 *
 */

#ifndef __MARKARRAYS_H
#define __MARKARRAYS_H

#include "LETypes.h"
#include "LEFontInstance.h"
#include "OpenTypeTables.h"

struct MarkRecord
{
    le_uint16   markClass;
    Offset      markAnchorTableOffset;
};

struct MarkArray
{
    le_uint16   markCount;
    MarkRecord  markRecordArray[ANY_NUMBER];

    le_int32 getMarkClass(LEGlyphID glyphID, le_int32 coverageIndex, const LEFontInstance *fontInstance,
        LEPoint &anchor) const;
};

#endif


