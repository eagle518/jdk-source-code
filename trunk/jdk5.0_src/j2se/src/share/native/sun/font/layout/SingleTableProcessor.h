/*
 * @(#)SingleTableProcessor.h	1.8 01/10/09 
 *
 * (C) Copyright IBM Corp. 1998, 1999, 2000, 2001 - All Rights Reserved
 *
 */

#ifndef __SINGLETABLEPROCESSOR_H
#define __SINGLETABLEPROCESSOR_H

#include "LETypes.h"
#include "MorphTables.h"
#include "SubtableProcessor.h"
#include "NonContextualGlyphSubst.h"
#include "NonContextualGlyphSubstProc.h"

class SingleTableProcessor : public NonContextualGlyphSubstitutionProcessor
{
public:
    virtual void process(LEGlyphID *glyphs, le_int32 *charIndices, le_int32 glyphCount);

    SingleTableProcessor(const MorphSubtableHeader *morphSubtableHeader);

    virtual ~SingleTableProcessor();

private:
    SingleTableProcessor();

protected:
    const SingleTableLookupTable *singleTableLookupTable;
};

#endif
