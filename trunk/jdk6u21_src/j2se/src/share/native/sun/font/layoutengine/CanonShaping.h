/*
 * @(#)CanonShaping.h	1.2 05/05/11
 *
 * (C) Copyright IBM Corp. 1998-2005 - All Rights Reserved
 *
 */

#ifndef __CANONSHAPING_H
#define __CANONSHAPING_H

#include "LETypes.h"

class LEGlyphStorage;

class CanonShaping
{
public:
    static const le_uint8 glyphSubstitutionTable[];
    static const le_uint8 glyphDefinitionTable[];

    static void reorderMarks(const LEUnicode *inChars, le_int32 charCount, 
        le_bool rightToLeft, LEUnicode *outChars, LEGlyphStorage &glyphStorage);

private:
    static void sortMarks(le_int32 *indices, const le_int32 *combiningClasses, 
        le_int32 index, le_int32 limit);
};

#endif
