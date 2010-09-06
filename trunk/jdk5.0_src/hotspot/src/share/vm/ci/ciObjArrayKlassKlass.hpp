#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)ciObjArrayKlassKlass.hpp	1.6 03/12/23 16:39:36 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// ciObjArrayKlassKlass
//
// This class represents a klassOop in the HotSpot virtual machine
// whose Klass part is a objArrayKlassKlass.
class ciObjArrayKlassKlass : public ciArrayKlassKlass {
  CI_PACKAGE_ACCESS

private:
  ciObjArrayKlassKlass(KlassHandle h_k)
    : ciArrayKlassKlass(h_k, ciSymbol::make("unique_objArrayKlassKlass")) {
    assert(h_k()->klass_part()->oop_is_objArrayKlass(), "wrong type");
  }

  objArrayKlassKlass* get_objArrayKlassKlass() {
    return (objArrayKlassKlass*)get_Klass();
  }
  
  const char* type_string() { return "ciObjArrayKlassKlass"; }

public:
  // What kind of ciObject is this?
  bool is_obj_array_klass_klass() { return true; }

  // Return the distinguished ciObjArrayKlassKlass instance.
  static ciObjArrayKlassKlass* make();
};

