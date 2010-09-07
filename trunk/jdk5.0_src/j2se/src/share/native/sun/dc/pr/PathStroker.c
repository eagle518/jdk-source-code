/*
 * @(#)PathStroker.c	1.21 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)PathStroker.c 3.2 97/11/19
 *
 * -----------------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * -----------------------------------------------------------------------------
 *
 */

#include "sun_dc_pr_PathStroker.h"
#include "CJError.h"
#include "CJPathConsumer.h"

#include "jlong.h"
#include "doe.h"
#include "dcPathStroker.h"
#include "dcPRError.h"

typedef struct PathStrokerData_*	PathStroker;
typedef struct PathStrokerData_ {
    doeE		env;
    dcPathStroker	stroker;

    dcPathConsumer	cout;		/* direct link to the C out path consumer */
    CJPathConsumer	cjout;		/* stub to do up calls */

} PathStrokerData;

static jobject		clsStroker;	/* cacheing the class and fieldID */
static jfieldID		fidCData;

static jint		jround;		/* cacheing some static final variables */
static jint		jsquare;
static jint		jbutt;
static jint		jmiter;
static jint		jbevel;


/*
 * Class:     sun_dc_pr_PathStroker
 * Method:    setPenDiameter
 * Signature: (F)V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathStroker_setPenDiameter
  (JNIEnv *env, jobject obj, jfloat d)
{
    PathStroker	cdata	= (PathStroker)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    (*cdata->stroker)->setPenDiameter(cenv, cdata->stroker, d);

    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathStroker
 * Method:    setPenT4
 * Signature: ([F)V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathStroker_setPenT4
  (JNIEnv *env, jobject obj, jfloatArray t4)
{
    PathStroker	cdata	= (PathStroker)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    if (t4 == NULL) {
	(*cdata->stroker)->setPenT4(cenv, cdata->stroker, NULL);
    } else {
	jfloat* ct4;
	if (((*env)->GetArrayLength(env, t4)) < 4) {
	    doeError_set(cenv, dcPRError, dcPRError_BAD_pent4);
	    CJError_throw(cenv);
	    return;
	}
	ct4 = (*env)->GetPrimitiveArrayCritical(env, t4, NULL);
	(*cdata->stroker)->setPenT4(cenv, cdata->stroker, (f32*)ct4);
	(*env)->ReleasePrimitiveArrayCritical(env, t4, ct4, JNI_ABORT);
    }
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathStroker
 * Method:    setPenFitting
 * Signature: (FI)V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathStroker_setPenFitting
  (JNIEnv *env, jobject obj, jfloat unit, jint mindiam)
{
    PathStroker	cdata	= (PathStroker)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    (*cdata->stroker)->setPenFitting(cenv, cdata->stroker, unit, mindiam);
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathStroker
 * Method:    setCaps
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathStroker_setCaps
  (JNIEnv *env, jobject obj, jint caps)
{
    PathStroker	cdata	= (PathStroker)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    if (caps == jround)
	caps = dcPathStroker_ROUND;
    else
	if (caps == jsquare)
	    caps = dcPathStroker_SQUARE;
	else
	    if (caps == jbutt)
		caps = dcPathStroker_BUTT;

    (*cdata->stroker)->setCaps(cenv, cdata->stroker, caps);
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathStroker
 * Method:    setCorners
 * Signature: (IF)V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathStroker_setCorners
  (JNIEnv *env, jobject obj, jint corners, jfloat miterlimit)
{
    PathStroker	cdata	= (PathStroker)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    if (corners == jround)
	corners = dcPathStroker_ROUND;
    else
	if (corners == jmiter)
	    corners = dcPathStroker_MITER;
	else
	    if (corners == jbevel)
		corners = dcPathStroker_BEVEL;

    (*cdata->stroker)->setCorners(cenv, cdata->stroker, corners, miterlimit);
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathStroker
 * Method:    setOutputT6
 * Signature: ([F)V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathStroker_setOutputT6
  (JNIEnv *env, jobject obj, jfloatArray t6)
{
    PathStroker	cdata	= (PathStroker)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    if (t6 == NULL) {
	(*cdata->stroker)->setOutputT6(cenv, cdata->stroker, NULL);
    } else {
	jfloat* ct6;
	if (((*env)->GetArrayLength(env, t6)) < 6) {
	    doeError_set(cenv, dcPRError, dcPRError_BAD_outputt6);
	    CJError_throw(cenv);
	    return;
	}
	ct6 = (*env)->GetPrimitiveArrayCritical(env, t6, NULL);
	(*cdata->stroker)->setOutputT6(cenv, cdata->stroker, (f32*)ct6);
	(*env)->ReleasePrimitiveArrayCritical(env, t6, ct6, JNI_ABORT);
    }
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathStroker
 * Method:    setOutputConsumer
 * Signature: (Lsun/dc/path/PathConsumer;)V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathStroker_setOutputConsumer
  (JNIEnv *env, jobject obj, jobject out)
{
    jclass	cls;
    jmethodID	mid;

    PathStroker	cdata	= (PathStroker)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    /* ________________________________________________
     * determines if "out" has a native implementation.
     */
    cls = (*env)->GetObjectClass(env, out);
    mid = (*env)->GetMethodID(env, cls, "getCPathConsumer", "()J");
    cdata->cout = (dcPathConsumer)
	jlong_to_ptr((*env)->CallLongMethod(env, out, mid));

    if (cdata->cout) {
	(*cdata->stroker)->setOutputConsumer(cenv, cdata->stroker, cdata->cout);
    } else {
	/*
	 * update CJ stub to use new java path consumer
	 */
	(*cdata->cjout)->setJPathConsumer(cenv, cdata->cjout, out);
	if (doeError_occurred(cenv)) {
	    CJError_throw(cenv);
	    return;
	}
	(*cdata->stroker)->setOutputConsumer(cenv, cdata->stroker,
					 (dcPathConsumer)cdata->cjout);
    }
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathStroker
 * Method:    reset
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathStroker_reset
  (JNIEnv *env, jobject obj)
{
    PathStroker	cdata	= (PathStroker)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    (*cdata->stroker)->reset(cenv, cdata->stroker);
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathStroker
 * Method:    beginPath
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathStroker_beginPath
  (JNIEnv *env, jobject obj)
{
    PathStroker	cdata	= (PathStroker)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));
    dcPathConsumer
		cstroker= (dcPathConsumer)(cdata->stroker);

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    (*cstroker)->beginPath(cenv, cstroker);
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathStroker
 * Method:    beginSubpath
 * Signature: (FF)V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathStroker_beginSubpath
  (JNIEnv *env, jobject obj, jfloat x0, jfloat y0)
{
    PathStroker	cdata	= (PathStroker)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));
    dcPathConsumer
		cstroker= (dcPathConsumer)(cdata->stroker);

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    (*cstroker)->beginSubpath(cenv, cstroker, x0, y0);
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathStroker
 * Method:    appendLine
 * Signature: (FF)V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathStroker_appendLine
  (JNIEnv *env, jobject obj, jfloat x1, jfloat y1)
{
    PathStroker	cdata	= (PathStroker)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));
    dcPathConsumer
		cstroker= (dcPathConsumer)(cdata->stroker);

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    (*cstroker)->appendLine(cenv, cstroker, x1, y1);
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathStroker
 * Method:    appendQuadratic
 * Signature: (FFFF)V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathStroker_appendQuadratic
  (JNIEnv *env, jobject obj, jfloat xm, jfloat ym, jfloat x1, jfloat y1)
{
    PathStroker	cdata	= (PathStroker)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));
    dcPathConsumer
		cstroker= (dcPathConsumer)(cdata->stroker);

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    (*cstroker)->appendQuadratic(cenv, cstroker, xm, ym, x1, y1);
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathStroker
 * Method:    appendCubic
 * Signature: (FFFFFF)V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathStroker_appendCubic
  (JNIEnv *env, jobject obj, jfloat xm, jfloat ym, jfloat xn, jfloat yn,
			     jfloat x1, jfloat y1)
{
    PathStroker	cdata	= (PathStroker)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));
    dcPathConsumer
		cstroker= (dcPathConsumer)(cdata->stroker);

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    (*cstroker)->appendCubic(cenv, cstroker, xm, ym, xn, yn, x1, y1);
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathStroker
 * Method:    closedSubpath
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathStroker_closedSubpath
  (JNIEnv *env, jobject obj)
{
    PathStroker	cdata	= (PathStroker)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));
    dcPathConsumer
		cstroker= (dcPathConsumer)(cdata->stroker);

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    (*cstroker)->closedSubpath(cenv, cstroker);
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathStroker
 * Method:    endPath
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathStroker_endPath
  (JNIEnv *env, jobject obj)
{
    PathStroker	cdata	= (PathStroker)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));
    dcPathConsumer
		cstroker= (dcPathConsumer)(cdata->stroker);

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    (*cstroker)->endPath(cenv, cstroker);
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathStroker
 * Method:    getCPathConsumer
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_sun_dc_pr_PathStroker_getCPathConsumer
  (JNIEnv *env, jobject obj)
{
    PathStroker	cdata	= (PathStroker)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));
    return ptr_to_jlong(cdata->stroker);
}

