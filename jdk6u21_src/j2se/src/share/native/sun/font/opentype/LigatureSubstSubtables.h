/*
 * @(#)LigatureSubstSubtables.h	1.4 05/05/11
 *
 * (C) Copyright IBM Corp. 1998-2004 - All Rights Reserved
 *
 */

#ifndef __LIGATURESUBSTITUTIONSUBTABLES_H
#define __LIGATURESUBSTITUTIONSUBTABLES_H

#include "LETypes.h"
#include "LEGlyphFilter.h"
#include "OpenTypeTables.h"
#include "GlyphSubstitutionTables.h"
#include "GlyphIterator.h"

struct LigatureSetTable
{
    le_uint16 ligatureCount;
    Offset    ligatureTableOffsetArray[ANY_NUMBER];
};

struct LigatureTable
{
    TTGlyphID ligGlyph;
    le_uint16 compCount;
    TTGlyphID componentArray[ANY_NUMBER];
};

struct LigatureSubstitutionSubtable : GlyphSubstitutionSubtable
{
    le_uint16 ligSetCount;
    Offset    ligSetTableOffsetArray[ANY_NUMBER];

    le_uint32  process(GlyphIterator *glyphIterator, const LEGlyphFilter *filter = NULL) const;
};

#endif
