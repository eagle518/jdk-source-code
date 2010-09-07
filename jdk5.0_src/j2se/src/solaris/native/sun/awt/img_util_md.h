/*
 * @(#)img_util_md.h	1.27 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "color.h"

#ifndef HEADLESS
typedef struct {
    ImgConvertData cvdata;	/* The data needed by ImgConvertFcn's */
    struct Hsun_awt_image_ImageRepresentation *hJavaObject;	/* backptr */
    XID pixmap;			/* The X11 pixmap containing the image */
    XID mask;			/* The X11 pixmap with the transparency mask */
    int bgcolor;		/* The current bg color installed in pixmap */

    int depth;			/* The depth of the destination image */
    int dstW;			/* The width of the destination pixmap */
    int dstH;			/* The height of the destination pixmap */

    XImage *xim;		/* The Ximage structure for the temp buffer */
    XImage *maskim;		/* The Ximage structure for the mask */

    int hints;			/* The delivery hints from the producer */

    Region curpixels;		/* The region of randomly converted pixels */
    struct {
	int num;		/* The last fully delivered scanline */
	char *seen;		/* The lines which have been delivered */
    } curlines;			/* For hints=COMPLETESCANLINES */
} IRData;

typedef unsigned int MaskBits;

extern int image_Done(IRData *ird, int x1, int y1, int x2, int y2);

extern void *image_InitMask(IRData *ird, int x1, int y1, int x2, int y2);

#define BufComplete(cvdata, dstX1, dstY1, dstX2, dstY2)		\
    image_Done((IRData *) cvdata, dstX1, dstY1, dstX2, dstY2)

#define SendRow(ird, dstY, dstX1, dstX2)

#define ImgInitMask(cvdata, x1, y1, x2, y2)			\
    image_InitMask((IRData *)cvdata, x1, y1, x2, y2)

#define ScanBytes(cvdata)	(((IRData *)cvdata)->xim->bytes_per_line)

#define MaskScan(cvdata)					\
	((((IRData *)cvdata)->maskim->bytes_per_line) >> 2)

#endif /* !HEADLESS */

#define MaskOffset(x)		((x) >> 5)

#define MaskInit(x)		(1U << (31 - ((x) & 31)))

#define SetOpaqueBit(mask, bit)		((mask) |= (bit))
#define SetTransparentBit(mask, bit)	((mask) &= ~(bit))

#define UCHAR_ARG(uc)    ((unsigned char)(uc))
#define ColorCubeFSMap(r, g, b) \
    cData->img_clr_tbl [    ((UCHAR_ARG(r)>>3)<<10) |                   \
                    ((UCHAR_ARG(g)>>3)<<5) | (UCHAR_ARG(b)>>3)]

#define ColorCubeOrdMapSgn(r, g, b) \
    ((dstLockInfo.inv_cmap)[    ((UCHAR_ARG(r)>>3)<<10) |                   \
                    ((UCHAR_ARG(g)>>3)<<5) | (UCHAR_ARG(b)>>3)])

#define GetPixelRGB(pixel, red, green, blue)			\
    do {							\
	ColorEntry *cp = &awt_Colors[pixel];			\
	red = cp->r;						\
	green = cp->g;						\
	blue = cp->b;						\
    } while (0)

#define CUBEMAP(r,g,b) ColorCubeOrdMapSgn(r, g, b)
#define cubemapArray 1

extern uns_ordered_dither_array img_oda_alpha;

extern void freeICMColorData(ColorData *pData);

extern void initInverseGrayLut(int* prgb, int rgbsize, ColorData* cData);
extern unsigned char* initCubemap(int* cmap, int cmap_len, int cube_dim);
extern void initDitherTables(ColorData* cData);

#define SET_CUBEMAPARRAY \
    lockInfo->inv_cmap = (const char*)lockInfo->colorData->img_clr_tbl


