/*
 * @(#)GlyphFilter.h	1.3 01/10/09
 *
 * (C) Copyright IBM Corp. 1998, 1999, 2000 - All Rights Reserved
 *
 */

#ifndef __GLYPHFILTER__H
#define __GLYPHFILTER__H

#include "dtypes.h"
#include "OpenTypeTables.h"

class GlyphFilter
{
public:
    virtual Boolean filter(tt_uint32 glyph) = 0;
};

#endif
