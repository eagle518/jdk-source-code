/*
 * @(#)ContextualGlyphSubstitution.h	1.6 05/05/11
 *
 * (C) Copyright IBM Corp. 1998-2004 - All Rights Reserved
 *
 */

#ifndef __CONTEXTUALGLYPHSUBSTITUTION_H
#define __CONTEXTUALGLYPHSUBSTITUTION_H

#include "LETypes.h"
#include "LayoutTables.h"
#include "StateTables.h"
#include "MorphTables.h"

struct ContextualGlyphSubstitutionHeader : MorphStateTableHeader
{
    ByteOffset  substitutionTableOffset;
};

enum ContextualGlyphSubstitutionFlags
{
    cgsSetMark      = 0x8000,
    cgsDontAdvance  = 0x4000,
    cgsReserved     = 0x3FFF
};

struct ContextualGlyphSubstitutionStateEntry : StateEntry
{
    WordOffset markOffset;
    WordOffset currOffset;
};

#endif
