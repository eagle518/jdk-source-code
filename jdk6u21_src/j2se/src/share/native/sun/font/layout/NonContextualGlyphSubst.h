/*
 * @(#)NonContextualGlyphSubst.h	1.3 05/05/11
 *
 * (C) Copyright IBM Corp. 1998-2003 - All Rights Reserved
 *
 */

#ifndef __NONCONTEXTUALGLYPHSUBSTITUTION_H
#define __NONCONTEXTUALGLYPHSUBSTITUTION_H

#include "LETypes.h"
#include "LayoutTables.h"
#include "LookupTables.h"
#include "MorphTables.h"

struct NonContextualGlyphSubstitutionHeader : MorphSubtableHeader
{
    LookupTable table;
};

#endif

