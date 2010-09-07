/*
 * @(#)SegmentArrayProcessor.h	1.8 01/10/09 
 *
 * (C) Copyright IBM Corp. 1998, 1999, 2000, 2001 - All Rights Reserved
 *
 */

#ifndef __SEGMENTARRAYPROCESSOR_H
#define __SEGMENTARRAYPROCESSOR_H

#include "LETypes.h"
#include "MorphTables.h"
#include "SubtableProcessor.h"
#include "NonContextualGlyphSubst.h"
#include "NonContextualGlyphSubstProc.h"

class SegmentArrayProcessor : public NonContextualGlyphSubstitutionProcessor
{
public:
    virtual void process(LEGlyphID *glyphs, le_int32 *charIndices, le_int32 glyph);

    SegmentArrayProcessor(const MorphSubtableHeader *morphSubtableHeader);

    virtual ~SegmentArrayProcessor();

private:
    SegmentArrayProcessor();

protected:
    const SegmentArrayLookupTable *segmentArrayLookupTable;
};

#endif

