/*
 * @(#)HotSpotDiagnostic.c	1.4 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <jni.h>
#include "jvm.h"
#include "management.h"
#include "sun_management_HotSpotDiagnostic.h"

JNIEXPORT void JNICALL
Java_sun_management_HotSpotDiagnostic_dumpHeap
  (JNIEnv *env, jobject dummy, jstring outputfile, jboolean live)
{
    jmm_interface->DumpHeap0(env, outputfile, live);
}
