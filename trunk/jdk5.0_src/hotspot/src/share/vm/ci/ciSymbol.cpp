#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ciSymbol.cpp	1.14 03/12/23 16:39:40 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_ciSymbol.cpp.incl"

// ------------------------------------------------------------------
// ciSymbol::ciSymbol
//
// Preallocated handle variant.  Used with handles from vmSymboHandles.
ciSymbol::ciSymbol(symbolHandle h_s) : ciObject(h_s) {
}

// ciSymbol
//
// This class represents a symbolOop in the HotSpot virtual
// machine.

// ------------------------------------------------------------------
// ciSymbol::as_utf8
//
// The text of the symbol as a null-terminated C string.
const char* ciSymbol::as_utf8() {
  VM_QUICK_ENTRY_MARK;
  symbolOop s = get_symbolOop();
  return s->as_utf8();
}

// ------------------------------------------------------------------
// ciSymbol::base
jbyte* ciSymbol::base() {
  GUARDED_VM_ENTRY(return get_symbolOop()->base();)
}

// ------------------------------------------------------------------
// ciSymbol::byte_at
int ciSymbol::byte_at(int i) {
  GUARDED_VM_ENTRY(return get_symbolOop()->byte_at(i);)
}

// ------------------------------------------------------------------
// ciSymbol::utf8_length
int ciSymbol::utf8_length() {
  GUARDED_VM_ENTRY(return get_symbolOop()->utf8_length();)
}

// ------------------------------------------------------------------
// ciSymbol::print_impl
//
// Implementation of the print method
void ciSymbol::print_impl() {
  tty->print(" value=");
  print_symbol();
}

// ------------------------------------------------------------------
// ciSymbol::print_symbol_on
//
// Print the value of this symbol on an outputStream
void ciSymbol::print_symbol_on(outputStream *st) {
  GUARDED_VM_ENTRY(get_symbolOop()->print_symbol_on(st);)
}

// ------------------------------------------------------------------
// ciSymbol::make_impl
//
// Make a ciSymbol from a C string (implementation).
ciSymbol* ciSymbol::make_impl(const char* s) {
  EXCEPTION_CONTEXT;
  // For some reason, oopFactory::new_symbol doesn't declare its
  // char* argument as const.
  symbolOop sym = oopFactory::new_symbol((char*)s, (int)strlen(s), THREAD);
#ifndef PRODUCT
  CURRENT_THREAD_ENV->maybe_fire_out_of_memory(THREAD);
#endif /* !PRODUCT */
  if (PENDING_EXCEPTION) {
    CLEAR_PENDING_EXCEPTION;
    CURRENT_THREAD_ENV->record_out_of_memory_failure();
    return ciEnv::_unloaded_cisymbol;
  }
  return CURRENT_THREAD_ENV->get_object(sym)->as_symbol();
}

// ------------------------------------------------------------------
// ciSymbol::make
//
// Make a ciSymbol from a C string.
ciSymbol* ciSymbol::make(const char* s) {
  GUARDED_VM_ENTRY(return make_impl(s);)
}

