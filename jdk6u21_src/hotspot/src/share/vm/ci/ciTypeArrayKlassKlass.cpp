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
#include "incls/_ciTypeArrayKlassKlass.cpp.incl"

// ciTypeArrayKlassKlass
//
// This class represents a klassOop in the HotSpot virtual machine
// whose Klass part is a arrayKlassKlass.

// ------------------------------------------------------------------
// ciTypeArrayKlassKlass::instance
//
// Return the distinguished instance of this class
ciTypeArrayKlassKlass* ciTypeArrayKlassKlass::make() {
  return CURRENT_ENV->_type_array_klass_klass_instance;
}
