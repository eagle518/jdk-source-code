#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ciMethodKlass.cpp	1.4 03/12/23 16:39:34 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
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
