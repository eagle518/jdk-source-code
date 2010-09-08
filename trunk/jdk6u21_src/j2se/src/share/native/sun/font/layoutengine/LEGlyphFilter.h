/*
 * @(#)LEGlyphFilter.h	1.9 05/05/11
 *
 * (C) Copyright IBM Corp. 1998-2004 - All Rights Reserved
 *
 */

#ifndef __LEGLYPHFILTER__H
#define __LEGLYPHFILTER__H

#include "LETypes.h"

/**
 * This is a helper class that is used to
 * recognize a set of glyph indices.
 *
 * @internal
 */
class LEGlyphFilter
{
public:
    /**
     * Destructor.
     * @internal
     */
    virtual ~LEGlyphFilter();

    /**
     * This method is used to test a particular
     * glyph index to see if it is in the set
     * recognized by the filter.
     *
     * @param glyph - the glyph index to be tested
     *
     * @return TRUE if the glyph index is in the set.
     *
     * @internal
     */
    virtual le_bool accept(LEGlyphID glyph) const = 0;
};

#endif
