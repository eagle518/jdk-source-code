/*
 * @(#)X11CachingSurfaceManager.c	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#include "sun_awt_motif_X11CachingSurfaceManager.h"
#include "malloc.h"

#include "SurfaceData.h"
#include "BufImgSurfaceData.h"
#include "Disposer.h"

#include "colordata.h"
#include "img_util_md.h"

#ifndef HEADLESS

GeneralDisposeFunc X11CSM_Dispose;

/* set by awt during startup in awt_GraphicsEnv */
extern Bool usingXinerama;

void X11CSM_Dispose(JNIEnv *env, jlong bitmask)
{
    Pixmap pixmap = (Pixmap)bitmask;
    if (pixmap != 0) {
	AWT_LOCK();
	XFreePixmap(awt_display, pixmap);
	AWT_UNLOCK();
    }
}

/*
 * Class:     sun_awt_motif_X11CachingSurfaceManager
 * Method:    updateBitmask
 * Signature: (Lsun/java2d/SurfaceData;JII)J
 */

JNIEXPORT jint JNICALL
Java_sun_awt_motif_X11CachingSurfaceManager_updateBitmask
    (JNIEnv *env, jobject xcsm, jobject bisd, jint oldBitmask,
     jint screen, jint width, jint height)
{

    Pixmap bitmask = (Pixmap)oldBitmask;
    BufImgSDOps *bisdo = (BufImgSDOps *)SurfaceData_GetOps(env, bisd);
    SurfaceDataRasInfo srcInfo;
    SurfaceDataOps *srcOps = (SurfaceDataOps *)bisdo;

    jint srcScan, dstScan;
    int rowCount = height;
    unsigned char *pDst;
    XImage *image;
    GC xgc;
    
    AWT_LOCK();
    if (bisdo == NULL) {
	JNU_ThrowNullPointerException(env, "Null BISD in updateMaskRegion");
	AWT_UNLOCK();
	return (jint)0;
    }

    if (usingXinerama) {
	screen = 0;
    }

    if (bitmask == 0) {
	/* create the bitmask if it's not yet created */
	bitmask = XCreatePixmap(awt_display, 
			       RootWindowOfScreen(ScreenOfDisplay(awt_display,
								  screen)),
			       width, height, 1);
	if (bitmask == 0) {
	    AWT_UNLOCK();
	    return (jint)0;
	}
	/* Register the Pixmap for disposal */
	Disposer_AddRecord(env, xcsm, 
			   X11CSM_Dispose,
			   (jlong)bitmask);
    }

    /* Create a bitmask image and then blit it to the pixmap. */
    image = XCreateImage(awt_display, DefaultVisual(awt_display, screen),
			 1, XYBitmap, 0, NULL, width, height, 32, 0);
    if (image == NULL) {
	AWT_UNLOCK();
	JNU_ThrowOutOfMemoryError(env, "Cannot allocate bitmask for mask");
	return 0;
    }
    dstScan = image->bytes_per_line;
    image->data = malloc(dstScan * height);
    if (image->data == NULL) {
	XFree(image);
	AWT_UNLOCK();
	JNU_ThrowOutOfMemoryError(env, "Cannot allocate bitmask for mask");
	return 0;
    }
    pDst = (unsigned char *)image->data;

    srcInfo.bounds.x1 = 0;
    srcInfo.bounds.y1 = 0;
    srcInfo.bounds.x2 = width;
    srcInfo.bounds.y2 = height;
    if (bisdo->icm == NULL) {
	/*DCM with ARGB*/
	unsigned int *pSrc;
	if (srcOps->Lock(env, srcOps, &srcInfo, SD_LOCK_READ) != SD_SUCCESS) {
	    XDestroyImage(image);
	    AWT_UNLOCK();
	    return (jint)0;
	}
	srcOps->GetRasInfo(env, srcOps, &srcInfo);
	/* this is a number of pixels in a row, not number of bytes */
	srcScan = srcInfo.scanStride;
	pSrc = (unsigned int *)srcInfo.rasBase;

	if (image->bitmap_bit_order == MSBFirst) {
	    do {
		int x = 0, bx = 0;
		unsigned int pix = 0;
		unsigned int bit = 0x80;
		unsigned int *srcPixel = pSrc;
		do {
		    if (bit == 0) {
			/* next word */
			pDst[bx++] = (unsigned char)pix;
			pix = 0;
			bit = 0x80;
		    }
		    if (*srcPixel++ & 0xff000000) {
			/* if src pixel is opaque, set the bit in the bitmap */
			pix |= bit;
		    }
		    bit >>= 1;
		} while (++x < width);
		/* last pixels in a row */
		pDst[bx] = (unsigned char)pix;

		pDst += dstScan;
		pSrc = (unsigned int *) (((intptr_t)pSrc) + srcScan);
	    } while (--rowCount > 0);
	} else {
	    do {
		int x = 0, bx = 0;
		unsigned int pix = 0;
		unsigned int bit = 1;
		unsigned int *srcPixel = pSrc;
		do {
		    if ((bit >> 8) != 0) {
			pDst[bx++] = (unsigned char)pix;
			pix = 0;
			bit = 1;
		    }
		    if (*srcPixel++ & 0xff000000) {
			pix |= bit;
		    }
		    bit <<= 1;
		} while (++x < width);
		pDst[bx] = (unsigned char)pix;
		pDst += dstScan;
		pSrc = (unsigned int *) (((intptr_t)pSrc) + srcScan);
	    } while (--rowCount > 0);
	}
	SurfaceData_InvokeRelease(env, srcOps, &srcInfo);
	SurfaceData_InvokeUnlock(env, srcOps, &srcInfo);
    } else {
	/*ICM*/
	unsigned char *pSrc;
	jint *srcLut;

	if (srcOps->Lock(env, srcOps, &srcInfo, SD_LOCK_LUT | SD_LOCK_READ) != SD_SUCCESS) {
	    XDestroyImage(image);
	    AWT_UNLOCK();
	    return (jint)0;
	}
	srcOps->GetRasInfo(env, srcOps, &srcInfo);
	srcScan = srcInfo.scanStride;
	srcLut = srcInfo.lutBase;
	pSrc = (unsigned char *)srcInfo.rasBase;

	if (image->bitmap_bit_order == MSBFirst) {
	    do {
		int x = 0, bx = 0;
		unsigned int pix = 0;
		unsigned int bit = 0x80;
		unsigned char *srcPixel = pSrc;
		do {
		    if (bit == 0) {
			pDst[bx++] = (unsigned char)pix;
			pix = 0;
			bit = 0x80;
		    }
		    if (srcLut[*srcPixel++] & 0xff000000) {
			pix |= bit;
		    }
		    bit >>= 1;
		} while (++x < width);
		pDst[bx] = (unsigned char)pix;
		pDst += dstScan;
		pSrc = (unsigned char *) (((intptr_t)pSrc) + srcScan);
	    } while (--rowCount > 0);
	} else {
	    do {
		int x = 0, bx = 0;
		unsigned int pix = 0;
		unsigned int bit = 1;
		unsigned char *srcPixel = pSrc;
		do {
		    if ((bit >> 8) != 0) {
			pDst[bx++] = (unsigned char) pix;
			pix = 0;
			bit = 1;
		    }
		    if (srcLut[*srcPixel++] & 0xff000000) {
			pix |= bit;
		    }
		    bit <<= 1;
		} while (++x < width);
		pDst[bx] = (unsigned char) pix;
		pDst += dstScan;
		pSrc = (unsigned char *) (((intptr_t)pSrc) + srcScan);
	    } while (--rowCount > 0);
	}
	SurfaceData_InvokeRelease(env, srcOps, &srcInfo);
	SurfaceData_InvokeUnlock(env, srcOps, &srcInfo);
    }

    xgc = XCreateGC(awt_display, bitmask, 0L, NULL);
    XSetForeground(awt_display, xgc, 1);
    XSetBackground(awt_display, xgc, 0);
    XPutImage(awt_display, bitmask, xgc,
	      image, 0, 0, 0, 0, width, height);

    XFreeGC(awt_display, xgc);
    XDestroyImage(image);

    AWT_UNLOCK();
    return (jint)bitmask;
}

#endif /* !HEADLESS */
