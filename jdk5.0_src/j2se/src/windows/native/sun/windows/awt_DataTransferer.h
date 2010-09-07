/*
 * @(#)awt_DataTransferer.h	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_DATATRANSFERER_H
#define AWT_DATATRANSFERER_H

#include "stdhdrs.h"
struct IUnknown;

class AwtDataTransferer {
  public:
    static jobject GetDataTransferer(JNIEnv* env);
    static jbyteArray ConvertData(JNIEnv* env, jobject source, jobject contents,
                                  jlong format, jobject formatMap);
    static jobject ConcatData(JNIEnv* env, jobject obj1, jobject obj2);

    static jbyteArray GetPaletteBytes(HGDIOBJ hGdiObj, DWORD dwGdiObjType, 
				      BOOL bFailSafe);
    static jbyteArray LCIDToTextEncoding(JNIEnv *env, LCID lcid);
    static void SecondaryMessageLoop();
};

class OleStrongLock {
  public:
    OleStrongLock(JNIEnv* env, IUnknown* pUnk, HRESULT& res);
    ~OleStrongLock();

  private:
    BOOL m_bLockSucceeded;
    IUnknown* m_pUnk;
    static const char * const LOCK_FAILED_MESSAGE;
};

/*
 * NOTE: You need these macros only if you take care of performance, since they
 * provide proper caching. Otherwise you can use JNU_CallMethodByName etc.
 */

/*
 * This macro defines a function which returns the class for the specified 
 * class name with proper caching and error handling.
 */
#define DECLARE_JAVA_CLASS(javaclazz, name)                                    \
static jclass                                                                  \
get_ ## javaclazz(JNIEnv* env) {                                               \
    static jclass javaclazz = NULL;                                            \
                                                                               \
    if (JNU_IsNull(env, javaclazz)) {                                          \
        jclass javaclazz ## Local = env->FindClass(name);                      \
                                                                               \
        if (!JNU_IsNull(env, javaclazz ## Local)) {                            \
            javaclazz = (jclass)env->NewGlobalRef(javaclazz ## Local);         \
            env->DeleteLocalRef(javaclazz ## Local);                           \
            if (JNU_IsNull(env, javaclazz)) {                                  \
                JNU_ThrowOutOfMemoryError(env, "");                            \
            }                                                                  \
        }                                                                      \
                                                                               \
        if (!JNU_IsNull(env, safe_ExceptionOccurred(env))) {                   \
            env->ExceptionDescribe();                                          \
            env->ExceptionClear();                                             \
        }                                                                      \
    }                                                                          \
                                                                               \
    DASSERT(!JNU_IsNull(env, javaclazz));                                      \
                                                                               \
    return javaclazz;                                                          \
}

/*
 * The following macros defines blocks of code which retrieve a method of the
 * specified class identified with the specified name and signature.
 * The specified class should be previously declared with DECLARE_JAVA_CLASS.
 * These macros should be placed at the beginning of a block, after definition
 * of local variables, but before the code begins.
 */
#define DECLARE_VOID_JAVA_METHOD(method, javaclazz, name, signature)           \
    static jmethodID method = NULL;                                            \
                                                                               \
    if (JNU_IsNull(env, method)) {                                             \
        jclass clazz = get_ ## javaclazz(env);                                 \
                                                                               \
        if (JNU_IsNull(env, clazz)) {                                          \
            return;                                                            \
        }                                                                      \
                                                                               \
        method = env->GetMethodID(clazz, name, signature);                     \
                                                                               \
        if (!JNU_IsNull(env, safe_ExceptionOccurred(env))) {                   \
            env->ExceptionDescribe();                                          \
            env->ExceptionClear();                                             \
        }                                                                      \
                                                                               \
        if (JNU_IsNull(env, method)) {                                         \
            DASSERT(FALSE);                                                    \
            return;                                                            \
        }                                                                      \
    }                                                               

#define DECLARE_JINT_JAVA_METHOD(method, javaclazz, name, signature)           \
    static jmethodID method = NULL;                                            \
                                                                               \
    if (JNU_IsNull(env, method)) {                                             \
        jclass clazz = get_ ## javaclazz(env);                                 \
                                                                               \
        if (JNU_IsNull(env, clazz)) {                                          \
            return java_awt_dnd_DnDConstants_ACTION_NONE;                      \
        }                                                                      \
                                                                               \
        method = env->GetMethodID(clazz, name, signature);                     \
                                                                               \
        if (!JNU_IsNull(env, safe_ExceptionOccurred(env))) {                   \
            env->ExceptionDescribe();                                          \
            env->ExceptionClear();                                             \
        }                                                                      \
                                                                               \
        if (JNU_IsNull(env, method)) {                                         \
            DASSERT(FALSE);                                                    \
            return java_awt_dnd_DnDConstants_ACTION_NONE;                      \
        }                                                                      \
    }                                                               

#define DECLARE_OBJECT_JAVA_METHOD(method, javaclazz, name, signature)         \
    static jmethodID method = NULL;                                            \
                                                                               \
    if (JNU_IsNull(env, method)) {                                             \
        jclass clazz = get_ ## javaclazz(env);                                 \
                                                                               \
        if (JNU_IsNull(env, clazz)) {                                          \
            return NULL;                                                       \
        }                                                                      \
                                                                               \
        method = env->GetMethodID(clazz, name, signature);                     \
                                                                               \
        if (!JNU_IsNull(env, safe_ExceptionOccurred(env))) {                   \
            env->ExceptionDescribe();                                          \
            env->ExceptionClear();                                             \
        }                                                                      \
                                                                               \
        if (JNU_IsNull(env, method)) {                                         \
            DASSERT(FALSE);                                                    \
            return NULL;                                                       \
        }                                                                      \
    }                                                               

#define DECLARE_STATIC_OBJECT_JAVA_METHOD(method, javaclazz, name, signature)  \
    static jmethodID method = NULL;                                            \
    jclass clazz = get_ ## javaclazz(env);                                     \
                                                                               \
    if (JNU_IsNull(env, clazz)) {                                              \
        return NULL;                                                           \
    }                                                                          \
                                                                               \
    if (JNU_IsNull(env, method)) {                                             \
        method = env->GetStaticMethodID(clazz, name, signature);               \
                                                                               \
        if (!JNU_IsNull(env, safe_ExceptionOccurred(env))) {                   \
            env->ExceptionDescribe();                                          \
            env->ExceptionClear();                                             \
        }                                                                      \
                                                                               \
        if (JNU_IsNull(env, method)) {                                         \
            DASSERT(FALSE);                                                    \
            return NULL;                                                       \
        }                                                                      \
    }                                                               

#endif /* AWT_DATATRANSFERER_H */
