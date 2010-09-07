
/*
 * @(#)CompositeLayoutEngine.cpp	1.1 01/03/23
 *
 * (C) Copyright IBM Corp. 1998, 1999, 2000, 2001 - All Rights Reserved
 *
 */

#include "LETypes.h"
#include "LayoutEngine.h"
#include "CompositeLayoutEngine.h"

#include "FontInstanceAdapter.h"

#include "OpenTypeUtilities.h"
#include "DefaultCharMapper.h"

#define ARRAY_SIZE(array) (sizeof array  / sizeof array[0])

CompositeLayoutEngine::CompositeLayoutEngine(const FontInstanceAdapter *fontInstance, le_int32 scriptCode, le_int32 languageCode)
    : LayoutEngine(fontInstance, scriptCode, languageCode), fWideGlyphs(NULL), fFontInstanceAdapter(fontInstance)
{
    // nothing else to do?
}

// Copy the glyphs into caller's (32-bit) glyph array, OR in extraBits
void CompositeLayoutEngine::getGlyphs(le_uint32 glyphs[], le_uint32 extraBits, LEErrorCode &success) const
{
    le_int32 i;
    
	if (LE_FAILURE(success)) {
		return;
	}

	if (glyphs == NULL) {
		success = LE_ILLEGAL_ARGUMENT_ERROR;
		return;
	}

	if (fWideGlyphs == NULL) {
		success = LE_NO_LAYOUT_ERROR;
		return;
	}

    for (i = 0; i < fGlyphCount; i += 1) {
        glyphs[i] = fWideGlyphs[i] /* | extraBits */ ;
    }
};

le_int32 CompositeLayoutEngine::computeGlyphs(const LEUnicode chars[], le_int32 offset, le_int32 count, le_int32 max, le_bool rightToLeft,
                                            le_uint32 *&glyphs, le_int32 *&charIndices, LEErrorCode &success)
{
	if (LE_FAILURE(success)) {
		return 0;
	}

	if (chars == NULL || offset < 0 || count < 0 || max < 0 || offset >= max || offset + count > max) {
		success = LE_ILLEGAL_ARGUMENT_ERROR;
		return 0;
	}

    mapCharsToGlyphs(chars, offset, count, rightToLeft, rightToLeft, glyphs, charIndices, success);

    return count;
}

// Input: glyphs
// Output: positions
void CompositeLayoutEngine::positionGlyphs(const le_uint32 glyphs[], le_int32 glyphCount, float x, float y, float *&positions, LEErrorCode &success)
{
	if (LE_FAILURE(success)) {
		return;
	}

	if (glyphCount < 0) {
		success = LE_ILLEGAL_ARGUMENT_ERROR;
		return;
	}

    if (positions == NULL) {
        positions = new float[2 * (glyphCount + 1)];

		if (positions == NULL) {
			success = LE_MEMORY_ALLOCATION_ERROR;
			return;
		}
    }

    le_int32 i;

    for (i = 0; i < glyphCount; i += 1) {
        LEPoint advance;

        positions[i * 2] = x;
        positions[i * 2 + 1] = y;

        fFontInstanceAdapter->getWideGlyphAdvance(glyphs[i], advance);
        x += advance.fX;
        y += advance.fY;
    }

    positions[glyphCount * 2] = x;
    positions[glyphCount * 2 + 1] = y;
}

void CompositeLayoutEngine::mapCharsToGlyphs(const LEUnicode chars[], le_int32 offset, le_int32 count, le_bool reverse, le_bool mirror,
                                    le_uint32 *&glyphs, le_int32 *&charIndices, LEErrorCode &success)
{
	if (LE_FAILURE(success)) {
		return;
	}

	if (chars == NULL || offset < 0 || count < 0) {
		success = LE_ILLEGAL_ARGUMENT_ERROR;
		return;
	}

    if (glyphs == NULL) {
        glyphs = new le_uint32[count];

		if (glyphs == NULL) {
			success = LE_MEMORY_ALLOCATION_ERROR;
			return;
		}
    }

    if (charIndices == NULL) {
        le_int32 i, dir = 1, out = 0;

        if (reverse) {
            out = count - 1;
            dir = -1;
        }

        charIndices = new le_int32[count];

		if (charIndices == NULL) {
			success = LE_MEMORY_ALLOCATION_ERROR;
			return;
		}

        for (i = 0; i < count; i += 1, out += dir) {
            charIndices[out] = i;
        }
    }

    DefaultCharMapper charMapper(true, mirror);

    fFontInstanceAdapter->mapCharsToWideGlyphs(chars, offset, count, reverse, &charMapper, glyphs);
}

// Input: characters, font?
// Output: glyphs, positions, char indices
// Returns: number of glyphs
le_int32 CompositeLayoutEngine::layoutChars(const LEUnicode chars[], le_int32 offset, le_int32 count, le_int32 max, le_bool rightToLeft,
                              float x, float y, LEErrorCode &success)
{
	if (LE_FAILURE(success)) {
		return 0;
	}

	if (chars == NULL || offset < 0 || count < 0 || max < 0 || offset >= max || offset + count > max) {
		success = LE_ILLEGAL_ARGUMENT_ERROR;
		return 0;
	}

    fGlyphCount = computeGlyphs(chars, offset, count, max, rightToLeft, fWideGlyphs, fCharIndices, success);
    positionGlyphs(fWideGlyphs, fGlyphCount, x, y, fPositions, success);
    //adjustGlyphPositions(chars, offset, count, rightToLeft, fGlyphs, fGlyphCount, fPositions, success);

    return fGlyphCount;
}

void CompositeLayoutEngine::reset()
{
	LayoutEngine::reset();

	if (fWideGlyphs != NULL) {
		delete[] fWideGlyphs;
		fWideGlyphs = NULL;
	}
}
    
CompositeLayoutEngine::~CompositeLayoutEngine() {
    reset();
}

