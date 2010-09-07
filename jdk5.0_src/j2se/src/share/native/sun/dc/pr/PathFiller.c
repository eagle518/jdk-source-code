/*
 * @(#)PathFiller.c	1.22 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)PathFiller.c 3.2 97/11/19
 *
 * ---------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * ---------------------------------------------------------------------
 *
 */

#include "sun_dc_pr_PathFiller.h"
#include "CJError.h"
#include "CJPathConsumer.h"

#include "jlong.h"
#include "doe.h"
#include "dcPathFiller.h"
#include "dcPRError.h"


typedef struct PathFillerData_*	PathFiller;
typedef struct PathFillerData_ {
    doeE		env;
    dcPathFiller	filler;
} PathFillerData;

static jobject		clsFiller;	/* cacheing the class and fieldID */
static jfieldID		fidCData;
static jint		jeofill;

/*
 * Class:     sun_dc_pr_PathFiller
 * Method:    setFillMode
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathFiller_setFillMode
  (JNIEnv * env, jobject obj, jint fmode)
{
    PathFiller	cdata	= (PathFiller)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    fmode  = (fmode == jeofill) ? dcPathFiller_EOFILL : dcPathFiller_NZFILL;

    (*(cdata->filler))->setFillMode(cenv, cdata->filler, fmode);
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathFiller
 * Method:    beginPath
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathFiller_beginPath
  (JNIEnv *env, jobject obj)
{
    PathFiller	cdata	= (PathFiller)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));
    dcPathConsumer
		cfiller	= (dcPathConsumer)(cdata->filler);

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    (*cfiller)->beginPath(cenv, cfiller);
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathFiller
 * Method:    beginSubpath
 * Signature: (FF)V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathFiller_beginSubpath
  (JNIEnv *env, jobject obj, jfloat x0, jfloat y0)
{
    PathFiller	cdata	= (PathFiller)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));
    dcPathConsumer
		cfiller	= (dcPathConsumer)(cdata->filler);

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    (*cfiller)->beginSubpath(cenv, cfiller, x0, y0);
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathFiller
 * Method:    appendLine
 * Signature: (FF)V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathFiller_appendLine
  (JNIEnv *env, jobject obj, jfloat x1, jfloat y1)
{
    PathFiller	cdata	= (PathFiller)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));
    dcPathConsumer
		cfiller	= (dcPathConsumer)(cdata->filler);

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    (*cfiller)->appendLine(cenv, cfiller, x1, y1);
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathFiller
 * Method:    appendQuadratic
 * Signature: (FFFF)V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathFiller_appendQuadratic
  (JNIEnv *env, jobject obj, jfloat xm, jfloat ym, jfloat x1, jfloat y1)
{
    PathFiller	cdata	= (PathFiller)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));
    dcPathConsumer
		cfiller	= (dcPathConsumer)(cdata->filler);

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    (*cfiller)->appendQuadratic(cenv, cfiller, xm, ym, x1, y1);
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathFiller
 * Method:    appendCubic
 * Signature: (FFFFFF)V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathFiller_appendCubic
  (JNIEnv *env, jobject obj, jfloat xm, jfloat ym, jfloat xn, jfloat yn,
			     jfloat x1, jfloat y1)
{
    PathFiller	cdata	= (PathFiller)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));
    dcPathConsumer
		cfiller	= (dcPathConsumer)(cdata->filler);

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    (*cfiller)->appendCubic(cenv, cfiller, xm, ym, xn, yn, x1, y1);
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathFiller
 * Method:    closedSubpath
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathFiller_closedSubpath
  (JNIEnv *env, jobject obj)
{
    PathFiller	cdata	= (PathFiller)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));
    dcPathConsumer
		cfiller	= (dcPathConsumer)(cdata->filler);

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    (*cfiller)->closedSubpath(cenv, cfiller);
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathFiller
 * Method:    endPath
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathFiller_endPath
  (JNIEnv *env, jobject obj)
{
    PathFiller	cdata	= (PathFiller)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));
    dcPathConsumer
		cfiller	= (dcPathConsumer)(cdata->filler);

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    (*cfiller)->endPath(cenv, cfiller);
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathFiller
 * Method:    getCPathConsumer
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_sun_dc_pr_PathFiller_getCPathConsumer
  (JNIEnv *env, jobject obj)
{
    PathFiller	cdata	= (PathFiller)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));
    return ptr_to_jlong(cdata->filler);
}

/*
 * Class:     sun_dc_pr_PathFiller
 * Method:    getAlphaBox
 * Signature: ([I)V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathFiller_getAlphaBox
  (JNIEnv *env, jobject obj, jintArray box)
{
    PathFiller	cdata	= (PathFiller)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));
    i32		cbox[4];

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    if ((box == NULL) || (((*env)->GetArrayLength(env, box)) < 4)) {
	doeError_set(cenv, dcPRError, dcPRError_BAD_boxdest);
	CJError_throw(cenv);
	return;
    }
    (*cdata->filler)->getAlphaBox(cenv, cdata->filler, cbox);
    if (doeError_occurred(cenv)) {
	CJError_throw(cenv);
	return;
    }
    (*env)->SetIntArrayRegion(env, box, 0, 4, (jint*)cbox);
}

/*
 * Class:     sun_dc_pr_PathFiller
 * Method:    setOutputArea
 * Signature: (FFII)V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathFiller_setOutputArea
  (JNIEnv *env, jobject obj, jfloat x0, jfloat y0, jint w, jint h)
{
    PathFiller	cdata	= (PathFiller)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    (*cdata->filler)->setOutputArea(cenv, cdata->filler, x0, y0, w, h);
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathFiller
 * Method:    getTileState
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_sun_dc_pr_PathFiller_getTileState
  (JNIEnv *env, jobject obj)
{
    PathFiller	cdata	= (PathFiller)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));
    jint	tstate;

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    tstate = (jint)((*cdata->filler)->getTileState(cenv, cdata->filler));
    if (doeError_occurred(cenv)) {
	CJError_throw(cenv);
	return -1;
    }
    return tstate;
}

/*
 * Class:     sun_dc_pr_PathFiller
 * Method:    writeAlpha8
 * Signature: ([BIII)V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathFiller_writeAlpha8
  (JNIEnv *env, jobject obj, jbyteArray alpha,
		jint xstride, jint ystride, jint pix0offset)
{
    PathFiller	cdata	= (PathFiller)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));
    jbyte*	calpha;

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    if (alpha == NULL) {
	doeError_set(cenv, dcPRError, dcPRError_BAD_alphadest);
	CJError_throw(cenv);
	return;
    }
    calpha = (*env)->GetByteArrayElements(env, alpha, NULL);
    /* calpha = (*env)->GetPrimitiveArrayCritical(env, alpha, NULL); */
    (*cdata->filler)->writeAlpha8(cenv, cdata->filler,
				  (u8*)calpha,
				  xstride, ystride, pix0offset);
    (*env)->ReleaseByteArrayElements(env, alpha, calpha, 0);
    /* (*env)->ReleasePrimitiveArrayCritical(env, alpha, calpha, 0); */
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathFiller
 * Method:    writeAlpha16
 * Signature: ([CIII)V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathFiller_writeAlpha16
  (JNIEnv *env, jobject obj, jcharArray alpha,
		jint xstride, jint ystride, jint pix0offset)
{
    PathFiller	cdata	= (PathFiller)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));
    jchar*	calpha;

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    if (alpha == NULL) {
	doeError_set(cenv, dcPRError, dcPRError_BAD_alphadest);
	CJError_throw(cenv);
	return;
    }
    calpha = (*env)->GetCharArrayElements(env, alpha, NULL);
    /* calpha = (*env)->GetPrimitiveArrayCritical(env, alpha, NULL); */
    (*cdata->filler)->writeAlpha16(cenv, cdata->filler,
				   (u16*)calpha,
				   xstride, ystride, pix0offset);
    (*env)->ReleaseCharArrayElements(env, alpha, calpha, 0);
    /* (*env)->ReleasePrimitiveArrayCritical(env, alpha, calpha, 0); */
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathFiller
 * Method:    nextTile
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathFiller_nextTile
  (JNIEnv *env, jobject obj)
{
    PathFiller	cdata	= (PathFiller)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    (*cdata->filler)->nextTile(cenv, cdata->filler);
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}


