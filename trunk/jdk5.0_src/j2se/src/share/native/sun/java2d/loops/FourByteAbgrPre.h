/*
 * @(#)FourByteAbgrPre.h	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef FourByteAbgrPre_h_Included
#define FourByteAbgrPre_h_Included

/*
 * This file contains macro and type definitions used by the macros in
 * LoopMacros.h to manipulate a surface of type "FourByteAbgrPre".
 */

typedef jint	FourByteAbgrPrePixelType;
typedef jubyte	FourByteAbgrPreDataType;

#define FourByteAbgrPrePixelStride		4

#define DeclareFourByteAbgrPreLoadVars(PREFIX)
#define DeclareFourByteAbgrPreStoreVars(PREFIX)
#define SetFourByteAbgrPreStoreVarsYPos(PREFIX, pRasInfo, y)
#define SetFourByteAbgrPreStoreVarsXPos(PREFIX, pRasInfo, x)
#define InitFourByteAbgrPreLoadVars(PREFIX, pRasInfo)
#define InitFourByteAbgrPreStoreVarsY(PREFIX, pRasInfo)
#define InitFourByteAbgrPreStoreVarsX(PREFIX, pRasInfo)
#define NextFourByteAbgrPreStoreVarsX(PREFIX)
#define NextFourByteAbgrPreStoreVarsY(PREFIX)


#define FourByteAbgrPrePixelFromArgb(pixel, rgb, pRasInfo) \
    do { \
        jint a, r, g, b; \
        if (((rgb) >> 24) == -1) { \
            (pixel) = (((rgb) << 8) | (((juint) (rgb)) >> 24)); \
        } else { \
            ExtractIntDcmComponents1234(rgb, a, r, g, b); \
            r = MUL8(a, r); \
            g = MUL8(a, g); \
            b = MUL8(a, b); \
            (pixel) = ComposeIntDcmComponents1234(r, g, b, a); \
        } \
    } while (0)

#define StoreFourByteAbgrPrePixel(pRas, x, pixel) \
    do { \
	(pRas)[4*(x)+0] = (jubyte) ((pixel) >> 0); \
	(pRas)[4*(x)+1] = (jubyte) ((pixel) >> 8); \
	(pRas)[4*(x)+2] = (jubyte) ((pixel) >> 16); \
	(pRas)[4*(x)+3] = (jubyte) ((pixel) >> 24); \
    } while (0)

#define DeclareFourByteAbgrPrePixelData(PREFIX) \
    jubyte PREFIX ## 0, PREFIX ## 1, PREFIX ## 2, PREFIX ## 3;

#define ExtractFourByteAbgrPrePixelData(PIXEL, PREFIX) \
    do { \
	PREFIX ## 0 = (jubyte) (PIXEL >> 0); \
	PREFIX ## 1 = (jubyte) (PIXEL >> 8); \
	PREFIX ## 2 = (jubyte) (PIXEL >> 16); \
	PREFIX ## 3 = (jubyte) (PIXEL >> 24); \
    } while (0)

#define StoreFourByteAbgrPrePixelData(pPix, x, pixel, PREFIX) \
    do { \
	pPix[4*(x)+0] = PREFIX ## 0; \
	pPix[4*(x)+1] = PREFIX ## 1; \
	pPix[4*(x)+2] = PREFIX ## 2; \
	pPix[4*(x)+3] = PREFIX ## 3; \
    } while (0)


#define LoadFourByteAbgrPreTo1IntRgb(pRas, PREFIX, x, rgb) \
    LoadFourByteAbgrPreTo1IntArgb(pRas, PREFIX, x, rgb)

#define LoadFourByteAbgrPreTo1IntArgb(pRas, PREFIX, x, argb) \
    do { \
        jint a = (pRas)[4*(x)+0]; \
        if ((a == 0xff) || (a == 0)) { \
            (argb) = (((pRas)[4*(x)+1] << 0) | \
	              ((pRas)[4*(x)+2] << 8) | \
	              ((pRas)[4*(x)+3] << 16) | \
	              (a << 24)); \
        } else { \
            jint r, g, b; \
            b = DIV8((pRas)[4*(x)+1], a); \
            g = DIV8((pRas)[4*(x)+2], a); \
            r = DIV8((pRas)[4*(x)+3], a); \
            (argb) = ComposeIntDcmComponents1234(a, r, g, b); \
        } \
    } while (0)

#define LoadFourByteAbgrPreTo3ByteRgb(pRas, PREFIX, x, r, g, b) \
    do { \
        jint a; \
        LoadFourByteAbgrPreTo4ByteArgb(pRas, PREFIX, x, a, r, g, b); \
    } while (0)

