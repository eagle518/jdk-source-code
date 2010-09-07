/*
 * @(#)awt_Mlib.cpp	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "awt_Mlib.h"
#include "java_awt_image_BufferedImage.h"

extern "C"
{
    /*
 * This is called by awt_ImagingLib.initLib() to figure out if there
 * is a native imaging lib tied to the ImagingLib.java (other than
 * the shared medialib).
 */
    void awt_getImagingLib() {
        return;
    }

    mlib_start_timer awt_setMlibStartTimer() {
        return NULL;
    }

    mlib_stop_timer awt_setMlibStopTimer() {
        return NULL;
    }
    
    void awt_getBIColorOrder(int type, int *colorOrder) {
        switch(type) {
        case java_awt_image_BufferedImage_TYPE_INT_ARGB:
        case java_awt_image_BufferedImage_TYPE_INT_ARGB_PRE:
            colorOrder[0] = 2;
            colorOrder[1] = 1;
            colorOrder[2] = 0;
            colorOrder[3] = 3;
            break;
        case java_awt_image_BufferedImage_TYPE_INT_BGR:
            colorOrder[0] = 0;
            colorOrder[1] = 1;
            colorOrder[2] = 2;
            break;
        case java_awt_image_BufferedImage_TYPE_INT_RGB:
            colorOrder[0] = 2;
            colorOrder[1] = 1;
            colorOrder[2] = 0;
            break;
        case java_awt_image_BufferedImage_TYPE_4BYTE_ABGR:
        case java_awt_image_BufferedImage_TYPE_4BYTE_ABGR_PRE:
            colorOrder[0] = 3;
            colorOrder[1] = 2;
            colorOrder[2] = 1;
            colorOrder[3] = 0;
            break;
        case java_awt_image_BufferedImage_TYPE_3BYTE_BGR:
            colorOrder[0] = 2;
            colorOrder[1] = 1;
            colorOrder[2] = 0;
            break;
        case java_awt_image_BufferedImage_TYPE_USHORT_565_RGB:
        case java_awt_image_BufferedImage_TYPE_USHORT_555_RGB:
            colorOrder[0] = 0;
            colorOrder[1] = 1;
            colorOrder[2] = 2;
            break;
        case java_awt_image_BufferedImage_TYPE_BYTE_GRAY:
        case java_awt_image_BufferedImage_TYPE_USHORT_GRAY:
        case java_awt_image_BufferedImage_TYPE_BYTE_BINARY:
        case java_awt_image_BufferedImage_TYPE_BYTE_INDEXED:
            colorOrder[0] = 0;
            break;
        }
    }
}