/*
 * Class:     sun_dc_pr_PathFiller
 * Method:    reset
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathFiller_reset
  (JNIEnv *env, jobject obj)
{
    PathFiller	cdata	= (PathFiller)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    (*cdata->filler)->reset(cenv, cdata->filler);
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathFiller
 * Method:    cClassInitialize
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_sun_dc_pr_PathFiller_cClassInitialize(JNIEnv * env, jclass cls)
{
    jfieldID	fid;
    doeE	cenv  = doeE_make();
    doeE_setPCtxt(cenv, env);

    CJPathConsumer_staticInitialize(cenv);
    if (doeError_occurred(cenv)) {
	CJError_throw(cenv);
	return;
    }
    dcPathFiller_staticInitialize(cenv);
    if (doeError_occurred(cenv)) {
	CJError_throw(cenv);
	return;
    }

    fid = (*env)->GetStaticFieldID(env, cls, "tileSizeL2S", "I");
    (*env)->SetStaticIntField(env, cls, fid, dcPathFiller_tileSizeL2S);

    fid = (*env)->GetStaticFieldID(env, cls, "tileSize", "I");
    (*env)->SetStaticIntField(env, cls, fid, dcPathFiller_tileSize);

    fid = (*env)->GetStaticFieldID(env, cls, "tileSizeF", "F");
    (*env)->SetStaticFloatField(env, cls, fid, dcPathFiller_tileSizeF);

    doeE_destroy(cenv);

    /*
     * Cacheing the class specific data
     */
    clsFiller = (*env)->NewGlobalRef(env, cls);
    fidCData  = (*env)->GetFieldID(env, cls, "cData", "J");

    fid	      = (*env)->GetStaticFieldID (env, cls, "EOFILL", "I");
    jeofill   = (*env)->GetStaticIntField(env, cls, fid);
}

