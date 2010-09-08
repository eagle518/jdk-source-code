/*
 * @(#)GlyphLookupTables.h	1.1 05/05/11
 *
 * (C) Copyright IBM Corp. 1998-2004 - All Rights Reserved
 *
 */

#ifndef __GLYPHLOOKUPTABLES_H
#define __GLYPHLOOKUPTABLES_H

#include "LETypes.h"
#include "OpenTypeTables.h"

struct GlyphLookupTableHeader
{
    fixed32 version;
    Offset  scriptListOffset;
    Offset  featureListOffset;
    Offset  lookupListOffset;

    le_bool coversScript(LETag scriptTag) const;
    le_bool coversScriptAndLanguage(LETag scriptTag, LETag languageTag, le_bool exactMatch = FALSE) const;
};

#endif

