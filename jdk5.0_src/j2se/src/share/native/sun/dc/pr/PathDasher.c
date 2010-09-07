/*
 * @(#)PathDasher.c	1.22 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)PathDasher.c 3.2 97/11/19
 *
 * ---------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * ---------------------------------------------------------------------
 *
 */

#include "sun_dc_pr_PathDasher.h"
#include "CJError.h"
#include "CJPathConsumer.h"

#include "jlong.h"
#include "doe.h"
#include "dcPathDasher.h"
#include "dcPRError.h"

typedef struct PathDasherData_* PathDasher;
typedef struct PathDasherData_ {
    doeE		env;
    dcPathDasher	dasher;

    dcPathConsumer	cout;		/* direct link to the C out path consumer */
    CJPathConsumer	cjout;		/* stub to do up calls if necessary */

} PathDasherData;

static jobject		clsDasher;	/* cacheing class and fieldID */
static jfieldID		fidCData;

/*
 * Class:     sun_dc_pr_PathDasher
 * Method:    setDash
 * Signature: ([FF)V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathDasher_setDash
  (JNIEnv *env, jobject obj, jfloatArray dash, jfloat offset)
{
    PathDasher	cdata	= (PathDasher)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    if (dash == NULL) {
	(*cdata->dasher)->setDash(cenv, cdata->dasher, NULL, 0, offset);
    } else {
	jint	len	= (*env)->GetArrayLength(env, dash);
	jfloat*	cdash	= (*env)->GetPrimitiveArrayCritical(env, dash, NULL);  

	(*cdata->dasher)->setDash(cenv, cdata->dasher, cdash, len, offset);
	(*env)->ReleasePrimitiveArrayCritical(env, dash, cdash, JNI_ABORT);
    }
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathDasher
 * Method:    setDashT4
 * Signature: ([F)V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathDasher_setDashT4
  (JNIEnv *env, jobject obj, jfloatArray t4)
{
    PathDasher	cdata	= (PathDasher)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    if (t4 == NULL) {
	(*cdata->dasher)->setDashT4(cenv, cdata->dasher, NULL);
    } else {
	jfloat*	ct4;
	if (((*env)->GetArrayLength(env, t4)) < 4) {
	    doeError_set(cenv, dcPRError, dcPRError_BAD_dasht4);
	    CJError_throw(cenv);
	    return;
	}
	ct4 = (*env)->GetPrimitiveArrayCritical(env, t4, NULL);
	(*cdata->dasher)->setDashT4(cenv, cdata->dasher, (f32*)ct4);
	(*env)->ReleasePrimitiveArrayCritical(env, t4, ct4, JNI_ABORT);
    }
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathDasher
 * Method:    setOutputT6
 * Signature: ([F)V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathDasher_setOutputT6
  (JNIEnv *env, jobject obj, jfloatArray t6)
{
    PathDasher	cdata	= (PathDasher)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    if (t6 == NULL) {
	(*cdata->dasher)->setOutputT6(cenv, cdata->dasher, NULL);
    } else {
	jfloat*	ct6;
	if (((*env)->GetArrayLength(env, t6)) < 6) {
	    doeError_set(cenv, dcPRError, dcPRError_BAD_outputt6);
	    CJError_throw(cenv);
	    return;
	}
	ct6 = (*env)->GetPrimitiveArrayCritical(env, t6, NULL);
	(*cdata->dasher)->setOutputT6(cenv, cdata->dasher, (f32*)ct6);
	(*env)->ReleasePrimitiveArrayCritical(env, t6, ct6, JNI_ABORT);
    }
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathDasher
 * Method:    setOutputConsumer
 * Signature: (Lsun/dc/path/PathConsumer;)V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathDasher_setOutputConsumer
  (JNIEnv *env, jobject obj, jobject out)
{
    jclass	cls;
    jmethodID	mid;

    PathDasher	cdata	= (PathDasher)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));

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
	(*cdata->dasher)->setOutputConsumer(cenv, cdata->dasher, cdata->cout);
    } else {
	/*
	 * update CJ stub to use new java path consumer
	 */
	(*cdata->cjout)->setJPathConsumer(cenv, cdata->cjout, out);
	if (doeError_occurred(cenv)) {
	    CJError_throw(cenv);
	    return;
	}
	(*cdata->dasher)->setOutputConsumer(cenv, cdata->dasher,
					(dcPathConsumer)cdata->cjout);
    }
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathDasher
 * Method:    reset
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathDasher_reset
  (JNIEnv *env, jobject obj)
{
    PathDasher	cdata	= (PathDasher)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    (*cdata->dasher)->reset(cenv, cdata->dasher);
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathDasher
 * Method:    beginPath
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathDasher_beginPath
  (JNIEnv *env, jobject obj)
{
    PathDasher	cdata	= (PathDasher)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));
    dcPathConsumer
		cdasher	= (dcPathConsumer)(cdata->dasher);

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    (*cdasher)->beginPath(cenv, cdasher);
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathDasher
 * Method:    beginSubpath
 * Signature: (FF)V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathDasher_beginSubpath
  (JNIEnv *env, jobject obj, jfloat x0, jfloat y0)
{
    PathDasher	cdata	= (PathDasher)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));
    dcPathConsumer
		cdasher	= (dcPathConsumer)(cdata->dasher);

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    (*cdasher)->beginSubpath(cenv, cdasher, x0, y0);
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathDasher
 * Method:    appendLine
 * Signature: (FF)V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathDasher_appendLine
  (JNIEnv *env, jobject obj, jfloat x1, jfloat y1)
{
    PathDasher	cdata	= (PathDasher)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));
    dcPathConsumer
		cdasher	= (dcPathConsumer)(cdata->dasher);

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    (*cdasher)->appendLine(cenv, cdasher, x1, y1);
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathDasher
 * Method:    appendQuadratic
 * Signature: (FFFF)V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathDasher_appendQuadratic
  (JNIEnv *env, jobject obj, jfloat xm, jfloat ym, jfloat x1, jfloat y1)
{
    PathDasher	cdata	= (PathDasher)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));
    dcPathConsumer
		cdasher	= (dcPathConsumer)(cdata->dasher);

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    (*cdasher)->appendQuadratic(cenv, cdasher, xm, ym, x1, y1);
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathDasher
 * Method:    appendCubic
 * Signature: (FFFFFF)V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathDasher_appendCubic
  (JNIEnv *env, jobject obj, jfloat xm, jfloat ym, jfloat xn, jfloat yn,
			     jfloat x1, jfloat y1)
{
    PathDasher	cdata	= (PathDasher)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));
    dcPathConsumer
		cdasher	= (dcPathConsumer)(cdata->dasher);

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    (*cdasher)->appendCubic(cenv, cdasher, xm, ym, xn, yn, x1, y1);
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathDasher
 * Method:    closedSubpath
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathDasher_closedSubpath
  (JNIEnv *env, jobject obj)
{
    PathDasher	cdata	= (PathDasher)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));
    dcPathConsumer
		cdasher	= (dcPathConsumer)(cdata->dasher);

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    (*cdasher)->closedSubpath(cenv, cdasher);
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathDasher
 * Method:    endPath
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathDasher_endPath
  (JNIEnv *env, jobject obj)
{
    PathDasher	cdata	= (PathDasher)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));
    dcPathConsumer
		cdasher	= (dcPathConsumer)(cdata->dasher);

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    (*cdasher)->endPath(cenv, cdasher);
    if (doeError_occurred(cenv))
	CJError_throw(cenv);
}

