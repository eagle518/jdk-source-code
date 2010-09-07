/*
 * Copyright (c) 1999, 2007, Oracle and/or its affiliates. All rights reserved.
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

// ciTypeArrayKlass
//
// This class represents a klassOop in the HotSpot virtual machine
// whose Klass part in a TypeArrayKlass.
class ciTypeArrayKlass : public ciArrayKlass {
  CI_PACKAGE_ACCESS

protected:
  ciTypeArrayKlass(KlassHandle h_k);

  typeArrayKlass* get_typeArrayKlass() {
    return (typeArrayKlass*)get_Klass();
  }

  const char* type_string() { return "ciTypeArrayKlass"; }

  // Helper method for make.
  static ciTypeArrayKlass* make_impl(BasicType type);

public:
  // The type of the array elements.
  BasicType element_type() {
    return Klass::layout_helper_element_type(layout_helper());
  }

  // What kind of ciObject is this?
  bool is_type_array_klass() { return true; }

  // Make an array klass corresponding to the specified primitive type.
  static ciTypeArrayKlass* make(BasicType type);
};
