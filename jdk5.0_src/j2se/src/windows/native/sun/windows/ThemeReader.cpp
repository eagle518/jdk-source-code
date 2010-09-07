/*
 * @(#)ThemeReader.cpp	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#define OEMRESOURCE

#include <windows.h>
#include "jni_util.h"
#include "alloc.h"

#include "stdhdrs.h"
#include "UnicowsLoader.h"

#define TR_OPAQUE 1       /* Transparency.OPAQUE      */
#define TR_BITMASK 2      /* Transparency.BITMASK     */
#define TR_TRANSLUCENT 3  /* Transparency.TRANSLUCENT */

extern "C" {

/*
 * Class:     sun_awt_windows_ThemeReader
 * Method:    getBitmapResource
 * Signature: (Ljava/lang/String;Ljava/lang/String;)[I
 */
JNIEXPORT jintArray JNICALL
Java_sun_awt_windows_ThemeReader_getBitmapResource(JNIEnv* env, jclass cls,
    jstring absolutePath, jstring resource)
{
    HBITMAP hBitmap = NULL;
    HINSTANCE hLib = NULL;
    HDC hDC = NULL;
    jintArray result = NULL;
    LPCTSTR str;

    str = (LPTSTR)JNU_GetStringPlatformChars(env, absolutePath, NULL);

    hLib = ::LoadLibrary(str);

    JNU_ReleaseStringPlatformChars(env, absolutePath, str);
    if (hLib == NULL) {
        goto exit;
    }

    str = (LPCTSTR)JNU_GetStringPlatformChars(env, resource, NULL);
    hBitmap = (HBITMAP)::LoadImage(hLib, str,
                                   IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    JNU_ReleaseStringPlatformChars(env, resource, str);
    if (hBitmap == NULL) {
        goto exit;
    }

    BITMAP bm;
    ::GetObject(hBitmap, sizeof(bm), (LPSTR)&bm);

    // Get the screen DC
    hDC = ::GetDC(NULL);
    if (hDC == NULL) {
        goto exit;
    }

    // Set up BITMAPINFO
    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = bm.bmWidth;
    bmi.bmiHeader.biHeight = -bm.bmHeight;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    int nPixels;
    nPixels = bm.bmWidth * bm.bmHeight;

    // Create java array
    jintArray bits;
    bits = env->NewIntArray(nPixels+2);

    long *pBits;
    pBits = (long*)env->GetPrimitiveArrayCritical(bits, 0);

    if (pBits == NULL) {
        goto exit;
    }

    long *colorBits;
    colorBits = pBits + 2;

    // Extract the color bitmap
    int res;
    res = ::GetDIBits(hDC, hBitmap, 0, bm.bmHeight, colorBits, &bmi, DIB_RGB_COLORS);    

    if (res == 0) {
        env->ReleasePrimitiveArrayCritical(bits, pBits, 0);
        goto exit;
    }

    long transparency;
    if (bm.bmBitsPixel == 32) {
        // ABGR has Alpha chanel
	// Note: We reserve the TR_OPAQUE value for non-32 bit images so that
	// we can distinguish them later in XPStyle.java
	transparency = TR_BITMASK;
        for (int i = 0; i < nPixels; i++) {
	    int alpha = (colorBits[i] & 0xff000000);
	    if (alpha != 0xff000000 && alpha != 0) {
                transparency = TR_TRANSLUCENT;
                break;
            }
        }
    } else {
	transparency = TR_OPAQUE;
    }

    pBits[0] = bm.bmWidth;
    pBits[1] = transparency;

    env->ReleasePrimitiveArrayCritical(bits, pBits, 0);

    result = bits;

exit:
    ::DeleteObject(hBitmap);
    ::FreeLibrary(hLib);
    ::ReleaseDC(NULL, hDC);
    return result;
}

static jstring getTextResource(JNIEnv* env,
    jstring absolutePath, jstring resType, LPCTSTR strResource) 
{
    LPCTSTR strPath = (LPCTSTR)JNU_GetStringPlatformChars(env, absolutePath, NULL);

    HINSTANCE hLib = ::LoadLibrary(strPath);

    JNU_ReleaseStringPlatformChars(env, absolutePath, strPath);
    if (hLib == NULL) {
        return NULL;
    }

    LPCTSTR strResType = (LPCTSTR)JNU_GetStringPlatformChars(env, resType, NULL);
    HRSRC hResource = ::FindResource(hLib, strResource, strResType);
    JNU_ReleaseStringPlatformChars(env, resType, strResType);
    if (hResource == NULL) {
        ::FreeLibrary(hLib);
        return NULL;
    }


    HGLOBAL hResData = (LPBYTE)::LoadResource(hLib, hResource); 
    if (hResData == NULL) {
        ::FreeLibrary(hLib);
        return NULL;
    }

    jstring result = env->NewString((jchar*)::LockResource(hResData),
         ::SizeofResource(hLib, hResource)/sizeof(jchar));

    ::FreeLibrary(hLib);
    return result;
}

/*
 * Class:     sun_awt_windows_ThemeReader
 * Method:    getTextResourceByName
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String
 */
JNIEXPORT jstring JNICALL
Java_sun_awt_windows_ThemeReader_getTextResourceByName(JNIEnv* env, jclass cls,
    jstring absolutePath, jstring resource, jstring resType)
{
    LPCTSTR resStr = (LPCTSTR)JNU_GetStringPlatformChars(env, resource, NULL);
    jstring res = getTextResource(env, absolutePath, resType, resStr);
    JNU_ReleaseStringPlatformChars(env, resource, resStr);
    return res;
}

/*
 * Class:     sun_awt_windows_ThemeReader
 * Method:    getTextResourceByInt
 * Signature: (Ljava/lang/String;ILjava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_sun_awt_windows_ThemeReader_getTextResourceByInt(JNIEnv* env, jclass cls,
    jstring absolutePath, jint resource, jstring resType)
{
    return getTextResource(env, absolutePath, resType, MAKEINTRESOURCE(resource));

}
}
