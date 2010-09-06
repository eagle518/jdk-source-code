#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)vmSymbols.cpp	1.20 03/12/23 16:41:40 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_vmSymbols.cpp.incl"


symbolOop vmSymbols::_symbols[vmSymbols::vm_symbols_terminating_enum];

symbolOop vmSymbols::_type_signatures[T_VOID+1] = { NULL /*, NULL...*/ };

#define VM_SYMBOL_INITIALIZE(name, string) { \
  symbolOop sym = oopFactory::new_symbol(string, CHECK);  \
  _symbols[VM_SYMBOL_ENUM_NAME(name)] = sym; }


void vmSymbols::initialize(TRAPS) {
  if (!UseSharedSpaces) {
    VM_SYMBOLS_DO(VM_SYMBOL_INITIALIZE)

      _type_signatures[T_BYTE]    = byte_signature();
    _type_signatures[T_CHAR]    = char_signature();
    _type_signatures[T_DOUBLE]  = double_signature();
    _type_signatures[T_FLOAT]   = float_signature();
    _type_signatures[T_INT]     = int_signature();
    _type_signatures[T_LONG]    = long_signature();
    _type_signatures[T_SHORT]   = short_signature();
    _type_signatures[T_BOOLEAN] = bool_signature();
    _type_signatures[T_VOID]    = void_signature();
    // no single signatures for T_OBJECT or T_ARRAY
  }
}


void vmSymbols::oops_do(OopClosure* f, bool do_all) {
  for (int index = 0; index < vm_symbols_terminating_enum; index++) {
    f->do_oop((oop*) &_symbols[index]);
  }
  for (int i = 0; i < T_VOID+1; i++) {
    if (_type_signatures[i] != NULL) {
      assert(i >= T_BOOLEAN, "checking");
      f->do_oop((oop*)&_type_signatures[i]);
    } else if (do_all) {
      f->do_oop((oop*)&_type_signatures[i]);
    }
  }
}


BasicType vmSymbols::signature_type(symbolOop s) {
  assert(s != NULL, "checking");
  for (int i = T_BOOLEAN; i < T_VOID+1; i++) {
    if (s == _type_signatures[i]) {
      return (BasicType)i;
    }
  }
  return T_OBJECT;
}
