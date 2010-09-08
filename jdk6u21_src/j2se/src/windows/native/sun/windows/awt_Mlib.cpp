/*
 * @(#)awt_Mlib.cpp	1.16 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "awt_Mlib.h"
#include "java_awt_image_BufferedImage.h"

#include <windows.h>
#include "alloc.h"

extern "C"
{
    /*
 * This is called by awt_ImagingLib.initLib() to figure out if there
 * is a native imaging lib tied to the ImagingLib.java (other than
 * the shared medialib).
 */
    mlib_status awt_getImagingLib(JNIEnv *env, mlibFnS_t *sMlibFns,
                                  mlibSysFnS_t *sMlibSysFns) {
        static HINSTANCE hDLL = NULL;
        mlibSysFnS_t tempSysFns;
        mlib_status ret = MLIB_SUCCESS;
        
        /* Try to load library. Routine should find the library successfully 
         * because this library is already loaded to the process space by
         * the System.loadLibrary() call. Here we just need to get handle to
         * initialize the pointers to required mlib routines.
         */ 
        hDLL = ::LoadLibrary(TEXT("mlib_image.dll"));

        if (hDLL == NULL) {
            return MLIB_FAILURE;
        }

        /* Initialize pointers to medilib routines... */
        tempSysFns.createFP = (MlibCreateFP_t)
            ::GetProcAddress(hDLL, "j2d_mlib_ImageCreate");
        if (tempSysFns.createFP == NULL) {
            ret = MLIB_FAILURE;
        }

        if (ret == MLIB_SUCCESS) {
            tempSysFns.createStructFP = (MlibCreateStructFP_t)
                ::GetProcAddress(hDLL, "j2d_mlib_ImageCreateStruct");
            if (tempSysFns.createStructFP == NULL) {
                ret = MLIB_FAILURE;
            }
        }
        
        if (ret == MLIB_SUCCESS) {
            tempSysFns.deleteImageFP = (MlibDeleteFP_t)
                ::GetProcAddress(hDLL, "j2d_mlib_ImageDelete");
            if (tempSysFns.deleteImageFP == NULL) {
                ret = MLIB_FAILURE;
            }
        }
        if (ret == MLIB_SUCCESS) {
            *sMlibSysFns = tempSysFns;
        }

        mlib_status (*fPtr)();
        mlibFnS_t* pMlibFns = sMlibFns;
        int i = 0;
        while ((ret == MLIB_SUCCESS) && (pMlibFns[i].fname != NULL)) {
            fPtr = (mlib_status (*)())
                ::GetProcAddress(hDLL, pMlibFns[i].fname);
            if (fPtr != NULL) {
                pMlibFns[i].fptr = fPtr;
            } else {
                ret = MLIB_FAILURE;
            }
            i++;
        }
        
        if (ret != MLIB_SUCCESS) {
            ::FreeLibrary(hDLL);
        }
        return ret;
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
