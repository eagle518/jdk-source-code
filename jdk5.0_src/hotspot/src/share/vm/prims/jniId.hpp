#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)jniId.hpp	1.4 03/06/19 00:03:50 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//
// Support classes for JNI IDs - jmethodID 
//

//////////////////////////////////////////////////////////////////////
// 
//   jniIdDetails
//
// Optional additional information held for a JNI ID.
// Currently only useful for jmethodID.

class jniIdDetails VALUE_OBJ_CLASS_SPEC {
 private:
  int _itable_index;    // interface table index;

 public:
  int itable_index() { return _itable_index; }
  void set_itable_index(int index) { _itable_index = index; }
};


//////////////////////////////////////////////////////////////////////
// 
//   jniIdMapBase
//
// Publicly visible portion of the per class JNI ID support for jmethodIDs
// Held by instanceKlass, only exists if there are jmethodIDs on the
// class.

class jniIdMapBase : public CHeapObj {
 public:
  void verify();

  // Garbage collection support
  void oops_do(OopClosure* f);
  static void deallocate(jniIdMapBase* map);
};



//////////////////////////////////////////////////////////////////////
// 
//   jniIdSupport
//
// External access to jmethodID support

class jniIdSupport : public AllStatic {
 public:
  enum CheckResult {
    valid_id,                // the jmethodID is valid
    not_in_class,            // method is not in the specified class
    bad_id,                  // the jmethodID is bad
    bad_index                // the index is bad
  };
  enum StaticCheck {
    must_be_static,
    must_be_instance,
    either
  };
  static jmethodID to_jmethod_id(methodOop method_oop);
  static jmethodID to_jmethod_id_or_null(methodOop method_oop);
  static jmethodID to_jmethod_id(klassOop k, int index);

  static methodOop to_method_oop(jmethodID id);
  static methodOop to_method_details(jmethodID id, jniIdDetails** details_p);
  static klassOop to_klass_oop(jmethodID id);

  static CheckResult check_method(klassOop k, jmethodID id);
  static inline bool is_method(klassOop k, jmethodID id) { 
    return check_method(k, id) == valid_id;
  }
  static inline bool is_method(jmethodID id) { 
    return check_method(NULL, id) == valid_id;
  }
};
