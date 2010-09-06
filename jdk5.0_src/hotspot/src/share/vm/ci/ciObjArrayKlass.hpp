#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)ciObjArrayKlass.hpp	1.8 03/12/23 16:39:36 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// ciObjArrayKlass
//
// This class represents a klassOop in the HotSpot virtual machine
// whose Klass part is an objArrayKlass.
class ciObjArrayKlass : public ciArrayKlass {
  CI_PACKAGE_ACCESS
  friend class ciEnv;

private:
  ciKlass* _element_klass;
  ciKlass* _base_element_klass;

protected:
  ciObjArrayKlass(KlassHandle h_k);
  ciObjArrayKlass(ciSymbol* array_name,
                  ciKlass* base_element_klass,
                  int dimension);

  objArrayKlass* get_objArrayKlass() {
    return (objArrayKlass*)get_Klass();
  }

  static ciObjArrayKlass* make_impl(ciKlass* element_klass);
  static ciSymbol* construct_array_name(ciSymbol* element_name,
                                        int       dimension);

  const char* type_string() { return "ciObjArrayKlass"; }

  oop     loader()        { return _base_element_klass->loader(); }
  jobject loader_handle() { return _base_element_klass->loader_handle(); }

  oop     protection_domain()        { return _base_element_klass->protection_domain(); }
  jobject protection_domain_handle() { return _base_element_klass->protection_domain_handle(); }


public:
  // The one-level type of the array elements.
  ciKlass* element_klass();

  // The innermost type of the array elements.
  ciKlass* base_element_klass() { return _base_element_klass; }

  // What kind of ciObject is this?
  bool is_obj_array_klass() { return true; }

  static ciObjArrayKlass* make(ciKlass* element_klass);
};

