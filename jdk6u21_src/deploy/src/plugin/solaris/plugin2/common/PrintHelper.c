/*
 * @(#)PrintHelper.c	1.1 07/12/05 12:37:09
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#include <stdio.h>

#include "sun_plugin2_main_server_ServerPrintHelper.h"

#include "jni.h"

/*
 * Class:     sun_plugin2_main_server_ServerPrintHelper
 * Method:    isPrinterDC0
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL
Java_sun_plugin2_main_server_ServerPrintHelper_isPrinterDC0
(JNIEnv *env, jclass unused, jlong hdc) {
    // No-op for Solaris/Linux
    return JNI_FALSE;
}


/*
 * Class:     sun_plugin2_main_server_ServerPrintHelper
 * Method:    printBand0
 * Signature: (JLjava/nio/ByteBuffer;IIIIIIIII)Z
 */
JNIEXPORT jboolean JNICALL Java_sun_plugin2_main_server_ServerPrintHelper_printBand0
(JNIEnv *env, jclass unused, jlong theHDC, jobject imageBuf, jint offset, jint srcX, jint srcY, jint srcWidth, jint srcHeight, jint destX, jint destY, jint destWidth, jint destHeight) {

    FILE *fp = NULL;
    jbyte *image = NULL;
    jlong bufSize = 0;

    if (theHDC == NULL || imageBuf == NULL) {
	    return JNI_FALSE;
    }   

    fp = (FILE*) theHDC;
    image = (jbyte *)(*env)->GetDirectBufferAddress(env, imageBuf); 
    bufSize = (*env)->GetDirectBufferCapacity(env, imageBuf);
    fwrite(image, 1, bufSize, fp); 

}

//
//----------------------------------------------------------------------
