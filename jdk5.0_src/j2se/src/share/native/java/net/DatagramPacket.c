/*
 * @(#)DatagramPacket.c	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "java_net_DatagramPacket.h"
#include "net_util.h"

/************************************************************************
 * DatagramPacket
 */

jfieldID dp_addressID;
jfieldID dp_portID;
jfieldID dp_bufID;
jfieldID dp_offsetID;
jfieldID dp_lengthID;
jfieldID dp_bufLengthID;

/*
 * Class:     java_net_DatagramPacket
 * Method:    init
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_java_net_DatagramPacket_init (JNIEnv *env, jclass cls) {
    dp_addressID = (*env)->GetFieldID(env, cls, "address",
				      "Ljava/net/InetAddress;");
    CHECK_NULL(dp_addressID);
    dp_portID = (*env)->GetFieldID(env, cls, "port", "I");
    CHECK_NULL(dp_portID);
    dp_bufID = (*env)->GetFieldID(env, cls, "buf", "[B");
    CHECK_NULL(dp_bufID);
    dp_offsetID = (*env)->GetFieldID(env, cls, "offset", "I");
    CHECK_NULL(dp_offsetID);
    dp_lengthID = (*env)->GetFieldID(env, cls, "length", "I");
    CHECK_NULL(dp_lengthID);
    dp_bufLengthID = (*env)->GetFieldID(env, cls, "bufLength", "I");
    CHECK_NULL(dp_bufLengthID);
}
