/*
 * @(#)DataBufferNative.c	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "malloc.h"

#include "SurfaceData.h"
#include "sun_awt_image_DataBufferNative.h"

#include "jni_util.h"
#include "debug_trace.h"
#include <stdio.h>

unsigned char *DBN_GetPixelPointer(JNIEnv *env, jint x, int y,
				   SurfaceDataRasInfo *lockInfo, 
				   SurfaceDataOps *ops, int lockFlag) 
{
    if (ops == NULL) {
	return NULL;
    }

    lockInfo->bounds.x1 = x;
    lockInfo->bounds.y1 = y;
    lockInfo->bounds.x2 = x + 1;
    lockInfo->bounds.y2 = y + 1;
    if (ops->Lock(env, ops, lockInfo, lockFlag) != SD_SUCCESS) {
	return NULL;
    }
    ops->GetRasInfo(env, ops, lockInfo);
    if (lockInfo->rasBase) {
	unsigned char *pixelPtr = (
	    (unsigned char*)lockInfo->rasBase + 
	    (x * lockInfo->pixelStride + y * lockInfo->scanStride));
	return pixelPtr;
    }
    SurfaceData_InvokeRelease(env, ops, lockInfo);
    SurfaceData_InvokeUnlock(env, ops, lockInfo);
    return NULL;
}

/*
 * Class:     sun_awt_image_DataBufferNative
 * Method:    getElem
 * Signature: 
 */
JNIEXPORT jint JNICALL
Java_sun_awt_image_DataBufferNative_getElem(JNIEnv *env, jobject dbn,
					    jint x, jint y, jobject sd)
{
    jint returnVal = -1;
    unsigned char *pixelPtr;
    SurfaceDataRasInfo lockInfo;
    SurfaceDataOps *ops;

    ops = SurfaceData_GetOps(env, sd);

    if (!(pixelPtr = DBN_GetPixelPointer(env, x, y, &lockInfo, 
					 ops, SD_LOCK_READ))) 
    {
	return returnVal;
    }
    switch (lockInfo.pixelStride) {
    case 4:
	returnVal = *(int *)pixelPtr;
	break;
    /* REMIND: do we need a 3-byte case (for 24-bit) here? */
    case 2:
	returnVal = *(unsigned short *)pixelPtr;
	break;
    case 1:
	returnVal = *pixelPtr;
	break;
    default:
	break;
    }
    SurfaceData_InvokeRelease(env, ops, &lockInfo);
    SurfaceData_InvokeUnlock(env, ops, &lockInfo);
    return returnVal;
}


/*
 * Class:     sun_awt_image_DataBufferNative
 * Method:    setElem
 * Signature: 
 */
JNIEXPORT void JNICALL
Java_sun_awt_image_DataBufferNative_setElem(JNIEnv *env, jobject dbn,
					    jint x, jint y, jint val, jobject sd)
{
    SurfaceDataRasInfo lockInfo;
    SurfaceDataOps *ops;
    unsigned char *pixelPtr;


    ops = SurfaceData_GetOps(env, sd);

    if (!(pixelPtr = DBN_GetPixelPointer(env, x, y, &lockInfo, 
					 ops, SD_LOCK_WRITE))) 
    {
	return;
    }

    switch (lockInfo.pixelStride) {
    case 4:
	*(int *)pixelPtr = val;
	break;
    /* REMIND: do we need a 3-byte case (for 24-bit) here? */
    case 2:
	*(unsigned short *)pixelPtr = (unsigned short)val;
	break;
    case 1:
	*pixelPtr = (unsigned char)val;
	break;
    default:
	break;
    }
    SurfaceData_InvokeRelease(env, ops, &lockInfo);
    SurfaceData_InvokeUnlock(env, ops, &lockInfo);
}
