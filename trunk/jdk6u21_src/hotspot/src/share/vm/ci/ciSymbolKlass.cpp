/*
 * Copyright (c) 1999, Oracle and/or its affiliates. All rights reserved.
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

#include "incls/_precompiled.incl"
#include "incls/_ciSymbolKlass.cpp.incl"

// ciSymbolKlass
//
// This class represents a klassOop in the HotSpot virtual machine
// whose Klass part is a symbolKlass.

// ------------------------------------------------------------------
// ciSymbolKlass::instance
//
// Return the distinguished instance of this class
ciSymbolKlass* ciSymbolKlass::make() {
  return CURRENT_ENV->_symbol_klass_instance;
}
