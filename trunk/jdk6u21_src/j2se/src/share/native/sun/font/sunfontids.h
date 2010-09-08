/*
 * @(#)sunfontids.h	1.8 03/23/10
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef SunFontIDIncludesDefined
#define SunFontIDIncludesDefined

#include "jni.h"

#ifdef  __cplusplus
extern "C" {
#endif 

typedef struct FontManagerNativeIDs {

    /* java/awt/Font methods & fields */
    jmethodID getFont2DMID;
    jfieldID font2DHandle;
    jfieldID createdFont;

    /* sun/font/Font2D methods */
    jmethodID getMapperMID;
    jmethodID getTableBytesMID;
    jmethodID canDisplayMID;
    jmethodID f2dCharToGlyphMID;

    /* sun/font/CharToGlyphMapper methods */
    jmethodID charToGlyphMID;

    /* sun/font/PhysicalStrike methods */
    jmethodID getGlyphMetricsMID;
    jmethodID getGlyphPointMID;
    jmethodID adjustPointMID;
    jfieldID  pScalerContextFID;

    /* java/awt/geom/Rectangle2D.Float */
    jclass rect2DFloatClass;
    jmethodID rect2DFloatCtr;
    jmethodID rect2DFloatCtr4;
    jfieldID rectF2DX, rectF2DY, rectF2DWidth, rectF2DHeight;

    /* java/awt/geom/Point2D.Float */
    jclass pt2DFloatClass;
    jmethodID pt2DFloatCtr;
    jfieldID xFID, yFID;

    /* java/awt/geom/GeneralPath */
    jclass gpClass;
    jmethodID gpCtr;
  
    /* sun/font/StrikeMetrics */
    jclass strikeMetricsClass;
    jmethodID strikeMetricsCtr;

    /* sun/font/TrueTypeFont */
    jclass ttFontClass;
    jmethodID ttReadBlockMID;
    jmethodID ttReadBytesMID;

    /* sun/font/Type1Font */
    jmethodID t1ReadBlockMID;

    /* sun/font/GlyphList */
    jfieldID glyphListX, glyphListY, glyphListLen,
      glyphImages, glyphListUsePos, glyphListPos, lcdRGBOrder, lcdSubPixPos;

    /* sun/font/FileFont */
    jfieldID pScaler;
} FontManagerNativeIDs;

extern FontManagerNativeIDs sunFontIDs;

#ifdef  __cplusplus
}
#endif 

#endif
