/*
 * @(#)vis_DrawLine.c	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#if !defined(JAVA2D_NO_MLIB) || defined(MLIB_ADD_SUFF)

#include <vis_proto.h>
#include "java2d_Mlib.h"

/***************************************************************/

#define SET_PIX(index, chan)   \
    pPix[chan] = pix##chan

#define XOR_PIX(index, chan)   \
    pPix[chan] ^= pix##chan

/***************************************************************/

#define EXTRA_1(FUNC, ANYTYPE, NCHAN, DO_PIX)
#define EXTRA_3(FUNC, ANYTYPE, NCHAN, DO_PIX)
#define EXTRA_4(FUNC, ANYTYPE, NCHAN, DO_PIX)                                \
    if ((((jint)pPix | scan) & 3) == 0) {                                    \
	mlib_s32 s_pixel = pixel, r_pixel;                                   \
	*(mlib_f32*)&r_pixel = vis_ldfa_ASI_PL(&s_pixel);                    \
	ADD_SUFF(AnyInt##FUNC)(pRasInfo, x1, y1, r_pixel, steps, error,      \
			       bumpmajormask, errmajor, bumpminormask,       \
			       errminor, pPrim, pCompInfo);                  \
	return;                                                              \
    }

/***************************************************************/

#define GET_PIXEL(pix)         \
    mlib_s32 pix = pixel

/***************************************************************/

#define DEFINE_SET_LINE(FUNC, ANYTYPE, NCHAN, DO_PIX)                  \
void ADD_SUFF(ANYTYPE##FUNC)(SurfaceDataRasInfo * pRasInfo,            \
			     jint x1,                                  \
			     jint y1,                                  \
			     jint pixel,                               \
			     jint steps,                               \
			     jint error,                               \
			     jint bumpmajormask,                       \
			     jint errmajor,                            \
			     jint bumpminormask,                       \
			     jint errminor,                            \
			     NativePrimitive * pPrim,                  \
			     CompositeInfo * pCompInfo)                \
{                                                                      \
    ANYTYPE##DataType *pPix = (void *)(pRasInfo->rasBase);             \
    mlib_s32 scan = pRasInfo->scanStride;                              \
    mlib_s32 bumpmajor, bumpminor, mask;                               \
    GET_PIXEL(pix);                                                    \
    EXTRACT_CONST_##NCHAN(pix);                                        \
                                                                       \
    EXTRA_##NCHAN(FUNC, AnyInt, NCHAN, DO_PIX);                        \
                                                                       \
    PTR_ADD(pPix, y1 * scan + x1 * ANYTYPE##PixelStride);              \
                                                                       \
    errminor += errmajor;                                              \
                                                                       \
    if (bumpmajormask & 0x1) bumpmajor =  ANYTYPE##PixelStride; else   \
    if (bumpmajormask & 0x2) bumpmajor = -ANYTYPE##PixelStride; else   \
    if (bumpmajormask & 0x4) bumpmajor =  scan; else                   \
	bumpmajor = - scan;                                            \
                                                                       \
    if (bumpminormask & 0x1) bumpminor =  ANYTYPE##PixelStride; else   \
    if (bumpminormask & 0x2) bumpminor = -ANYTYPE##PixelStride; else   \
    if (bumpminormask & 0x4) bumpminor =  scan; else                   \
    if (bumpminormask & 0x8) bumpminor = -scan; else                   \
	bumpminor = 0;                                                 \
                                                                       \
    if (errmajor == 0) {                                               \
	do {                                                           \
	    PROCESS_PIX_##NCHAN(DO_PIX);                               \
	    PTR_ADD(pPix, bumpmajor);                                  \
	} while (--steps > 0);                                         \
	return;                                                        \
    }                                                                  \
                                                                       \
    do {                                                               \
	PROCESS_PIX_##NCHAN(DO_PIX);                                   \
	mask = error >> 31;                                            \
	PTR_ADD(pPix, bumpmajor + (bumpminor &~ mask));                \
	error += errmajor - (errminor &~ mask);                        \
    } while (--steps > 0);                                             \
}

DEFINE_SET_LINE(SetLine, AnyInt,   1, SET_PIX)
DEFINE_SET_LINE(SetLine, AnyShort, 1, SET_PIX)
DEFINE_SET_LINE(SetLine, AnyByte,  1, SET_PIX)
DEFINE_SET_LINE(SetLine, Any3Byte, 3, SET_PIX)
DEFINE_SET_LINE(SetLine, Any4Byte, 4, SET_PIX)

/***************************************************************/

#undef  GET_PIXEL
#define GET_PIXEL(pix)                                 \
    mlib_s32 xorpixel = pCompInfo->details.xorPixel;   \
    mlib_s32 alphamask = pCompInfo->alphaMask;         \
    mlib_s32 pix = (pixel ^ xorpixel) &~ alphamask

#undef  EXTRA_4
#define EXTRA_4(FUNC, ANYTYPE, NCHAN, DO_PIX)

DEFINE_SET_LINE(XorLine, AnyInt,   1, XOR_PIX)
DEFINE_SET_LINE(XorLine, AnyShort, 1, XOR_PIX)
DEFINE_SET_LINE(XorLine, AnyByte,  1, XOR_PIX)
DEFINE_SET_LINE(XorLine, Any3Byte, 3, XOR_PIX)
DEFINE_SET_LINE(XorLine, Any4Byte, 4, XOR_PIX)

/***************************************************************/

#endif
