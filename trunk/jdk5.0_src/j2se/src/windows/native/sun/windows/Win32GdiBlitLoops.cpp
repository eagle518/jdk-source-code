/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)Win32GdiBlitLoops.cpp	1.8 03/12/19
 */

#include <sun_awt_windows_Win32GdiBlitLoops.h> 
#include "gdefs.h"
#include "ddrawUtils.h"


static RGBQUAD *byteGrayPalette = NULL;

extern "C" {

typedef struct tagBitmapheader  {
    BITMAPINFOHEADER bmiHeader;
    union {
	DWORD		dwMasks[3];
	RGBQUAD		palette[256];
    } colors;
} BmiType;

/*
 * Class:     sun_awt_windows_Win32GdiBlitLoops
 * Method:    nativeBlit
 * Signature: (Lsun/java2d/SurfaceData;Lsun/java2d/SurfaceData;IIIIIIZ)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_Win32GdiBlitLoops_nativeBlit
    (JNIEnv *env, jobject joSelf,
     jobject srcData, jobject dstData,
     jobject clip,
     jint srcx, jint srcy,
     jint dstx, jint dsty,
     jint width, jint height,
     jint rmask, jint gmask, jint bmask,
     jboolean needLut)
{
    DTRACE_PRINTLN("Win32GdiBlitLoops::nativeBlit");

    POINT ptDst = {0, 0};
    POINT ptSrc = {0, 0};
    SurfaceDataRasInfo srcInfo;
    SurfaceDataOps *srcOps = SurfaceData_GetOps(env, srcData);
    Win32SDOps *dstOps = Win32SurfaceData_GetOps(env, dstData);
    jint lockFlags;    
    HDC hDC = dstOps->GetDC(env, dstOps, 0, NULL, clip, NULL, 0);

    if (hDC == NULL) {
        return;
    }
    srcInfo.bounds.x1 = srcx;
    srcInfo.bounds.y1 = srcy;
    srcInfo.bounds.x2 = srcx + width;
    srcInfo.bounds.y2 = srcy + height;
    if (needLut) {
	lockFlags = (SD_LOCK_READ | SD_LOCK_LUT);
    } else {
	lockFlags = SD_LOCK_READ;
    }
    if (srcOps->Lock(env, srcOps, &srcInfo, lockFlags) != SD_SUCCESS) {
	dstOps->ReleaseDC(env, dstOps, hDC);
	return;
    }
    if (srcInfo.bounds.x2 > srcInfo.bounds.x1 &&
	srcInfo.bounds.y2 > srcInfo.bounds.y1)
    {
	BmiType bmi;
	// REMIND: A performance tweak here would be to make some of this 
	// data static.  For example, we could have one structure that is
	// always used for ByteGray copies and we only change dynamic data
	// in the structure with every new copy.  Also, we could store
	// structures with Ops or with the Java objects so that surfaces
	// could retain their own DIB info and we would not need to 
	// recreate it every time.

	srcOps->GetRasInfo(env, srcOps, &srcInfo);
	void *rasBase = ((char *)srcInfo.rasBase) + srcInfo.scanStride * srcy +
			srcInfo.pixelStride * srcx;

	// If scanlines are DWORD-aligned (scanStride is a multiple of 4),
	// then we can do the work much faster.  This is due to a constraint
	// in the way DIBs are structured and parsed by GDI
	jboolean fastBlt = ((srcInfo.scanStride & 0x03) == 0);

	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth = srcInfo.scanStride/srcInfo.pixelStride;
	// fastBlt copies whole image in one call; else copy line-by-line
	bmi.bmiHeader.biHeight = (fastBlt) ? 
	    -(srcInfo.bounds.y2 - srcInfo.bounds.y1) : -1;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = srcInfo.pixelStride * 8;
	// 1,3,4 byte use BI_RGB, 2 byte use BI_BITFIELD...
	// 4 byte _can_ use BI_BITFIELD, but this seems to cause a performance
	// penalty.  Since we only ever have one format (xrgb) for 32-bit
	// images that enter this function, just use BI_RGB.
	// Could do BI_RGB for 2-byte 555 format, but no perceived
	// performance benefit.
	bmi.bmiHeader.biCompression = (srcInfo.pixelStride != 2)
		? BI_RGB : BI_BITFIELDS;
	bmi.bmiHeader.biSizeImage = (bmi.bmiHeader.biWidth * 
				     bmi.bmiHeader.biHeight * 
				     srcInfo.pixelStride);
	bmi.bmiHeader.biXPelsPerMeter = 0;
	bmi.bmiHeader.biYPelsPerMeter = 0;
	bmi.bmiHeader.biClrUsed = 0;
	bmi.bmiHeader.biClrImportant = 0;
	if (srcInfo.pixelStride == 1) {
	    // Copy palette info into bitmap for 8-bit image
	    if (needLut) {
		memcpy(bmi.colors.palette, srcInfo.lutBase, srcInfo.lutSize * sizeof(RGBQUAD));
		if (srcInfo.lutSize != 256) {
		    bmi.bmiHeader.biClrUsed = srcInfo.lutSize;
		}
	    } else {
		// If no LUT needed, must be ByteGray src.  If we have not
		// yet created the byteGrayPalette, create it now and copy
		// it into our temporary bmi structure.
		// REMIND: byteGrayPalette is a leak since we do not have
		// a mechansim to free it up.  This should be fine, since it
		// is only 256 bytes for any process and only gets malloc'd
		// when using ByteGray surfaces.  Eventually, we should use
		// the new Disposer mechanism to delete this native memory.
		if (byteGrayPalette == NULL) {
		    byteGrayPalette = (RGBQUAD *)safe_Malloc(256 * sizeof(RGBQUAD));
		    for (int i = 0; i < 256; ++i) {
			byteGrayPalette[i].rgbRed = i;
			byteGrayPalette[i].rgbGreen = i;
			byteGrayPalette[i].rgbBlue = i;
		    }
		}
		memcpy(bmi.colors.palette, byteGrayPalette, 256 * sizeof(RGBQUAD));
	    }
	} else if (srcInfo.pixelStride == 2) {
	    // For 16-bit case, init the masks for the pixel depth
	    bmi.colors.dwMasks[0] = rmask;
	    bmi.colors.dwMasks[1] = gmask;
	    bmi.colors.dwMasks[2] = bmask;
	} 
	if (fastBlt) {
	    // Window could go away at any time, leaving bits on the screen
	    // from this GDI call, so make sure window still exists
	    if (::IsWindowVisible(dstOps->window)) {
		// Could also call StretchDIBits.  Testing showed slight
		// performance advantage of SetDIBits instead, so since we
		// have no need of scaling, might as well use SetDIBits.
		SetDIBitsToDevice(hDC, dstx, dsty, width, height, 
		    0, 0, 0, height, rasBase, 
		    (BITMAPINFO*)&bmi, DIB_RGB_COLORS);
	    }
	} else {
	    // Source scanlines not DWORD-aligned - copy each scanline individually
	    for (int i = 0; i < height; i += 1) {
		if (::IsWindowVisible(dstOps->window)) {
		    SetDIBitsToDevice(hDC, dstx, dsty+i, width, 1, 
			0, 0, 0, 1, rasBase, 
			(BITMAPINFO*)&bmi, DIB_RGB_COLORS);
		    rasBase = (void*)((char*)rasBase + srcInfo.scanStride);
		} else {
		    break;
		}
	    }
	}	
	SurfaceData_InvokeRelease(env, srcOps, &srcInfo);
    }
    SurfaceData_InvokeUnlock(env, srcOps, &srcInfo);
    dstOps->ReleaseDC(env, dstOps, hDC);

    return;
}

} // extern "C"
