/*
 * @(#)DefaultCharMapper.h	1.5 02/12/11
 *
 * (C) Copyright IBM Corp. 1998, 1999, 2000, 2001 - All Rights Reserved
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
