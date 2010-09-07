
/*
 * @(#)FontInstanceAdapter.h	1.8 03/12/19
 *
 * (C) Copyright IBM Corp. 1998-2001 - All Rights Reserved
 *
 * Portions Copyright 2004 by Sun Microsystems, Inc.,
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

#ifndef __FONTINSTANCEADAPTER_H
#define __FONTINSTANCEADAPTER_H

#include "LETypes.h"
#include "LEFontInstance.h"
#include "jni.h"
#include "sunfontids.h"
#include <jni_util.h>
#include "sunt2kscaler.h"

class FontInstanceAdapter : public LEFontInstance {
private:
    JNIEnv *env;
    jobject font2D;
    jobject fontStrike;

    float xppem;
    float yppem;

    float xScaleUnitsToPoints;
    float yScaleUnitsToPoints;

    float xScalePixelsToUnits;
    float yScalePixelsToUnits;

    float upem;
    float xPointSize, yPointSize;
    float txMat[4];

    float euclidianDistance(float a, float b);

    TTLayoutTableCache* layoutTables;

public:
    FontInstanceAdapter(JNIEnv *env,
			jobject theFont2D, jobject theFontStrike,
			float *matrix, le_int32 xRes, le_int32 yRes);

    virtual ~FontInstanceAdapter() { };

    virtual const LEFontInstance *getSubFont(const LEUnicode chars[], le_int32 *offset, le_int32 limit, le_int32 script, LEErrorCode &success) const {
      return this;
    }

    // tables are cached with the native font scaler data
    // only supports gsub, gpos, gdef, mort tables at present
    virtual const void *getFontTable(LETag tableTag) const;

    virtual le_bool canDisplay(LEUnicode32 ch) const
    {
	return  (le_bool)env->CallBooleanMethod(font2D,
						sunFontIDs.canDisplayMID, ch);
    };

    virtual le_int32 getUnitsPerEM() const
    {
      T2KScalerInfo* scaler = NULL;
      if (env->IsInstanceOf(font2D, sunFontIDs.ttFontClass)) {
	  scaler = (T2KScalerInfo*)env->GetLongField(font2D,
						     sunFontIDs.pScaler);
      }
      return getUnitsPerEmForLayout(scaler);
    };

    virtual void mapCharsToGlyphs(const LEUnicode chars[], le_int32 offset, le_int32 count, le_bool reverse, const LECharMapper *mapper, LEGlyphID glyphs[]) const;

    virtual LEGlyphID mapCharToGlyph(LEUnicode32 ch, const LECharMapper *mapper) const;

    virtual LEGlyphID mapCharToGlyph(LEUnicode32 ch) const;

    virtual void mapCharsToWideGlyphs(const LEUnicode chars[], le_int32 offset, le_int32 count, le_bool reverse, const LECharMapper *mapper, le_uint32 glyphs[]) const;

    virtual le_uint32 mapCharToWideGlyph(LEUnicode32 ch, const LECharMapper *mapper) const;

    //    virtual le_int32 getName(le_uint16 platformID, le_uint16 scriptID, le_uint16 languageID, le_uint16 nameID, LEUnicode *name);

    virtual void getGlyphAdvance(LEGlyphID glyph, LEPoint &advance) const;

    virtual void getWideGlyphAdvance(le_uint32 glyph, LEPoint &advance) const;

    virtual le_bool getGlyphPoint(LEGlyphID glyph, le_int32 pointNumber, LEPoint &point) const;

    float getXPixelsPerEm() const
    {
        return xppem;
    };

    float getYPixelsPerEm() const
    {
        return yppem;
    };

    float xUnitsToPoints(float xUnits) const
    {
        return xUnits * xScaleUnitsToPoints;
    };

    float yUnitsToPoints(float yUnits) const
    {
        return yUnits * yScaleUnitsToPoints;
    };

    void unitsToPoints(LEPoint &units, LEPoint &points) const
    {
        points.fX = xUnitsToPoints(units.fX);
        points.fY = yUnitsToPoints(units.fY);
    }

    float xPixelsToUnits(float xPixels) const
    {
        return xPixels * xScalePixelsToUnits;
    };

    float yPixelsToUnits(float yPixels) const
    {
        return yPixels * yScalePixelsToUnits;
    };

    void pixelsToUnits(LEPoint &pixels, LEPoint &units) const
    {
        units.fX = xPixelsToUnits(pixels.fX);
        units.fY = yPixelsToUnits(pixels.fY);
    };

    virtual float getScaleFactorX() const {
        return xScalePixelsToUnits;
    };

    virtual float getScaleFactorY() const {
        return yScalePixelsToUnits;
    };

    void transformFunits(float xFunits, float yFunits, LEPoint &pixels) const;

    virtual le_int32 getAscent() const { return 0; };  // not used
    virtual le_int32 getDescent() const { return 0; }; // not used
    virtual le_int32 getLeading() const { return 0; }; // not used
};

#endif
