#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ciInstanceKlassKlass.cpp	1.4 03/12/23 16:39:31 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
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
