/*
 * @(#)SegmentArrayProcessor.h	1.9 05/05/11
 *
 * (C) Copyright IBM Corp. 1998-2004 - All Rights Reserved
 *
 */

#ifndef __SEGMENTARRAYPROCESSOR_H
#define __SEGMENTARRAYPROCESSOR_H

#include "LETypes.h"
#include "MorphTables.h"
#include "SubtableProcessor.h"
#include "NonContextualGlyphSubst.h"
#include "NonContextualGlyphSubstProc.h"

class LEGlyphStorage;

class SegmentArrayProcessor : public NonContextualGlyphSubstitutionProcessor
{
public:
    virtual void process(LEGlyphStorage &glyphStorage);

    SegmentArrayProcessor(const MorphSubtableHeader *morphSubtableHeader);

    virtual ~SegmentArrayProcessor();

private:
    SegmentArrayProcessor();

protected:
    const SegmentArrayLookupTable *segmentArrayLookupTable;
};

#endif

