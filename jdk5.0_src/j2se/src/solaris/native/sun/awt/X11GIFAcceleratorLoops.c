/*
 * @(#)X11GIFAcceleratorLoops.c	1.28 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <jni.h>
#include <jni_util.h>
#include <sun_awt_motif_X11GIFAcceleratorLoops.h>
#include <math.h>
#include <memory.h>
#include <img_globals.h>
#include <awt_p.h>

#if 0 /* REMIND */

#ifndef _Included_ImageData
#include <ImageData.h>
#endif

#ifndef _Included_BlitMacros
#include <BlitMacros.h>
#endif

#include <img_util_md.h>

/* 8BIT_INDEXED family START */

#define INNER_LOOP_XPAR_REGION_BIDX( INNER_COUNT, SRC_TYPE, SRC_BUMP,\
                    DST_TYPE, DST_BUMP) \
    do { \
        unsigned int pixels_left = INNER_COUNT; \
        unsigned char *srcPixel = srcRow; \
        unsigned char *dstPixel = dstRow; \
	BIDX_DITHER_INIT; \
        while (pixels_left--) { \
            unsigned int p = srcLut[*srcPixel++]; \
            if (p & 0xff000000) { \
                int red, green, blue; \
                red     = (p >> 16) & 0xff; \
                green   = (p >> 8)  & 0xff; \
                blue    = p & 0xff; \
                red     += rerr[__xdither]; \
                green   += gerr[__xdither]; \
                blue    += berr[__xdither]; \
		BYTECLAMP_3COMP(red, green, blue); \
                *dstPixel++ = CUBEMAP(red,green,blue); \
                if (newrun) { \
                    newrun = 0; \
                    recty=(bltHeight-rowCount)-1; \
                    rectx=(INNER_COUNT-pixels_left)-1; \
                } \
            } else { \
                dstPixel++; \
                if (!newrun) { \
                    AddToRegion(regionP, rectx, recty, x, y, width, \
                                (INNER_COUNT-pixels_left)-1, \
                                (bltHeight-rowCount)-1); \
                    newrun = 1; \
                } \
            } \
	    BIDX_DITHER_NEXTX; \
        } \
	BIDX_DITHER_NEXTY; \
    } while (0);

/*
 * Class:     sun_awt_motif_X11GIFAcceleratorLoops
 * Method:    LUTxparToIndexed
 * Signature: (Lsun/awt/image/ImageData;Lsun/awt/image/ImageData;II)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_motif_X11GIFAcceleratorLoops_LUTxparToIndexed(
    JNIEnv *env, jobject joSelf,
    jobject srcImage, jobject dstImage, jint width, jint height)
{
    ScreenData* pScreen = 
                    (ScreenData *)getPlatformInfoFromImageData(env, dstImage);
    if (!pScreen) {
        return;
    }

    if (!cubemapArray) {
        return;
    }

    if (pScreen->method == METHOD_XIMAGE) {
        Region *regionP = &(pScreen->region);
        int x, y;
        int rectx = 0;
        int recty = 0;
        int newrun = 1;
        getViewOriginFromImageData(env, dstImage, &x, &y);
        pScreen->fUsingRegion = 1;
	#undef DECLARE_COLORDATA
	#define DECLARE_COLORDATA ColorData *cData = dstLockInfo.colorData
	#undef DECLARE_SRC_LUT
	#define DECLARE_SRC_LUT unsigned int *srcLut = srcLockInfo.lockedLut
	#undef DECLARE_DITHER
	#define DECLARE_DITHER DECLARE_BIDX_DITHER
#define INNER_LOOP( INNER_COUNT, SRC_TYPE, SRC_BUMP, DST_TYPE, DST_BUMP) \
    INNER_LOOP_XPAR_REGION_BIDX( INNER_COUNT, SRC_TYPE, SRC_BUMP,\
                                                DST_TYPE, DST_BUMP)

        INVOKE_BLIT(ByteIndexed, char, ByteIndexed, char, 1, 1, 1);

        if (!newrun) {
            if (*regionP != NULL) {
                AddToRegion(regionP, rectx, recty, x, y, width,
                width-1, height-1);
            }
        }
        if (*regionP == NULL) {
            pScreen->fUsingRegion = 0;
        }

        #undef INNER_LOOP
	#undef DECLARE_DITHER
	#define DECLARE_DITHER DEFAULT_DECLARE_DITHER
        #undef DECLARE_SRC_LUT
        #define DECLARE_SRC_LUT DEFAULT_SRC_LUT
        #undef DECLARE_COLORDATA
        #define DECLARE_COLORDATA DEFAULT_COLORDATA
    } else {
	#undef DECLARE_COLORDATA
	#define DECLARE_COLORDATA ColorData *cData = dstLockInfo.colorData
        #undef DECLARE_SRC_LUT
        #define DECLARE_SRC_LUT unsigned int *srcLut = srcLockInfo.lockedLut

	#undef DECLARE_DITHER
	#define DECLARE_DITHER DECLARE_BIDX_DITHER
        #define INNER_LOOP( INNER_COUNT, SRC_TYPE, SRC_BUMP,\
                            DST_TYPE, DST_BUMP) \
            INNER_LOOP_XPAR_DITHER_BIDX( INNER_COUNT, SRC_TYPE, SRC_BUMP,\
                            DST_TYPE, DST_BUMP)

        INVOKE_BLIT(ByteIndexed, char, ByteIndexed, char, 1, 1, 1);

        #undef INNER_LOOP
	#undef DECLARE_DITHER
	#define DECLARE_DITHER DEFAULT_DECLARE_DITHER
        #undef DECLARE_SRC_LUT
        #define DECLARE_SRC_LUT DEFAULT_SRC_LUT
        #undef DECLARE_COLORDATA
        #define DECLARE_COLORDATA DEFAULT_COLORDATA
    }
}
/* 8BIT_INDEXED family END */



