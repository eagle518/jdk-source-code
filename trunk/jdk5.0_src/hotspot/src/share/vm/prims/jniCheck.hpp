#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)jniCheck.hpp	1.2 03/12/23 16:43:06 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//
// Checked JNI routines that are useful for outside of checked JNI
//

class jniCheck : public AllStatic {
 public:
  static oop validate_handle(JavaThread* thr, jobject obj);
  static oop validate_object(JavaThread* thr, jobject obj);
  static klassOop validate_class(JavaThread* thr, jclass clazz, bool allow_primitive = false);
  static void validate_call_object(JavaThread* thr, jobject obj, jmethodID method_id);
  static void validate_call_class(JavaThread* thr, jclass clazz, jmethodID method_id);
};
