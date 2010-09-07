/*
 * @(#)Region.c	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdlib.h>

#include "jni_util.h"

#include "Region.h"

static jfieldID endIndexID;
static jfieldID bandsID;
static jfieldID loxID;
static jfieldID loyID;
static jfieldID hixID;
static jfieldID hiyID;

#define InitField(var, env, jcl, name, type) \
do { \
    var = (*env)->GetFieldID(env, jcl, name, type); \
    if (var == NULL) { \
	return; \
    } \
} while (0)

/*
 * Class:     sun_java2d_pipe_Region
 * Method:    initIDs
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_sun_java2d_pipe_Region_initIDs(JNIEnv *env, jclass reg)
{
    InitField(endIndexID, env, reg, "endIndex", "I");
    InitField(bandsID, env, reg, "bands", "[I");

    InitField(loxID, env, reg, "lox", "I");
    InitField(loyID, env, reg, "loy", "I");
    InitField(hixID, env, reg, "hix", "I");
    InitField(hiyID, env, reg, "hiy", "I");
}

JNIEXPORT jint JNICALL
Region_GetInfo(JNIEnv *env, jobject region, RegionData *pRgnInfo)
{
    if (JNU_IsNull(env, region)) {
	pRgnInfo->bounds.x1 = pRgnInfo->bounds.y1 = 0x80000000;
	pRgnInfo->bounds.x2 = pRgnInfo->bounds.y2 = 0x7fffffff;
	pRgnInfo->endIndex = 0;
    } else {
	pRgnInfo->bounds.x1 = (*env)->GetIntField(env, region, loxID);
	pRgnInfo->bounds.y1 = (*env)->GetIntField(env, region, loyID);
	pRgnInfo->bounds.x2 = (*env)->GetIntField(env, region, hixID);
	pRgnInfo->bounds.y2 = (*env)->GetIntField(env, region, hiyID);
	pRgnInfo->endIndex = (*env)->GetIntField(env, region, endIndexID);
    }
    pRgnInfo->bands = (Region_IsRectangular(pRgnInfo)
		       ? NULL
		       : (*env)->GetObjectField(env, region, bandsID));
    return 0;
}

JNIEXPORT void JNICALL
Region_StartIteration(JNIEnv *env, RegionData *pRgnInfo)
{
    pRgnInfo->pBands =
	(Region_IsRectangular(pRgnInfo)
	 ? NULL
	 : (*env)->GetPrimitiveArrayCritical(env, pRgnInfo->bands, 0));
    pRgnInfo->index = 0;
    pRgnInfo->numrects = 0;
}

JNIEXPORT jint JNICALL
Region_CountIterationRects(RegionData *pRgnInfo)
{
    jint totalrects;
    if (Region_IsEmpty(pRgnInfo)) {
	totalrects = 0;
    } else if (Region_IsRectangular(pRgnInfo)) {
	totalrects = 1;
    } else {
	jint *pBands = pRgnInfo->pBands;
	int index = 0;
	totalrects = 0;
	while (index < pRgnInfo->endIndex) {
	    jint xy1 = pBands[index++];
	    jint xy2 = pBands[index++];
	    jint numrects = pBands[index++];
	    if (xy1 >= pRgnInfo->bounds.y2) {
		break;
	    }
	    if (xy2 > pRgnInfo->bounds.y1) {
		while (numrects > 0) {
		    xy1 = pBands[index++];
		    xy2 = pBands[index++];
		    numrects--;
		    if (xy1 >= pRgnInfo->bounds.x2) {
			break;
		    }
		    if (xy2 > pRgnInfo->bounds.x1) {
			totalrects++;
		    }
		}
	    }
	    index += numrects * 2;
	}
    }
    return totalrects;
}

JNIEXPORT jint JNICALL
Region_NextIteration(RegionData *pRgnInfo, SurfaceDataBounds *pSpan)
{
    jint index = pRgnInfo->index;
    if (Region_IsRectangular(pRgnInfo)) {
	if (index > 0 || Region_IsEmpty(pRgnInfo)) {
	    return 0;
	}
	pSpan->x1 = pRgnInfo->bounds.x1;
	pSpan->x2 = pRgnInfo->bounds.x2;
	pSpan->y1 = pRgnInfo->bounds.y1;
	pSpan->y2 = pRgnInfo->bounds.y2;
	index = 1;
    } else {
	jint *pBands = pRgnInfo->pBands;
	jint xy1, xy2;
	jint numrects = pRgnInfo->numrects;
	while (JNI_TRUE) {
	    if (numrects <= 0) {
		if (index >= pRgnInfo->endIndex) {
		    return 0;
		}
		xy1 = pBands[index++];
		if (xy1 >= pRgnInfo->bounds.y2) {
		    return 0;
		}
		if (xy1 < pRgnInfo->bounds.y1) {
		    xy1 = pRgnInfo->bounds.y1;
		}
		xy2 = pBands[index++];
		numrects = pBands[index++];
		if (xy2 > pRgnInfo->bounds.y2) {
		    xy2 = pRgnInfo->bounds.y2;
		}
		if (xy2 <= xy1) {
		    index += numrects * 2;
		    numrects = 0;
		    continue;
		}
		pSpan->y1 = xy1;
		pSpan->y2 = xy2;
	    }
	    xy1 = pBands[index++];
	    xy2 = pBands[index++];
	    numrects--;
	    if (xy1 >= pRgnInfo->bounds.x2) {
		index += numrects * 2;
		numrects = 0;
		continue;
	    }
	    if (xy1 < pRgnInfo->bounds.x1) {
		xy1 = pRgnInfo->bounds.x1;
	    }
	    if (xy2 > pRgnInfo->bounds.x2) {
		xy2 = pRgnInfo->bounds.x2;
	    }
	    if (xy2 > xy1) {
		pSpan->x1 = xy1;
		pSpan->x2 = xy2;
		break;
	    }
	}
	pRgnInfo->numrects = numrects;
    }
    pRgnInfo->index = index;
    return 1;
}

JNIEXPORT void JNICALL
Region_EndIteration(JNIEnv *env, RegionData *pRgnInfo)
{
    if (pRgnInfo->endIndex != 0) {
	(*env)->ReleasePrimitiveArrayCritical(env, pRgnInfo->bands,
					      pRgnInfo->pBands, JNI_ABORT);
    }
}
