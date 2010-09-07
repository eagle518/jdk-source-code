/*
 * @(#)JavaObject.h	1.3 10/03/24 12:03:40
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef __JAVAOBJECT_H_
#define __JAVAOBJECT_H_

#include <jni.h>
#include "npapi.h"
#include "npruntime.h"

class MozPluginInstance;

// NPRuntime representation of Java objects to the web browser.

struct JavaObject : public NPObject {
    // We maintain a reference to the Java-side plugin object rather
    // than the C++ MozPluginInstance because the shutdown sequence
    // for plugins first calls NPP_Destroy and then invalidates and
    // deallocates NPObjects created by that plugin instance. In
    // theory we should be maintaining a per-plugin-instance map of
    // the NPObjects we export to the browser and invalidating its
    // objects during NPP_Destroy, but our desire to be able to refer
    // to Java objects in other plugin instances that are hosted by
    // the same JVM seems to preclude this. We are in discussions with
    // mozilla.org to change the mechanism by which NPObjects are
    // shared between plugins on the same page, which would require
    // changing how we determine whether a given NPObject is one we
    // created wrapping a Java object (by having synthetic properties
    // queried in HasProperty and GetProperty).
    jobject parentJavaPlugin;
    jobject targetObject;
    // If this JavaObject actually represents a Java namespace (i.e., the "Packages" object),
    // this will be set initially, and the target object will be null
    char* javaNameSpace;

    // Static helper methods
    static void initialize();
    // Expected that "obj" is a JNI global reference
    static JavaObject* allocate(NPP npp, jobject obj);
    static JavaObject* allocateForJavaNameSpace(NPP npp, const char* javaNameSpace);
    static bool isJavaObject(NPObject* obj);
};

#endif  //__JAVAOBJECT_H_
