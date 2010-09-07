/*
 * Copyright (c) 2003, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

class JvmtiGetLoadedClasses : AllStatic {
public:
  static jvmtiError getLoadedClasses(JvmtiEnv *env, jint* classCountPtr, jclass** classesPtr);
  static jvmtiError getClassLoaderClasses(JvmtiEnv *env, jobject initiatingLoader,
                                          jint* classCountPtr, jclass** classesPtr);
};
