/*
 * @(#)InetAddress.c	1.18 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <string.h>

#include "java_net_InetAddress.h"
#include "net_util.h"

/************************************************************************
 * InetAddress
 */

jclass ia_class;
jfieldID ia_addressID;
jfieldID ia_familyID;
jfieldID ia_preferIPv6AddressID;

/*
 * Class:     java_net_InetAddress
 * Method:    init
 * Signature: ()V 
 */
JNIEXPORT void JNICALL
Java_java_net_InetAddress_init(JNIEnv *env, jclass cls) {
    jclass c = (*env)->FindClass(env,"java/net/InetAddress");
    CHECK_NULL(c);
    ia_class = (*env)->NewGlobalRef(env, c);
    CHECK_NULL(ia_class);
    ia_addressID = (*env)->GetFieldID(env, ia_class, "address", "I");
    CHECK_NULL(ia_addressID);
    ia_familyID = (*env)->GetFieldID(env, ia_class, "family", "I");
    CHECK_NULL(ia_familyID);
    ia_preferIPv6AddressID = (*env)->GetStaticFieldID(env, ia_class, "preferIPv6Address", "Z"); 
    CHECK_NULL(ia_preferIPv6AddressID);
}
