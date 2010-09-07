/*
 * @(#)HindiReordering.h	1.5 03/12/19 
 *
 * (C) Copyright IBM Corp. 1998 - All Rights Reserved
 *
 * Portions Copyright 2004 by Sun Microsystems, Inc.,
 * 901 San Antonio Road, Palo Alto, California, 94303, U.S.A.
 * All rights reserved.
 *
 * This software is the confidential and proprietary information
 * of Sun Microsystems, Inc. ("Confidential Information").  You
 * shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Sun.
 *
 * The original version of this source code and documentation is
 * copyrighted and owned by IBM. These materials are provided
 * under terms of a License Agreement between IBM and Sun.
 * This technology is protected by multiple US and International
 * patents. This notice and attribution to IBM may not be removed.
 */

#ifndef __HINDIREORDERING_H
#define __HINDIREORDERING_H

#include "LETypes.h"
#include "OpenTypeTables.h"

class HindiReordering
{
public:

    enum CharTypeValues
    {
        CT_RESERVED             = 0,
        CT_MODIFYING_MARK       = 1,
        CT_INDEPENDENT_VOWEL    = 2,
        CT_CONSONANT            = 3,
        CT_CONSONANT_WITH_NUKTA = 4,
        CT_NUKTA                = 5,
        CT_DEPENDENT_VOWEL      = 6,
        CT_VIRAMA               = 7,
        CT_COUNT                = 8
    };

    typedef le_int32 CharType;

    static void reorder(const LEUnicode *theChars, le_uint32 charCount, le_uint32 *glyphs, le_uint32 *charIndices, const LETag **charTags);
    static LEUnicode nuktalize(const LEUnicode theChar);

private:
    static CharType getCharType(const LEUnicode ch);
    static le_bool isConsonant(const LEUnicode ch);
    static le_int32 findSyllable(const LEUnicode *chars, le_int32 prev, le_int32 charCount, const LETag **charTags);

};

inline le_bool HindiReordering::isConsonant(const LEUnicode ch)
{
    CharType charType = getCharType(ch);

    return charType == CT_CONSONANT || charType == CT_CONSONANT_WITH_NUKTA;
}


#endif
