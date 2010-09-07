/*
 * @(#)AlternateSubstSubtables.cpp	1.3 03/08/01 
 *
 * (C) Copyright IBM Corp. 1998-2003 - All Rights Reserved
 *
 */

#include "LETypes.h"
#include "LEGlyphFilter.h"
#include "OpenTypeTables.h"
#include "GlyphSubstitutionTables.h"
#include "AlternateSubstSubtables.h"
#include "GlyphIterator.h"
#include "LESwaps.h"

le_uint32 AlternateSubstitutionSubtable::process(GlyphIterator *glyphIterator, const LEGlyphFilter *filter) const
{
    // NOTE: For now, we'll just pick the first alternative...
    LEGlyphID glyph = glyphIterator->getCurrGlyphID();
    le_int32 coverageIndex = getGlyphCoverage(glyph);

    if (coverageIndex >= 0) {
        le_uint16 altSetCount = SWAPW(alternateSetCount);

        if (coverageIndex < altSetCount) {
            Offset alternateSetTableOffset = SWAPW(alternateSetTableOffsetArray[coverageIndex]);
            const AlternateSetTable *alternateSetTable =
                (const AlternateSetTable *) ((char *) this + alternateSetTableOffset);
            TTGlyphID alternate = SWAPW(alternateSetTable->alternateArray[0]);

            if (filter == NULL || filter->accept(LE_SET_GLYPH(glyph, alternate))) {
                glyphIterator->setCurrGlyphID(SWAPW(alternateSetTable->alternateArray[0]));
            }
            
            return 1;
        }

        // XXXX If we get here, the table's mal-formed...
    }

    return 0;
}
