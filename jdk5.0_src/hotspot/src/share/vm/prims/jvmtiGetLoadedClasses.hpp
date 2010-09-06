#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)jvmtiGetLoadedClasses.hpp	1.2 03/12/23 16:43:21 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class JvmtiGetLoadedClasses : AllStatic {
public:
  static jvmtiError getLoadedClasses(JvmtiEnv *env, jint* classCountPtr, jclass** classesPtr);
  static jvmtiError getClassLoaderClasses(JvmtiEnv *env, jobject initiatingLoader, 
                                          jint* classCountPtr, jclass** classesPtr);
};
