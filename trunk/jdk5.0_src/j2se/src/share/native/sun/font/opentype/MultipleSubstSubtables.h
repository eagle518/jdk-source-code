/*
 * @(#)MultipleSubstSubtables.h	1.3 03/08/01 
 *
 * (C) Copyright IBM Corp. 1998-2003 - All Rights Reserved
 *
 */

#ifndef __MULTIPLESUBSTITUTIONSUBTABLES_H
#define __MULTIPLESUBSTITUTIONSUBTABLES_H

#include "LETypes.h"
#include "LEGlyphFilter.h"
#include "OpenTypeTables.h"
#include "GlyphSubstitutionTables.h"
#include "GlyphIterator.h"

struct SequenceTable
{
    le_uint16 glyphCount;
    TTGlyphID substituteArray[ANY_NUMBER];
};

struct MultipleSubstitutionSubtable : GlyphSubstitutionSubtable
{
    le_uint16 sequenceCount;
    Offset    sequenceTableOffsetArray[ANY_NUMBER];

    le_uint32 process(GlyphIterator *glyphIterator, const LEGlyphFilter *filter = NULL) const;
};

#endif
