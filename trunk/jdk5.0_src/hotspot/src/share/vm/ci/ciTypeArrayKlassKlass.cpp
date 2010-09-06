#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ciTypeArrayKlassKlass.cpp	1.4 03/12/23 16:39:43 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
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