/*
 * Class:     sun_dc_pr_PathDasher
 * Method:    getCPathConsumer
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_sun_dc_pr_PathDasher_getCPathConsumer
  (JNIEnv *env, jobject obj)
{
    PathDasher	cdata	= (PathDasher)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));
    return ptr_to_jlong(cdata->dasher);
}


/*
 * Class:     sun_dc_pr_PathDasher
 * Method:    cClassInitialize
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathDasher_cClassInitialize
  (JNIEnv *env, jclass cls)
{
    doeE	cenv	= doeE_make();
    doeE_setPCtxt(cenv, env);

    CJPathConsumer_staticInitialize(cenv);
    if (doeError_occurred(cenv)) {
	CJError_throw(cenv);
	return;
    }

    dcPathDasher_staticInitialize(cenv);
    if (doeError_occurred(cenv)) {
	CJError_throw(cenv);
	return;
    }

    doeE_destroy(cenv);

    /*
     * Cacheing the class specific data
     */
    clsDasher	= (*env)->NewGlobalRef(env, cls);
    fidCData	= (*env)->GetFieldID(env, cls, "cData", "J");
}

/*
 * Class:     sun_dc_pr_PathDasher
 * Method:    cClassFinalize
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathDasher_cClassFinalize
  (JNIEnv *env, jclass cls)
{
    doeE	cenv	= doeE_make();
    doeE_setPCtxt(cenv, env);

    CJPathConsumer_staticFinalize(cenv);
    if (doeError_occurred(cenv)) {
	CJError_throw(cenv);
	return;
    }

    dcPathDasher_staticFinalize(cenv);
    if (doeError_occurred(cenv)) {
	CJError_throw(cenv);
	return;
    }

    doeE_destroy(cenv);

    (*env)->DeleteGlobalRef(env, clsDasher);
}

/*
 * Class:     sun_dc_pr_PathDasher
 * Method:    cInitialize
 * Signature: (Lsun/dc/path/PathConsumer;)V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathDasher_cInitialize
  (JNIEnv *env, jobject obj, jobject out)
{
    jclass	cls;
    jfieldID	fid;
    jmethodID	mid;

    PathDasher	cdata;

    doeE	cenv	= doeE_make();
    doeE_setPCtxt(cenv, env);

    cdata = (PathDasher)doeMem_malloc(cenv, sizeof(PathDasherData));
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


    /* _______________________
     * the actual c PathDasher
     */
    if (cdata->cout) {
	cdata->dasher = dcPathDasher_create(cenv, cdata->cout);
    } else {
	cdata->dasher = dcPathDasher_create(cenv, (dcPathConsumer)cdata->cjout);
    }
    if (doeError_occurred(cenv)) {
	CJError_throw(cenv);
	return;
    }
}

/*
 * Class:     sun_dc_pr_PathDasher
 * Method:    dispose
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_dc_pr_PathDasher_dispose
  (JNIEnv *env, jobject obj)
{
    PathDasher	cdata	= (PathDasher)jlong_to_ptr(((*env)->GetLongField(env, obj, fidCData)));
    doeObject	cobj;

    doeE	cenv	= cdata->env;
    doeE_setPCtxt(cenv, env);
    doeError_reset(cenv);

    cobj = (doeObject)(cdata->dasher);
    (*cobj)->_cleanup(cenv, cobj);
    doeMem_free(cenv, cobj);

    cobj = (doeObject)(cdata->cjout);
    (*cobj)->_cleanup(cenv, cobj);
    doeMem_free(cenv, cobj);

    doeMem_free(cenv, cdata);

    doeE_destroy(cenv);
}
