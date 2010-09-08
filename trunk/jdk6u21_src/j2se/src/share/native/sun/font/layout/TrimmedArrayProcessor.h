/*
 * @(#)TrimmedArrayProcessor.h	1.9 05/05/11
 *
 * (C) Copyright IBM Corp. 1998-2004 - All Rights Reserved
 *
 */

#ifndef __TRIMMEDARRAYPROCESSOR_H
#define __TRIMMEDARRAYPROCESSOR_H

#include "LETypes.h"
#include "MorphTables.h"
#include "SubtableProcessor.h"
#include "NonContextualGlyphSubst.h"
#include "NonContextualGlyphSubstProc.h"

class LEGlyphStorage;

class TrimmedArrayProcessor : public NonContextualGlyphSubstitutionProcessor
{
public:
    virtual void process(LEGlyphStorage &glyphStorage);

    TrimmedArrayProcessor(const MorphSubtableHeader *morphSubtableHeader);

    virtual ~TrimmedArrayProcessor();

private:
    TrimmedArrayProcessor();

protected:
    TTGlyphID firstGlyph;
    TTGlyphID lastGlyph;
    const TrimmedArrayLookupTable *trimmedArrayLookupTable;
};

#endif

