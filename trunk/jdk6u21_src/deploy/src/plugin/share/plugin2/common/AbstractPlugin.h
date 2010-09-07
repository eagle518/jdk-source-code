/*
 * @(#)AbstractPlugin.h	1.2 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef __ABSTRACTPLUGIN_H_
#define __ABSTRACTPLUGIN_H_

#include <jni.h>

// This class provides helper methods for interacting with the
// sun.plugin2.main.server.AbstractPlugin class on the Java side,
// which factors out significant portions of JavaScript -> Java
// LiveConnect support between browser implementations (even on the
// browser side).
//
// This class is provided as a set of static helper methods, as opposed
// to something that needs to be inherited from, to provide more
// flexibility to the structure of the browser plugin's native code.
//
// The "javaPluginInstance" argument to all of the methods of this
// class is the Java-side object which must inherit from the
// AbstractPlugin class.

class AbstractPlugin {
 public:
    // Call this before calling any of the other methods on this
    // class. It is permissible to call it more than once (for
    // example, upon each instantiation of a plugin instance).
    static bool initialize();

    // Methods which wrap primitive values into boxing objects
    static jobject newBoolean  (JNIEnv* env, jobject javaPluginInstance, jboolean value);
    static jobject newByte     (JNIEnv* env, jobject javaPluginInstance, jbyte value);
    static jobject newCharacter(JNIEnv* env, jobject javaPluginInstance, jchar value);
    static jobject newShort    (JNIEnv* env, jobject javaPluginInstance, jshort value);
    static jobject newInteger  (JNIEnv* env, jobject javaPluginInstance, jint value);
    static jobject newLong     (JNIEnv* env, jobject javaPluginInstance, jlong value);
    static jobject newFloat    (JNIEnv* env, jobject javaPluginInstance, jfloat value);
    static jobject newDouble   (JNIEnv* env, jobject javaPluginInstance, jdouble value);

    // Either wraps a browser-side scripting object in a BrowserSideObject
    // or unwraps one which we previously passed it into a RemoteJavaObject.
    static jobject wrapOrUnwrapScriptingObject(JNIEnv* env,
                                               jobject javaPluginInstance,
                                               jlong scriptingObject);

    // Fetches a scripting object for the applet.
    // See the Java-side Javadoc for description of argument and return value.
    static jlong getScriptingObjectForApplet(jobject javaPluginInstance,
                                             jlong exceptionInfo);

    // Fetches a JavaNameSpace object. Passing the empty string
    // returns a reference to the root of the Java name space. A NULL
    // argument is not allowed. The caller is responsible for
    // maintaining the storage of the passed string; a persistent
    // reference to it is not retained.
    // See the Java-side Javadoc for description of argument and return value.
    // Returns a JNI global reference which needs to be later freed.
    static jobject getJavaNameSpace(jobject javaPluginInstance,
                                    const char* namespaceUTF8);

    // Invoke a method against a Java object.
    // See the Java-side Javadoc for description of arguments.
    // NOTE: in case of severe error, this might leave a JNI exception pending;
    // caller is responsible for clearing it up if so.
    static bool javaObjectInvoke(jobject  javaPluginInstance,
                                 jobject  javaObject,
                                 jboolean objectIsApplet,
                                 jlong    methodIdentifier,
                                 jlong    variantArgs,
                                 jint     argCount,
                                 jlong    variantResult,
                                 jlong    exceptionInfo);

    // Call the constructor of a given Java class.
    // See the Java-side Javadoc for description of arguments.
    // NOTE: in case of severe error, this might leave a JNI exception pending;
    // caller is responsible for clearing it up if so.
    static bool javaObjectInvokeConstructor(jobject  javaPluginInstance,
                                            jobject  javaObject,
                                            jboolean objectIsApplet,
                                            jlong    variantArgs,
                                            jint     argCount,
                                            jlong    variantResult,
                                            jlong    exceptionInfo);

    // Get a field of the given Java object.
    // See the Java-side Javadoc for description of arguments.
    // NOTE: in case of severe error, this might leave a JNI exception pending;
    // caller is responsible for clearing it up if so.
    static bool javaObjectGetField(jobject  javaPluginInstance,
                                   jobject  javaObject,
                                   jboolean objectIsApplet,
                                   jlong    fieldIdentifier,
                                   jlong    variantResult,
                                   jlong    exceptionInfo);

    // Set a field of the given Java object.
    // See the Java-side Javadoc for description of arguments.
    // NOTE: in case of severe error, this might leave a JNI exception pending;
    // caller is responsible for clearing it up if so.
    static bool javaObjectSetField(jobject  javaPluginInstance,
                                   jobject  javaObject,
                                   jboolean objectIsApplet,
                                   jlong    fieldIdentifier,
                                   jlong    variantValue,
                                   jlong    exceptionInfo);

    // "Remove" a field of the given Java object. This only causes the
    // appropriate exception to be set in the exceptionInfo and exists
    // only to keep more localization code in Java.
    static void javaObjectRemoveField(jobject  javaPluginInstance,
                                      jobject  javaObject,
                                      jlong    fieldIdentifier,
                                      jlong    exceptionInfo);

    // Indicate whether the given Java object has the named
    // field. This is a concession to the Mozilla JavaScript engine,
    // which requires fairly precise answers to this question. See
    // sun.plugin2.liveconnect.JavaClass.hasField().
    static bool javaObjectHasField(jobject javaPluginInstance,
                                   jobject javaObject,
                                   jlong fieldIdentifier);

    // Indicate whether the given Java object has the named
    // method. This is a concession to the Mozilla JavaScript engine,
    // which requires fairly precise answers to this question. See
    // sun.plugin2.liveconnect.JavaClass.hasMethod().
    static bool javaObjectHasMethod(jobject javaPluginInstance,
                                    jobject javaObject,
                                    jlong methodIdentifier);

    // Releases a remote Java object reference in an attached JVM instance.
    // Also has the side-effect of releasing the (expected global) JNI reference
    // associated with the remote object.
    static void releaseRemoteJavaObject(jobject javaPluginInstance,
                                        jobject remoteObject);

    // Helper method which invokes the given Runnable
    static void runRunnable(jobject runnable);

 private:
    static bool initialized;
    // Methods for converting JavaScript values to Java
    static jmethodID newBooleanID;
    static jmethodID newByteID;
    static jmethodID newCharacterID;
    static jmethodID newShortID;
    static jmethodID newIntegerID;
    static jmethodID newLongID;
    static jmethodID newFloatID;
    static jmethodID newDoubleID;
    static jmethodID wrapOrUnwrapScriptingObjectID;
    // Methods for calling Java from JavaScript
    static jmethodID getScriptingObjectForAppletID;
    static jmethodID getJavaNameSpaceID;
    static jmethodID javaObjectInvokeID;
    static jmethodID javaObjectInvokeConstructorID;
    static jmethodID javaObjectGetFieldID;
    static jmethodID javaObjectSetFieldID;
    static jmethodID javaObjectRemoveFieldID;
    static jmethodID javaObjectHasFieldID;
    static jmethodID javaObjectHasMethodID;
    static jmethodID releaseRemoteJavaObjectID;
    // Miscellaneous method IDs
    static jmethodID runnableRunID;
};

#endif //__ABSTRACTPLUGIN_H_
