/*
 * @(#)SimpleArrayProcessor.h	1.10 05/05/11
 *
 * (C) Copyright IBM Corp. 1998-2004 - All Rights Reserved
 *
 */

#ifndef __SIMPLEARRAYPROCESSOR_H
#define __SIMPLEARRAYPROCESSOR_H

#include "LETypes.h"
#include "MorphTables.h"
#include "SubtableProcessor.h"
#include "NonContextualGlyphSubst.h"
#include "NonContextualGlyphSubstProc.h"

class LEGlyphStorage;

class SimpleArrayProcessor : public NonContextualGlyphSubstitutionProcessor
{
public:
    virtual void process(LEGlyphStorage &glyphStorage);

    SimpleArrayProcessor(const MorphSubtableHeader *morphSubtableHeader);

    virtual ~SimpleArrayProcessor();

private:
    SimpleArrayProcessor();

protected:
    const SimpleArrayLookupTable *simpleArrayLookupTable;
};

#endif

