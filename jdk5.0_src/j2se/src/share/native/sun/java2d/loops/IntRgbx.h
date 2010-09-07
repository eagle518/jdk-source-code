/*
 * @(#)IntRgbx.h	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef IntRgbx_h_Included
#define IntRgbx_h_Included

#include "IntDcm.h"

/*
 * This file contains macro and type definitions used by the macros in
 * LoopMacros.h to manipulate a surface of type "IntRgbx".
 */

typedef jint	IntRgbxPixelType;
typedef jint	IntRgbxDataType;

#define IntRgbxPixelStride	4

#define DeclareIntRgbxLoadVars(PREFIX)
#define DeclareIntRgbxStoreVars(PREFIX)
#define SetIntRgbxStoreVarsYPos(PREFIX, pRasInfo, y)
#define SetIntRgbxStoreVarsXPos(PREFIX, pRasInfo, x)
#define InitIntRgbxLoadVars(PREFIX, pRasInfo)
#define InitIntRgbxStoreVarsY(PREFIX, pRasInfo)
#define InitIntRgbxStoreVarsX(PREFIX, pRasInfo)
#define NextIntRgbxStoreVarsX(PREFIX)
#define NextIntRgbxStoreVarsY(PREFIX)

#define IntRgbxXparLutEntry			1
#define IntRgbxIsXparLutEntry(pix)		((pix & 1) != 0)
#define StoreIntRgbxNonXparFromArgb		StoreIntRgbxFromArgb


#define IntRgbxPixelFromArgb(pixel, rgb, pRasInfo) \
    (pixel) = (rgb << 8)

#define StoreIntRgbxPixel(pRas, x, pixel) \
    (pRas)[x] = (pixel)

#define DeclareIntRgbxPixelData(PREFIX)

#define ExtractIntRgbxPixelData(PIXEL, PREFIX)

#define StoreIntRgbxPixelData(pPix, x, pixel, PREFIX) \
    (pPix)[x] = (pixel)


#define LoadIntRgbxTo1IntRgb(pRas, PREFIX, x, rgb) \
    (rgb) = ((pRas)[x] >> 8)

#define LoadIntRgbxTo1IntArgb(pRas, PREFIX, x, argb) \
    (argb) = 0xff000000 | ((pRas)[x] >> 8)

#define LoadIntRgbxTo3ByteRgb(pRas, PREFIX, x, r, g, b) \
    do { \
	jint pixel = (pRas)[x]; \
	ExtractIntDcmComponents123X(pixel, r, g, b); \
    } while (0)

#define LoadIntRgbxTo4ByteArgb(pRas, PREFIX, x, a, r, g, b) \
    do { \
	LoadIntRgbxTo3ByteRgb(pRas, PREFIX, x, r, g, b); \
	(a) = 0xff; \
    } while (0)

#define StoreIntRgbxFrom1IntRgb(pRas, PREFIX, x, rgb) \
    (pRas)[x] = ((rgb) << 8)

#define StoreIntRgbxFrom1IntArgb(pRas, PREFIX, x, argb) \
    (pRas)[x] = ((argb) << 8)

#define StoreIntRgbxFrom3ByteRgb(pRas, PREFIX, x, r, g, b) \
    (pRas)[x] = ComposeIntDcmComponents123X(r, g, b)

#define StoreIntRgbxFrom4ByteArgb(pRas, PREFIX, x, a, r, g, b) \
    StoreIntRgbxFrom3ByteRgb(pRas, PREFIX, x, r, g, b)

#endif /* IntRgbx_h_Included */
