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
#include "incls/_ciKlassKlass.cpp.incl"

// ciKlassKlass
//
// This class represents a klassOop in the HotSpot virtual machine
// whose Klass part is a klassKlass.

// ------------------------------------------------------------------
// ciKlassKlass::instance
//
// Return the distinguished instance of this class
ciKlassKlass* ciKlassKlass::make() {
  return CURRENT_ENV->_klass_klass_instance;
}
