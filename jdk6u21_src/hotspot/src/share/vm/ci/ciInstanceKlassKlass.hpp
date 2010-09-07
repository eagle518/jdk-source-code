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

// ciInstanceKlassKlass
//
// This class represents a klassOop in the HotSpot virtual machine
// whose Klass part is a instanceKlassKlass.
class ciInstanceKlassKlass : public ciKlassKlass {
  CI_PACKAGE_ACCESS

private:
  ciInstanceKlassKlass(KlassHandle h_k)
    : ciKlassKlass(h_k, ciSymbol::make("unique_instanceKlassKlass")) {
    assert(h_k()->klass_part()->oop_is_instanceKlass(), "wrong type");
  }

  instanceKlassKlass* get_instanceKlassKlass() {
    return (instanceKlassKlass*)get_Klass();
  }

  const char* type_string() { return "ciInstanceKlassKlass"; }

public:
  // What kind of ciObject is this?
  bool is_instance_klass_klass() { return true; }

  // Return the distinguished ciInstanceKlassKlass instance.
  static ciInstanceKlassKlass* make();
};
