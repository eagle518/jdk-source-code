/*
 * @(#)SingleTableProcessor.cpp	1.10 05/05/11
 *
 * (C) Copyright IBM Corp. 1998-2004 - All Rights Reserved
 *
 */

#include "LETypes.h"
#include "MorphTables.h"
#include "SubtableProcessor.h"
#include "NonContextualGlyphSubst.h"
#include "NonContextualGlyphSubstProc.h"
#include "SingleTableProcessor.h"
#include "LEGlyphStorage.h"
#include "LESwaps.h"

SingleTableProcessor::SingleTableProcessor()
{
}

SingleTableProcessor::SingleTableProcessor(const MorphSubtableHeader *moprhSubtableHeader)
  : NonContextualGlyphSubstitutionProcessor(moprhSubtableHeader)
{
    const NonContextualGlyphSubstitutionHeader *header = (const NonContextualGlyphSubstitutionHeader *) moprhSubtableHeader;

    singleTableLookupTable = (const SingleTableLookupTable *) &header->table;
}

SingleTableProcessor::~SingleTableProcessor()
{
}

void SingleTableProcessor::process(LEGlyphStorage &glyphStorage)
{
    const LookupSingle *entries = singleTableLookupTable->entries;
    le_int32 glyph;
    le_int32 glyphCount = glyphStorage.getGlyphCount();

    for (glyph = 0; glyph < glyphCount; glyph += 1) {
        const LookupSingle *lookupSingle = singleTableLookupTable->lookupSingle(entries, glyphStorage[glyph]);

        if (lookupSingle != NULL) {
            glyphStorage[glyph] = SWAPW(lookupSingle->value);
        }
    }
} 
