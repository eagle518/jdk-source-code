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
#include "incls/_ciNullObject.cpp.incl"

// ciNullObject
//
// This class represents a null reference.  It can be used
// as a class loader or as the null constant.

// ------------------------------------------------------------------
// ciNullObject::print_impl
//
// Implementation of the print method.
void ciNullObject::print_impl(outputStream* st) {
  ciObject::print_impl(st);
  st->print(" unique");
}

// ------------------------------------------------------------------
// ciNullObject::make
//
// Get the distinguished instance of this class.
ciNullObject* ciNullObject::make() {
  return CURRENT_ENV->_null_object_instance->as_null_object();
}
