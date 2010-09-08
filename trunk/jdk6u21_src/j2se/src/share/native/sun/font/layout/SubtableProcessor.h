/*
 * @(#)SubtableProcessor.h	1.10 05/05/11
 *
 * (C) Copyright IBM Corp. 1998-2004 - All Rights Reserved
 *
 */

#ifndef __SUBTABLEPROCESSOR_H
#define __SUBTABLEPROCESSOR_H

#include "LETypes.h"
#include "MorphTables.h"

class LEGlyphStorage;

class SubtableProcessor
{
public:
    virtual void process(LEGlyphStorage &glyphStorage) = 0;
    virtual ~SubtableProcessor();

protected:
    SubtableProcessor(const MorphSubtableHeader *morphSubtableHeader);

    SubtableProcessor();

    le_int16 length;
    SubtableCoverage coverage;
    FeatureFlags subtableFeatures;

    const MorphSubtableHeader *subtableHeader;

private:

    SubtableProcessor(const SubtableProcessor &other); // forbid copying of this class
    SubtableProcessor &operator=(const SubtableProcessor &other); // forbid copying of this class
};

#endif