/*
 * Class:     sun_dc_pr_PathStroker
 * Method:    cClassInitialize
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathStroker_cClassInitialize
  (JNIEnv *env, jclass cls)
{
    jfieldID	fid;

    doeE	cenv	= doeE_make();
    doeE_setPCtxt(cenv, env);

    CJPathConsumer_staticInitialize(cenv);
    if (doeError_occurred(cenv)) {
	CJError_throw(cenv);
	return;
    }
    dcPathStroker_staticInitialize(cenv);
    if (doeError_occurred(cenv)) {
	CJError_throw(cenv);
	return;
    }
    doeE_destroy(cenv);

    /* ________________________________
     * Cacheing the class specific data
     */
    clsStroker	= (*env)->NewGlobalRef(env, cls);
    fidCData	= (*env)->GetFieldID(env, cls, "cData", "J");

    fid	    = (*env)->GetStaticFieldID (env, cls, "ROUND", "I");
    jround  = (*env)->GetStaticIntField(env, cls, fid);

    fid	    = (*env)->GetStaticFieldID (env, cls, "SQUARE", "I");
    jsquare = (*env)->GetStaticIntField(env, cls, fid);

    fid	    = (*env)->GetStaticFieldID (env, cls, "BUTT", "I");
    jbutt   = (*env)->GetStaticIntField(env, cls, fid);

    fid	    = (*env)->GetStaticFieldID (env, cls, "MITER", "I");
    jmiter  = (*env)->GetStaticIntField(env, cls, fid);

    fid	    = (*env)->GetStaticFieldID (env, cls, "BEVEL", "I");
    jbevel  = (*env)->GetStaticIntField(env, cls, fid);
}

