#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)ciTypeArrayKlass.hpp	1.6 03/12/23 16:39:43 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// ciTypeArrayKlass
//
// This class represents a klassOop in the HotSpot virtual machine
// whose Klass part in a TypeArrayKlass.
class ciTypeArrayKlass : public ciArrayKlass {
  CI_PACKAGE_ACCESS

private:
  BasicType _element_type;

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
  BasicType element_type() { return _element_type; }

  // What kind of ciObject is this?
  bool is_type_array_klass() { return true; }

  // Make an array klass corresponding to the specified primitive type.
  static ciTypeArrayKlass* make(BasicType type);
};
