/*
 * @(#)HindiReordering.cpp	1.5 03/12/19 
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

#include "LETypes.h"
#include "OpenTypeTables.h"
#include "HindiReordering.h"

// Characters that get refered to by name...
enum
{
    C_LETTER_KA     = 0x0915,
    C_LETTER_KHA    = 0x0916,
    C_LETTER_GA     = 0x0917,
    C_LETTER_JA     = 0x091C,
    C_LETTER_DDA    = 0x0921,
    C_LETTER_DDHA   = 0x0922,
    C_LETTER_NA     = 0x0928,
    C_LETTER_NNNA   = 0x0929,
    C_LETTER_PHA    = 0x092B,
    C_LETTER_YA     = 0x092F,
    C_LETTER_RA     = 0x0930,
    C_LETTER_RRA    = 0x0931,
    C_LETTER_LLA    = 0x0933,
    C_LETTER_LLLA   = 0x0934,
    C_SIGN_NUKTA    = 0x093C,
    C_VOWEL_SIGN_I  = 0x093F,
    C_SIGN_VIRAMA   = 0x094D,
    C_LETTER_QA     = 0x0958,
    C_LETTER_KHHA   = 0x0959,
    C_LETTER_GHHA   = 0x095A,
    C_LETTER_ZA     = 0x095B,
    C_LETTER_DDDHA  = 0x095C,
    C_LETTER_RHA    = 0x095D,
    C_LETTER_FA     = 0x095E,
    C_LETTER_YYA    = 0x095F,
    C_SIGN_ZWNJ     = 0x200C,
    C_SIGN_ZWJ      = 0x200D
};

const LETag emptyTag       = 0x00000000; // ''
const LETag oopsFeatureTag = 0x6F6F7073; // 'oops'

const LETag nuktFeatureTag = 0x6E756B74; // 'nukt'
const LETag akhnFeatureTag = 0x616B686E; // 'akhn'
const LETag rphfFeatureTag = 0x72706866; // 'rphf'
const LETag blwfFeatureTag = 0x626C7766; // 'blwf'
const LETag halfFeatureTag = 0x68616C66; // 'half'
const LETag pstfFeatureTag = 0x70737466; // 'pstf'
const LETag vatuFeatureTag = 0x76617475; // 'vatu'
const LETag presFeatureTag = 0x70726573; // 'pres'
const LETag blwsFeatureTag = 0x626C7773; // 'blws'
const LETag abvsFeatureTag = 0x61627673; // 'abvs'
const LETag pstsFeatureTag = 0x70737473; // 'psts'
const LETag halnFeatureTag = 0x68616C6E; // 'haln'

const LETag blwmFeatureTag = 0x626C776D; // 'blwm'
const LETag abvmFeatureTag = 0x6162766D; // 'abvm'
const LETag distFeatureTag = 0x64697374; // 'dist'

const LETag tagArray[] =
{
    rphfFeatureTag, blwfFeatureTag, halfFeatureTag, nuktFeatureTag, akhnFeatureTag, pstfFeatureTag,
    vatuFeatureTag, presFeatureTag, blwsFeatureTag, abvsFeatureTag, pstsFeatureTag, halnFeatureTag,
    blwmFeatureTag, abvmFeatureTag, emptyTag
};

const LETag oopsTagArray[] = {oopsFeatureTag, emptyTag};

#if 1
enum
{
    _xx = HindiReordering::CT_RESERVED,
    _mm = HindiReordering::CT_MODIFYING_MARK,
    _iv = HindiReordering::CT_INDEPENDENT_VOWEL,
    _ct = HindiReordering::CT_CONSONANT,
    _cn = HindiReordering::CT_CONSONANT_WITH_NUKTA,
    _nu = HindiReordering::CT_NUKTA,
    _dv = HindiReordering::CT_DEPENDENT_VOWEL,
    _vr = HindiReordering::CT_VIRAMA
};

const HindiReordering::CharType charTypes[] =
{
    _xx, _mm, _mm, _mm, _xx, _iv, _iv, _iv, _iv, _iv, _iv, _iv, _iv, _iv, _iv, _iv, // 0900 - 090F
    _iv, _iv, _iv, _iv, _iv, _ct, _ct, _ct, _ct, _ct, _ct, _ct, _ct, _ct, _ct, _ct, // 0910 - 091F
    _ct, _ct, _ct, _ct, _ct, _ct, _ct, _ct, _ct, _cn, _ct, _ct, _ct, _ct, _ct, _ct, // 0920 - 092F
    _ct, _cn, _ct, _ct, _ct, _ct, _ct, _ct, _ct, _ct, _xx, _xx, _nu, _xx, _dv, _dv, // 0930 - 093F
    _dv, _dv, _dv, _dv, _dv, _dv, _dv, _dv, _dv, _dv, _dv, _dv, _dv, _vr, _xx, _xx, // 0940 - 094F
    _xx, _xx, _xx, _xx, _xx, _xx, _xx, _xx, _cn, _cn, _cn, _cn, _cn, _cn, _cn, _cn, // 0950 - 095F
    _iv, _iv, _dv, _dv, _xx, _xx, _xx, _xx, _xx, _xx, _xx, _xx, _xx, _xx, _xx, _xx, // 0960 - 096F
    _xx                                                                             // 0970
};

#else
enum
{
    _xx = HindiReordering::CT_RESERVED,
    _mm = HindiReordering::CT_MODIFYING_MARK,
    _iv = HindiReordering::CT_INDEPENDENT_VOWEL,
    _ct = HindiReordering::CT_CONSONANT,
    _cn = HindiReordering::CT_CONSONANT_WITH_NUKTA,
    _ra = HindiReordering::CT_LETTER_RA,
    _nu = HindiReordering::CT_NUKTA,
    _dv = HindiReordering::CT_DEPENDENT_VOWEL,
    _mi = HindiReordering::CT_VOWEL_SIGN_I,
    _vr = HindiReordering::CT_VIRAMA
};

const HindiReordering::CharType charTypes[] =
{
    _xx, _mm, _mm, _mm, _xx, _iv, _iv, _iv, _iv, _iv, _iv, _iv, _iv, _iv, _iv, _iv, // 0900 - 090F
    _iv, _iv, _iv, _iv, _iv, _ct, _ct, _ct, _ct, _ct, _ct, _ct, _ct, _ct, _ct, _ct, // 0910 - 091F
    _ct, _ct, _ct, _ct, _ct, _ct, _ct, _ct, _ct, _cn, _ct, _ct, _ct, _ct, _ct, _ct, // 0920 - 092F
    _ra, _cn, _ct, _ct, _ct, _ct, _ct, _ct, _ct, _ct, _xx, _xx, _nu, _av, _dv, _mi, // 0930 - 093F
    _dv, _dv, _dv, _dv, _dv, _dv, _dv, _dv, _dv, _dv, _dv, _dv, _dv, _vr, _xx, _xx, // 0940 - 094F
    _xx, _xx, _xx, _xx, _xx, _xx, _xx, _xx, _cn, _cn, _cn, _cn, _cn, _cn, _cn, _cn, // 0950 - 095F
    _iv, _iv, _dv, _dv, _xx, _xx, _xx, _xx, _xx, _xx, _xx, _xx, _xx, _xx, _xx, _xx, // 0960 - 096F
    _xx                                                                             // 0970
};
#endif

HindiReordering::CharType HindiReordering::getCharType(const LEUnicode ch)
{
    if (ch == C_SIGN_ZWJ) {
        return CT_CONSONANT;
    }

    if (ch < 0x0900 || ch > 0x0970) {
        return CT_RESERVED;
    }

    return charTypes[ch - 0x0900];
}

#if 1
const le_int8 stateTable[][HindiReordering::CT_COUNT] =
{
//   xx  mm  iv  ct  cn  nu  dv  vr
    { 1,  1,  5,  3,  2,  1,  1,  1},
    {-1, -1, -1, -1, -1, -1, -1, -1},
    {-1,  6, -1, -1, -1, -1,  5,  4},
    {-1,  6, -1, -1, -1,  2,  5,  4},
    {-1, -1, -1,  3,  2, -1, -1, -1},
    {-1,  6, -1, -1, -1, -1, -1, -1},
    {-1,  1, -1, -1, -1, -1, -1, -1}
};
#else
struct StateEntry
{
    le_int8 newState;
    le_int8 action;
};

enum
{
    na = 0, // no action
    rp = 1, // set reph flag, note base limit, tag previous 2 glyphs with 'repf'
    v1 = 2, // tag previous 2 glyphs with 'blwf'
    v2 = 3, // tag the 2 glyphs 2 back with 'blwf'
    ii = 4, // set the short_i flag
    md = 5, // note the start of the modifiers
    vi = 6, // same as v1 and ii
    nv = 7  // remove 'half' and 'blwf' flags from current glyph
};

const StateEntry stateTable[][HindiReodering::CT_COUNT] = 
{
//         xx        mm        iv        ct        cn        ra        nu        dv        mi        vr
/*00*/ {{12, na}, {12, na}, {11, na}, { 3, na}, { 8, na}, { 1, nv}, {12, na}, {12, na}, {12, na}, {12, na}},
/*01*/ {{-1, na}, {10, md}, {-1, na}, {-1, na}, {-1, na}, {-1, na}, { 8, na}, { 9, na}, { 9, ii}, { 2, na}},
/*02*/ {{-1, na}, {-1, na}, {-1, na}, { 3, rp}, { 3, rp}, { 3, rp}, {-1, na}, {-1, na}, {-1, na}, {-1, na}},
/*03*/ {{-1, na}, {10, md}, {-1, na}, {-1, na}, {-1, na}, {-1, na}, { 8, na}, { 9, na}, { 9, ii}, { 4, na}},
/*04*/ {{-1, na}, {-1, na}, {-1, na}, { 3, na}, { 3, na}, { 5, na}, {-1, na}, {-1, na}, {-1, na}, {-1, na}},
/*05*/ {{-1, v1}, {10, v1}, {-1, v1}, {-1, v1}, {-1, v1}, {-1, v1}, { 8, na}, { 9, v1}, { 9, vi}, { 6, na}},
/*06*/ {{-1, v2}, {-1, v2}, {-1, v2}, { 3, v2}, { 8, v2}, { 7, v1}, {-1, v2}, {-1, v2}, {-1, v2}, {-1, v2}},
/*07*/ {{-1, v1}, {10, v1}, {-1, v1}, {-1, v1}, {-1, v1}, {-1, v1}, { 8, v1}, { 9, v1}, { 9, vi}, { 4, v1}},
/*08*/ {{-1, na}, {10, md}, {-1, na}, {-1, na}, {-1, na}, {-1, na}, {-1, na}, { 9, na}, { 9, ii}, { 4, na}},
/*09*/ {{-1, na}, {10, md}, {-1, na}, {-1, na}, {-1, na}, {-1, na}, {-1, na}, {-1, na}, {-1, na}, {-1, na}},
/*10*/ {{-1, na}, {12, na}, {-1, na}, {-1, na}, {-1, na}, {-1, na}, {-1, na}, {-1, na}, {-1, na}, {-1, na}},
/*11*/ {{-1, na}, {10, md}, {-1, na}, {-1, na}, {-1, na}, {-1, na}, {-1, na}, {-1, na}, {-1, na}, {-1, na}},
/*12*/ {{-1, na}, {-1, na}, {-1, na}, {-1, na}, {-1, na}, {-1, na}, {-1, na}, {-1, na}, {-1, na}, {-1, na}},

};
#endif

