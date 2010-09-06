#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ciArray.cpp	1.6 03/12/23 16:39:23 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
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
void ciArray::print_impl() {
  tty->print(" length=%d type=", length());
  klass()->print();
}
