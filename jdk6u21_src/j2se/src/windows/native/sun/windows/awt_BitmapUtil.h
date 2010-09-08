/*
 * @(#)awt_BitmapUtil.h	1.7 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_BITMAP_UTIL_H
#define AWT_BITMAP_UTIL_H

class BitmapUtil {
public:
    /**
     * Creates B&W Bitmap with transparency mask from specified ARGB input data
     * 0 for opaque pixels, 1 for transparent.
     * MSDN article for ICONINFO says that 'for color icons, this mask only 
     * defines the AND bitmask of the icon'. That's wrong! If mask bit for
     * specific pixel is 0, the pixel is drawn opaque, otherwise it's XORed
     * with background. 
     */
    static HBITMAP CreateTransparencyMaskFromARGB(int width, int height, int* imageData);

    /**
     * Creates 32-bit ARGB V4 Bitmap (Win95-compatible) from specified ARGB input data
     * The color for transparent pixels (those with 0 alpha) is reset to 0 (BLACK)
     * to prevent errors on systems prior to XP.
     */
    static HBITMAP CreateV4BitmapFromARGB(int width, int height, int* imageData);

    /**
     * Creates 32-bit premultiplied ARGB V4 Bitmap (Win95-compatible) from
     * specified ARGB Pre input data.
     */
    static HBITMAP CreateBitmapFromARGBPre(int width, int height,
                                           int srcStride,
                                           int* imageData);

    /**
     * Transforms the given bitmap into an HRGN representing the transparency
     * of the bitmap.
     */
    static HRGN BitmapToRgn(HBITMAP hBitmap);

    /**
     * Makes a copy of the given bitmap. Blends every pixel of the source
     * with the given blendColor and alpha. If alpha == 0, the function
     * simply makes a plain copy of the source without any blending.
     */
    static HBITMAP BlendCopy(HBITMAP hSrcBitmap, COLORREF blendColor, BYTE alpha);

    /**
     * Creates a 32 bit ARGB bitmap. Returns the bitmap handle. 
     * The pointer to the bitmap data is stored into bitmapBitsPtr.
     */
    static HBITMAP CreateARGBBitmap(int width, int height, void ** bitmapBitsPtr);
};

#endif