LEUnicode HindiReordering::nuktalize(const LEUnicode theChar)
{
    switch (theChar) {
    case C_LETTER_KA:
        return C_LETTER_QA;

    case C_LETTER_KHA:
        return C_LETTER_KHHA;

    case C_LETTER_GA:
        return C_LETTER_GHHA;

    case C_LETTER_JA:
        return C_LETTER_ZA;

    case C_LETTER_DDA:
        return C_LETTER_DDDHA;

    case C_LETTER_DDHA:
        return C_LETTER_RHA;

    case C_LETTER_NA:
        return C_LETTER_NNNA;

    case C_LETTER_PHA:
        return C_LETTER_FA;

    case C_LETTER_YA:
        return C_LETTER_YYA;

    case C_LETTER_RA:
        return C_LETTER_RRA;

    case C_LETTER_LLA:
        return C_LETTER_LLLA;

    default:
        return 0xFFFF;
    }
}

#if 1
le_int32 HindiReordering::findSyllable(const LEUnicode *chars, le_int32 prev, le_int32 charCount, const LETag **charTags)
{
    le_int32 cursor = prev;
    le_int8 state = 0;

    while (cursor < charCount) {
        CharType charType = getCharType(chars[cursor]);

        charTags[cursor] = &tagArray[1];

        state = stateTable[state][charType];

        if (state < 0) {
            break;
        }

        cursor += 1;
    }

    return cursor;
}
#else
le_int32 HindiReordering::findSyllable(const LEUnicode *chars, le_int32 prev, le_int32 charCount, const LETag **charTags)
{
    le_int32 cursor = prev;
    le_int8 state = 0;

    while (cursor < charCount) {
        CharType charType = getCharType(chars[cursor]);
        StateEntry entry = stateTable[state][charType];
        le_int8 action = entry.action;

        charTags[cursor] = &tagArray[1];

        state = doAction(action, cursor, charTags);

        if (state < 0) {
            break;
        }

        cursor += 1;
    }

    return cursor;
}
#endif

