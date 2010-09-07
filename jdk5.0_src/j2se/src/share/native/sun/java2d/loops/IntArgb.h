/*
 * @(#)IntArgb.h	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef IntArgb_h_Included
#define IntArgb_h_Included

#include "IntDcm.h"
#include "ByteGray.h"
#include "UshortGray.h"

/*
 * This file contains macro and type definitions used by the macros in
 * LoopMacros.h to manipulate a surface of type "IntArgb".
 */

typedef jint	IntArgbPixelType;
typedef jint	IntArgbDataType;

#define IntArgbPixelStride	4

#define DeclareIntArgbLoadVars(PREFIX)
#define DeclareIntArgbStoreVars(PREFIX)
#define InitIntArgbLoadVars(PREFIX, pRasInfo)
#define SetIntArgbStoreVarsYPos(PREFIX, pRasInfo, y)
#define SetIntArgbStoreVarsXPos(PREFIX, pRasInfo, x)
#define InitIntArgbStoreVarsY(PREFIX, pRasInfo)
#define InitIntArgbStoreVarsX(PREFIX, pRasInfo)
#define NextIntArgbStoreVarsX(PREFIX)
#define NextIntArgbStoreVarsY(PREFIX)
#define DeclareIntArgbInitialLoadVars(pRas, PREFIX, x)
#define InitialLoadIntArgb(pRas, PREFIX)
#define ShiftBitsIntArgb(PREFIX)
#define FinalStoreIntArgb(pRas, PREFIX)

#define IntArgbPixelFromArgb(pixel, rgb, pRasInfo) \
    (pixel) = (rgb)

#define StoreIntArgbPixel(pRas, x, pixel) \
    (pRas)[x] = (pixel)

#define DeclareIntArgbPixelData(PREFIX)

#define ExtractIntArgbPixelData(PIXEL, PREFIX)

#define StoreIntArgbPixelData(pPix, x, pixel, PREFIX) \
    (pPix)[x] = (pixel)


#define LoadIntArgbTo1IntRgb(pRas, PREFIX, x, rgb) \
    (rgb) = (pRas)[x]

#define LoadIntArgbTo1IntArgb(pRas, PREFIX, x, argb) \
    (argb) = (pRas)[x]

#define LoadIntArgbTo3ByteRgb(pRas, PREFIX, x, r, g, b) \
    do { \
	jint pixel = (pRas)[x]; \
	ExtractIntDcmComponentsX123(pixel, r, g, b); \
    } while (0)

#define LoadIntArgbTo4ByteArgb(pRas, PREFIX, x, a, r, g, b) \
    do { \
	jint pixel = (pRas)[x]; \
	ExtractIntDcmComponents1234(pixel, a, r, g, b); \
    } while (0)

#define StoreIntArgbFrom1IntRgb(pRas, PREFIX, x, rgb) \
    (pRas)[x] = 0xff000000 | (rgb)

#define StoreIntArgbFrom1IntArgb(pRas, PREFIX, x, argb) \
    (pRas)[x] = (argb)

#define StoreIntArgbFrom3ByteRgb(pRas, PREFIX, x, r, g, b) \
    StoreIntArgbFrom4ByteArgb(pRas, PREFIX, x, 0xff, r, g, b)

#define StoreIntArgbFrom4ByteArgb(pRas, PREFIX, x, a, r, g, b) \
    (pRas)[x] = ComposeIntDcmComponents1234(a, r, g, b)


#define DeclareIntArgbAlphaLoadData(PREFIX) \
    jint PREFIX;

#define InitIntArgbAlphaLoadData(PREFIX, pRasInfo)

#define LoadAlphaFromIntArgbFor4ByteArgb(pRas, PREFIX, COMP_PREFIX) \
    do { \
	PREFIX = (pRas)[0]; \
	COMP_PREFIX ## A = ((juint) PREFIX) >> 24; \
    } while (0)

#define LoadAlphaFromIntArgbFor1ByteGray(pRas, PREFIX, COMP_PREFIX) \
    LoadAlphaFromIntArgbFor4ByteArgb(pRas, PREFIX, COMP_PREFIX)

