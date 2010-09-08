/*
 * @(#)SingleTableProcessor.h	1.9 05/05/11
 *
 * (C) Copyright IBM Corp. 1998-2004 - All Rights Reserved
 *
 */

#ifndef __SINGLETABLEPROCESSOR_H
#define __SINGLETABLEPROCESSOR_H

#include "LETypes.h"
#include "MorphTables.h"
#include "SubtableProcessor.h"
#include "NonContextualGlyphSubst.h"
#include "NonContextualGlyphSubstProc.h"

class LEGlyphStorage;

class SingleTableProcessor : public NonContextualGlyphSubstitutionProcessor
{
public:
    virtual void process(LEGlyphStorage &glyphStorage);

    SingleTableProcessor(const MorphSubtableHeader *morphSubtableHeader);

    virtual ~SingleTableProcessor();

private:
    SingleTableProcessor();

protected:
    const SingleTableLookupTable *singleTableLookupTable;
};

#endif
