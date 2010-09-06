#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)ciSymbol.hpp	1.8 03/12/23 16:39:40 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// ciSymbol
//
// This class represents a symbolOop in the HotSpot virtual
// machine.
class ciSymbol : public ciObject {
  CI_PACKAGE_ACCESS
  friend class ciEnv;
  friend class ciInstanceKlass;
  friend class ciSignature;
  friend class ciMethod;
  friend class ciObjArrayKlass;

private:
  ciSymbol(symbolOop s) : ciObject(s) {}
  ciSymbol(symbolHandle s);   // for use with vmSymbolHandles

  symbolOop get_symbolOop() { return (symbolOop)get_oop(); }

  const char* type_string() { return "ciSymbol"; }
  
  void print_impl();

  int         byte_at(int i);
  jbyte*      base();

  // Make a ciSymbol from a C string (implementation).
  static ciSymbol* make_impl(const char* s);

public:
  // The text of the symbol as a null-terminated utf8 string.
  const char* as_utf8();
  int         utf8_length();

  // What kind of ciObject is this?
  bool is_symbol() { return true; }

  void print_symbol_on(outputStream* st);
  void print_symbol() {
    print_symbol_on(tty);
  }

  // Make a ciSymbol from a C string.
  // Consider adding to vmSymbols.hpp instead of using this constructor.
  // (Your code will be less subject to typographical bugs.)
  static ciSymbol* make(const char* s);

#define CI_SYMBOL_DECLARE(name, string) \
  static ciSymbol* name() { return ciObjectFactory::vm_symbol_at(vmSymbols::VM_SYMBOL_ENUM_NAME(name)); }
  VM_SYMBOLS_DO(CI_SYMBOL_DECLARE)
#undef CI_SYMBOL_DECLARE
};