void HindiReordering::reorder(const LEUnicode *chars, le_int32 charCount, le_uint32 *glyphs, le_uint32 *charIndices, const LETag **charTags)
{
    le_int32 prev = 0;

    while (prev < charCount) {
        le_int32 syllable = findSyllable(chars, prev, charCount, charTags);
        le_int32 modifiers = syllable;
        le_int16 flags = 0;

        if (syllable - prev == 1) {
            switch (getCharType(chars[prev])) {
            case CT_MODIFYING_MARK:
            case CT_NUKTA:
            case CT_DEPENDENT_VOWEL:
            case CT_VIRAMA:
                charTags[prev] = oopsTagArray;
                prev = syllable;
                continue;
                break;
            }
        }

        while (modifiers > prev && getCharType(chars[modifiers - 1]) == CT_MODIFYING_MARK) {
            modifiers -= 1;
        }

        le_uint32 length = modifiers - prev;
        le_int32 lastConsonant = modifiers - 1;
        le_int32 baseLimit = prev;

        // Check for REPH at front of syllable
        if (length > 2 && chars[prev] == C_LETTER_RA && chars[prev + 1] == C_SIGN_VIRAMA) {
            flags |= 1;
            baseLimit = prev + 2;
        }

        // Check for SHORT_I at end of syllable
        if (length > 1 && chars[modifiers - 1] == C_VOWEL_SIGN_I) {
            flags |= 2;
        }

        while (lastConsonant >= baseLimit && !isConsonant(chars[lastConsonant])) {
            lastConsonant -= 1;
        }

        le_int32 baseConsonant = lastConsonant;

        if (lastConsonant >= prev) {
            le_bool seenVattu = false;

            while (baseConsonant >= baseLimit) {
                LEUnicode ch = chars[baseConsonant];

                if (isConsonant(ch)) {
                    if (seenVattu || ch != C_LETTER_RA) {
                        break;
                    }

                    seenVattu = true;
                }

                baseConsonant -= 1;
            }

            // Move halant from base consonant to last consonant
            if (baseConsonant < baseLimit) {
               baseConsonant = baseLimit;
            } else if (baseConsonant != lastConsonant) {
                le_uint32 g1 = glyphs[baseConsonant + 1];
                le_uint32 i1 = charIndices[baseConsonant + 1];
                const LETag *t1 = charTags[baseConsonant + 1];
                le_int32 i;

                for (i = baseConsonant + 1; i < lastConsonant; i += 1) {
                    glyphs[i] = glyphs[i + 1];
                    charIndices[i] = charIndices[i + 1];
                    charTags[i] = charTags[i + 1];
                }

                glyphs[lastConsonant] = g1;
                charIndices[lastConsonant] = i1;
                charTags[lastConsonant] = t1;

                lastConsonant -= 1;
            }

            // Skip features which don't apply to the base consonant
            le_int32 bcSpan = baseConsonant + 1;

            charTags[baseConsonant] = &tagArray[3];

            // A following NUKTA is part of the base consonant
            if (bcSpan < modifiers && chars[bcSpan] == C_SIGN_NUKTA) {
                charTags[bcSpan] = &tagArray[3];
                bcSpan += 1;
            }

            // The VIRAMA's only part of the base consonant if there
            // aren't any post-base consonants.
            if (baseConsonant == lastConsonant && bcSpan < modifiers &&
                chars[bcSpan] == C_SIGN_VIRAMA) {
                charTags[bcSpan] = &tagArray[3];
            }

            // Deal with pre-base RA's...
            le_int32 i = baseLimit;
            le_bool supressVattu = true;
            
            while (i < baseConsonant) {
                LEUnicode ch = chars[i];

                if (isConsonant(ch)) {
                    if (ch == C_LETTER_RA && supressVattu) {
                        charTags[i] = &tagArray[3];
                    }

                    supressVattu = ch == C_LETTER_RA;
                }

                i += 1;
            }

            switch (flags) {
            case 0:
                break;

            // Move REPH to end of syllable and tag it
            case 1:
            {
                le_uint32 g1 = glyphs[prev], g2 = glyphs[prev + 1];
                le_uint32 i1 = charIndices[prev], i2 = charIndices[prev + 1];
                le_uint32 i;

                for (i = prev + 2; i < modifiers; i += 1)
                {
                    glyphs[i - 2] = glyphs[i];
                    charIndices[i - 2] = charIndices[i];
                    charTags[i - 2] = charTags[i];
                }

                glyphs[modifiers - 2] = g1;
                glyphs[modifiers - 1] = g2;

                charIndices[modifiers - 2] = i1;
                charIndices[modifiers - 1] = i2;
            
                // tag the REPH
                charTags[modifiers - 2] = &tagArray[0];
                charTags[modifiers - 1] = &tagArray[0];

                if (modifiers != syllable)
                {
                    charTags[modifiers] = &tagArray[0];
                }

                break;
            }
            
            // Move SHORT_I to front of syllable
            case 2:
            {
                le_uint32 g1 = glyphs[modifiers - 1];
                le_uint32 i1 = charIndices[modifiers - 1];
                const LETag *t1 = charTags[modifiers - 1];
                le_uint32 i;

                for (i = modifiers - 1; i > prev; i -= 1)
                {
                    glyphs[i] = glyphs[i - 1];
                    charIndices[i] = charIndices[i - 1];
                    charTags[i] = charTags[i - 1];
                }

                glyphs[prev] = g1;
                charIndices[prev] = i1;
                charTags[prev] = t1;

                break;
            }

            // Move REPH to end and SHORT_I to front
            case 3:
            {
                le_uint32 g1 = glyphs[prev], g2 = glyphs[prev + 1];
                le_uint32 i1 = charIndices[prev], i2 = charIndices[prev + 1];
                le_uint32 i;

                // move SHORT_I to front of syllable
                glyphs[prev] = glyphs[modifiers - 1];
                charIndices[prev] = charIndices[modifiers - 1];
                charTags[prev] = charTags[modifiers - 1];

                // slide everything else down one position
                for (i = prev + 2; i < modifiers - 1; i += 1)
                {
                    glyphs[i - 1] = glyphs[i];
                    charIndices[i - 1] = charIndices[i];
                    charTags[i - 1] = charTags[i];
                }

                // put the REPH at the end
                glyphs[modifiers - 2] = g1;
                glyphs[modifiers - 1] = g2;

                charIndices[modifiers - 2] = i1;
                charIndices[modifiers - 1] = i2;
            
                // tag the REPH
                charTags[modifiers - 2] = &tagArray[0];
                charTags[modifiers - 1] = &tagArray[0];

                if (modifiers != syllable)
                {
                    charTags[modifiers] = &tagArray[0];
                }

                break;
            }

            default:
                break;
            }
        }

        prev = syllable;
    }
}