#define LoadFourByteAbgrPreTo4ByteArgb(pRas, PREFIX, x, a, r, g, b) \
    do { \
        (a) = (pRas)[4*(x)+0]; \
	(b) = (pRas)[4*(x)+1]; \
	(g) = (pRas)[4*(x)+2]; \
	(r) = (pRas)[4*(x)+3]; \
        if ((a != 0xff) && (a != 0)) { \
            r = DIV8(r, a); \
            g = DIV8(g, a); \
            b = DIV8(b, a); \
        } \
    } while (0)

#define StoreFourByteAbgrPreFrom1IntRgb(pRas, PREFIX, x, rgb) \
    do { \
        (pRas)[4*(x)+0] = (jubyte) 0xff; \
	(pRas)[4*(x)+1] = (jubyte) ((rgb) >> 0); \
	(pRas)[4*(x)+2] = (jubyte) ((rgb) >> 8); \
	(pRas)[4*(x)+3] = (jubyte) ((rgb) >> 16); \
    } while (0)

#define StoreFourByteAbgrPreFrom1IntArgb(pRas, PREFIX, x, argb) \
    do { \
        if ((((argb) >> 24) + 1) == 0) { \
            (pRas)[4*(x)+0] = (jubyte) ((argb) >> 24); \
	    (pRas)[4*(x)+1] = (jubyte) ((argb) >> 0); \
	    (pRas)[4*(x)+2] = (jubyte) ((argb) >> 8); \
	    (pRas)[4*(x)+3] = (jubyte) ((argb) >> 16); \
        } else { \
            jint a, r, g, b; \
            ExtractIntDcmComponents1234(argb, a, r, g, b); \
            (pRas)[4*(x)+0] = (jubyte) a; \
            (pRas)[4*(x)+1] = MUL8(a, b); \
            (pRas)[4*(x)+2] = MUL8(a, g); \
            (pRas)[4*(x)+3] = MUL8(a, r); \
        } \
    } while (0)

#define StoreFourByteAbgrPreFrom3ByteRgb(pRas, PREFIX, x, r, g, b) \
    do { \
        (pRas)[4*(x)+0] = (jubyte) 0xff; \
	(pRas)[4*(x)+1] = (jubyte) (b); \
	(pRas)[4*(x)+2] = (jubyte) (g); \
	(pRas)[4*(x)+3] = (jubyte) (r); \
    } while (0)    

#define StoreFourByteAbgrPreFrom4ByteArgb(pRas, PREFIX, x, a, r, g, b) \
    do { \
        if ((a) == 0xff) { \
            StoreFourByteAbgrPreFrom3ByteRgb(pRas, PREFIX, x, r, g, b); \
        } else { \
            (pRas)[4*(x)+0] = (jubyte) (a); \
            (pRas)[4*(x)+1] = MUL8(a, b); \
            (pRas)[4*(x)+2] = MUL8(a, g); \
            (pRas)[4*(x)+3] = MUL8(a, r); \
        } \
    } while (0)


#define DeclareFourByteAbgrPreAlphaLoadData(PREFIX)
#define InitFourByteAbgrPreAlphaLoadData(PREFIX, pRasInfo)

#define LoadAlphaFromFourByteAbgrPreFor4ByteArgb(pRas, PREFIX, COMP_PREFIX) \
    COMP_PREFIX ## A = (pRas)[0]

#define Postload4ByteArgbFromFourByteAbgrPre(pRas, PREFIX, COMP_PREFIX) \
    do { \
	COMP_PREFIX ## B = (pRas)[1]; \
	COMP_PREFIX ## G = (pRas)[2]; \
	COMP_PREFIX ## R = (pRas)[3]; \
    } while (0)


#define FourByteAbgrPreIsPremultiplied	1

#define DeclareFourByteAbgrPreBlendFillVars(PREFIX)

#define ClearFourByteAbgrPreBlendFillVars(PREFIX, argb)

#define InitFourByteAbgrPreBlendFillVarsNonPre(PREFIX, argb, COMP_PREFIX)

#define InitFourByteAbgrPreBlendFillVarsPre(PREFIX, argb, COMP_PREFIX)

#define StoreFourByteAbgrPreBlendFill(pRas, PREFIX, x, argb, COMP_PREFIX) \
    StoreFourByteAbgrPreFrom4ByteArgbComps(pRas, PREFIX, x, COMP_PREFIX)

#define StoreFourByteAbgrPreFrom4ByteArgbComps(pRas, PREFIX, x, COMP_PREFIX)\
    do { \
	(pRas)[4*(x)+0] = (jubyte) COMP_PREFIX ## A; \
	(pRas)[4*(x)+1] = (jubyte) COMP_PREFIX ## B; \
	(pRas)[4*(x)+2] = (jubyte) COMP_PREFIX ## G; \
	(pRas)[4*(x)+3] = (jubyte) COMP_PREFIX ## R; \
    } while (0)

#endif /* FourByteAbgrPre_h_Included */
