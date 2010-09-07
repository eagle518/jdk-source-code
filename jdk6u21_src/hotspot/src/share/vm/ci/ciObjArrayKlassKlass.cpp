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
#include "incls/_ciObjArrayKlassKlass.cpp.incl"

// ciObjArrayKlassKlass
//
// This class represents a klassOop in the HotSpot virtual machine
// whose Klass part is an arrayKlassKlass.

// ------------------------------------------------------------------
// ciObjArrayKlassKlass::instance
//
// Return the distinguished instance of this class
ciObjArrayKlassKlass* ciObjArrayKlassKlass::make() {
  return CURRENT_ENV->_obj_array_klass_klass_instance;
}
