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
#include "incls/_ciMethodKlass.cpp.incl"

// ciMethodKlass
//
// This class represents a klassOop in the HotSpot virtual machine
// whose Klass part is a methodKlass.

// ------------------------------------------------------------------
// ciMethodKlass::instance
//
// Return the distinguished instance of this class
ciMethodKlass* ciMethodKlass::make() {
  return CURRENT_ENV->_method_klass_instance;
}