/*
 * Class:     sun_dc_pr_PathFiller
 * Method:    cClassFinalize
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_sun_dc_pr_PathFiller_cClassFinalize(JNIEnv * env, jclass cls)
{
    doeE cenv = doeE_make();
    doeE_setPCtxt(cenv, env);

    CJPathConsumer_staticFinalize(cenv);
    if (doeError_occurred(cenv)) {
	CJError_throw(cenv);
	return;
    }
    dcPathFiller_staticFinalize(cenv);
    if (doeError_occurred(cenv)) {
	CJError_throw(cenv);
	return;
    }

    doeE_destroy(cenv);

    (*env)->DeleteGlobalRef(env, clsFiller);
}

/*
 * Class:     sun_dc_pr_PathFiller
 * Method:    cInitialize
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_sun_dc_pr_PathFiller_cInitialize(JNIEnv * env, jobject obj)
{
    jfieldID	fid;
    PathFiller	cdata;

    doeE	cenv	= doeE_make();
    doeE_setPCtxt(cenv, env);

    cdata = (PathFiller)doeMem_malloc(cenv, sizeof(PathFillerData));
    if (doeError_occurred(cenv)) {
	CJError_throw(cenv);
	return;
    }
    (*env)->SetLongField(env, obj, fidCData, ptr_to_jlong(cdata));


    /* __________________________
     * the c environment variable
     */
    cdata->env = cenv;

    /* _______________________ 
     * the actual c PathFiller
     */
    cdata->filler = dcPathFiller_create(cenv);
    if (doeError_occurred(cenv)) {
	CJError_throw(cenv);
	return;
    }
}

/*
 * Class:     sun_dc_pr_PathFiller
 * Method:    dispose
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathFiller_dispose
  (JNIEnv *env, jobject obj)
{
    PathFiller	cdata	= (PathFiller)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));
    doeObject	cobj;

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    cobj = (doeObject)(cdata->filler);
    (*cobj)->_cleanup(cenv, cobj);
    doeMem_free(cenv, cobj);

    doeMem_free(cenv, cdata);

    doeE_destroy(cenv);
}
