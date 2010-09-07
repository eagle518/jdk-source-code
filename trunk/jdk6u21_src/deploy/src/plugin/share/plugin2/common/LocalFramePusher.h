/*
 * @(#)LocalFramePusher.h	1.3 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef __LOCALFRAMEPUSHER_H_
#define __LOCALFRAMEPUSHER_H_

#include <assert.h>
#include "JavaVM.h"

// A helper class to properly match calls to the JNI PushLocalFrame /
// PopLocalFrame functions.

class LocalFramePusher {
 private:
    JNIEnv* env;
    bool popped;

 public:
    LocalFramePusher(int capacity = 1) {
        env = JavaVM_GetJNIEnv();
        assert(env != NULL); // Fail in debug builds
        if (env != NULL) {
            env->PushLocalFrame(capacity);
        }
        popped = false;
    }

    ~LocalFramePusher() {
        if (!popped) {
            if (env != NULL) {
                env->PopLocalFrame(NULL);
            }
            popped = true;
        }
    }

    // Fetches the JNIEnv* this LocalFramePusher implicitly fetches
    // during its construction.
    JNIEnv* getEnv() {
        return env;
    }

    // Protects the given jobject return value and pops the local frame.
    jobject protect(jobject result) {
        assert(!popped);
        popped = true;
        if (env == NULL) {
            return NULL;
        }
        return env->PopLocalFrame(result);
    }
};

#endif // __LOCALFRAMEPUSHER_H_
