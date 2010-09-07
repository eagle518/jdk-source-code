/*
 * @(#)GlyphPositioningTables.h	1.12 03/08/01
 *
 * (C) Copyright IBM Corp. 1998-2003 - All Rights Reserved
 *
 */

#ifndef __GLYPHPOSITIONINGTABLES_H
#define __GLYPHPOSITIONINGTABLES_H

#include "LETypes.h"
#include "LEFontInstance.h"
#include "OpenTypeTables.h"
#include "Lookups.h"
#include "GlyphDefinitionTables.h"
#include "GlyphPositionAdjustments.h"

struct GlyphPositioningTableHeader
{
    fixed32 version;
    Offset  scriptListOffset;
    Offset  featureListOffset;
    Offset  lookupListOffset;

    void    process(LEGlyphID *glyphs, GlyphPositionAdjustment *glyphPositionAdjustments,
                const LETag **glyphTags, le_int32 glyphCount,
                le_bool rightToLeft, LETag scriptTag, LETag languageTag,
                const GlyphDefinitionTableHeader *glyphDefinitionTableHeader,
                const LEFontInstance *fontInstance, const LETag *featureOrder) const;
};

enum GlyphPositioningSubtableTypes
{
    gpstSingle          = 1,
    gpstPair            = 2,
    gpstCursive         = 3,
    gpstMarkToBase      = 4,
    gpstMarkToLigature  = 5,
    gpstMarkToMark      = 6,
    gpstContext         = 7,
    gpstChainedContext  = 8,
    gpstExtension       = 9
};

typedef LookupSubtable GlyphPositioningSubtable;

#endif
