/*
 * @(#)MPreFixups.h	1.1 03/08/01
 *
 * (C) Copyright IBM Corp. 2002-2003 - All Rights Reserved
 *
 */

#ifndef __MPREFIXUPS_H
#define __MPREFIXUPS_H

#include "LETypes.h"

// Might want to make this a private member...
struct FixupData;

class MPreFixups // : public UMemory
{
public:
    MPreFixups(le_int32 charCount);
   ~MPreFixups();

    void add(le_int32 baseIndex, le_int32 mpreIndex);
    
    void apply(LEGlyphID *glyphs, le_int32 *charIndicies);

private:
    FixupData *fFixupData;
    le_int32   fFixupCount;
};

#endif

