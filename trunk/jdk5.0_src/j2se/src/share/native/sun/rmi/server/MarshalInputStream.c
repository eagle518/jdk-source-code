/*
 * @(#)MarshalInputStream.c	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jvm.h"
#include "jni_util.h"

#include "sun_rmi_server_MarshalInputStream.h"

/*
 * Class:     sun_rmi_server_MarshalInputStream
 * Method:    latestUserDefinedLoader
 * Signature: ()Ljava/lang/ClassLoader;
 * 
 * Returns the first non-null class loader up the execution stack, or null
 * if only code from the null class loader is on the stack.
 */
JNIEXPORT jobject JNICALL
Java_sun_rmi_server_MarshalInputStream_latestUserDefinedLoader(JNIEnv *env, jclass cls)
{
    return JVM_LatestUserDefinedLoader(env);
}
