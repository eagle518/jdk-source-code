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
#include "incls/_ciInstanceKlassKlass.cpp.incl"

// ciInstanceKlassKlass
//
// This class represents a klassOop in the HotSpot virtual machine
// whose Klass part is an instanceKlassKlass.

// ------------------------------------------------------------------
// ciInstanceKlassKlass::instance
//
// Return the distinguished instance of this class
ciInstanceKlassKlass* ciInstanceKlassKlass::make() {
  return CURRENT_ENV->_instance_klass_klass_instance;
}
