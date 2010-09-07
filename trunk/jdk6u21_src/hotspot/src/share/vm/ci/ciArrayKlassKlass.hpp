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

// ciArrayKlassKlass
//
// This class represents a klassOop in the HotSpot virtual machine
// whose Klass part in a arrayKlassKlass.
class ciArrayKlassKlass : public ciKlassKlass {
protected:
  ciArrayKlassKlass(KlassHandle h_k, ciSymbol* name)
    : ciKlassKlass(h_k, name) {}

  arrayKlassKlass* get_arrayKlassKlass() {
    return (arrayKlassKlass*)get_Klass();
  }

  const char* type_string() { return "ciArrayKlassKlass"; }

public:
  // What kind of ciObject is this?
  bool is_array_klass_klass() { return true; }
};
