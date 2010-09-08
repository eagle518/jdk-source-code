/*
 * @(#)SegmentSingleProcessor.h	1.9 05/05/11
 *
 * (C) Copyright IBM Corp. 1998-2004 - All Rights Reserved
 *
 */

#ifndef __SEGMENTSINGLEPROCESSOR_H
#define __SEGMENTSINGLEPROCESSOR_H

#include "LETypes.h"
#include "MorphTables.h"
#include "SubtableProcessor.h"
#include "NonContextualGlyphSubst.h"
#include "NonContextualGlyphSubstProc.h"

class LEGlyphStorage;

class SegmentSingleProcessor : public NonContextualGlyphSubstitutionProcessor
{
public:
    virtual void process(LEGlyphStorage &glyphStorage);

    SegmentSingleProcessor(const MorphSubtableHeader *morphSubtableHeader);

    virtual ~SegmentSingleProcessor();

private:
    SegmentSingleProcessor();

protected:
    const SegmentSingleLookupTable *segmentSingleLookupTable;
};

#endif

