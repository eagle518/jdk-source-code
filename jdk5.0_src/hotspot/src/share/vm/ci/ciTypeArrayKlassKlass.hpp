#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)ciTypeArrayKlassKlass.hpp	1.6 03/12/23 16:39:43 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// ciTypeArrayKlassKlass
//
// This class represents a klassOop in the HotSpot virtual machine
// whose Klass part is a typeArrayKlassKlass.
class ciTypeArrayKlassKlass : public ciArrayKlassKlass {
  CI_PACKAGE_ACCESS

private:
  ciTypeArrayKlassKlass(KlassHandle h_k)
    : ciArrayKlassKlass(h_k, ciSymbol::make("unique_typeArrayKlassKlass")) {
    assert(h_k()->klass_part()->oop_is_typeArrayKlass(), "wrong type");
  }


  typeArrayKlassKlass* get_typeArrayKlassKlass() {
    return (typeArrayKlassKlass*)get_Klass();
  }
  
  const char* type_string() { return "ciTypeArrayKlassKlass"; }

public:
  // What kind of ciTypeect is this?
  bool is_type_array_klass_klass() { return true; }

  // Return the distinguished ciTypeArrayKlassKlass instance.
  static ciTypeArrayKlassKlass* make();
};
