#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)ciKlassKlass.hpp	1.7 03/12/23 16:39:32 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// ciKlassKlass
//
// This class represents a klassOop in the HotSpot virtual machine
// whose Klass part is a klassKlass or one of its subclasses
// (instanceKlassKlass, objArrayKlassKlass, typeArrayKlassKlass).
class ciKlassKlass : public ciKlass {
  CI_PACKAGE_ACCESS

protected:
  ciKlassKlass(KlassHandle h_k)
    : ciKlass(h_k, ciSymbol::make("unique_klassKlass")) {
    assert(h_k()->klass_part()->oop_is_klass(), "wrong type");
  }
  ciKlassKlass(KlassHandle h_k, ciSymbol *name)
    : ciKlass(h_k, name) {}

  klassKlass* get_klassKlass() { return (klassKlass*)get_Klass(); }
  
  const char* type_string() { return "ciKlassKlass"; }

public:
  // What kind of ciObject is this?
  bool is_klass_klass() { return true; }

  // Return the distinguished ciKlassKlass instance.
  static ciKlassKlass* make();
};

