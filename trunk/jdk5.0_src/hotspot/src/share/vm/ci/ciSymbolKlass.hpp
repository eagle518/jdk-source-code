#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)ciSymbolKlass.hpp	1.7 03/12/23 16:39:41 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// ciSymbolKlass
//
// This class represents a klassOop in the HotSpot virtual machine
// whose Klass part in a symbolKlass.  Although, in the VM
// Klass hierarchy, symbolKlass is a direct subclass of typeArrayKlass,
// we do not model this relationship in the ciObject hierarchy -- the
// subclassing is used to share implementation and is not of note
// to compiler writers.
class ciSymbolKlass : public ciKlass {
  CI_PACKAGE_ACCESS

protected:
  ciSymbolKlass(KlassHandle h_k)
    : ciKlass(h_k, ciSymbol::make("unique_symbolKlass")) {
    assert(get_Klass()->oop_is_symbol(), "wrong type");
  }

  symbolKlass* get_symbolKlass() { return (symbolKlass*)get_Klass(); }
  
  const char* type_string() { return "ciSymbolKlass"; }

public:
  // What kind of ciObject is this?
  bool is_symbol_klass() { return true; }

  // Return the distinguished ciSymbolKlass instance.
  static ciSymbolKlass* make();
};

