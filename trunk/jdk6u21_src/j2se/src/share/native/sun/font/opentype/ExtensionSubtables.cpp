/*
 * @(#)ExtensionSubtables.cpp	1.2 05/05/11
 *
 * (C) Copyright IBM Corp. 2003 - All Rights Reserved
 *
 */

#include "LETypes.h"
#include "OpenTypeTables.h"
#include "GlyphSubstitutionTables.h"
#include "LookupProcessor.h"
#include "ExtensionSubtables.h"
#include "GlyphIterator.h"
#include "LESwaps.h"

// FIXME: should look at the format too... maybe have a sub-class for it?
le_uint32 ExtensionSubtable::process(const LookupProcessor *lookupProcessor, le_uint16 lookupType,
                                      GlyphIterator *glyphIterator, const LEFontInstance *fontInstance) const
{
    le_uint16 elt = SWAPW(extensionLookupType);

    if (elt != lookupType) {
        le_uint32 extOffset = SWAPL(extensionOffset);
        LookupSubtable *subtable = (LookupSubtable *) ((char *) this + extOffset);

        return lookupProcessor->applySubtable(subtable, elt, glyphIterator, fontInstance);
    }

    return 0;
}