#define LoadAlphaFromIntArgbFor1ShortGray(pRas, PREFIX, COMP_PREFIX) \
    do { \
        LoadAlphaFromIntArgbFor4ByteArgb(pRas, PREFIX, COMP_PREFIX); \
        COMP_PREFIX ## A = (COMP_PREFIX ## A << 8) + COMP_PREFIX ## A; \
    } while (0)

#define Postload4ByteArgbFromIntArgb(pRas, PREFIX, COMP_PREFIX) \
    do { \
	COMP_PREFIX ## R = (PREFIX >> 16) & 0xff; \
	COMP_PREFIX ## G = (PREFIX >>  8) & 0xff; \
	COMP_PREFIX ## B = (PREFIX >>  0) & 0xff; \
    } while (0)

#define Postload1ByteGrayFromIntArgb(pRas, PREFIX, COMP_PREFIX) \
    do { \
        int r, g, b; \
        ExtractIntDcmComponentsX123(PREFIX, r, g, b); \
        COMP_PREFIX ## G = ComposeByteGrayFrom3ByteRgb(r, g, b); \
    } while (0)
    
#define Postload1ShortGrayFromIntArgb(pRas, PREFIX, COMP_PREFIX) \
    do { \
        int r, g, b; \
        ExtractIntDcmComponentsX123(PREFIX, r, g, b); \
        COMP_PREFIX ## G = ComposeUshortGrayFrom3ByteRgb(r, g, b); \
    } while (0)


#define IntArgbIsPremultiplied	0

#define DeclareIntArgbBlendFillVars(PREFIX)

#define ClearIntArgbBlendFillVars(PREFIX, argb) \
    argb = 0

#define InitIntArgbBlendFillVarsNonPre(PREFIX, argb, COMP_PREFIX) \
    argb = (COMP_PREFIX ## A << 24) | (argb & 0x00ffffff); \

#define InitIntArgbBlendFillVarsPre(PREFIX, argb, COMP_PREFIX)

#define StoreIntArgbBlendFill(pRas, PREFIX, x, argb, COMP_PREFIX) \
    (pRas)[x] = (argb)

#define StoreIntArgbFrom4ByteArgbComps(pRas, PREFIX, x, COMP_PREFIX) \
    StoreIntArgbFrom4ByteArgb(pRas, PREFIX, x, \
                              COMP_PREFIX ## A, COMP_PREFIX ## R, \
                              COMP_PREFIX ## G, COMP_PREFIX ## B)


/* 
 * Extract ## STRATEGY ## CompsAndAlphaFromArgb(pixel, COMP_PREFIX) 
 */
#define Extract3ByteRgbCompsAndAlphaFromArgb(pixel, COMP_PREFIX) \
    ExtractIntDcmComponents1234(pixel, COMP_PREFIX ## A, COMP_PREFIX ## R, \
                                COMP_PREFIX ## G, COMP_PREFIX ## B)

#define Extract4ByteArgbCompsAndAlphaFromArgb(pixel, COMP_PREFIX) \
    Extract3ByteRgbCompsAndAlphaFromArgb(pixel, COMP_PREFIX)

#define Extract1ByteGrayCompsAndAlphaFromArgb(pixel, COMP_PREFIX) \
    do { \
	int r, g, b; \
	ExtractIntDcmComponents1234(pixel, COMP_PREFIX ## A, r, g, b); \
        COMP_PREFIX ## G = ComposeByteGrayFrom3ByteRgb(r, g, b); \
    } while (0)

#define Extract1ShortGrayCompsAndAlphaFromArgb(pixel, COMP_PREFIX) \
    do { \
        int r, g, b; \
        ExtractIntDcmComponents1234(pixel, COMP_PREFIX ## A, r, g, b); \
        COMP_PREFIX ## G = ComposeUshortGrayFrom3ByteRgb(r, g, b); \
        COMP_PREFIX ## A = (COMP_PREFIX ## A << 8) + COMP_PREFIX ## A; \
    } while (0)

#endif /* IntArgb_h_Included */
