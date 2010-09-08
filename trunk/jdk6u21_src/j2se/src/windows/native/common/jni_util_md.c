/*
 * %W% %E%
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <locale.h>

#include "jni.h"
#include "jni_util.h"

static void getParent(const TCHAR *path, TCHAR *dest) {
    char* lastSlash = max(strrchr(path, '\\'), strrchr(path, '/'));
    if (lastSlash == NULL) {
        *dest = 0;
        return;
    }
    if (path != dest)
        strcpy(dest, path);
    *lastSlash = 0;
}

BOOL useNativeConverter(JNIEnv *env) {
    static BOOL initialized;
    static BOOL useNative;
    if (!initialized) {
        HMODULE jvm = GetModuleHandle("jvm");
        useNative = FALSE;
        if (jvm != NULL) {
            TCHAR *jvmPath = NULL;
            int bufferSize = MAX_PATH;
            while (jvmPath == NULL) {
                DWORD result;
                jvmPath = malloc(bufferSize);
                if (jvmPath == NULL)
                    return FALSE;
                result = GetModuleFileName(jvm, jvmPath, bufferSize);
                if (result == 0)
                    return FALSE;
                if (result == bufferSize) { // didn't fit
                    bufferSize += MAX_PATH; // increase buffer size, try again
                    free(jvmPath);
                    jvmPath = NULL;
                }
            }
                    
            getParent(jvmPath, jvmPath);
            useNative = (!strcmp("kernel", jvmPath + strlen(jvmPath) - 
                    strlen("kernel"))); // true if jvm.dll lives in "kernel"
            if (useNative)
                setlocale(LC_ALL, "");
            free(jvmPath);
        }
        initialized = TRUE;
    }
    return useNative;
}

jstring nativeNewStringPlatform(JNIEnv *env, const char *str) {
    static String_char_constructor = NULL;
    if (useNativeConverter(env)) {
        // use native Unicode conversion so Kernel isn't required during 
        // System.initProperties
        jcharArray chars = 0;
        wchar_t *utf16;
        int len;
        jstring result = NULL;

        if (getFastEncoding() == NO_ENCODING_YET)
            initializeEncoding(env);

        len = mbstowcs(NULL, str, strlen(str));
        if (len == -1)
            return NULL;
        utf16 = calloc(len + 1, 2);
        if (mbstowcs(utf16, str, len) == -1)
            return NULL;
        chars = (*env)->NewCharArray(env, len);
        if (chars == NULL)
            return NULL;
        (*env)->SetCharArrayRegion(env, chars, 0, len, utf16);
        if (String_char_constructor == NULL)
            String_char_constructor = (*env)->GetMethodID(env, 
                    JNU_ClassString(env), "<init>", "([C)V");
        result = (*env)->NewObject(env, JNU_ClassString(env), 
                String_char_constructor, chars);
        free(utf16);
        return result;
    }
    else
        return NULL;
}


char* nativeGetStringPlatformChars(JNIEnv *env, jstring jstr, jboolean *isCopy) {
    if (useNativeConverter(env)) {
        // use native Unicode conversion so Kernel isn't required during 
        // System.initProperties
        char *result = NULL;
        size_t len;
        const jchar* utf16 = (*env)->GetStringChars(env, jstr, NULL);
        len = wcstombs(NULL, utf16, (*env)->GetStringLength(env, jstr) * 4) + 1;
        if (len == -1)
            return NULL;
        result = (char*) malloc(len);
        if (result != NULL) {
            if (wcstombs(result, utf16, len) == -1)
                return NULL;
            (*env)->ReleaseStringChars(env, jstr, utf16);
            if (isCopy)
                *isCopy = JNI_TRUE;
        }
        return result;
    }
    else
        return NULL;
}
