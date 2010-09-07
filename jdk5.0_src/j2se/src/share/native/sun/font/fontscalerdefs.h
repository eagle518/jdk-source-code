/*
 * @(#)fontscalerdefs.h	1.5 03/30/04
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef FontScalerDefsIncludesDefined
#define FontScalerDefsIncludesDefined

#include "AccelGlyphCache.h"

#ifdef  __cplusplus
extern "C" {
#endif 


#define kPosInfinity16          (32767)
#define kNegInfinity16          (-32768)

#define kPosInfinity32          (0x7fffffff)
#define kNegInfinity32          (0x80000000)


#ifdef _LP64
typedef unsigned int            UInt32;
typedef int                     Int32;
#else
typedef unsigned long           UInt32;
typedef long                    Int32;
#endif
typedef unsigned short          UInt16;
typedef short                   Int16;
typedef unsigned char           UInt8;

typedef UInt8                   Byte;
typedef Int32                   hsFixed;
typedef Int32                   hsFract;
typedef UInt32                  Bool32;

#ifndef false
        #define false           0
#endif

#ifndef true
        #define true            1
#endif

#define kPosInfinity32		(0x7fffffff)
#define kNegInfinity32		(0x80000000)

#define F26Dot6ToFixed(n)  ((n) << 10)
#define F26Dot6ToScalar(n) (((t2kScalar)(n)) / (t2kScalar)64)

  /* t2kFixed is the same as F16Dot16 format although T2K also uses 26.6 */
typedef Int32 t2kFixed;
typedef float t2kScalar;

#define t2kIntToFixed(x) ((t2kFixed)(x) << 16)
#define t2kFixedToInt(x) ((x) >> 16)

#define t2kFixedRound(x) ((x) + 0x8000 >> 16)
#define t2kFixed1 t2kIntToFixed(1)

#define t2kFloatToFixed(f) (t2kFixed)((f) * (float)(t2kFixed1))
#define t2kFixedToFloat(x) ((x) / (float)(65536))

#define t2kScalarAverage(a, b) (((a) + (b)) / (t2kScalar)(2))

typedef struct T2KFixedPoint {
    t2kFixed x, y;
} T2KFixedPoint;

typedef struct T2KRect {
    t2kScalar left, top, right, bottom;
} T2KRect;

typedef struct GlyphInfo {  
    float        advanceX;
    float        advanceY;
    UInt16       width;
    UInt16       height;
    float        topLeftX;
    float        topLeftY;
    struct _CacheCellInfo *cellInfo;
    UInt8        *image;
} GlyphInfo;

  /* We use fffe and ffff as meaning invisible glyphs which have no
   * image, or advance and an empty outline.
   * Since there are no valid glyphs with this great a value (watch out for
   * large fonts in the future!) we can safely use check for >= this value
   */
#define INVISIBLE_GLYPHS 0xfffe

#ifdef  __cplusplus
}
#endif 

#endif
