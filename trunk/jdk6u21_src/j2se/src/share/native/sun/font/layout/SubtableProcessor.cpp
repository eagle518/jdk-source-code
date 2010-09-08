/*
 * @(#)SubtableProcessor.cpp	1.8 05/05/11
 *
 * (C) Copyright IBM Corp. 1998-2004 - All Rights Reserved
 *
 */

#include "LETypes.h"
#include "MorphTables.h"
#include "SubtableProcessor.h"
#include "LESwaps.h"

SubtableProcessor::SubtableProcessor()
{
}

SubtableProcessor::SubtableProcessor(const MorphSubtableHeader *morphSubtableHeader)
{
    subtableHeader = morphSubtableHeader;

    length = SWAPW(subtableHeader->length);
    coverage = SWAPW(subtableHeader->coverage);
    subtableFeatures = SWAPL(subtableHeader->subtableFeatures);
}

SubtableProcessor::~SubtableProcessor()
{
}

