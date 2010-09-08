/*
 * @(#)DefaultCharMapper.h	1.7 05/08/24
 *
 * (C) Copyright IBM Corp. 1998-2005 - All Rights Reserved
 *
 */

#ifndef __DEFAULTCHARMAPPER_H
#define __DEFAULTCHARMAPPER_H

#include "LETypes.h"
#include "LEFontInstance.h"

/**
 * This class is an instance of LECharMapper which
 * implements control character filtering and bidi
 * mirroring.
 *
 * @see LECharMapper
 */
class DefaultCharMapper : public LECharMapper
{
private:
    le_bool fFilterControls;
    le_bool fMirror;
    le_bool fZWJ;

    static const LEUnicode32 controlChars[];

    static const le_int32 controlCharsCount;

    static const LEUnicode32 controlCharsZWJ[];

    static const le_int32 controlCharsZWJCount;

    static const LEUnicode32 mirroredChars[];
    static const LEUnicode32 srahCderorrim[];

    static const le_int32 mirroredCharsCount;

public:
    DefaultCharMapper(le_bool filterControls, le_bool mirror, le_bool zwj = 0)
        : fFilterControls(filterControls), fMirror(mirror), fZWJ(zwj)
    {
        // nothing
    };

    ~DefaultCharMapper()
    {
        // nada
    };

    LEUnicode32 mapChar(LEUnicode32 ch) const;
};

#endif
