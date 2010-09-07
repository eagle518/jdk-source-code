/*
 * @(#)Inet6Address.c	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <string.h>

#include "java_net_Inet6Address.h"
#include "net_util.h"

/************************************************************************
 * Inet6Address
 */

jclass ia6_class;
jfieldID ia6_ipaddressID;
jfieldID ia6_scopeidID;
jfieldID ia6_scopeidsetID;
jfieldID ia6_scopeifnameID;
jfieldID ia6_scopeifnamesetID;
jmethodID ia6_ctrID;

/*
 * Class:     java_net_Inet6Address
 * Method:    init
 * Signature: ()V 
 */
JNIEXPORT void JNICALL
Java_java_net_Inet6Address_init(JNIEnv *env, jclass cls) {
    jclass c = (*env)->FindClass(env, "java/net/Inet6Address");
    CHECK_NULL(c);
    ia6_class = (*env)->NewGlobalRef(env, c);
    CHECK_NULL(ia6_class);
    ia6_ipaddressID = (*env)->GetFieldID(env, ia6_class, "ipaddress", "[B");
    CHECK_NULL(ia6_ipaddressID);
    ia6_scopeidID = (*env)->GetFieldID(env, ia6_class, "scope_id", "I");
    CHECK_NULL(ia6_scopeidID);
    ia6_scopeidsetID = (*env)->GetFieldID(env, ia6_class, "scope_id_set", "Z");
    CHECK_NULL(ia6_scopeidID);
    ia6_scopeifnameID = (*env)->GetFieldID(env, ia6_class, "scope_ifname", "Ljava/net/NetworkInterface;");
    CHECK_NULL(ia6_scopeifnameID);
    ia6_scopeifnamesetID = (*env)->GetFieldID(env, ia6_class, "scope_ifname_set", "Z");
    CHECK_NULL(ia6_scopeifnamesetID);
    ia6_ctrID = (*env)->GetMethodID(env, ia6_class, "<init>", "()V");
    CHECK_NULL(ia6_ctrID);
}
