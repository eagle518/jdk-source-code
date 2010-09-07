/*
 * @(#)GlyphPositioningTables.cpp	1.12 03/10/24
 *
 * (C) Copyright IBM Corp. 1998-2003 - All Rights Reserved
 *
 */

#include "LETypes.h"
#include "LEFontInstance.h"
#include "OpenTypeTables.h"
#include "Lookups.h"
#include "GlyphDefinitionTables.h"
#include "GlyphPositionAdjustments.h"
#include "GlyphPositioningTables.h"
#include "GlyphPosnLookupProc.h"

void GlyphPositioningTableHeader::process(LEGlyphID *glyphs, GlyphPositionAdjustment *glyphPositionAdjustments,
                                          const LETag **glyphTags, le_int32 glyphCount, le_bool rightToLeft,
                                          LETag scriptTag, LETag languageTag,
                                          const GlyphDefinitionTableHeader *glyphDefinitionTableHeader,
                                          const LEFontInstance *fontInstance, const LETag *featureOrder) const
{
    GlyphPositioningLookupProcessor processor(this, scriptTag, languageTag, featureOrder);
    le_int32 *charIndices = NULL;

    processor.process(glyphs, glyphPositionAdjustments, glyphTags, charIndices, 
        glyphCount, rightToLeft, glyphDefinitionTableHeader, fontInstance);
}

