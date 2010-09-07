/*
 * @(#)FourByteAbgr.h	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef FourByteAbgr_h_Included
#define FourByteAbgr_h_Included

/*
 * This file contains macro and type definitions used by the macros in
 * LoopMacros.h to manipulate a surface of type "FourByteAbgr".
 */

typedef jint	FourByteAbgrPixelType;
typedef jubyte	FourByteAbgrDataType;

#define FourByteAbgrPixelStride		4

#define DeclareFourByteAbgrLoadVars(PREFIX)
#define DeclareFourByteAbgrStoreVars(PREFIX)
#define SetFourByteAbgrStoreVarsYPos(PREFIX, pRasInfo, y)
#define SetFourByteAbgrStoreVarsXPos(PREFIX, pRasInfo, x)
#define InitFourByteAbgrLoadVars(PREFIX, pRasInfo)
#define InitFourByteAbgrStoreVarsY(PREFIX, pRasInfo)
#define InitFourByteAbgrStoreVarsX(PREFIX, pRasInfo)
#define NextFourByteAbgrStoreVarsX(PREFIX)
#define NextFourByteAbgrStoreVarsY(PREFIX)


#define FourByteAbgrPixelFromArgb(pixel, rgb, pRasInfo) \
    (pixel) = (((rgb) << 8) | (((juint) (rgb)) >> 24))

#define StoreFourByteAbgrPixel(pRas, x, pixel) \
    do { \
	(pRas)[4*(x)+0] = (jubyte) ((pixel) >> 0); \
	(pRas)[4*(x)+1] = (jubyte) ((pixel) >> 8); \
	(pRas)[4*(x)+2] = (jubyte) ((pixel) >> 16); \
	(pRas)[4*(x)+3] = (jubyte) ((pixel) >> 24); \
    } while (0)

#define DeclareFourByteAbgrPixelData(PREFIX) \
    jubyte PREFIX ## 0, PREFIX ## 1, PREFIX ## 2, PREFIX ## 3;

#define ExtractFourByteAbgrPixelData(PIXEL, PREFIX) \
    do { \
	PREFIX ## 0 = (jubyte) (PIXEL >> 0); \
	PREFIX ## 1 = (jubyte) (PIXEL >> 8); \
	PREFIX ## 2 = (jubyte) (PIXEL >> 16); \
	PREFIX ## 3 = (jubyte) (PIXEL >> 24); \
    } while (0)

#define StoreFourByteAbgrPixelData(pPix, x, pixel, PREFIX) \
    do { \
	pPix[4*x+0] = PREFIX ## 0; \
	pPix[4*x+1] = PREFIX ## 1; \
	pPix[4*x+2] = PREFIX ## 2; \
	pPix[4*x+3] = PREFIX ## 3; \
    } while (0)


#define LoadFourByteAbgrTo1IntRgb(pRas, PREFIX, x, rgb) \
    (rgb) = (((pRas)[4*(x)+1] << 0) | \
	     ((pRas)[4*(x)+2] << 8) | \
	     ((pRas)[4*(x)+3] << 16))

#define LoadFourByteAbgrTo1IntArgb(pRas, PREFIX, x, argb) \
    (argb) = (((pRas)[4*(x)+0] << 24) | \
	      ((pRas)[4*(x)+1] << 0) | \
	      ((pRas)[4*(x)+2] << 8) | \
	      ((pRas)[4*(x)+3] << 16))

#define LoadFourByteAbgrTo3ByteRgb(pRas, PREFIX, x, r, g, b) \
    do { \
	(b) = (pRas)[4*(x)+1]; \
	(g) = (pRas)[4*(x)+2]; \
	(r) = (pRas)[4*(x)+3]; \
    } while (0)

#define LoadFourByteAbgrTo4ByteArgb(pRas, PREFIX, x, a, r, g, b) \
    do { \
	(a) = (pRas)[4*(x)+0]; \
	LoadFourByteAbgrTo3ByteRgb(pRas, PREFIX, x, r, g, b); \
    } while (0)

