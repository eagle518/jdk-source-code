/*
 * Copyright (c) 1999, 2000, Oracle and/or its affiliates. All rights reserved.
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
#include "incls/_ciArray.cpp.incl"

// ciArray
//
// This class represents an arrayOop in the HotSpot virtual
// machine.

// ------------------------------------------------------------------
// ciArray::print_impl
//
// Implementation of the print method.
void ciArray::print_impl(outputStream* st) {
  st->print(" length=%d type=", length());
  klass()->print(st);
}
