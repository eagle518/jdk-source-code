/*
 * @(#)AlternateSubstSubtables.h	1.4 05/05/11
 *
 * (C) Copyright IBM Corp. 1998-2004 - All Rights Reserved
 *
 */

#ifndef __ALTERNATESUBSTITUTIONSUBTABLES_H
#define __ALTERNATESUBSTITUTIONSUBTABLES_H

#include "LETypes.h"
#include "LEGlyphFilter.h"
#include "OpenTypeTables.h"
#include "GlyphSubstitutionTables.h"
#include "GlyphIterator.h"

struct AlternateSetTable
{
    le_uint16 glyphCount;
    TTGlyphID alternateArray[ANY_NUMBER];
};

struct AlternateSubstitutionSubtable : GlyphSubstitutionSubtable
{
    le_uint16 alternateSetCount;
    Offset    alternateSetTableOffsetArray[ANY_NUMBER];

    le_uint32 process(GlyphIterator *glyphIterator, const LEGlyphFilter *filter = NULL) const;
};

#endif
