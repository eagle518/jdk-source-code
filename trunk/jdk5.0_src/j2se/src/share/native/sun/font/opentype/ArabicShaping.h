/*
 * @(#)ArabicShaping.h	1.9 03/08/01
 *
 * (C) Copyright IBM Corp. 1998-2003 - All Rights Reserved
 *
 */

#ifndef __ARABICSHAPING_H
#define __ARABICSHAPING_H

#include "LETypes.h"
#include "OpenTypeTables.h"

class Shaper {
public:
    virtual inline ~Shaper() {};
    virtual void init(LEUnicode ch, le_int32 outIndex, le_bool isloate) = 0;
    virtual void shape(le_int32 outIndex, le_int32 shapeOffset) = 0;
};

class ArabicShaping {
public:
    // shaping bit masks
    enum ShapingBitMasks
    {
        MASK_SHAPE_RIGHT    = 1, // if this bit set, shapes to right
        MASK_SHAPE_LEFT     = 2, // if this bit set, shapes to left
        MASK_TRANSPARENT    = 4, // if this bit set, is transparent (ignore other bits)
        MASK_NOSHAPE        = 8  // if this bit set, don't shape this char, i.e. tatweel
    };

    // shaping values
    enum ShapeTypeValues
    {
        ST_NONE         = 0,
        ST_RIGHT        = MASK_SHAPE_RIGHT,
        ST_LEFT         = MASK_SHAPE_LEFT,
        ST_DUAL         = MASK_SHAPE_RIGHT | MASK_SHAPE_LEFT,
        ST_TRANSPARENT  = MASK_TRANSPARENT,
        ST_NOSHAPE_DUAL = MASK_NOSHAPE | ST_DUAL,
        ST_NOSHAPE_NONE = MASK_NOSHAPE
    };

    typedef le_int32 ShapeType;

    static void shape(const LEUnicode *chars, le_int32 offset, le_int32 charCount, le_int32 charMax,
                      le_bool rightToLeft, Shaper &shaper);

    static const LETag *getFeatureOrder();

    static const le_uint8 glyphSubstitutionTable[];
  //static le_uint8 ligatureSubstitutionSubtable[];
    static const le_uint8 glyphDefinitionTable[];

private:
    // forbid instantiation
    ArabicShaping();

    static ShapeType getShapeType(LEUnicode c);

    static const ShapeType shapeTypes[];
};

class GlyphShaper : public Shaper
{
public:
    virtual void init(LEUnicode ch, le_int32 outIndex, le_bool isolate);
    virtual void shape(le_int32 outIndex, le_int32 shapeOffset);

    GlyphShaper(const LETag **outputTags);
    virtual ~GlyphShaper();

private:
    const LETag **charTags;

    static const LETag tagArray[];

    GlyphShaper(const GlyphShaper &other); // forbid copying of this class
    GlyphShaper &operator=(const GlyphShaper &other); // forbid copying of this class
};

class CharShaper : public Shaper
{
public:
    virtual void init(LEUnicode ch, le_int32 outIndex, le_bool isolate);
    virtual void shape(le_int32 outIndex, le_int32 shapeOffset);

    CharShaper(LEUnicode *outputChars);
    virtual ~CharShaper();

private:
    LEUnicode *chars;
    
    static const LEUnicode isolateShapes[];

    static LEUnicode getToIsolateShape(LEUnicode ch);

    CharShaper(const CharShaper &other); // forbid copying of this class
    CharShaper &operator=(const CharShaper &other); // forbid copying of this class
};


#endif