/*
 * Class:     sun_dc_pr_PathStroker
 * Method:    cClassFinalize
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathStroker_cClassFinalize
  (JNIEnv *env, jclass cls)
{
    doeE	cenv	= doeE_make();
    doeE_setPCtxt(cenv, env);

    CJPathConsumer_staticFinalize(cenv);
    if (doeError_occurred(cenv)) {
	CJError_throw(cenv);
	return;
    }
    dcPathStroker_staticFinalize(cenv);
    if (doeError_occurred(cenv)) {
	CJError_throw(cenv);
	return;
    }
    doeE_destroy(cenv);

    (*env)->DeleteGlobalRef(env, clsStroker);
}

/*
 * Class:     sun_dc_pr_PathStroker
 * Method:    cInitialize
 * Signature: (Lsun/dc/path/PathConsumer;)V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathStroker_cInitialize
  (JNIEnv *env, jobject obj, jobject out)
{
    jclass	cls;
    jfieldID	fid;
    jmethodID	mid;

    PathStroker	cdata;

    doeE	cenv	= doeE_make();
    doeE_setPCtxt(cenv, env);

    cdata = (PathStroker)doeMem_malloc(cenv, sizeof(PathStrokerData));
    if (doeError_occurred(cenv)) {
	CJError_throw(cenv);
	return;
    }
    (*env)->SetLongField(env, obj, fidCData, ptr_to_jlong(cdata));

    /* __________________________
     * the c environment variable
     */
    cdata->env = cenv;

    /* __________________________________
     * the corresponding CJ path consumer
     * (always created so as to be able to deal with any type of
     *  incoming out path consumers)
     */
    cdata->cjout = CJPathConsumer_create(cenv, out);
    if (doeError_occurred(cenv)) {
	CJError_throw(cenv);
	return;
    }

    /* ________________________________________________
     * determines if "out" has a native implementation.
     */
    cls = (*env)->GetObjectClass(env, out);
    mid = (*env)->GetMethodID(env, cls, "getCPathConsumer", "()J");
    cdata->cout = (dcPathConsumer)
	jlong_to_ptr((*env)->CallLongMethod(env, out, mid));

    /* ________________________
     * the actual c PathStroker
     */
    if (cdata->cout) {
	cdata->stroker = dcPathStroker_create(cenv, cdata->cout);
    } else {
	cdata->stroker = dcPathStroker_create(cenv, (dcPathConsumer)cdata->cjout);
    }
    if (doeError_occurred(cenv)) {
	CJError_throw(cenv);
	return;
    }
}

/*
 * Class:     sun_dc_pr_PathStroker
 * Method:    dispose
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathStroker_dispose
  (JNIEnv *env, jobject obj)
{
    PathStroker	cdata	= (PathStroker)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));
    doeObject	cobj;

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    cobj = (doeObject)(cdata->stroker);
    (*cobj)->_cleanup(cenv, cobj);
    doeMem_free(cenv, cobj);

    cobj = (doeObject)(cdata->cjout);
    (*cobj)->_cleanup(cenv, cobj);
    doeMem_free(cenv, cobj);

    doeMem_free(cenv, cdata);

    doeE_destroy(cenv);
}
