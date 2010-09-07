/*
 * Copyright (c) 2003, 2008, Oracle and/or its affiliates. All rights reserved.
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

extern "C" {
  // Report a JNI failure caught by -Xcheck:jni.  Perform a core dump.
  // Note: two variations -- one to be called when in VM state (e.g. when
  // within IN_VM macro), one to be called when in NATIVE state.

  // When in VM state:
  static void ReportJNIFatalError(JavaThread* thr, const char *msg) {
    tty->print_cr("FATAL ERROR in native method: %s", msg);
    thr->print_stack();
    os::abort(true);
  }
}

//
// Checked JNI routines that are useful for outside of checked JNI
//

class jniCheck : public AllStatic {
 public:
  static oop validate_handle(JavaThread* thr, jobject obj);
  static oop validate_object(JavaThread* thr, jobject obj);
  static klassOop validate_class(JavaThread* thr, jclass clazz, bool allow_primitive = false);
  static void validate_class_descriptor(JavaThread* thr, const char* name);
  static void validate_throwable_klass(JavaThread* thr, klassOop klass);
  static void validate_call_object(JavaThread* thr, jobject obj, jmethodID method_id);
  static void validate_call_class(JavaThread* thr, jclass clazz, jmethodID method_id);
  static methodOop validate_jmethod_id(JavaThread* thr, jmethodID method_id);
};
