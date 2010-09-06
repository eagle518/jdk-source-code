#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ciKlassKlass.cpp	1.4 03/12/23 16:39:32 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
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
