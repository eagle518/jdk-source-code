/*
 * @(#)awt_IconCursor.cpp	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt_IconCursor.h"

/* common code used by awt_Frame.cpp and awt_Cursor.cpp */

HBITMAP create_BMP(HWND hW,int* imageData,int nSS, int nW, int nH)
{
    Bitmapheader    bmhHeader;
    HDC             hDC;
    char            *ptrImageData;
    HBITMAP         hbmpBitmap;
    HBITMAP         hBitmap;

    int             nNumChannels    = 3;

    if (!hW) {
        hW = ::GetDesktopWindow();
    }
    hDC = ::GetDC(hW);
    if (!hDC) {
        return NULL;
    }

    memset(&bmhHeader, 0, sizeof(Bitmapheader));
    bmhHeader.bmiHeader.biSize              = sizeof(BITMAPINFOHEADER);
    bmhHeader.bmiHeader.biWidth             = nW;
    bmhHeader.bmiHeader.biHeight            = -nH;
    bmhHeader.bmiHeader.biPlanes            = 1;

    bmhHeader.bmiHeader.biBitCount          = 24;
    bmhHeader.bmiHeader.biCompression       = BI_RGB;

    hbmpBitmap = ::CreateDIBSection(hDC, (BITMAPINFO*)&(bmhHeader),
                                    DIB_RGB_COLORS,
                                    (void**)&(ptrImageData),
                                    NULL, 0);
    int  *srcPtr = imageData;
    char *dstPtr = ptrImageData;
    if (!dstPtr) {
	ReleaseDC(hW, hDC);
        return NULL;
    }
    for (int nOutern = 0; nOutern < nH; nOutern++ ) {
        for (int nInner = 0; nInner < nSS; nInner++ ) {
            dstPtr[2] = (*srcPtr >> 0x10) & 0xFF;
            dstPtr[1] = (*srcPtr >> 0x08) & 0xFF;
            dstPtr[0] = *srcPtr & 0xFF;

            srcPtr++;
            dstPtr += nNumChannels;
        }
    }

    // convert it into DDB to make CustomCursor work on WIN95
    hBitmap = CreateDIBitmap(hDC, 
			     (BITMAPINFOHEADER*)&bmhHeader,
			     CBM_INIT,
			     (void *)ptrImageData,
			     (BITMAPINFO*)&bmhHeader,
			     DIB_RGB_COLORS);

    ::DeleteObject(hbmpBitmap);
    ::ReleaseDC(hW, hDC);
    ::GdiFlush();
    return hBitmap;
}

void destroy_BMP(HBITMAP hBMP)
{
    if (hBMP) {
        ::DeleteObject(hBMP);
    }
}
