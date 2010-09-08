/*
 * @(#)MPreFixups.h	1.2 05/05/11
 *
 * (C) Copyright IBM Corp. 2002-2004 - All Rights Reserved
 *
 */

#ifndef __MPREFIXUPS_H
#define __MPREFIXUPS_H

#include "LETypes.h"

class LEGlyphStorage;

// Might want to make this a private member...
struct FixupData;

class MPreFixups {
public:
    MPreFixups(le_int32 charCount);
   ~MPreFixups();

    void add(le_int32 baseIndex, le_int32 mpreIndex);
    
    void apply(LEGlyphStorage &glyphStorage);

private:
    FixupData *fFixupData;
    le_int32   fFixupCount;
};

#endif

