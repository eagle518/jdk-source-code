/*
 * Copyright (c) 1999, 2001, Oracle and/or its affiliates. All rights reserved.
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

// ciMethodKlass
//
// This class represents a klassOop in the HotSpot virtual machine
// whose Klass part in a methodKlass.
class ciMethodKlass : public ciKlass {
  CI_PACKAGE_ACCESS

protected:
  ciMethodKlass(KlassHandle h_k) :
         ciKlass(h_k, ciSymbol::make("unique_methodKlass")) {
    assert(get_Klass()->oop_is_method(), "wrong type");
  }

  methodKlass* get_methodKlass() { return (methodKlass*)get_Klass(); }

  const char* type_string() { return "ciMethodKlass"; }

public:
  // What kind of ciObject is this?
  bool is_method_klass() { return true; }

  // Return the distinguished ciMethodKlass instance.
  static ciMethodKlass* make();
};