#define INNER_LOOP_BIDX_REGION_TO_3COMP(INNER_COUNT, \
                                      SRC_TYPE, SRC_BUMP, DST_TYPE, DST_BUMP) \
    do { \
        unsigned int pixels_left = INNER_COUNT; \
        unsigned char *srcPixel = srcRow; \
        unsigned DST_TYPE *dstPixel = dstRow; \
        while (pixels_left--) { \
            int alpha, red, green, blue; \
            BIDX_XPAR_TO_THREE_COMPONENTS(srcPixel, red, green, blue, alpha); \
            COMPONENTS_TO_DEST(dstPixel, red, green, blue, alpha); \
            srcPixel += SRC_BUMP; \
            dstPixel += DST_BUMP; \
            if (alpha != 0) { \
                if (newrun) { \
                    newrun = 0; \
                    recty=(bltHeight-rowCount)-1; \
                    rectx=(INNER_COUNT-pixels_left)-1; \
                } \
            } else if (!newrun) { \
                AddToRegion(regionP, rectx, recty, x, y, width, \
                            (INNER_COUNT-pixels_left)-1, \
                            (bltHeight-rowCount)-1); \
                newrun = 1; \
            } \
        } \
    } while (0);

/*
 * Class:     sun_awt_motif_X11GIFAcceleratorLoops
 * Method:    LUTxparToIntBgr
 * Signature: (Lsun/awt/image/ImageData;Lsun/awt/image/ImageData;II)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_motif_X11GIFAcceleratorLoops_LUTxparToIntBgr(
    JNIEnv *env, jobject joSelf,
    jobject srcImage, jobject dstImage, jint width, jint height)
{
    ScreenData* pScreen = 
                    (ScreenData *)getPlatformInfoFromImageData(env, dstImage);
    if (!pScreen) {
        return;
    }

    if (!cubemapArray) {
        return;
    }

    if (pScreen->method == METHOD_XIMAGE) {
        Region *regionP = &(pScreen->region);
        int x, y;
        int rectx = 0;
        int recty = 0;
        int newrun = 1;
        getViewOriginFromImageData(env, dstImage, &x, &y);
        pScreen->fUsingRegion = 1;
#undef DECLARE_SRC_LUT
#define DECLARE_SRC_LUT unsigned int *srcLut = srcLockInfo.lockedLut
#define COMPONENTS_TO_DEST(d,r,g,b,a) THREE_COMPONENTS_TO_XBGR(d,r,g,b)

#define INNER_LOOP( INNER_COUNT, SRC_TYPE, SRC_BUMP, DST_TYPE, DST_BUMP) \
            INNER_LOOP_BIDX_REGION_TO_3COMP( INNER_COUNT, \
                                                        SRC_TYPE, SRC_BUMP,\
                                                        DST_TYPE, DST_BUMP)

        INVOKE_BLIT(ByteIndexed, char, Int, int, 1, 1, 1);

        if (!newrun) {
	        if (*regionP != NULL) {
		        AddToRegion(regionP, rectx, recty, x, y, width,
                                                        width-1, height-1);
	        }
        }
        if (*regionP == NULL) {
    		pScreen->fUsingRegion = 0;
        }
        #undef INNER_LOOP
        #undef COMPONENTS_TO_DEST
        #undef DECLARE_SRC_LUT
        #define DECLARE_SRC_LUT DEFAULT_SRC_LUT
    } else {
        #undef DECLARE_SRC_LUT
        #define DECLARE_SRC_LUT unsigned int *srcLut = srcLockInfo.lockedLut
        #define COMPONENTS_TO_DEST(d,r,g,b,a) \
            THREE_COMPONENTS_TO_XBGR(d,r,g,b)

        #define INNER_LOOP( INNER_COUNT, \
                            SRC_TYPE, SRC_BUMP, \
                            DST_TYPE, DST_BUMP) \
                    INNER_LOOP_BIDX_XPAR_TO_3COMP(   INNER_COUNT, \
                                                SRC_TYPE, SRC_BUMP,\
                                                DST_TYPE, DST_BUMP)

        INVOKE_BLIT(ByteIndexed, char, Int, int, 1, 1, 1);

        #undef INNER_LOOP
        #undef COMPONENTS_TO_DEST
        #undef DECLARE_SRC_LUT
        #define DECLARE_SRC_LUT DEFAULT_SRC_LUT
    }
}
/* 32BIT_xBGR family END */    


#endif /* REMIND */
