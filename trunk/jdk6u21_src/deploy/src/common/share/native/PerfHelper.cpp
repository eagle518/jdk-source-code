/*
 * @(#)PerfHelper.cpp	1.2 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "PerfHelper.h"
#include "com_sun_deploy_perf_NativePerfHelper.h"


extern "C" {
    jobjectArray JNI_toArray(JNIEnv * pEnv) {
        PerfHelper   * helper = (PerfHelper *) GetDeployPerf();
        jobjectArray   result = NULL;

        // Ask the helper for a Java array of PerfLabels.
        if (helper != NULL) {
            result = helper->toArray(pEnv);
        }

        // If the result is still NULL, then return a zero sized array.
        if (result == NULL) {
            jclass clazz  = pEnv->FindClass("com/sun/deploy/perf/PerfLabel");

            if (clazz != NULL) {
                result = pEnv->NewObjectArray((jsize) 0, clazz, NULL);
            }
        }

        return (result);
    }
} // End extern "C"


extern "C" {
    void JNI_put(JNIEnv * pEnv, jstring jLabel) {
        PerfHelper * helper = (PerfHelper *) GetDeployPerf();

        // Ask the helper to put this Java String into the local PerfLabel
        // storage.
        if (helper != NULL) {
            helper->put(pEnv, jLabel);
        }
    }
} // End extern "C"


extern "C" {
    /*
    * Class:     com_sun_deploy_perf_NativePerfHelper
    * Method:    toArray
    * Signature: ()[Lsun/deploy/perf/PerfLabel;
    */
    JNIEXPORT jobjectArray JNICALL
        Java_com_sun_deploy_perf_NativePerfHelper_toArray(JNIEnv * pEnv,
                                                          jobject  jThis) {
        return (JNI_toArray(pEnv));
    }
} // End extern "C"


extern "C" {
    /*
    * Class:     com_sun_deploy_perf_NativePerfHelper
    * Method:    put
    * Signature: (Ljava/lang/String;)V
    */
    JNIEXPORT void JNICALL
        Java_com_sun_deploy_perf_NativePerfHelper_put(JNIEnv * pEnv,
                                                      jobject  jThis,
                                                      jstring  jLabel) {
        JNI_put(pEnv, jLabel);
    }
} // End extern "C"
