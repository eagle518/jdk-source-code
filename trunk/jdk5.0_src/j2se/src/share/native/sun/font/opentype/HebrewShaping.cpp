/*
 * @(#)HebrewShaping.cpp	1.3 03/08/01
 *
 * (C) Copyright IBM Corp. 1998-2003 - All Rights Reserved
 *
 */

#include "LETypes.h"
#include "OpenTypeTables.h"
#include "HebrewShaping.h"

const LETag ligaFeatureTag  = 0x6C696761; // 'liga'
const LETag emptyTag        = 0x00000000; // ''

const LETag hebrewTags[] =
{
    ligaFeatureTag, emptyTag
};

void HebrewShaping::shape(const LEUnicode * /*chars*/, le_int32 /*offset*/, le_int32 charCount, le_int32 /*charMax*/,
                          le_bool rightToLeft, const LETag **tags)
{

    le_int32 count, out = 0, dir = 1;

    if (rightToLeft) {
        out = charCount - 1;
        dir = -1;
    }

    for (count = 0; count < charCount; count += 1, out += dir) {
		tags[out] = hebrewTags;
	}
}
