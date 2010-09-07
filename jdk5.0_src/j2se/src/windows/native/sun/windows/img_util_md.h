/*
 * @(#)img_util_md.h	1.28 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "windows.h"

#ifdef __cplusplus
extern "C" {
#include "colordata.h"
}
#else
#include "colordata.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


typedef unsigned char MaskBits;

extern uns_ordered_dither_array img_oda_alpha;

#define BufComplete(cvdata, dstX1, dstY1, dstX2, dstY2)		\
    (((AwtImage *)cvdata)->BufDone(dstX1, dstY1, dstX2, dstY2))

#define SendRow(cvdata, dstY, dstX1, dstX2)

#define ImgInitMask(cvdata, x1, y1, x2, y2)			\
    (((AwtImage *) cvdata)->GetMaskBuf(TRUE, x1, y1, x2, y2))

#define ScanBytes(cvdata)	(((AwtImage *) cvdata)->GetBufScan())

#define MaskScan(cvdata)					\
    MaskOffset((((AwtImage *) cvdata)->GetWidth() + 31) & (~31))

#define MaskOffset(x)		((x) >> 3)

#define MaskInit(x)		(0x80 >> ((x) & 7))

#define SetOpaqueBit(mask, bit)		((mask) &= ~(bit))
#define SetTransparentBit(mask, bit)	((mask) |= (bit))

#define ColorCubeFSMap(r, g, b)		AwtImage::CubeMap(r, g, b)

#define ColorCubeOrdMapSgn(r, g, b)	AwtImage::CubeMap(r, g, b);

#define GetPixelRGB(pixel, red, green, blue)			\
    do {							\
	RGBQUAD *cp = AwtImage::PixelColor(pixel);		\
	red = cp->rgbRed;					\
	green = cp->rgbGreen;					\
	blue = cp->rgbBlue;					\
    } while (0)

#ifdef DEBUG
#undef img_check
#define img_check(condition)					\
    do {							\
	if (!(condition)) {					\
	    SignalError(0, JAVAPKG "InternalError",		\
			"assertion failed:  " #condition);	\
	    return SCALEFAILURE;				\
	}							\
    } while (0)
#else /* DEBUG */
#define img_check(condition)	do {} while (0)
#endif /* DEBUG */

void color_init();
extern const char *cubemapArray;
#define CUBEMAP(r,g,b) \
    ((dstLockInfo.inv_cmap)[(((r)>>3)<<10) | (((g)>>3)<<5) | ((b)>>3)])

extern void freeICMColorData(ColorData *pData);
extern void initInverseGrayLut(int* prgb, int rgbsize, ColorData* cData);
extern unsigned char* initCubemap(int* cmap, int cmap_len, int cube_dim);
extern void initDitherTables(ColorData* cData);

#define SET_CUBEMAPARRAY \
    if (lockInfo->lockedLut) { \
        lockInfo->inv_cmap = (const char *)cubemapArray; \
    } else { \
        lockInfo->inv_cmap = (const char*)lockInfo->colorData->img_clr_tbl; \
    }


#ifdef __cplusplus
}; /* end of extern "C" */
#endif
