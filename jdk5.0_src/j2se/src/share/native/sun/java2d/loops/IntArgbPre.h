/*
 * @(#)IntArgbPre.h	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef IntArgbPre_h_Included
#define IntArgbPre_h_Included

#include "IntDcm.h"

/*
 * This file contains macro and type definitions used by the macros in
 * LoopMacros.h to manipulate a surface of type "IntArgbPre".
 */

typedef jint	IntArgbPrePixelType;
typedef jint	IntArgbPreDataType;

#define IntArgbPrePixelStride	4

#define DeclareIntArgbPreLoadVars(PREFIX)
#define DeclareIntArgbPreStoreVars(PREFIX)
#define InitIntArgbPreLoadVars(PREFIX, pRasInfo)
#define SetIntArgbPreStoreVarsYPos(PREFIX, pRasInfo, y)
#define SetIntArgbPreStoreVarsXPos(PREFIX, pRasInfo, x)
#define InitIntArgbPreStoreVarsY(PREFIX, pRasInfo)
#define InitIntArgbPreStoreVarsX(PREFIX, pRasInfo)
#define NextIntArgbPreStoreVarsX(PREFIX)
#define NextIntArgbPreStoreVarsY(PREFIX)


#define IntArgbPrePixelFromArgb(pixel, rgb, pRasInfo) \
    do { \
        if ((((rgb) >> 24) + 1) == 0) { \
	    (pixel) = (rgb); \
        } else { \
            jint a, r, g, b; \
            ExtractIntDcmComponents1234(rgb, a, r, g, b); \
            r = MUL8(a, r); \
            g = MUL8(a, g); \
            b = MUL8(a, b); \
            (pixel) = ComposeIntDcmComponents1234(a, r, g, b); \
        } \
    } while (0)    

#define StoreIntArgbPrePixel(pRas, x, pixel) \
    (pRas)[x] = (pixel)

#define DeclareIntArgbPrePixelData(PREFIX)

#define ExtractIntArgbPrePixelData(PIXEL, PREFIX)

#define StoreIntArgbPrePixelData(pPix, x, pixel, PREFIX) \
    (pPix)[x] = (pixel)


/* 
 * REMIND: we delegate to the ...To1IntArgb macro here, although it does
 *         slightly more work (may pack the alpha value into the RGB result)
 */
#define LoadIntArgbPreTo1IntRgb(pRas, PREFIX, x, rgb) \
    LoadIntArgbPreTo1IntArgb(pRas, PREFIX, x, rgb)

#define LoadIntArgbPreTo1IntArgb(pRas, PREFIX, x, argb) \
    do { \
        jint pixel = (pRas)[x]; \
        jint a = ((juint) pixel) >> 24; \
        if ((a == 0xff) || (a == 0)) { \
	    (argb) = pixel; \
        } else { \
            jint r, g, b; \
            ExtractIntDcmComponentsX123(pixel, r, g, b); \
            r = DIV8(r, a); \
            g = DIV8(g, a); \
            b = DIV8(b, a); \
            (argb) = ComposeIntDcmComponents1234(a, r, g, b); \
        } \
    } while (0)

#define LoadIntArgbPreTo3ByteRgb(pRas, PREFIX, x, r, g, b) \
    do { \
        jint a; \
        LoadIntArgbPreTo4ByteArgb(pRas, PREFIX, x, a, r, g, b); \
    } while (0)

#define LoadIntArgbPreTo4ByteArgb(pRas, PREFIX, x, a, r, g, b) \
    do { \
        jint pixel = (pRas)[x]; \
	ExtractIntDcmComponents1234(pixel, a, r, g, b); \
        if (((a) != 0xff) && ((a) != 0)) { \
            (r) = DIV8(r, a); \
            (g) = DIV8(g, a); \
            (b) = DIV8(b, a); \
        } \
    } while (0)

#define StoreIntArgbPreFrom1IntRgb(pRas, PREFIX, x, rgb) \
    (pRas)[x] = 0xff000000 | (rgb)

#define StoreIntArgbPreFrom1IntArgb(pRas, PREFIX, x, argb) \
    do { \
        if ((((argb) >> 24) + 1) == 0) { \
	    (pRas)[x] = (argb); \
        } else { \
            jint a, r, g, b; \
            ExtractIntDcmComponents1234(argb, a, r, g, b); \
            r = MUL8(a, r); \
            g = MUL8(a, g); \
            b = MUL8(a, b); \
            (pRas)[x] = ComposeIntDcmComponents1234(a, r, g, b); \
        } \
    } while (0)    

#define StoreIntArgbPreFrom3ByteRgb(pRas, PREFIX, x, r, g, b) \
    (pRas)[x] = ComposeIntDcmComponents1234(0xff, r, g, b)

#define StoreIntArgbPreFrom4ByteArgb(pRas, PREFIX, x, a, r, g, b) \
    do { \
        if ((a) != 0xff) { \
            (r) = MUL8(a, r); \
            (g) = MUL8(a, g); \
            (b) = MUL8(a, b); \
        } \
        (pRas)[x] = ComposeIntDcmComponents1234(a, r, g, b); \
    } while (0)


#define DeclareIntArgbPreAlphaLoadData(PREFIX) \
    jint PREFIX;

#define InitIntArgbPreAlphaLoadData(PREFIX, pRasInfo)

#define LoadAlphaFromIntArgbPreFor4ByteArgb(pRas, PREFIX, COMP_PREFIX) \
    do { \
	PREFIX = (pRas)[0]; \
	COMP_PREFIX ## A = ((juint) PREFIX) >> 24; \
    } while (0)

#define Postload4ByteArgbFromIntArgbPre(pRas, PREFIX, COMP_PREFIX) \
    do { \
	ExtractIntDcmComponentsX123(PREFIX, COMP_PREFIX ## R, \
                                    COMP_PREFIX ## G, COMP_PREFIX ## B); \
    } while (0)


#define IntArgbPreIsPremultiplied	1

#define DeclareIntArgbPreBlendFillVars(PREFIX)

#define ClearIntArgbPreBlendFillVars(PREFIX, argb) \
    argb = 0

#define InitIntArgbPreBlendFillVarsNonPre(PREFIX, argb, COMP_PREFIX)

#define InitIntArgbPreBlendFillVarsPre(PREFIX, argb, COMP_PREFIX) \
    argb = ComposeIntDcmComponents1234(COMP_PREFIX ## A, \
				       COMP_PREFIX ## R, \
				       COMP_PREFIX ## G, \
				       COMP_PREFIX ## B)

#define StoreIntArgbPreBlendFill(pRas, PREFIX, x, argb, COMP_PREFIX) \
    (pRas)[x] = (argb)
    
#define StoreIntArgbPreFrom4ByteArgbComps(pRas, PREFIX, x, COMP_PREFIX) \
    (pRas)[x] = ComposeIntDcmComponents1234(COMP_PREFIX ## A, \
                                            COMP_PREFIX ## R, \
                                            COMP_PREFIX ## G, \
                                            COMP_PREFIX ## B)

#endif /* IntArgbPre_h_Included */
