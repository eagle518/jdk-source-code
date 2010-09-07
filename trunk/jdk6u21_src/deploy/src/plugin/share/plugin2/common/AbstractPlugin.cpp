/*
 * @(#)AbstractPlugin.cpp	1.3 10/03/24 12:03:38
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "AbstractPlugin.h"
#include "JavaVM.h"
#include "JNIExceptions.h"

bool      AbstractPlugin::initialized                   = false;
jmethodID AbstractPlugin::newBooleanID                  = NULL;
jmethodID AbstractPlugin::newByteID                     = NULL;
jmethodID AbstractPlugin::newCharacterID                = NULL;
jmethodID AbstractPlugin::newShortID                    = NULL;
jmethodID AbstractPlugin::newIntegerID                  = NULL;
jmethodID AbstractPlugin::newLongID                     = NULL;
jmethodID AbstractPlugin::newFloatID                    = NULL;
jmethodID AbstractPlugin::newDoubleID                   = NULL;
jmethodID AbstractPlugin::wrapOrUnwrapScriptingObjectID = NULL;
jmethodID AbstractPlugin::getScriptingObjectForAppletID = NULL;
jmethodID AbstractPlugin::getJavaNameSpaceID            = NULL;
jmethodID AbstractPlugin::javaObjectInvokeID            = NULL;
jmethodID AbstractPlugin::javaObjectInvokeConstructorID = NULL;
jmethodID AbstractPlugin::javaObjectGetFieldID          = NULL;
jmethodID AbstractPlugin::javaObjectSetFieldID          = NULL;
jmethodID AbstractPlugin::javaObjectRemoveFieldID       = NULL;
jmethodID AbstractPlugin::javaObjectHasFieldID          = NULL;
jmethodID AbstractPlugin::javaObjectHasMethodID         = NULL;
jmethodID AbstractPlugin::releaseRemoteJavaObjectID     = NULL;
jmethodID AbstractPlugin::runnableRunID                 = NULL;

bool AbstractPlugin::initialize() {
    if (initialized) {
        return true;
    }

    JNIEnv* env = JavaVM_GetJNIEnv();
    if (env == NULL)
        return false;

    jclass absPluginClass = env->FindClass("sun/plugin2/main/server/AbstractPlugin");
    if (absPluginClass == NULL)
        return false;

    newBooleanID   = env->GetMethodID(absPluginClass, "newBoolean",   "(Z)Ljava/lang/Boolean;");
    newByteID      = env->GetMethodID(absPluginClass, "newByte",      "(B)Ljava/lang/Byte;");
    newCharacterID = env->GetMethodID(absPluginClass, "newCharacter", "(C)Ljava/lang/Character;");
    newShortID     = env->GetMethodID(absPluginClass, "newShort",     "(S)Ljava/lang/Short;");
    newIntegerID   = env->GetMethodID(absPluginClass, "newInteger",   "(I)Ljava/lang/Integer;");
    newLongID      = env->GetMethodID(absPluginClass, "newLong",      "(J)Ljava/lang/Long;");
    newFloatID     = env->GetMethodID(absPluginClass, "newFloat",     "(F)Ljava/lang/Float;");
    newDoubleID    = env->GetMethodID(absPluginClass, "newDouble",    "(D)Ljava/lang/Double;");
    wrapOrUnwrapScriptingObjectID = env->GetMethodID(absPluginClass, "wrapOrUnwrapScriptingObject", "(J)Ljava/lang/Object;");
    getScriptingObjectForAppletID = env->GetMethodID(absPluginClass, "getScriptingObjectForApplet", "(J)J");
    getJavaNameSpaceID            = env->GetMethodID(absPluginClass, "getJavaNameSpace",        "(Ljava/lang/String;)Ljava/lang/Object;");
    javaObjectInvokeID            = env->GetMethodID(absPluginClass, "javaObjectInvoke",        "(Lsun/plugin2/liveconnect/RemoteJavaObject;ZJJIJJ)Z");
    javaObjectInvokeConstructorID = env->GetMethodID(absPluginClass, "javaObjectInvokeConstructor", "(Lsun/plugin2/liveconnect/RemoteJavaObject;ZJIJJ)Z");
    javaObjectGetFieldID          = env->GetMethodID(absPluginClass, "javaObjectGetField",      "(Lsun/plugin2/liveconnect/RemoteJavaObject;ZJJJ)Z");
    javaObjectSetFieldID          = env->GetMethodID(absPluginClass, "javaObjectSetField",      "(Lsun/plugin2/liveconnect/RemoteJavaObject;ZJJJ)Z");
    javaObjectRemoveFieldID       = env->GetMethodID(absPluginClass, "javaObjectRemoveField",   "(Lsun/plugin2/liveconnect/RemoteJavaObject;JJ)V");
    javaObjectHasFieldID          = env->GetMethodID(absPluginClass, "javaObjectHasField",      "(Lsun/plugin2/liveconnect/RemoteJavaObject;J)Z");
    javaObjectHasMethodID         = env->GetMethodID(absPluginClass, "javaObjectHasMethod",     "(Lsun/plugin2/liveconnect/RemoteJavaObject;J)Z");
    releaseRemoteJavaObjectID     = env->GetMethodID(absPluginClass, "releaseRemoteJavaObject", "(Lsun/plugin2/liveconnect/RemoteJavaObject;)V");
    runnableRunID = env->GetMethodID(env->FindClass("java/lang/Runnable"), "run", "()V");

    CHECK_EXCEPTION_VAL(env, false);

    initialized = true;
    return true;
}

jobject AbstractPlugin::newBoolean(JNIEnv* env, jobject javaPluginInstance, jboolean value) {
    if (javaPluginInstance == NULL) {
        return NULL;
    }
    jobject res = env->CallObjectMethod(javaPluginInstance, newBooleanID, value);
    CHECK_EXCEPTION_VAL(env, NULL);
    return res;
}

jobject AbstractPlugin::newByte(JNIEnv* env, jobject javaPluginInstance, jbyte value) {
    if (javaPluginInstance == NULL) {
        return NULL;
    }
    jobject res = env->CallObjectMethod(javaPluginInstance, newByteID, value);
    CHECK_EXCEPTION_VAL(env, NULL);
    return res;
}

jobject AbstractPlugin::newCharacter(JNIEnv* env, jobject javaPluginInstance, jchar value) {
    if (javaPluginInstance == NULL) {
        return NULL;
    }
    jobject res = env->CallObjectMethod(javaPluginInstance, newCharacterID, value);
    CHECK_EXCEPTION_VAL(env, NULL);
    return res;
}

jobject AbstractPlugin::newShort(JNIEnv* env, jobject javaPluginInstance, jshort value) {
    if (javaPluginInstance == NULL) {
        return NULL;
    }
    jobject res = env->CallObjectMethod(javaPluginInstance, newShortID, value);
    CHECK_EXCEPTION_VAL(env, NULL);
    return res;
}

jobject AbstractPlugin::newInteger(JNIEnv* env, jobject javaPluginInstance, jint value) {
    if (javaPluginInstance == NULL) {
        return NULL;
    }
    jobject res = env->CallObjectMethod(javaPluginInstance, newIntegerID, value);
    CHECK_EXCEPTION_VAL(env, NULL);
    return res;
}

jobject AbstractPlugin::newLong(JNIEnv* env, jobject javaPluginInstance, jlong value) {
    if (javaPluginInstance == NULL) {
        return NULL;
    }
    jobject res = env->CallObjectMethod(javaPluginInstance, newLongID, value);
    CHECK_EXCEPTION_VAL(env, NULL);
    return res;
}

jobject AbstractPlugin::newFloat(JNIEnv* env, jobject javaPluginInstance, jfloat value) {
    if (javaPluginInstance == NULL) {
        return NULL;
    }
    jobject res = env->CallObjectMethod(javaPluginInstance, newFloatID, value);
    CHECK_EXCEPTION_VAL(env, NULL);
    return res;
}

jobject AbstractPlugin::newDouble(JNIEnv* env, jobject javaPluginInstance, jdouble value) {
    if (javaPluginInstance == NULL) {
        return NULL;
    }
    jobject res = env->CallObjectMethod(javaPluginInstance, newDoubleID, value);
    CHECK_EXCEPTION_VAL(env, NULL);
    return res;
}

jobject AbstractPlugin::wrapOrUnwrapScriptingObject(JNIEnv* env,
                                                    jobject javaPluginInstance,
                                                    jlong scriptingObject) {
    if (javaPluginInstance == NULL) {
        return NULL;
    }
    jobject res = env->CallObjectMethod(javaPluginInstance, wrapOrUnwrapScriptingObjectID, scriptingObject);
    CHECK_EXCEPTION_VAL(env, NULL);
    return res;
}

jlong AbstractPlugin::getScriptingObjectForApplet(jobject javaPluginInstance,
                                                  jlong exceptionInfo) {
    if (javaPluginInstance == NULL) {
        return NULL;
    }
    JNIEnv* env = JavaVM_GetJNIEnv();
    jlong res = env->CallLongMethod(javaPluginInstance, getScriptingObjectForAppletID, exceptionInfo);
    CHECK_EXCEPTION_VAL(env, NULL);
    return res;
}

jobject AbstractPlugin::getJavaNameSpace(jobject javaPluginInstance,
                                         const char* namespaceUTF8) {
    if (javaPluginInstance == NULL) {
        return NULL;
    }
    if (namespaceUTF8 == NULL) {
        return NULL;
    }

    JNIEnv* env = JavaVM_GetJNIEnv();
    jstring nameSpace = env->NewStringUTF(namespaceUTF8);
    jobject res = env->CallObjectMethod(javaPluginInstance, getJavaNameSpaceID, nameSpace);
    CHECK_EXCEPTION_VAL(env, NULL);
    return env->NewGlobalRef(res);
}

bool AbstractPlugin::javaObjectInvoke(jobject  javaPluginInstance,
                                      jobject  javaObject,
                                      jboolean objectIsApplet,
                                      jlong    methodIdentifier,
                                      jlong    variantArgs,
                                      jint     argCount,
                                      jlong    variantResult,
                                      jlong    exceptionInfo) {
    if (javaPluginInstance == NULL) {
        return false;
    }
    JNIEnv* env = JavaVM_GetJNIEnv();
    return (bool) env->CallBooleanMethod(javaPluginInstance, javaObjectInvokeID,
                                         javaObject, objectIsApplet,
                                         methodIdentifier,
                                         variantArgs, argCount,
                                         variantResult,
                                         exceptionInfo);
}

bool AbstractPlugin::javaObjectInvokeConstructor(jobject  javaPluginInstance,
                                                 jobject  javaObject,
                                                 jboolean objectIsApplet,
                                                 jlong    variantArgs,
                                                 jint     argCount,
                                                 jlong    variantResult,
                                                 jlong    exceptionInfo) {
    if (javaPluginInstance == NULL) {
        return false;
    }
    JNIEnv* env = JavaVM_GetJNIEnv();
    return (bool) env->CallBooleanMethod(javaPluginInstance, javaObjectInvokeConstructorID,
                                         javaObject, objectIsApplet,
                                         variantArgs, argCount,
                                         variantResult,
                                         exceptionInfo);
}

bool AbstractPlugin::javaObjectGetField(jobject  javaPluginInstance,
                                        jobject  javaObject,
                                        jboolean objectIsApplet,
                                        jlong    fieldIdentifier,
                                        jlong    variantResult,
                                        jlong    exceptionInfo) {
    if (javaPluginInstance == NULL) {
        return false;
    }
    JNIEnv* env = JavaVM_GetJNIEnv();
    return (bool) env->CallBooleanMethod(javaPluginInstance,
                                         javaObjectGetFieldID,
                                         javaObject, objectIsApplet,
                                         fieldIdentifier,
                                         variantResult,
                                         exceptionInfo);
}

bool AbstractPlugin::javaObjectSetField(jobject  javaPluginInstance,
                                        jobject  javaObject,
                                        jboolean objectIsApplet,
                                        jlong    fieldIdentifier,
                                        jlong    variantValue,
                                        jlong    exceptionInfo) {
    if (javaPluginInstance == NULL) {
        return false;
    }
    JNIEnv* env = JavaVM_GetJNIEnv();
    return (bool) env->CallBooleanMethod(javaPluginInstance,
                                         javaObjectSetFieldID,
                                         javaObject, objectIsApplet,
                                         fieldIdentifier,
                                         variantValue,
                                         exceptionInfo);
}

void AbstractPlugin::javaObjectRemoveField(jobject  javaPluginInstance,
                                           jobject  javaObject,
                                           jlong    fieldIdentifier,
                                           jlong    exceptionInfo) {
    if (javaPluginInstance == NULL) {
        return;
    }
    JNIEnv* env = JavaVM_GetJNIEnv();
    return env->CallVoidMethod(javaPluginInstance,
                               javaObjectRemoveFieldID,
                               javaObject,
                               fieldIdentifier,
                               exceptionInfo);
}

bool AbstractPlugin::javaObjectHasField(jobject  javaPluginInstance,
                                        jobject  javaObject,
                                        jlong    fieldIdentifier) {
    if (javaPluginInstance == NULL) {
        return false;
    }
    JNIEnv* env = JavaVM_GetJNIEnv();
    return (bool) env->CallBooleanMethod(javaPluginInstance,
                                         javaObjectHasFieldID,
                                         javaObject,
                                         fieldIdentifier);
}

bool AbstractPlugin::javaObjectHasMethod(jobject  javaPluginInstance,
                                         jobject  javaObject,
                                         jlong    methodIdentifier) {
    if (javaPluginInstance == NULL) {
        return false;
    }
    JNIEnv* env = JavaVM_GetJNIEnv();
    return (bool) env->CallBooleanMethod(javaPluginInstance,
                                         javaObjectHasMethodID,
                                         javaObject,
                                         methodIdentifier);
}

void AbstractPlugin::releaseRemoteJavaObject(jobject javaPluginInstance,
                                             jobject remoteObject) {
    if (javaPluginInstance == NULL) {
        return;
    }
    JNIEnv* env = JavaVM_GetJNIEnv();
    env->CallVoidMethod(javaPluginInstance, releaseRemoteJavaObjectID, remoteObject);
    env->DeleteGlobalRef(remoteObject);
    CLEAR_EXCEPTION(env);
}

void AbstractPlugin::runRunnable(jobject runnable) {
    if (runnable == NULL) {
        return;
    }
    JNIEnv* env = JavaVM_GetJNIEnv();
    env->CallVoidMethod(runnable, runnableRunID);
    CLEAR_EXCEPTION(env);
}
