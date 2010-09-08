/*
 * @(#)GlyphSubstitutionTables.h	1.17 05/08/24
 *
 * (C) Copyright IBM Corp. 1998-2005 - All Rights Reserved
 *
 */

#ifndef __GLYPHSUBSTITUTIONTABLES_H
#define __GLYPHSUBSTITUTIONTABLES_H

#include "LETypes.h"
#include "OpenTypeTables.h"
#include "Lookups.h"
#include "GlyphLookupTables.h"

class  LEGlyphStorage;
class  LEGlyphFilter;
struct GlyphDefinitionTableHeader;

struct GlyphSubstitutionTableHeader : public GlyphLookupTableHeader
{
    le_int32 process(LEGlyphStorage &glyphStorage,
        le_bool rightToLeft, LETag scriptTag, LETag languageTag,
        const GlyphDefinitionTableHeader *glyphDefinitionTableHeader,
        const LEGlyphFilter *filter, const FeatureMap *featureMap, 
        le_int32 featureMapCount, le_bool featureOrder) const;
};

enum GlyphSubstitutionSubtableTypes
{
    gsstSingle          = 1,
    gsstMultiple        = 2,
    gsstAlternate       = 3,
    gsstLigature        = 4,
    gsstContext         = 5,
    gsstChainingContext = 6,
    gsstExtension       = 7,
    gsstReverseChaining = 8
};

typedef LookupSubtable GlyphSubstitutionSubtable;

#endif