#define StoreFourByteAbgrFrom1IntRgb(pRas, PREFIX, x, rgb) \
    do { \
        (pRas)[4*(x)+0] = (jubyte) 0xff; \
	(pRas)[4*(x)+1] = (jubyte) ((rgb) >> 0); \
	(pRas)[4*(x)+2] = (jubyte) ((rgb) >> 8); \
	(pRas)[4*(x)+3] = (jubyte) ((rgb) >> 16); \
    } while (0)

#define StoreFourByteAbgrFrom1IntArgb(pRas, PREFIX, x, argb) \
    do { \
        (pRas)[4*(x)+0] = (jubyte) ((argb) >> 24); \
	(pRas)[4*(x)+1] = (jubyte) ((argb) >> 0); \
	(pRas)[4*(x)+2] = (jubyte) ((argb) >> 8); \
	(pRas)[4*(x)+3] = (jubyte) ((argb) >> 16); \
    } while (0)

#define StoreFourByteAbgrFrom3ByteRgb(pRas, PREFIX, x, r, g, b) \
    StoreFourByteAbgrFrom4ByteArgb(pRas, PREFIX, x, 0xff, r, g, b)

#define StoreFourByteAbgrFrom4ByteArgb(pRas, PREFIX, x, a, r, g, b) \
    do { \
        (pRas)[4*(x)+0] = (jubyte) (a); \
	(pRas)[4*(x)+1] = (jubyte) (b); \
	(pRas)[4*(x)+2] = (jubyte) (g); \
	(pRas)[4*(x)+3] = (jubyte) (r); \
    } while (0)


#define DeclareFourByteAbgrAlphaLoadData(PREFIX)
#define InitFourByteAbgrAlphaLoadData(PREFIX, pRasInfo)

#define LoadAlphaFromFourByteAbgrFor4ByteArgb(pRas, PREFIX, COMP_PREFIX) \
    COMP_PREFIX ## A = (pRas)[0]

#define Postload4ByteArgbFromFourByteAbgr(pRas, PREFIX, COMP_PREFIX) \
    LoadFourByteAbgrTo3ByteRgb(pRas, PREFIX, 0, COMP_PREFIX ## R, \
                               COMP_PREFIX ## G, COMP_PREFIX ## B)


#define FourByteAbgrIsPremultiplied	0

#define DeclareFourByteAbgrBlendFillVars(PREFIX) \
    jubyte PREFIX ## 0, PREFIX ## 1, PREFIX ## 2, PREFIX ## 3;

#define ClearFourByteAbgrBlendFillVars(PREFIX, argb) \
    (PREFIX ## 0 = PREFIX ## 1 = PREFIX ## 2 = PREFIX ## 3 = 0)

#define InitFourByteAbgrBlendFillVarsNonPre(PREFIX, argb, COMP_PREFIX) \
    do { \
        PREFIX ## 0 = (jubyte) COMP_PREFIX ## A; \
	PREFIX ## 1 = (jubyte) COMP_PREFIX ## B; \
	PREFIX ## 2 = (jubyte) COMP_PREFIX ## G; \
	PREFIX ## 3 = (jubyte) COMP_PREFIX ## R; \
    } while (0)

#define InitFourByteAbgrBlendFillVarsPre(PREFIX, argb, COMP_PREFIX)

#define StoreFourByteAbgrBlendFill(pRas, PREFIX, x, argb, COMP_PREFIX) \
    do { \
        (pRas)[4*x+0] = PREFIX ## 0; \
	(pRas)[4*x+1] = PREFIX ## 1; \
	(pRas)[4*x+2] = PREFIX ## 2; \
	(pRas)[4*x+3] = PREFIX ## 3; \
    } while (0)

#define StoreFourByteAbgrFrom4ByteArgbComps(pRas, PREFIX, x, COMP_PREFIX) \
    StoreFourByteAbgrFrom4ByteArgb(pRas, PREFIX, x, \
                                   COMP_PREFIX ## A, COMP_PREFIX ## R, \
                                   COMP_PREFIX ## G, COMP_PREFIX ## B)

#endif /* FourByteAbgr_h_Included */
